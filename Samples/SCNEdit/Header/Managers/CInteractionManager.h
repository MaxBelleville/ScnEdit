#include "SkylichtEngine.h"
#include "Utils/CSingleton.h"
#include "Collision/CCollisionManager.h"
#include "Header/Base/util.h"
#include "Header/Base/scntypes.h"
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
	Debug
};
enum SelectedType {
	Solid,
	SolidExtra,
	Portal,
	Entity,
	Empty
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
public:
	CInteractionManager();

	virtual ~CInteractionManager();

	bool OnEvent(const SEvent& event);

	void resetLeftClick() {
		m_leftToggle = std::make_pair(false, KeyAugment::None);
	}
	core::array<indexedVec3df_t> getSharedVerts() {
		return vertexdata.second;
	}
	core::array<indexedVec3df_t> getVerts() {
		return vertexdata.first;
	}
	void setVertData(indexed_vertices verts) {
		vertexdata = verts;
	}
	void resetVerts() {
		vertexdata.first.clear();
		vertexdata.second.clear();
	}
	void updateVertPos(indexedVec3df_t vert);

	CGameObject* getSelectObj() {
		return selectobj;
	}
	void setSelectObj(CGameObject* obj) {
		selectobj = obj;
	}
	void resetSelectObj() {
		selectobj = NULL;
	}
	indexedVec3df_t getMoveableVert() {
		return moveablevert;
	}
	void setMoveableVert(indexedVec3df_t obj) {
		moveablevert = obj;
	}

	bool getKeyState(key_pair key) {
		return m_keyMap[key];
	}
	bool isLeftClicked() {
		return m_leftToggle.first;
	}
	bool isLeftDown() {
		return m_leftDown.first;
	}
	KeyAugment getLeftClickAug() {
		return m_leftToggle.second;
	}
	KeyAugment getLeftDownAug() {
		return m_leftDown.second;
	}
	bool isLeftClicked(KeyAugment aug) {
		return m_leftToggle.first && aug ==m_leftToggle.second;
	}
	bool isLeftDown(KeyAugment aug) {
		return m_leftDown.first && aug == m_leftToggle.second;
	}
	guiSettings_t* getGuiSettings() {
		return gui;
	}
	bool isGuiSettingsUpdated();
	void resetPrevGui() {
		*prevgui = *gui;
	}

	core::vector2df getMouse() {
		return m_mouse;
	}
	void setMouse(core::vector2df mouse) {
		m_mouse = mouse;
	}
	template<typename... Arg>
	
	GUIState getState() {
		return m_guiState;
	}
	SelectedType getSelectedType() {
		return m_selectedType;
	}
	
	std::pair<int, int> getSurfISelected() {
		return m_surfselected;
	}
	std::pair<int, int> getPortalISelected() {
		return m_portalselected;
	}
	int getEntityISelected() {
		return m_entityselected;
	}
	ImGuiAl::Log* getLog() {
		return m_logs[page];
	}
	size_t getLogSize() {
		return m_logs.size();
	}
	u16 getCurrentLogPage() {
		return page;
	}
	void setLogPage(u16 current) {
		page= current;
	}

	template<typename...States>
	bool findState(States... states) {
		return ((m_guiState == states) || ...);
	}
	bool findKeyState(std::vector< key_pair> keys) {
		for (int i = 0; i < keys.size(); i++) {
			if (m_keyMap[keys[i]]) return true;
		}
		return false;
	}
	template<typename...States>
	bool setFoundState(GUIState reset,States... states) {
		if (((m_guiState == states) || ...)) {
			m_guiState = reset;
			return true;
		}
		return false;
	}
	void setState(GUIState state) {
		m_guiState = state;
	}

	template<typename...States>
	bool findSelectedType(States... states) {
		return ((m_selectedType == states) || ...);
	}
	template<typename...States>
	bool setFoundSelectedType(SelectedType reset, States... states) {
		if (((m_selectedType == states) || ...)) {
			m_selectedType = reset;
			return true;
		}
		return false;
	}
	void setSelectedType(SelectedType state) {
		m_selectedType = state;
	}

	void setSurfISelected(std::pair<int, int> selected) {
		m_surfselected =selected;
	}
	void setPortalISelected(std::pair<int,int> selected) {
		m_portalselected = selected;
	}
	void setEntityISelected(int indx) {
		m_entityselected = indx;
	}

	void OnKeyEvent(std::function<void(key_pair)> func) {
		m_KeyEvents.push_back(func);
	}
	void OnMouseMoveEvent(std::function<void()> func) {
		m_MouseMoveEvents.push_back(func);
	}
	void OnMouseDownEvent(std::function<void(EMOUSE_INPUT_EVENT)> func) {
		m_MouseDownEvents.push_back(func);
	}
	void OnMouseUpEvent(std::function<void(EMOUSE_INPUT_EVENT)> func) {
		m_MouseUpEvents.push_back(func);
	}
	void OnLogEvent(std::function<void(const char*)> func) {
		m_LogEvents.push_back(func);
	}
	void OnCollisionEvent(std::function<void(core::triangle3df, core::vector3df)> func) {
		m_ColliderEvents.push_back(func);
	}
	void OnVertexUpdateEvent(std::function<void()> func) {
		m_VertexUpdateEvents.push_back(func);
	}
	void CollisionCallback(core::triangle3df tri, core::vector3df intersection) {
		for (int i = 0; i < m_ColliderEvents.size(); i++) m_ColliderEvents[i](tri, intersection);
	}

	void VertexCallback() {
		for (int i = 0; i < m_VertexUpdateEvents.size(); i++) m_VertexUpdateEvents[i]();
	}

protected:
	// Function to read the custom settings block (initialize)
	static void* readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name);

	// Function to parse each line in the settings block
	static void readLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
	
	// Function to write all settings to the INI file
	static void writeAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

};
