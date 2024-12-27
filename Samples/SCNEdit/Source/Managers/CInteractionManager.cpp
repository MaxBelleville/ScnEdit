#include "pch.h"
#include "Header/Managers/CInteractionManager.h"

IMPLEMENT_SINGLETON(CInteractionManager);

CInteractionManager::CInteractionManager()
{
	m_logs.push_back(new ImGuiAl::BufferedLog<81920>());
	CEventManager::getInstance()->registerEvent("InteractionManager",this);
	// Register custom INI settings handler
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName = "SCN";
	ini_handler.TypeHash = ImHashStr("SCN");
	ini_handler.ReadOpenFn = readOpen;
	ini_handler.ReadLineFn = readLine;
	ini_handler.WriteAllFn = writeAll;
	ini_handler.UserData = gui;
	ImGui::AddSettingsHandler(&ini_handler);
}
CInteractionManager::~CInteractionManager() {
	CEventManager::getInstance()->unRegisterEvent(this);
}

bool CInteractionManager::OnEvent(const SEvent& event)
{
	KeyAugment aug = KeyAugment::None;
	if (event.EventType == EET_LOG_TEXT_EVENT) {
		if (getLog()->available() < 50) {
			m_logs.push_back(new ImGuiAl::BufferedLog<81920>());
			page += 1;
		}
		for (int i = 0; i < m_LogEvents.size(); i++) {
			m_LogEvents[i](event.LogEvent.Text);
		}
		if (event.LogEvent.Level == ELOG_LEVEL::ELL_INFORMATION) {
			getLog()->info(event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELOG_LEVEL::ELL_DEBUG) {
			getLog()->debug(event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELOG_LEVEL::ELL_ERROR) {
			getLog()->error(event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELOG_LEVEL::ELL_WARNING) {
			getLog()->warning(event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELOG_LEVEL::ELL_NONE) {
			getLog()->debug(event.LogEvent.Text);
			return true;
		}
		
		return false;
	}

	if (event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		if (event.MouseInput.Control && event.MouseInput.Shift) aug = KeyAugment::CtrlShift;
		else if (event.MouseInput.Shift) aug = KeyAugment::Shift;
		else if (event.MouseInput.Control) aug = KeyAugment::Ctrl;
		m_mouse = core::vector2df((float)event.MouseInput.X,(float)event.MouseInput.Y);
		if (event.MouseInput.Event == EMIE_MOUSE_MOVED) {
			for (int i = 0; i < m_MouseMoveEvents.size(); i++) {
				m_MouseMoveEvents[i]();
			}
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			m_leftDown = std::make_pair(true,aug);
			m_leftToggle = std::make_pair(false,aug);

		}
		if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
		{
			m_leftDown = std::make_pair(false,aug);
			m_leftToggle = std::make_pair(true,aug);
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN || event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN || 
			event.MouseInput.Event == EMIE_MMOUSE_PRESSED_DOWN) {
			for (int i = 0; i < m_MouseDownEvents.size(); i++) {
				m_MouseDownEvents[i](event.MouseInput.Event);
			}
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP || event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ||
			event.MouseInput.Event == EMIE_MMOUSE_LEFT_UP) {
			for (int i = 0; i < m_MouseUpEvents.size(); i++) {
				m_MouseUpEvents[i](event.MouseInput.Event);
			}
		}
	}
	if (event.EventType == EET_KEY_INPUT_EVENT) {
		if (event.KeyInput.Control && event.KeyInput.Shift) aug = KeyAugment::CtrlShift;
		else if (event.KeyInput.Shift) aug = KeyAugment::Shift;
		else if (event.KeyInput.Control) aug = KeyAugment::Ctrl;
		std::pair pair = std::make_pair(event.KeyInput.Key, aug);
		if (event.KeyInput.PressedDown)
			m_keyMap[pair] = true;
		else
			m_keyMap[pair] = false;
		for (int i = 0; i < m_KeyEvents.size(); i++) {
			m_KeyEvents[i](pair);
		}
	}
	return true;
}

void* CInteractionManager::readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name) {

	if (strcmp(name, "VISIBLITY") == 0) {
		return handler->UserData;
	}
	return nullptr;

}
void CInteractionManager::readLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
	guiSettings_t* settings = static_cast<guiSettings_t*>(entry);
	sscanf(line, "scn=%d", &settings->vis_scn);
	sscanf(line, "doors=%d", &settings->vis_doors);
	sscanf(line, "portals=%d", &settings->vis_portals);
	sscanf(line, "bb=%d", &settings->vis_bb);
	sscanf(line, "entities=%d", &settings->vis_entities);
	sscanf(line, "lightmaps=%d", &settings->vis_lightmaps);
	settings->scrape_lightmaps = false;
}

void CInteractionManager::writeAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
	guiSettings_t* settings = static_cast<guiSettings_t*>(handler->UserData);

	buf->appendf("[SCN][VISIBLITY]\n");
	buf->appendf("scn=%d\n", settings->vis_scn);
	buf->appendf("doors=%d\n", settings->vis_doors);
	buf->appendf("portals=%d\n", settings->vis_portals);
	buf->appendf("bb=%d\n", settings->vis_bb);
	buf->appendf("entities=%d\n", settings->vis_entities);
	buf->appendf("lightmaps=%d\n", settings->vis_lightmaps);
}
bool CInteractionManager::isGuiSettingsUpdated() {
	return gui->vis_scn != prevgui->vis_scn ||
		gui->vis_doors != prevgui->vis_doors ||
		gui->vis_portals != prevgui->vis_portals ||
		gui->vis_bb != prevgui->vis_bb ||
		gui->vis_entities != prevgui->vis_entities ||
		gui->vis_lightmaps != prevgui->vis_lightmaps ||
		gui->scrape_lightmaps != prevgui->scrape_lightmaps;
}
void CInteractionManager::updateVertPos(indexedVec3df_t vert) {
	moveablevert.pos = vert.pos;
	for (int i = 0; i < getVerts().size(); i++) {
		if (getVerts()[i].vertindx == vert.vertindx) {
			vertexdata.first[i].pos = vert.pos;
			return;
		}
	}
	for (int i = 0; i < getSharedVerts().size(); i++) {
		if (getSharedVerts()[i].vertindx == moveablevert.vertindx) {
			vertexdata.second[i].pos = vert.pos;
			return;
		}
	}
}