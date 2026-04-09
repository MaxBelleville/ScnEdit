#include "SkylichtEngine.h"
#include "Utils/CSingleton.h"
#include "Collision/CCollisionManager.h"
#include "Header/Managers/CInteractionEnumLayer.h"
#include "Header/Base/util.h"
#include "Header/Base/scntypes.h"
#include "UserInterface/CUITextBox.h"
#include "Header/CGUILogger.h"
#include <imgui.h>
#include <imgui_internal.h>


struct guiSettings_t
{
	bool vis_scn = true;
	bool vis_doors = true;
	bool vis_portals = true;
	bool vis_bb = false;
	bool vis_entities = true;
	bool vis_lightmaps = true;
	bool scrape_lightmaps = false;
};
enum GUIState
{
	Default,
	Hover,
	CloseFile,
	OpenScn,
	Quit,
	Save,
	ScrapeLightSave,
	OpenTexture,
	Help,
	Export,
	Debug,
	EditEntity,
	EditFlags,
	EditAlpha,
	EditSurfShading,
	EditShading,
};

enum SelectedType {
	Empty,
	Solid,
	SolidExtra,
	Portal,
	Entity
};

class CInteractionManager :IEventReceiver
{
public:
	DECLARE_SINGLETON(CInteractionManager)
	CCollisionNode* node = NULL;

protected:
	static inline guiSettings_t* gui =new guiSettings_t();
	static inline guiSettings_t* prevgui = new guiSettings_t();
	key_map m_keyMap;

	std::pair<bool,KeyAugment> m_leftDown = std::make_pair(false,KeyAugment::None);
	std::pair<bool, KeyAugment> m_leftToggle = std::make_pair(false, KeyAugment::None);

	core::vector2df m_mouse;

	UVMode m_uvMode = UVMode::Move;
	GUIState m_guiState= GUIState::Default;
	SelectedType m_selectedType = SelectedType::Empty;
	KeyAugment m_augment= KeyAugment::None;

	CInteractionEnumLayer<GUIState> stateLayer = CInteractionEnumLayer(m_guiState);
	CInteractionEnumLayer<SelectedType> selectedTypeLayer= CInteractionEnumLayer(m_selectedType);
	CInteractionEnumLayer<KeyAugment> augmentLayer = CInteractionEnumLayer(m_augment);
	CGUILogger m_logger;
	float m_uvScalar = 0.01;

	core::array<u32> shared;
	core::array<surfaceBox_t> surfsels;

	vertBox_t m_verthover = vertBox_t(-1, -1, -1);
	vertBox_t m_vertsel = vertBox_t(-1, -1, -1);
	portalBox_t m_portalsel = portalBox_t(-1, -1);
	int m_entitysel = -1;
	core::vector2di highlightMax = core::vector2di(0);
	int hightlightIdx = 0;

	bool m_blockCursor = false;

	std::vector<std::function<void(key_pair)>> m_KeyEvents;
	std::vector<std::function<void()>> m_MouseMoveEvents;
	std::vector<std::function<void(bool,bool)>> m_CursorModeEvents;
	std::vector<std::function<void(const char*)>> m_LogEvents;
	std::vector<std::function<void(EMOUSE_INPUT_EVENT)>> m_MouseDownEvents;
	std::vector<std::function<void(EMOUSE_INPUT_EVENT)>> m_MouseUpEvents;
	std::vector<std::function<void(core::triangle3df, core::vector3df)>> m_ColliderEvents;
	std::vector<std::function<void()>> m_UIUpdateEvents;

	CGameObject* selectobj = NULL;


public:
	CInteractionManager();

	virtual ~CInteractionManager();

	bool OnEvent(const SEvent& event);

	bool isGuiSettingsUpdated();
	bool findKeyState(std::vector<key_pair> keys);
	static void activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string msg, int size);
	static void activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string head_msg, std::string body_msg, int size);
	static void resetText(UI::CUITextBox* textbox, int size);
	static bool ToggleButton(const char* str_id, bool* v, ImGuiKey key);
	static void registerImgui(const wchar_t* dir);

	u32 changeHighlightSurfIdx();
	void swapCursorMode(bool isRightClick);
	void setCursorMode(bool state);

	inline void resetLeftClick() {
		m_leftToggle = std::make_pair(false, KeyAugment::None);
	}

	inline static void getNumeric(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
		for (int i = 0; i < 10; i++)
			keys.push_back(std::make_pair(static_cast<irr::EKEY_CODE>(irr::KEY_KEY_0 + i), KeyAugment::None));
	}

	inline static void getAlphaNumeric(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
		getNumeric(keys);
		getAlphabetic(keys);
	}

	inline static void getAlphabetic(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
		for (char c = 'A'; c <= 'Z'; ++c) {
			irr::EKEY_CODE key = static_cast<irr::EKEY_CODE>(irr::KEY_KEY_A + (c - 'A'));
			keys.push_back(std::make_pair(key, KeyAugment::AnyKey));
		}
	}

	inline void setBlockCursor(bool block) {
		m_blockCursor = block;
	}

	inline CInteractionEnumLayer<GUIState> guiStateLayer() {
		return stateLayer;
	}
	inline CInteractionEnumLayer<KeyAugment> keyAugLayer() {
		return augmentLayer;
	}
	
	inline CInteractionEnumLayer<SelectedType> selTypeLayer() {
		return selectedTypeLayer;
	}

	inline CGameObject* getSelectObj() {
		return selectobj;
	}
	inline void setSelectObj(CGameObject* obj) {
		selectobj = obj;
	}

	inline void resetSelected() {
		selectobj = NULL;
		surfsels.clear();
		shared.clear();
		m_verthover = vertBox_t(-1, -1, -1);
		m_vertsel = vertBox_t(-1, -1, -1);
		m_portalsel = portalBox_t(-1, -1);
		m_entitysel = -1;
		highlightMax = core::vector2di(0);
		hightlightIdx = 0;
	}

	inline bool getKeyState(key_pair key) { 
		if (key.second == KeyAugment::AnyKey) 
			key.second = m_augment;
		
		return m_keyMap.find(key) != m_keyMap.end();
	}
	inline bool getKeyAugment(KeyAugment aug) {
		return m_augment == aug;
	}

	inline void resetKeyState() {
		m_keyMap.clear();
	}
	inline void removeKeyState(key_pair key) {
		m_keyMap.erase(key);
	}

	inline bool isLeftClicked() {
		return m_leftToggle.first;
	}
	inline bool isLeftDown() {
		return m_leftDown.first;
	}
	inline KeyAugment getLeftClickAug() {
		return m_leftToggle.second;
	}
	inline KeyAugment getLeftDownAug() {
		return m_leftDown.second;
	}
	inline bool isLeftClicked(KeyAugment aug) {
		return m_leftToggle.first && aug == m_leftToggle.second;
	}
	inline bool isLeftDown(KeyAugment aug) {
		return m_leftDown.first && aug == m_leftToggle.second;
	}
	inline guiSettings_t* getGuiSettings() {
		return gui;
	}

	inline guiSettings_t* getPrevGuiSettings() {
		return prevgui;
	}

	inline void drawLog(const char* title, bool * open) {
		m_logger.Draw(title, open);
	}

	inline bool getCursorMode() {
		gui::ICursorControl* cursor = getApplication()->getDevice()->getCursorControl();
		return cursor->isVisible();
	}

	inline void resetPrevGui() {
		*prevgui = *gui;
	}

	inline core::vector2df getMouse() {
		return m_mouse;
	}
	inline void setMouse(core::vector2df mouse) {
		m_mouse = mouse;
	}
	inline core::array<u32> getSharedSurfs() {
		return shared;
	}

	inline portalBox_t getPortalIdx() {
		return m_portalsel;
	}

	inline surfaceBox_t getLastSurfSelected() {
		if (!surfsels.empty()) {
			return surfsels.getLast();
		}
		return surfaceBox_t(-1, -1);
	}

	inline core::array<surfaceBox_t> getSurfSelected() {
		return surfsels;
	}

	inline vertBox_t getVertSelected() {
		return m_vertsel;
	}

	inline vertBox_t getVertHover() {
		return m_verthover;
	}

	inline int getEntityIdx() {
		return m_entitysel;
	}

	inline float getUVScalar() { 
		return m_uvScalar; 
	}

	inline void setUVScalar(float scalar) {
		m_uvScalar = scalar; 
	}
	inline UVMode getUVMode() { 
		return m_uvMode; 
	}
	inline bool findUVMode(UVMode mode) { 
		return m_uvMode == mode; 
	}

	inline void swapUVMode() {
		m_uvMode = m_uvMode == UVMode::Move ? UVMode::Resize : UVMode::Move;
	}

	inline void setSurfSelected(core::array<surfaceBox_t> sels) {
		surfsels = sels;
	}
	inline void setSharedSurfs(core::array<u32> surfshared) {
		shared = surfshared;
	}
	
	inline void setPortalIdx(portalBox_t selected) {
		m_portalsel = selected;
	}
	inline void setEntityIdx(int indx) {
		m_entitysel = indx;
	}
	inline void setVertSelected(vertBox_t vert) {
		m_vertsel = vert;
	}
	inline void setVertHover(vertBox_t vert) {
		m_verthover = vert;
	}
	inline void setHighlighterMax(core::vector2di max) {
		highlightMax = max; // x is regular surfaces, y is shared surfaces
	}


	inline void OnKeyEvent(std::function<void(key_pair)> func) {
		m_KeyEvents.push_back(func);
	}
	inline void OnMouseMoveEvent(std::function<void()> func) {
		m_MouseMoveEvents.push_back(func);
	}
	inline void OnCursorModeEvent(std::function<void(bool,bool)> func) {
		m_CursorModeEvents.push_back(func);
	}
	inline void OnMouseDownEvent(std::function<void(EMOUSE_INPUT_EVENT)> func) {
		m_MouseDownEvents.push_back(func);
	}
	inline void OnMouseUpEvent(std::function<void(EMOUSE_INPUT_EVENT)> func) {
		m_MouseUpEvents.push_back(func);
	}
	inline void OnLogEvent(std::function<void(const char*)> func) {
		m_LogEvents.push_back(func);
	}
	inline void OnCollisionEvent(std::function<void(core::triangle3df, core::vector3df)> func) {
		m_ColliderEvents.push_back(func);
	}
	inline void OnUIUpdateEvent(std::function<void()> func) {
		m_UIUpdateEvents.push_back(func);
	}
	inline void CollisionCallback(core::triangle3df tri, core::vector3df intersection) {
		for (int i = 0; i < m_ColliderEvents.size(); i++) 
			m_ColliderEvents[i](tri, intersection);
	}
	inline void UICallback() {
		for (int i = 0; i < m_UIUpdateEvents.size(); i++)
			m_UIUpdateEvents[i]();
	}

protected:
	// Function to read the custom settings block (initialize)
	inline static void* readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name) {
		if (strcmp(name, "VISIBLITY") == 0)
			return handler->UserData;
		return nullptr;
	};

	// Function to parse each line in the settings block
	static void readLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
	
	// Function to write all settings to the INI file
	static void writeAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

};
