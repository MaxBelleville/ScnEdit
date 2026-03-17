#include "pch.h"
#include "Header/CViewGui.h"

#include "Graphics2D/CGUIImporter.h"
#include "Header/Managers/CContext.h"
#include "Header/SCNEdit.h"
#include "Header/Managers/CInteractionManager.h"
#include "Header/CViewInit.h"
#include "CImguiManager.h"
CViewGui::CViewGui(CScnArguments* args)
{
	m_arguments = args;
	CScn* scn = SCNEdit::getSCN();
	
	bool lightmaps = false;
	if (scn) 
		lightmaps = scn->getLightmap()->hasLightmaps();
	if (!lightmaps) 
		m_missingItems += 3;
	if (!m_arguments->isInternalDebug()) 
		m_missingItems += 1;
}

CViewGui::~CViewGui()
{
	m_missingItems = 0;
	//delete m_canvas;
	m_tooltip = "";

}

void CViewGui::onInit()
{
	//Get singletons and variables then load gui
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CGameObject* gui = zone->createEmptyObject();
	m_canvas = gui->addComponent<CCanvas>();
	CGUIImporter::loadGUI("sedata/gui/SCNEdit.gui", m_canvas);

	//TODO: Work on this scaling solution for canvas.
	float scale = min(m_arguments->getRes().Width / m_arguments->getDesktopRes().Width,
		m_arguments->getRes().Height / m_arguments->getDesktopRes().Height);
	float scale2 = min(m_arguments->getRes().Width / 1920,
		m_arguments->getRes().Height / 1080);

	float finalScale = clamp(min(scale, scale2),0.0f,1.0f);

	m_canvas->applyScaleGUI(finalScale);
	m_canvas->setSortDepth(0);

	//Extract elements from gui.
	m_uiContainer = gui->addComponent<UI::CUIContainer>();

	m_openButton = addButton("Canvas/Container/Open_Btn");
	addTooltip(m_openButton, "Open a new .scn mission/map.", "");

	m_closeButton = addButton("Canvas/Container/Close_Btn");
	addTooltip(m_closeButton, "Close current .scn map.", "No map found so does nothing.");
	
	m_exportButton = addButton("Canvas/Container/Export_Btn");
	addTooltip(m_exportButton, "Export obj,map mtl, 3ds files\nfor use in other programs.", 
		"Nothing to export,\nplease open a .scn map.");
	
	m_saveButton = addButton("Canvas/Container/Save_Btn");
	addTooltip(m_saveButton, "Save over current .scn file.", "Nothing to save,\nplease open a .scn map");
	
	m_quitButton  = addButton("Canvas/Container/Quit_Btn");
	m_textSection1 = addTextbox("Canvas/Container/BottomBar/Section1_Rect/Section1_Input");

	m_textSection2  = addTextbox("Canvas/Container/BottomBar/Section2_Rect/Section2_Input");
	m_textSection3a = addTextbox("Canvas/Container/BottomBar/Section3_Rect/A_Input");
	m_textSection3b = addTextbox("Canvas/Container/BottomBar/Section3_Rect/B_Input");
	m_textSection3c = addTextbox("Canvas/Container/BottomBar/Section3_Rect/C_Input");


	setupEvents();
}
UI::CUITextBox* CViewGui::addTextbox(const char* textPath) {
	CGUIElement* element = m_canvas->getGUIByPath(textPath);
	if (element) {
		UI::CUITextBox* textbox = new UI::CUITextBox(m_uiContainer, element);
		CGUIElement* bg = textbox->getElement();
		textbox->setText("");
		textbox->setEnable(false);
		textbox->setContinueGameEvent(true);
		return textbox;
	}
}

UI::CUIButton* CViewGui::addButton(const char* btnPath) {
	CGUIElement* element = m_canvas->getGUIByPath(btnPath);

	if (element) {
		UI::CUIButton* button = new UI::CUIButton(m_uiContainer, element);
		if (button) {
			CGUIElement* bg = button->getBackground();
			button->addMotion(UI::EMotionEvent::PointerDown, bg, 
				new UI::CColorMotion(SColor(255, 125, 125, 175)))->setTime(0.0f, 50.0f);
			button->addMotion(UI::EMotionEvent::PointerHover, bg, 
				new UI::CColorMotion(SColor(255, 175, 175, 225)))->setTime(0.0f, 50.0f);
			button->addMotion(UI::EMotionEvent::PointerUp, bg, 
				new UI::CColorMotion(SColor(255, 175, 175, 225)))->setTime(0.0f, 50.0f);

			button->addMotion(UI::EMotionEvent::PointerOut, bg, new UI::CColorMotion())->setTime(0.0f, 50.0f);
			button->setSkipPointerEventWhenDrag(true);
			button->setContinueGameEvent(true);
			return button;
		}
	}
}


void CViewGui::addTooltip(UI::CUIButton* btn, std::string text, std::string altText) {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	btn->OnPointerHover = [interaction,altText,text](float x, float y) {
		
		if (interaction->guiStateLayer().swap(GUIState::Hover, GUIState::Default)) {
			interaction->setMouse(core::vector2df(x, y));
			if (!altText.empty()) {
				if (SCNEdit::getSCN()) 
					m_tooltip = text;
				else 
					m_tooltip = altText;
			}
			else 
				m_tooltip = text;
		}
	};
	btn->OnPointerOut = [interaction](float x, float y) {
		interaction->guiStateLayer().swap(GUIState::Default, GUIState::Hover);
	};
}

void CViewGui::setupEvents() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	//UI Button events
	m_quitButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		interaction->guiStateLayer().set(GUIState::Quit);
	};

	m_openButton->OnPressed = [interaction](UI::CUIBase* base)
	{
			if (interaction->guiStateLayer().swap(GUIState::OpenScn, GUIState::Default, GUIState::Hover)) {
				IGFD::FileDialogConfig config;
				config.path = ".";
				ImGuiFileDialog::Instance()->OpenDialog("ChooseSCN", "Open SCN", ".scn", config);
			}
	};

	m_closeButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		if (SCNEdit::getSCN()) 
			interaction->guiStateLayer().set(GUIState::CloseFile);
	};

	m_exportButton->OnPressed = [interaction](UI::CUIBase* base)
	{
			if (SCNEdit::getSCN()) {
				SCNEdit::exportSCN();
				static const char* buttons[] = { "Ok", NULL };
				m_msgbox.Init("Exported!", NULL, "Should have exported changes!", buttons, false);
				int finish = m_msgbox.Draw();
				if (finish > 0)
					interaction->guiStateLayer().set(GUIState::Default);
				else
					m_msgbox.Open();
			}
	};

	m_saveButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		//Save data will initalize a warning msgbox before save.
		if (SCNEdit::getSCN()) {	
			guiSettings_t* gui = interaction->getGuiSettings();
			if (gui->scrape_lightmaps) {
				static const char* buttons[] = { "Save", "Cancel", NULL };
				m_msgbox.Init("Save with Scraped Lightmaps", NULL,
					"Notice Lightmaps are Scraped.\n\nWill save without lightmaps.", buttons, false);
				if (!interaction->guiStateLayer().swap(GUIState::ScrapeLightSave, GUIState::Default, GUIState::Hover)) {
					interaction->guiStateLayer().set(GUIState::Default);
				}
			}
			else {
				if (interaction->guiStateLayer().swap(GUIState::Save, GUIState::Default, GUIState::Hover)) 
					savePressed = true;
				
				else {
					interaction->guiStateLayer().set(GUIState::Default);
					savePressed = false;
				}
			}
		}
	};
	bool isVertInf = m_arguments->isVertInfo();

	//Section 1 and 2 text edit events
	m_textSection1->OnTextSet = [interaction, isVertInf](UI::CUIBase* base) {
		CScn* scn = SCNEdit::getSCN();
		if (interaction->guiStateLayer().find(GUIState::EditFlags,GUIState::EditAlpha)) {
			//Uses the ": " in the input to proccess the number and update surf
			std::string msg = str_split(m_textSection1->getText(), ": ")[1];
			u8 num = std::atoi(msg.c_str());
			//This section is repeated maybe I can improve it so it's shared between textsections?

			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();

			for (int i = 0; i < comp->selsurfs.size(); i++) {
				int si = comp->selsurfs[i];
				CScnSolid* solid = scn->getSolid(comp->solididx);
				scnSurf_t* surfi = &solid->surfs[si];
				if (interaction->guiStateLayer().get() == GUIState::EditAlpha) {
					if (num > 255) num = 255;
					surfi->alpha = num;
				}
				else
					surfi->flag1 = num;
			}

		}
		else if (interaction->guiStateLayer().find(GUIState::EditShading,EditSurfShading)) {
			std::string msg = str_split(m_textSection1->getText(), ": ")[1];
			core::array<std::string> split = str_split(msg.c_str(), " ");
			u8 color[4] = { 255,255,255,255 };
			for (int i = 0; i < split.size() && i<4; i++) {
				color[i] = std::atoi(split[i].c_str());
			}
			
			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
			if (interaction->guiStateLayer().get() == GUIState::EditSurfShading) {
				for (int i = 0; i < comp->selsurfs.size(); i++) {
					int si = comp->selsurfs[i];
					CScnSolid* solid = scn->getSolid(comp->solididx);
					scnSurf_t* surfi = &solid->surfs[si];
					if (surfi->shading && surfi->hasVertexColors) {
						for (u32 j = 0; j < surfi->faceidxlen; j++)
						{
							u8* ps = surfi->shading + (j * 4);
							for (int i = 0; i < 4; i++) {

								if (color[i] > 255) color[i] = 255;
								ps[i] = color[i];
							}
						}
					}
				}

			}
			else if (interaction->hasMoveableVert()) {
				indexedVec3df_t vert = interaction->getMoveableVert();
				CScnSolid* solid = scn->getSolid(vert.solididx);
				scnSurf_t* surfi = &solid->surfs[vert.surfidx];


				if (surfi->shading && surfi->hasVertexColors) {

					u8* ps = surfi->shading + (vert.surf_vertidx * 4);
					for (int i = 0; i < 4; i++) {
						
						if (color[i] > 255) color[i] = 255;
						ps[i] = color[i];
					}
				}
			}
		}
	
		isSection1Set = true;
		if (interaction->guiStateLayer().find(GUIState::EditFlags) && isSection1Set && isSection2Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}
	
		if (interaction->guiStateLayer().find(GUIState::EditAlpha, GUIState::EditShading, 
			GUIState::EditSurfShading) && isSection1Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}
		
	};

	m_textSection2->OnTextSet = [interaction, isVertInf](UI::CUIBase* base) {
		if (interaction->guiStateLayer().find(GUIState::EditFlags)) {
			std::string msg = str_split(m_textSection2->getText(), ": ")[1];
			u8 flag2 = std::atoi(msg.c_str());
			CScn* scn = SCNEdit::getSCN();


			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
				
			for (int i = 0; i < comp->selsurfs.size(); i++) {
				int si = comp->selsurfs[i];
				CScnSolid* solid = scn->getSolid(comp->solididx);
				scnSurf_t* surfi = &solid->surfs[si];


				surfi->flag2 = flag2;
			}
	
		}
		else if (interaction->guiStateLayer().find(GUIState::EditEntity)) {
			core::array<std::string> msg = str_split(m_textSection2->getText(), ",");
			CScn* scn = SCNEdit::getSCN();
			int entindx = interaction->getEntityISelected();
			CScnEnt* ent = scn->getEnt(entindx);
			core::array<std::string> keys;

			//Get keys that aren't filtered.
			for (u32 i = 0; i < ent->n_fields; i++) {
				if (!str_equiv(ent->fields[i].key, "Classname") && !str_equiv(ent->fields[i].key, "Origin")) {
					keys.push_back(ent->fields[i].key);
				}
			}

			if (keys.size() == msg.size()) {
				for (u32 i = 0; i < keys.size(); i++) {

					std::string trimmed = str_trim(msg[i].c_str(), " \t\n\r");
					if (!trimmed.empty()) {
						ent->setField(keys[i].c_str(), trimmed.c_str());
					}
				}
			}
			
		}
		isSection2Set = true;
		if (interaction->guiStateLayer().find(GUIState::EditEntity) && isSection2Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}

		if (interaction->guiStateLayer().find(GUIState::EditFlags) && isSection1Set && isSection2Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}
	};

	//Cursor swap/set interaction event only work when not visible and you right click rather then set.

	interaction->OnCursorModeEvent([interaction, isVertInf](bool visiblity, bool isRightClick) {
		if (!visiblity && isRightClick) {
			interaction->guiStateLayer().swap(GUIState::Default, GUIState::Debug);

			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::OpenScn, GUIState::OpenTexture)) {
				ImGuiFileDialog::Instance()->Close();
			}

			if (interaction->guiStateLayer().swap(GUIState::Default,
				GUIState::EditEntity, GUIState::EditFlags, GUIState::EditShading, GUIState::EditSurfShading,
				GUIState::EditAlpha)) {
				resetEditText(isVertInf);
			}
		}
	});
	bool debug = m_arguments->isInternalDebug();
	//Custom interaction manager events/callbacks for key input.
	interaction->OnKeyEvent([interaction, isVertInf, debug](key_pair pair) {
		CScn* scn = SCNEdit::getSCN();
		//Update UV mode and scale
		if (interaction->getKeyState(make_pair(KEY_KEY_F, KeyAugment::None))) 
			interaction->swapUVMode();
		
		if (interaction->getKeyState(make_pair(KEY_KEY_Q, KeyAugment::None))) {
			if (interaction->getUVScalar() > 0.0f) 
				interaction->setUVScalar(interaction->getUVScalar() / 2.0f);

		}
		if (interaction->getKeyState(make_pair(KEY_KEY_E, KeyAugment::None))) {
			if (interaction->getUVScalar() < 1.0f) 
				interaction->setUVScalar(interaction->getUVScalar() * 2.0f);
		}
		// help menu oem_2 = ?
		if (interaction->getKeyState(make_pair(KEY_OEM_2, KeyAugment::None))) {
			if (interaction->guiStateLayer().swap(GUIState::Help, GUIState::Hover, GUIState::Default)){ 
				interaction->setCursorMode(true);
				interaction->resetKeyState();
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_P, KeyAugment::AnyKey))) {
			printSections(isVertInf);
		}

		if (interaction->getKeyState(make_pair(KEY_OEM_3, KeyAugment::None))) {
			if (debug) {
				if (interaction->guiStateLayer().swap(GUIState::Debug, GUIState::Hover, GUIState::Default)) {
					interaction->setCursorMode(true);
					interaction->resetKeyState();
				}
			}
		}


		if (interaction->getKeyState(make_pair(KEY_KEY_E, KeyAugment::Ctrl))) {
			if (interaction->selTypeLayer().get() != SelectedType::Entity) return;

			CScn* scn = SCNEdit::getSCN();

			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags,
				GUIState::EditAlpha, GUIState::EditShading, GUIState::EditSurfShading))
				resetEditText(isVertInf);
			

			if (interaction->guiStateLayer().swap(GUIState::EditEntity, GUIState::Default)) {
				interaction->setCursorMode(true);
				interaction->resetKeyState();
			
				int entindx = interaction->getEntityISelected();
				CScnEnt* ent = scn->getEnt(entindx);
				std::string valueDisplay;
				std::string keyDisplay;

				bool first = true;

				for (u32 i = 0; i < ent->n_fields; i++) {
					if (str_equiv(ent->fields[i].key, "Classname") ||
						str_equiv(ent->fields[i].key, "Origin")) {
						continue;
					}

					if (!first) {
						keyDisplay += ", ";
						valueDisplay += ", ";
					}

					keyDisplay += ent->fields[i].key;
					valueDisplay += ent->fields[i].value;

					first = false;
				}

				core::array<std::pair<irr::EKEY_CODE, int>> allowedKeys;
				CInteractionManager::getAlphaNumeric(allowedKeys);
				allowedKeys.push_back(std::make_pair(KEY_COMMA, KeyAugment::AnyKey));
				allowedKeys.push_back(std::make_pair(KEY_SPACE, KeyAugment::AnyKey));
				allowedKeys.push_back(std::make_pair(KEY_PERIOD, KeyAugment::None));
				allowedKeys.push_back(std::make_pair(KEY_MINUS, KeyAugment::AnyKey));

				m_textSection1->setText(format("Read-only entity keys: {}",keyDisplay).c_str());
				
				CInteractionManager::activateText(m_textSection2, allowedKeys,
					"Edit entity values here: ",valueDisplay, 256);

			}
			else if (interaction->guiStateLayer().find(GUIState::EditEntity)) {
				interaction->setCursorMode(false);
				resetEditText(isVertInf);
			}
		}

		if (!interaction->selTypeLayer().find(SelectedType::Solid, SelectedType::SolidExtra)) 
			return;


		//__________________SOLIDS ONLY AFTER THIS____________________

		//Open texture popup if t is pressed.
		if (interaction->findKeyState({ make_pair(KEY_KEY_T, KeyAugment::None), 
			make_pair(KEY_KEY_T, KeyAugment::Shift) })) {

			if (interaction->guiStateLayer().swap(GUIState::OpenTexture, GUIState::Hover, GUIState::Default)) {
				getApplication()->getDevice()->getCursorControl()->setVisible(true);
				interaction->resetKeyState();
				IGFD::FileDialogConfig config;
				config.path = "./textures";
				ImGuiFileDialog::Instance()->OpenDialog("ChooseTEXT", "Open Textures", ".tga,.bmp,.png", config);
			}
		}
		core::array<std::pair<irr::EKEY_CODE, int>> numerickeys;
		CInteractionManager::getNumeric(numerickeys);


		//Proccess Alpha,Shading,Flag editing. 
		if (interaction->getKeyState(make_pair(KEY_KEY_F, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditAlpha, 
				GUIState::EditShading, GUIState::EditSurfShading, GUIState::EditEntity))
				resetEditText(isVertInf);
			if (interaction->guiStateLayer().swap(GUIState::EditFlags, GUIState::Default)) {
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				CInteractionManager::activateText(m_textSection1, numerickeys, "Set Flag 1 (0 - 999): ", 3);
				CInteractionManager::activateText(m_textSection2, numerickeys, "Set Flag 2 (0 - 999): ", 3);
			}
			else if (interaction->guiStateLayer().find(GUIState::EditFlags)) {
				interaction->setCursorMode(false);
				resetEditText(isVertInf);
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_T, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags, 
				GUIState::EditShading,GUIState::EditSurfShading, GUIState::EditEntity))
				resetEditText(isVertInf);
			
			
			if (interaction->guiStateLayer().swap(GUIState::EditAlpha, GUIState::Default)) {
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				CInteractionManager::activateText(m_textSection1, numerickeys, "Set Alpha (0 - 255): ", 3);
				m_textSection2->setText("");
			}
			else if (interaction->guiStateLayer().find(GUIState::EditAlpha)) {
				interaction->setCursorMode(false);
				resetEditText(isVertInf);
			}
		}
		if (pair.second == KeyAugment::CtrlShift) {
			resetEditText(isVertInf);
		}
		if (pair.first == KEY_TAB) {
			resetEditText(isVertInf);
		}

		if (interaction->getKeyState(make_pair(KEY_KEY_C, KeyAugment::None))) {
			solidSelect_t surfdata = interaction->getSurfISelected();

			CScnSolid* solid = scn->getSolid(surfdata.solididx);
			scnSurf_t surf = solid->surfs[surfdata.surfsel];
			if (surf.shading && surf.hasVertexColors) {
				if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags,
					GUIState::EditAlpha, GUIState::EditShading, GUIState::EditEntity))
					resetEditText(isVertInf);


				if (interaction->guiStateLayer().swap(GUIState::EditSurfShading, GUIState::Default)) {
					interaction->setCursorMode(true);
					interaction->resetKeyState();
					numerickeys.push_back(std::make_pair(KEY_SPACE, KeyAugment::AnyKey));
					CInteractionManager::activateText(m_textSection1, numerickeys, "Set Surf Color (0-255) (0-255) (0-255) (0-255): ", 15);
					numerickeys.erase(numerickeys.size() - 1);
					m_textSection2->setText("");
				}
				else if (interaction->guiStateLayer().find(GUIState::EditSurfShading)) {
					interaction->setCursorMode(false);
					resetEditText(isVertInf);
				}
			}
		}
		//Should only do this is vert is 1.
		if (interaction->getKeyState(make_pair(KEY_KEY_C, KeyAugment::Ctrl))) {
			CScn* scn = SCNEdit::getSCN();
			solidSelect_t surfdata = interaction->getSurfISelected();
			
			CScnSolid* solid = scn->getSolid(surfdata.solididx);
			scnSurf_t surf = solid->surfs[surfdata.surfsel];
			if (surf.shading && surf.hasVertexColors) {
				if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags,
					GUIState::EditAlpha, GUIState::EditSurfShading, GUIState::EditEntity))
					resetEditText(isVertInf);


				if (interaction->guiStateLayer().swap(GUIState::EditShading, GUIState::Default)) {
					interaction->setCursorMode(true);
					interaction->resetKeyState();
					numerickeys.push_back(std::make_pair(KEY_SPACE, KeyAugment::AnyKey));
					CInteractionManager::activateText(m_textSection1, numerickeys, 
						"Set Color (0-255) (0-255) (0-255) (0-255): ", 15);
					numerickeys.erase(numerickeys.size() - 1);
					m_textSection2->setText("");
				}
				else if (interaction->guiStateLayer().find(GUIState::EditShading)) {
					interaction->setCursorMode(false);
					resetEditText(isVertInf);
				}
			}
		}

	});

	interaction->OnUIUpdateEvent([interaction, isVertInf]() {
		updateSections(isVertInf);
	});

}


void CViewGui::resetEditText(bool isVertInf) {
	isSection1Set = false; 
	isSection2Set = false;
	CInteractionManager* interaction = CInteractionManager::getInstance();
	interaction->guiStateLayer().set(GUIState::Default);
	CInteractionManager::resetText(m_textSection1,512);
	CInteractionManager::resetText(m_textSection2, 512);
	updateSections(isVertInf);
}

void CViewGui::onDestroy(){
	CInteractionManager::releaseInstance();
}

void CViewGui::onData(){}

void CViewGui::onUpdate()
{
	if (CImguiManager::getInstance()) CImguiManager::getInstance()->onNewFrame();
}

void CViewGui::onRender()
{
	// imgui render
	if (CImguiManager::getInstance()) {
		onGUI();
		CImguiManager::getInstance()->onRender();
	}
}


void CViewGui::onPostRender(){}

void CViewGui::onGUI()
{
	bool open = false;

	ImGuiWindowFlags window_flags = 2;
	float size = getApplication()->getDriver()->getScreenSize().Width;
	ImGui::SetNextWindowPos(ImVec2(size - 160, 15), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(150, 290 - (21 * m_missingItems)), ImGuiCond_Always);

	if (!ImGui::Begin("SCN Options", &open, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	// BEGIN WINDOW
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	
	ImGui::Text("FPS: %.1f", 1000.0f/getApplication()->getTimeStep());
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();

	bool lightmaps = false;
	CScn* scn = SCNEdit::getSCN();

	if (scn) lightmaps = scn->getLightmap()->hasLightmaps();
	ImGui::SeparatorText("Scn Visibility");
	 CInteractionManager::ToggleButton("Solids", &gui->vis_scn, ImGuiKey_1);
	

	CInteractionManager::ToggleButton("Doors/Others", &gui->vis_doors, ImGuiKey_2);
	CInteractionManager::ToggleButton("Portals", &gui->vis_portals, ImGuiKey_3);
	CInteractionManager::ToggleButton("Bounding Boxes", &gui->vis_bb, ImGuiKey_4);
	CInteractionManager::ToggleButton("Entities", &gui->vis_entities, ImGuiKey_5);
	if(lightmaps) CInteractionManager::ToggleButton("Lightmap", &gui->vis_lightmaps, ImGuiKey_6);
	if(lightmaps) ImGui::SeparatorText("Save Settings");
	if(lightmaps) CInteractionManager::ToggleButton("Scrape Lightmap", &gui->scrape_lightmaps,ImGuiKey_7);
	if (m_arguments->isInternalDebug()) 
		ImGui::SeparatorText("Debug");

	if (m_arguments->isInternalDebug()) {
		if (interaction->guiStateLayer().find(GUIState::Debug)) {
			if (ImGui::Button("Close Debug", ImVec2(130, 20))) 
				interaction->guiStateLayer().set(GUIState::Default);
			
		}
		else {
			if (ImGui::Button("Open Debug", ImVec2(130, 20))) 
				interaction->guiStateLayer().swap(GUIState::Debug, GUIState::Hover, GUIState::Default);
			
		}
		
	}
	ImGui::End();
	
	if (interaction->guiStateLayer().find(GUIState::Hover))
		drawTooltip();
	else if (interaction->guiStateLayer().find(GUIState::Debug))
		openLogger();
	else if (interaction->guiStateLayer().find(GUIState::ScrapeLightSave))
		scrapeLight();
	else if (interaction->guiStateLayer().find(GUIState::Save))	
		saveFile();
	else if (interaction->guiStateLayer().find(GUIState::Help))	
		helpDialog();
	else if (interaction->guiStateLayer().find(GUIState::CloseFile)) 
		closeFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenScn)) 
		openFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenTexture)) 
		openTextures();
	else if (interaction->guiStateLayer().find(GUIState::Quit))
		quit();

	//Bottom bar gui
	if (scn) {
		core::vector3df pos = camera->getPosition();
		core::vector3df look = camera->getLookVector();
		s16 cellindx = scn->getSolid(0)->getCellAtPos(pos);

		const char* uvmode = interaction->findUVMode(UVMode::Move) ? "Move" : "Resize";
		

		if (cellindx >= 0) {
			scnRawCell_t cell = scn->getSolid(0)->rawcells[cellindx];
			m_textSection3b->setText(std::format("UV {}. Grid: {:.3f} | Cell: {}", uvmode, interaction->getUVScalar(), cell.name).c_str());
		}
		else 
			m_textSection3b->setText(std::format("UV {}. Grid: {:.3f} ", uvmode, interaction->getUVScalar()).c_str());
		
		std::string lookDir;

		if (look.X >= 0.4f)       lookDir += " +X ";
		else if (look.X <= -0.4f) lookDir += " -X ";

		if (look.Y >= 0.4f)       lookDir += " +Y ";
		else if (look.Y <= -0.4f) lookDir += " -Y ";

		if (look.Z >= 0.4f)       lookDir += " +Z ";
		else if (look.Z <= -0.4f) lookDir += " -Z ";

		m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f} | Looking {}", pos.X, pos.Y, pos.Z, lookDir).c_str());
	}
}
void CViewGui::drawTooltip() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	core::vector2df mouse = interaction->getMouse();
	ImGui::SetNextWindowPos(ImVec2(mouse.X, 35), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(230, 40), ImGuiCond_Always);
	bool tmp = false;
	if (ImGui::Begin("Tooltip", &tmp, ImGuiWindowFlags_NoDecoration))
		ImGui::Text(m_tooltip.c_str());
	ImGui::End();
}


void CViewGui::openLogger() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
    bool found = interaction->guiStateLayer().find(GUIState::Debug);  
    bool * open = &found; // Assign the address of the found variable to x

	ImGui::SetNextWindowPos(ImVec2(100, 50), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Once);
	interaction->drawLog("Log Output", open);
}
void CViewGui::scrapeLight() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	if (gui->scrape_lightmaps) {
		int selected = m_msgbox.Draw();
		m_msgbox.Open();
		if (selected == 1) {
			interaction->guiStateLayer().set(GUIState::Save);
			savePressed = true;
		}
		else if (selected > 0)
			interaction->guiStateLayer().set(GUIState::Default);
	}
}

void CViewGui::saveFile() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	if (savePressed) {
		if (!SCNEdit::saveSCN()) {
			interaction->guiStateLayer().set(GUIState::Default);
			return;
		}
		savePressed = false;
	}
	static const char* buttons[] = { "Ok", NULL };
	m_msgbox.Init("Saved!", NULL, "Successfully Saved Changes!", buttons, false);
	int finish = m_msgbox.Draw();
	if (finish > 0)
		interaction->guiStateLayer().set(GUIState::Default);
	else
		m_msgbox.Open();

}
void CViewGui::helpDialog() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	static const char* buttons[] = { "Ok", NULL };
	m_msgbox.Init("Controls and help", NULL, 
		"~: Open internal debug (if enabled)\n"
		"Scroll wheel: increase/decrease camera rotation speed\n"
		"P/CTRL+P/SHIFT+P (ANY SELECTED): Print to console\n\n"

		"H (ANY SELECTED): Hides element from editor\n"
		"SHIFT+H (SOLID SELECTED): Hides shared and selected surfaces\n"
		"CTRL+H: Unhides hidden elements\n\n"

		"T (SOLID SELECTED): Change texture using file dialog.\n\n"

		"Ctrl+T (SOLID SELECTED): Change alpha of selected surface\n"
		"CTRL+F (SOLID SELECTED): Change flags of selected surface\n"
		"CTRL+L (SOLID SELECTED): Change shading of selected surface\n"

		"C (SOLID SELECTED & HAS SHADING): Change shading of surface\n"
		"CTRL+C (VERT SELECT & HAS SHADING): Change shading of vertex\n\n"

		
		"F: Change face uv mode\n"
		"Q: Decrease UV Scalar\n"
		"E: Increase UV Scalar\n\n"

		"TAB (SOLID SELECTED): Node details about surface \n"
		"CTRL + SHIFT(SOLID SELECTED): View geometry entity for surface\n\n"

		"X or V (SOLID SELECTED): Flip uv horizontal/vertical\n"
		"R (SOLID SELECTED): Reset uv\n"
		"UP (SOLID SELECTED): Change UV v+\n"
		"DOWN (SOLID SELECTED): Change UV v -\n"
		"LEFT (SOLID SELECTED): Change UV u+\n"
		"RIGHT (SOLID SELECTED): Change UV v-\n\n"

		"Ctrl+E (ENTITY SELECTED): Change non-protected entity fields\n"

		"CTRL *or* SHIFT (SOLID SELECTED): Select vertex to move\n\n"

		"CTRL+R (SOLID or ENTITY SELECTED): Reset position\n"
		"SHIFT+UP (SOLID or ENTITY SELECTED): Move X+\n"
		"SHIFT+DOWN (SOLID or ENTITY SELECTED): Move X-\n"
		"SHIFT+LEFT (SOLID or ENTITY SELECTED): Move Z+\n"
		"SHIFT+RIGHT (SOLID or ENTITY SELECTED): Move Z-\n"
		"CTRL+UP (SOLID or ENTITY SELECTED): Move Y+\n"
		"CTRL+DOWN (SOLID/ENTITY SELECTED): Move Y-\n", buttons, false);

	float size = getApplication()->getDriver()->getScreenSize().Width;
	float sizeH = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(size / 2 - 260, sizeH / 2 - 300), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(475, 595), ImGuiCond_Always);
	int selected = m_msgbox.Draw();
	

	if (selected > 0) {
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->setCursorMode(false);	
	}
	else
		m_msgbox.Open();
	
}


void CViewGui::closeFile() {

	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CGameObject* obj = zone->searchObject(L"body");
	CGameObject* skybox = zone->searchObject(L"skybox");
	if (obj) {
		SCNEdit::closeScnFile();
		obj->remove();
		skybox->setVisible(false);
	}
	interaction->guiStateLayer().set(GUIState::Default);

}


void CViewGui::openFile() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	float w = getApplication()->getDriver()->getScreenSize().Width;
	float h = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(15, 60), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(w / 1.5, h / 1.5), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->Display("ChooseSCN")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

			m_arguments->setSCNPath(filePathName.c_str());
			m_arguments->setScnLoaded(false);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
		interaction->guiStateLayer().set(GUIState::Default);

		if (!m_arguments->getScnLoaded()) {
			//interaction->getLog()->clear();
			CViewManager::getInstance()->getLayer(1)->destroyAllView();
			CViewManager::getInstance()->getLayer(0)->changeView<CViewInit>(m_arguments);

		}
	}
}

void CViewGui::openTextures() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	CGameObject* selected = interaction->getSelectObj();
	float w = getApplication()->getDriver()->getScreenSize().Width;
	float h = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(15, 60), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(w / 1.5, h / 1.5), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->Display("ChooseTEXT")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

			if (selected)
				selected->getComponent<CScnMeshComponent>()->setTexture(SCNEdit::getSCN(), filePathName.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
		interaction->setCursorMode(false);

		interaction->guiStateLayer().set(GUIState::Default);
	}
}
void CViewGui::quit() {
	//Should probably generalize this in SCNEdit as there two instances of this function.
	SCNEdit::proccessQuit();
	delete m_arguments;
	std::exit(0);
}



void CViewGui::updateSections(bool inVertInf) {
	InfoText info = getSections(inVertInf);
	m_textSection1->setText(info.section1.c_str());
	m_textSection2->setText(info.section2.c_str());
	m_textSection3a->setText(info.section3.c_str());
}

void CViewGui::printSections(bool inVertInf) {
	InfoText info = getSections(inVertInf);
	os::Printer::log(info.section1.c_str(),ELL_DEBUG);
	if(!info.section2.empty()) os::Printer::log(info.section2.c_str(), ELL_DEBUG);
	if (!info.section3.empty()) os::Printer::log(info.section3.c_str(), ELL_DEBUG);
}


/// Splits the data into solid info, portal info, entity info sections and updates it.
InfoText CViewGui::getSections(bool inVertInf) {
	InfoText info;
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	if (interaction->guiStateLayer().find(GUIState::Default)) {
		if (interaction->selTypeLayer().find(SelectedType::SolidExtra, SelectedType::Solid)) 
			return getSolidInfo(inVertInf);
		if (interaction->selTypeLayer().get() == SelectedType::Portal) 
			return getPortalInfo();
		if (interaction->selTypeLayer().get() == SelectedType::Entity)
			return getEntityInfo();
	}
	return info;
}

InfoText CViewGui::getSolidInfo(bool inVertInf) {
	InfoText info;
	
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CScn* scn = SCNEdit::getSCN();
	solidSelect_t surfdata = interaction->getSurfISelected();

	if (surfdata.solididx == -1 || surfdata.surfsel == -1) 
		return info;

	CScnSolid* solid = scn->getSolid(surfdata.solididx);
	scnSurf_t surf = solid->surfs[surfdata.surfsel];
	scnSurfParamFrame_t params = solid->paramFrames[surfdata.surfsel];
	scnPlane_t plane = solid->planes[surf.planeidx];

	if (!inVertInf || interaction->keyAugLayer().get() == KeyAugment::None) {
		info.section1 = format("surf[{}]: | texture = {} | flags = {} {} | alpha = {} | lighmap size = {}x{} "
			"| texture size = {}x{} |",
			surfdata.surfsel, surf.texture, surf.flag1, surf.flag2, surf.alpha, surf.lmsize_h, surf.lmsize_v, 
			surf.height, surf.width);

		info.section2 = format("face vert idx = from {} to {} | plane idx = {} | normal = ({:.1f}, {:.1f},"
			" {:.1f}), d={:.1f} | has shading = {} |",
			surf.faceidxstart, surf.faceidxstart + surf.faceidxlen, surf.planeidx, plane.a, plane.b, 
			plane.c, plane.d, surf.hasVertexColors);
	}
	else if (interaction->getSelectObj() && interaction->hasMoveableVert()) {
		indexedVec3df_t vert = interaction->getMoveableVert();
		CScnSolid* solidv = scn->getSolid(vert.solididx);
		u32 uvidx = solidv->uvidxs[vert.faceidx];
		core::vector2df uv = solidv->uvpos[uvidx];
		u32 vertidx = solidv->vertidxs[vert.faceidx];
		scnSurf_t surfv = solidv->surfs[vert.surfidx];

		if (surfv.shading&& surfv.hasVertexColors && vert.bShared && vert.sharesWith.size()>0) {
			u8* ps = surfv.shading + vert.surf_vertidx * 4;
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | shading color(rgba) "
				"= {} {} {} {} | shared surfs = {} | uv idx = {} |",
				vert.faceidx, vertidx, vert.surf_vertidx, ps[0], ps[1], ps[2], ps[3], str_join(vert.sharesWith), uvidx);
		}
		else if (surfv.shading && surfv.hasVertexColors) {
			u8* ps = surfv.shading + vert.surf_vertidx * 4;
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | shading color(rgba) "
				"= {} {} {} {} | no shared | uv idx = {} | ",
				vert.faceidx, vertidx, vert.surf_vertidx, ps[0], ps[1], ps[2], ps[3], uvidx);
		}
		else if (vert.bShared && vert.sharesWith.size() > 0) {
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | no vertex color | "
				"shared surfs = {} | uv idx = {} |",
				vert.faceidx, vertidx, vert.surf_vertidx, str_join(vert.sharesWith), uvidx);
		}
		else {
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | no vertex color | "
				"no shared | uv idx = {} |",
				vert.faceidx, vertidx, vert.surf_vertidx, uvidx);
		}
		
	


		if (scn->getLightmap()->hasLightmaps()) {

			core::vector3di atlas = scn->getLightmap()->getAtlasPos(vert.solididx, vert.surfidx);
			core::vector3df delta = vert.pos - params.origin;

			float* mult = scn->getLightmap()->getMults(vert.solididx, vert.surfidx);
			info.section2 = format("uv_pos = {} | lm mult w={:.3f} h={:.3f} x={:.3f} y={:.3f} | solidref_index = {} | ",
				vec2_to_str(uv, 3), mult[0], mult[1], mult[2], mult[3], surf.solidref_index);
		}
		else {
			info.section2 = format("uv_pos = {} | solidref_index = {} | ", vec2_to_str(uv, 3), surf.solidref_index);
		}
	}
	if (interaction->getKeyState(make_pair(KEY_TAB,KeyAugment::None))) {
		scnPlane_t* surfPlane = &solid->planes[surf.planeidx];
	
		core::vector3df center = interaction->getVertCenter();
		// Create a normal vector from the plane constants
		core::vector3df normal(surfPlane->a, surfPlane->b, surfPlane->c);

		// Nudge the point slightly BACKWARDS (into the material)
		// Use a small epsilon like 0.1 or 0.01
		core::vector3df pushIn = center - (normal * 0.01f);

		s16 nodeindx = solid->tree->findNodePos(pushIn);
		scnNode_t node = solid->tree->nodes[nodeindx];
		info.section1 = format("node[{}]: | 'edge' 1 = {} | 'edge' 2= {} | parent= {} | area/portal visiblity flag={:d} | ",nodeindx, node.node1, node.node2, node.nodep, node.area);
		if (node.cell >= 0) {
			scnRawCell_t cell = solid->rawcells[node.cell];
			info.section1 = format("node 1 = {} | node 2= {} | parent= {} | cell=#{}, name=\"{}\" | area/portal visiblity flag={:d} |", node.node1, node.node2, node.nodep,
				node.cell,cell.name, node.area);
		}
		
		info.section2 = format("material = {} | material flags = {} | no special geom | unk = {} |", 
			CScn::getMaterialName(node.material), CScn::getMaterialFlags(node.material), node.visframe);
		if (node.specialGeomIdx>=0&&node.specialGeomIdx<solid->n_names) {
			if (!solid->names[node.specialGeomIdx].empty()) {
				info.section2 = format("material ={} | material flags ={} | special geom = {} | unk = {} |",
					CScn::getMaterialName(node.material), CScn::getMaterialFlags(node.material), solid->names[node.specialGeomIdx], node.visframe);
			}
		}
	}
	if (interaction->getKeyAugment(KeyAugment::CtrlShift)) {
		if (surf.solidref_index >= 1) {
			CScnEnt* ent = scn->getSolidRef(surf.solidref_index);
			int split = ent->n_fields / 2;
			std::string sec1;
			std::string sec2;

			for (int i = 0; i < ent->n_fields; i++) {
				if (i > split && split >= 4) {
					sec2 += format("{}:({}) | ", ent->fields[i].key, ent->fields[i].value);
				}
				else sec1 += format("{}:({}) | ", ent->fields[i].key, ent->fields[i].value);
			}
			info.section1 = sec1;
			info.section2 = sec2;
		}
	}


	if (interaction->getSelectObj() && interaction->hasMoveableVert()) {
		CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
		const char* mode = interaction->keyAugLayer().get() == KeyAugment::None ? "Hover" : "Select";
		indexedVec3df_t vert = interaction->getMoveableVert();
	
		info.section3 = format("Surfs: ({}), Shared: ({}) {} V: {}", comp->selsurfs.size(),
			comp->sharedsurfs.size(), mode, vec3_to_str(vert.pos, 0));
	
	}

	return info;
}

InfoText CViewGui::getPortalInfo() {
	InfoText info;
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	portalSelect_t portaldata = interaction->getPortalISelected();
	CScnSolid* solid = scn->getSolid(0);
	scnRawCell_t cell = solid->rawcells[portaldata.cellidx];
	scnPortal_t portal = solid->rawcells[portaldata.cellidx].portals[portaldata.portalidx];
	scnRawCell_t nextcell = solid->rawcells[portal.nextcell];
	scnPlane_t plane = portal.plane;

	info.section1 = format(" | portal id: {} name: \"{}\" | cell #{}, name: \"{}\" | nextcell id: {}, name: \"{}\" "
		"| normal = ({:.1f}, {:.1f}, {:.1f}), d = {:.1f} | unknown= {} {} |",
		portaldata.portalidx, portal.name, portaldata.cellidx, cell.name, portal.nextcell, 
		nextcell.name, plane.a, plane.b, plane.c, plane.d, portal.flag1, portal.flag2);
	return info;
}

InfoText CViewGui::getEntityInfo() {
	InfoText info;
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	int entindx = interaction->getEntityISelected();
	CScnEnt* ent = scn->getEnt(entindx);
	int split = ent->n_fields / 2;

	for (int i = 0; i < ent->n_fields; i++) {
		if (i > split  && split >= 4) {
			info.section2 += format("{}:({}) | ", ent->fields[i].key, ent->fields[i].value);
		}
		else info.section1 += format("{}:({}) | ", ent->fields[i].key, ent->fields[i].value);
	}
	
	return info;
}
