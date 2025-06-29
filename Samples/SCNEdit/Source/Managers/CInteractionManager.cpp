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
	m_KeyEvents.clear();
	m_ColliderEvents.clear();
	m_KeyEvents.clear();
	m_LogEvents.clear();
	m_MouseDownEvents.clear();
	m_MouseUpEvents.clear();
	m_VertexUpdateEvents.clear();
	resetLeftClick();
	resetPrevGui();
	resetSelectObj();
	resetVerts();
	m_selectedType = SelectedType::Empty;
	m_guiState = GUIState::Default;
	m_logs.clear();
	m_surfselected = std::make_pair(-1, -1);
	m_portalselected = std::make_pair(-1, -1);
	m_entityselected = -1;
	page = 0;
}

bool CInteractionManager::OnEvent(const SEvent& event)
{
	augment = KeyAugment::None;
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
		if (event.MouseInput.Control && event.MouseInput.Shift) augment = KeyAugment::CtrlShift;
		else if (event.MouseInput.Shift) augment = KeyAugment::Shift;
		else if (event.MouseInput.Control) augment = KeyAugment::Ctrl;
		m_mouse = core::vector2df((float)event.MouseInput.X,(float)event.MouseInput.Y);
		if (event.MouseInput.Event == EMIE_MOUSE_MOVED) {
			for (int i = 0; i < m_MouseMoveEvents.size(); i++) {
				m_MouseMoveEvents[i]();
			}
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			m_leftDown = std::make_pair(true, augment);
			m_leftToggle = std::make_pair(false, augment);

		}
		if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
		{
			m_leftDown = std::make_pair(false, augment);
			m_leftToggle = std::make_pair(true, augment);
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
		if (event.KeyInput.Control && event.KeyInput.Shift) augment = KeyAugment::CtrlShift;
		else if (event.KeyInput.Shift) augment = KeyAugment::Shift;
		else if (event.KeyInput.Control) augment = KeyAugment::Ctrl;

		std::pair pair = std::make_pair(event.KeyInput.Key, augment);
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

core::array<std::pair<irr::EKEY_CODE, int>> CInteractionManager::getNumeric() {
	core::array<std::pair<irr::EKEY_CODE, int>> keys;
	keys.push_back(std::make_pair(irr::KEY_KEY_0,KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_1, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_2, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_3, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_4, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_5, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_6, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_7, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_8, KeyAugment::None));
	keys.push_back(std::make_pair(irr::KEY_KEY_9, KeyAugment::None));
	return keys;
}

core::array<std::pair<irr::EKEY_CODE, int>> CInteractionManager::getAlphaNumeric() {
	core::array<std::pair<irr::EKEY_CODE, int>> keys= CInteractionManager::getNumeric();
	for (char c = 'A'; c <= 'Z'; ++c) {
		irr::EKEY_CODE key = static_cast<irr::EKEY_CODE>(irr::KEY_KEY_A + (c - 'A'));
		keys.push_back(std::make_pair(key, KeyAugment::AnyKey));
	}
	return keys;
}

core::array<std::pair<irr::EKEY_CODE, int>> CInteractionManager::getAlphabetic() {
	core::array<std::pair<irr::EKEY_CODE, int>> keys;
	for (char c = 'A'; c <= 'Z'; ++c) {
		irr::EKEY_CODE key = static_cast<irr::EKEY_CODE>(irr::KEY_KEY_A + (c - 'A'));
		keys.push_back(std::make_pair(key, KeyAugment::AnyKey));
	}
	return keys;
}
void CInteractionManager::activateText(UI::CUITextBox* textbox, core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string msg, int size) {
	textbox->setAcceptedKeys(accepted);
	textbox->setText(msg.c_str());
	textbox->setLength(msg.length(), msg.length() + size);
	textbox->setEnable(true);
}

void CInteractionManager::resetText(UI::CUITextBox* textbox, int size) {
	textbox->resetAcceptedKeys();
	textbox->setText("");
	textbox->setLength(0, size);
	textbox->setEnable(false);

}

bool CInteractionManager::findKeyState(std::vector<key_pair> keys) {
	for (int i = 0; i < keys.size(); i++) {
		if (keys[i].second == KeyAugment::AnyKey) {
			if (m_keyMap[std::make_pair(keys[i].first,KeyAugment::None)]) return true;
			if (m_keyMap[std::make_pair(keys[i].first, KeyAugment::Shift)]) return true;
			if (m_keyMap[std::make_pair(keys[i].first, KeyAugment::Ctrl)]) return true;
			if (m_keyMap[std::make_pair(keys[i].first, KeyAugment::CtrlShift)]) return true;
		}
		else {
			if (m_keyMap[keys[i]]) return true;
		}
	}
	return false;
}

bool CInteractionManager::ToggleButton(const char* str_id, bool* v)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight() *.85f;
	float width = height * 1.4f;
	float radius = height * 0.475f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	float t = *v ? 1.0f : 0.0f;

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.58f, 0.58f, 0.58f, 1.0f), ImVec4(0.44f, 0.16f, 0.73f, 1.0f), t));
	else
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), ImVec4(0.58f, 0.24f, 0.73f, 1.0f), t));

	if (ImGui::IsItemClicked())
		*v = !*v;
	ImGui::SameLine();
	ImGui::Text(str_id);

	
	ImGuiContext& g = *GImGui;
	float ANIM_SPEED = 0.08f;
	if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
	{
		float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
		t = *v ? (t_anim) : (1.0f - t_anim);
	}
	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	return ImGui::IsItemClicked();
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