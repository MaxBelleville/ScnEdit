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
enum UVMode {
	Move,
	Resize,
	FlipH,
	FlipV
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

	solidSelect_t m_surfselected = solidSelect_t(-1, -1);
	portalSelect_t m_portalselected = portalSelect_t(-1, -1);
	int m_entityselected = -1;
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
	indexed_vertices vertexdata = std::make_pair(core::array<indexedVec3df_t>(), core::array<indexedVec3df_t>());
	opt_indexedVec3df moveablevert;

public:
	CInteractionManager();

	virtual ~CInteractionManager();

	bool OnEvent(const SEvent& event);

	void updateVertPos();
	bool isGuiSettingsUpdated();
	bool findKeyState(std::vector<key_pair> keys);
	static void getNumeric(core::array<std::pair<irr::EKEY_CODE, int>>&);
	static void getAlphaNumeric(core::array<std::pair<irr::EKEY_CODE, int>>&);
	static void getAlphabetic(core::array<std::pair<irr::EKEY_CODE, int>>&);
	static void activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string msg, int size);
	static void activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string head_msg, std::string body_msg, int size);
	static void resetText(UI::CUITextBox* textbox, int size);
	static bool ToggleButton(const char* str_id, bool* v, ImGuiKey key);
	static void registerImgui(const wchar_t* dir);
	inline void resetLeftClick() {
		m_leftToggle = std::make_pair(false, KeyAugment::None);
	}

	inline void setBlockCursor(bool block) {
		m_blockCursor = block;
	}
	inline core::array<indexedVec3df_t> getVerts() {
		return vertexdata.first;
	}
	inline core::array<indexedVec3df_t> getSharedVerts() {
		return vertexdata.second;
	}
	inline void setVertData(indexed_vertices verts) {
		vertexdata = verts;
	}
	inline void resetVerts() {
		vertexdata.first.clear();
		vertexdata.second.clear();
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
		m_surfselected = solidSelect_t(-1, -1);
		m_portalselected = portalSelect_t(-1, -1);
		m_entityselected = -1;
		moveablevert = {};
	}
	inline bool hasMoveableVert() {
		return moveablevert.has_value();
	}
	inline indexedVec3df_t &getMoveableVert() {
		return moveablevert.value();
	}
	inline void setMoveableVert(indexedVec3df_t obj) {
		moveablevert = obj;
	}

	inline bool getKeyState(key_pair key) { 
		if (key.second == KeyAugment::AnyKey) 
			key.second = m_augment;
		
		return m_keyMap.find(key) != m_keyMap.end();
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

	void swapCursorMode(bool isRightClick);
	void setCursorMode(bool state);
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

	inline solidSelect_t getSurfISelected() {
		return m_surfselected;
	}
	inline portalSelect_t getPortalISelected() {
		return m_portalselected;
	}
	inline int getEntityISelected() {
		return m_entityselected;
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

	inline void setSurfISelected(solidSelect_t selected) {
		m_surfselected = selected;
	}
	inline void setPortalISelected(portalSelect_t selected) {
		m_portalselected = selected;
	}
	inline void setEntityISelected(int indx) {
		m_entityselected = indx;
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
	static void* readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name);

	// Function to parse each line in the settings block
	static void readLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
	
	// Function to write all settings to the INI file
	static void writeAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

};
