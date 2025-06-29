#include "SkylichtEngine.h"
#include "Utils/CSingleton.h"
#include "Collision/CCollisionManager.h"
#include "Header/Base/util.h"
#include "Header/Base/scntypes.h"
#include "UserInterface/CUITextBox.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <term/imguial_term.h>
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
	OpenTexture,
	Export,
	Debug,
	EditAll,
	EditFlags,
	EditAlpha,
	EditShading
};
enum UVMode {
	Move,
	Resize,
	FlipH,
	FlipV
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
	guiSettings_t* gui =new guiSettings_t();
	guiSettings_t* prevgui = new guiSettings_t();
	key_map m_keyMap;
	std::pair<bool,KeyAugment> m_leftDown = std::make_pair(false,KeyAugment::None);
	std::pair<bool, KeyAugment> m_leftToggle = std::make_pair(false, KeyAugment::None);
	core::vector2df m_mouse;
	GUIState m_guiState= GUIState::Default;
	UVMode m_uvMode = UVMode::Move;
	float m_uvScalar = 0.01;
	std::vector<ImGuiAl::Log*> m_logs;
	u16 page =0;
	std::pair<int, int> m_surfselected = std::make_pair(-1, -1);
	std::pair<int, int> m_portalselected = std::make_pair(-1, -1);
	int m_entityselected = -1;
	SelectedType m_selectedType = SelectedType::Empty;
	std::vector<std::function<void(key_pair)>> m_KeyEvents;
	std::vector<std::function<void()>> m_MouseMoveEvents;
	std::vector<std::function<void(const char*)>> m_LogEvents;
	std::vector<std::function<void(EMOUSE_INPUT_EVENT)>> m_MouseDownEvents;
	std::vector<std::function<void(EMOUSE_INPUT_EVENT)>> m_MouseUpEvents;
	std::vector<std::function<void(core::triangle3df, core::vector3df)>> m_ColliderEvents;
	std::vector<std::function<void()>> m_VertexUpdateEvents;
	CGameObject* selectobj = NULL;
	indexed_vertices vertexdata = std::make_pair(core::array<indexedVec3df_t>(), core::array<indexedVec3df_t>());
	indexedVec3df_t moveablevert;
	KeyAugment augment;
public:
	CInteractionManager();

	virtual ~CInteractionManager();

	bool OnEvent(const SEvent& event);

	inline void resetLeftClick() {
		m_leftToggle = std::make_pair(false, KeyAugment::None);
	}
	inline core::array<indexedVec3df_t> getSharedVerts() {
		return vertexdata.second;
	}
	inline core::array<indexedVec3df_t> getVerts() {
		return vertexdata.first;
	}
	inline void setVertData(indexed_vertices verts) {
		vertexdata = verts;
	}
	inline void resetVerts() {
		vertexdata.first.clear();
		vertexdata.second.clear();
	}
	void updateVertPos(indexedVec3df_t vert);

	inline CGameObject* getSelectObj() {
		return selectobj;
	}
	inline void setSelectObj(CGameObject* obj) {
		selectobj = obj;
	}
	inline void resetSelectObj() {
		selectobj = NULL;
	}
	inline indexedVec3df_t getMoveableVert() {
		return moveablevert;
	}
	inline void setMoveableVert(indexedVec3df_t obj) {
		moveablevert = obj;
	}



	inline bool getKeyState(key_pair key) {
		if (key.second == KeyAugment::AnyKey) key.second = KeyAugment::None;
		return m_keyMap[key];
	}
	inline KeyAugment getKeyAugment() {
		return augment;
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
		return m_leftToggle.first && aug ==m_leftToggle.second;
	}
	inline bool isLeftDown(KeyAugment aug) {
		return m_leftDown.first && aug == m_leftToggle.second;
	}
	inline guiSettings_t* getGuiSettings() {
		return gui;
	}
    bool isGuiSettingsUpdated();
	inline void resetPrevGui() {
		*prevgui = *gui;
	}

	inline core::vector2df getMouse() {
		return m_mouse;
	}
	inline void setMouse(core::vector2df mouse) {
		m_mouse = mouse;
	}
	inline GUIState getState() {
		return m_guiState;
	}
	inline SelectedType getSelectedType() {
		return m_selectedType;
	}
	
	inline std::pair<int, int> getSurfISelected() {
		return m_surfselected;
	}
	inline std::pair<int, int> getPortalISelected() {
		return m_portalselected;
	}
	inline int getEntityISelected() {
		return m_entityselected;
	}
	inline ImGuiAl::Log* getLog() {
		return m_logs[page];
	}
	inline size_t getLogSize() {
		return m_logs.size();
	}
	inline u16 getCurrentLogPage() {
		return page;
	}
	inline void setLogPage(u16 current) {
		page= current;
	}

	inline float getUVScalar() { return m_uvScalar; }
	inline void setUVScalar(float scalar) {
		m_uvScalar = scalar; 
	}
	inline UVMode getUVMode() { return m_uvMode; }
	inline bool findUVMode(UVMode mode) { return m_uvMode == mode; }
	inline void swapUVMode() {
		if (m_uvMode == UVMode::Move) m_uvMode = UVMode::Resize;
		else m_uvMode = UVMode::Move;
	}

	template<typename...States>
	inline bool findState(States... states) {
		return ((m_guiState == states) || ...);
	}
	template<typename...States>
	inline bool findKeyAugment(States... states) {
		return ((augment == states) || ...);
	}



	template<typename...States>
	inline bool findStateIgnoreEdit(States... states) {
		return ((m_guiState == states || m_guiState > 8) || ... );
	}

	bool findKeyState(std::vector<key_pair> keys);

	template<typename...States>
	inline bool setFoundState(GUIState reset,States... states) {
		bool args = ((m_guiState == states) || ...);
		if (args) m_guiState = reset;
		return args;
	}
	inline void setState(GUIState state) {
		m_guiState = state;
	}

	template<typename...States>
	inline bool findSelectedType(States... states) {
		return ((m_selectedType == states) || ...);
	}
	template<typename...States>
	inline bool setFoundSelectedType(SelectedType reset, States... states) {
		bool args = ((m_selectedType == states) || ...);
		if (args) m_selectedType = reset;
		return args;
	}
	inline void setSelectedType(SelectedType state) {
		m_selectedType = state;
	}

	inline void setSurfISelected(std::pair<int, int> selected) {
		m_surfselected =selected;
	}
	inline void setPortalISelected(std::pair<int,int> selected) {
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
	inline void OnVertexUpdateEvent(std::function<void()> func) {
		m_VertexUpdateEvents.push_back(func);
	}
	inline void CollisionCallback(core::triangle3df tri, core::vector3df intersection) {
		for (int i = 0; i < m_ColliderEvents.size(); i++) 
			m_ColliderEvents[i](tri, intersection);
	}
	inline void VertexCallback() {
		for (int i = 0; i < m_VertexUpdateEvents.size(); i++) 
			m_VertexUpdateEvents[i]();
	}
	static core::array<std::pair<irr::EKEY_CODE, int>> getNumeric();
	static core::array<std::pair<irr::EKEY_CODE, int>> getAlphaNumeric();
	static core::array<std::pair<irr::EKEY_CODE, int>> getAlphabetic();
	static void activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string msg, int size);
	static void resetText(UI::CUITextBox* textbox, int size);
	static bool ToggleButton(const char* str_id, bool* v);
protected:
	// Function to read the custom settings block (initialize)
	static void* readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name);

	// Function to parse each line in the settings block
	static void readLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
	
	// Function to write all settings to the INI file
	static void writeAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

};
