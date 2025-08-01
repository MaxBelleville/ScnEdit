#include "pch.h"
#include "Header/Managers/CInteractionManager.h"

IMPLEMENT_SINGLETON(CInteractionManager);

CInteractionManager::CInteractionManager()
{
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
	m_surfselected = solidSelect_t(-1, -1);
	m_portalselected = portalSelect_t(-1, -1);
	m_entityselected = -1;
}

void* CInteractionManager::readOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name) {

	if (strcmp(name, "VISIBLITY") == 0)
		return handler->UserData;

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

bool CInteractionManager::OnEvent(const SEvent& event)
{
	m_augment = KeyAugment::None;
	if (event.EventType == EET_LOG_TEXT_EVENT) {

		for (int i = 0; i < m_LogEvents.size(); i++) {
			m_LogEvents[i](event.LogEvent.Text);
		}
		if (event.LogEvent.Level == ELL_INFORMATION) {
			m_logger.AddLog("#ffffff %s\n",event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELL_DEBUG) {
			m_logger.AddLog("#ffff00 %s\n", event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELL_ERROR) {
			m_logger.AddLog("#6666ff %s\n", event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELL_WARNING) {
			m_logger.AddLog("#00ffff %s\n", event.LogEvent.Text);
			return true;
		}
		if (event.LogEvent.Level == ELL_NONE) {
			m_logger.AddLog("#aaaaaa %s\n", event.LogEvent.Text);
			return true;
		}
		
		return false;
	}

	if (event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		if (event.MouseInput.Control && event.MouseInput.Shift) m_augment = KeyAugment::CtrlShift;
		else if (event.MouseInput.Shift) m_augment = KeyAugment::Shift;
		else if (event.MouseInput.Control) m_augment = KeyAugment::Ctrl;
		m_mouse = core::vector2df((float)event.MouseInput.X,(float)event.MouseInput.Y);
		if (event.MouseInput.Event == EMIE_MOUSE_MOVED) {
			for (int i = 0; i < m_MouseMoveEvents.size(); i++) 
				m_MouseMoveEvents[i]();
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			m_leftDown = std::make_pair(true, m_augment);
			m_leftToggle = std::make_pair(false, m_augment);

		}
		if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
		{
			m_leftDown = std::make_pair(false, m_augment);
			m_leftToggle = std::make_pair(true, m_augment);
		}
		if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
			swapCursorMode(true);

		if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN || event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN || 
			event.MouseInput.Event == EMIE_MMOUSE_PRESSED_DOWN) {
			for (int i = 0; i < m_MouseDownEvents.size(); i++) 
				m_MouseDownEvents[i](event.MouseInput.Event);
			
		}
		if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP || event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ||
			event.MouseInput.Event == EMIE_MMOUSE_LEFT_UP) {
			for (int i = 0; i < m_MouseUpEvents.size(); i++) 
				m_MouseUpEvents[i](event.MouseInput.Event);
			
		}
	}
	if (event.EventType == EET_KEY_INPUT_EVENT) {
		if (event.KeyInput.Control && event.KeyInput.Shift) m_augment = KeyAugment::CtrlShift;
		else if (event.KeyInput.Shift) m_augment = KeyAugment::Shift;
		else if (event.KeyInput.Control) m_augment = KeyAugment::Ctrl;

		std::pair pair = std::make_pair(event.KeyInput.Key, m_augment);
		if (event.KeyInput.PressedDown) 
			m_keyMap.insert(pair);
		else 
			m_keyMap.erase(pair);

		for (int i = 0; i < m_KeyEvents.size(); i++)
			m_KeyEvents[i](pair);
		
	}
	return true;
}

void CInteractionManager::getNumeric(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
	for (int i = 0; i < 10; i++) 
		keys.push_back(std::make_pair(static_cast<irr::EKEY_CODE>(irr::KEY_KEY_0+i), KeyAugment::None));
}

void CInteractionManager::getAlphaNumeric(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
	CInteractionManager::getNumeric(keys);
	CInteractionManager::getAlphabetic(keys);
}

void CInteractionManager::getAlphabetic(core::array<std::pair<irr::EKEY_CODE, int>>& keys) {
	for (char c = 'A'; c <= 'Z'; ++c) {
		irr::EKEY_CODE key = static_cast<irr::EKEY_CODE>(irr::KEY_KEY_A + (c - 'A'));
		keys.push_back(std::make_pair(key, KeyAugment::AnyKey));
	}
}
void CInteractionManager::activateText(UI::CUITextBox* textbox, 
	core::array<std::pair<irr::EKEY_CODE, int>> accepted, std::string msg, int size) {
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
			for (int a = 0; a < 4; a++) {
				if (m_keyMap.find(std::make_pair(keys[i].first, static_cast<KeyAugment>(a))) != m_keyMap.end())
					return true;
			}
		}
		else {
			if (m_keyMap.find(keys[i]) != m_keyMap.end())
				return true;
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

void CInteractionManager::swapCursorMode(bool isRightClick)
{
	if (m_blockCursor) return;
	gui::ICursorControl* cursor = getApplication()->getDevice()->getCursorControl();
	cursor->setVisible(!cursor->isVisible());

	if (!cursor->isVisible()) 
		resetLeftClick();
	for (int i = 0; i < m_CursorModeEvents.size(); i++)
		m_CursorModeEvents[i](cursor->isVisible(), isRightClick);
}

void CInteractionManager::setCursorMode(bool state)
{
	if (m_blockCursor) return;
	gui::ICursorControl* cursor = getApplication()->getDevice()->getCursorControl();
	cursor->setVisible(state);
	if (!state)
		resetLeftClick();
	for (int i = 0; i < m_CursorModeEvents.size(); i++)
		m_CursorModeEvents[i](state,false);
}


void CInteractionManager::updateVertPos(indexedVec3df_t vert) {
	moveablevert.pos = vert.pos;
	for (int i = 0; i < getVerts().size(); i++) {
		if (getVerts()[i].vertidxidx == vert.vertidxidx) {
			vertexdata.first[i].pos = vert.pos;
			return;
		}
	}
	for (int i = 0; i < getSharedVerts().size(); i++) {
		if (getSharedVerts()[i].vertidxidx == moveablevert.vertidxidx) {
			vertexdata.second[i].pos = vert.pos;
			return;
		}
	}
}