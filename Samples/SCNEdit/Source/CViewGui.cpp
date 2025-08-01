#include "pch.h"
#include "Header/CViewGui.h"

#include "Graphics2D/CGUIImporter.h"
#include "Header/Managers/CContext.h"
#include "pfd/pfd.h" 
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
	float scale = min(m_arguments->getCurrentRes().Width / m_arguments->getDesktopRes().Width, 
		m_arguments->getCurrentRes().Height / m_arguments->getDesktopRes().Height);
	m_canvas->applyScaleGUI(std::clamp(scale, 0.0f, 0.75f));
	m_canvas->setSortDepth(0);

	//Extract elements from gui.
	m_uiContainer = gui->addComponent<UI::CUIContainer>();

	m_openButton = addButton("Canvas/Open_Btn");
	addTooltip(m_openButton, "Open a new .scn mission/map.", "");

	m_closeButton = addButton("Canvas/Close_Btn");
	addTooltip(m_closeButton, "Close current .scn map.", "No map found so does nothing.");
	
	m_exportButton = addButton("Canvas/Export_Btn");
	addTooltip(m_exportButton, "Export obj,map mtl, 3ds files\nfor use in other programs.", 
		"Nothing to export,\nplease open a .scn map.");
	
	m_saveButton = addButton("Canvas/Save_Btn");
	addTooltip(m_saveButton, "Save over current .scn file.", "Nothing to save,\nplease open a .scn map");
	
	m_quitButton  = addButton("Canvas/Quit_Btn");
	m_textSection1 = addTextbox("Canvas/BottomBar/Section1_Rect/Section1_Input");

	m_textSection2  = addTextbox("Canvas/BottomBar/Section2_Rect/Section2_Input");
	m_textSection3a = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect1/Section3_Input1");
	m_textSection3b = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect2/Section3_Input2");
	m_textSection3c = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect3/Section3_Input3");


	CGUIElement* el = m_canvas->getGUIByPath("Canvas/BottomBar");
	CGUIElement* el2 = m_canvas->getGUIByPath("Canvas/BottomBar/SectionExtra_Layout");

	updateUVInfo();
	RECT workArea;
	//Try to get were the exact bottom of the screen is and shift bottom bar to fit. said area.
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	int taskbarHeight = m_arguments->getDesktopRes().Height - workArea.bottom;
	float diff = m_arguments->getDesktopRes().Height - m_arguments->getRes().Height;
	float diffW = m_arguments->getDesktopRes().Width - m_arguments->getRes().Width;
	el->setWidth(m_arguments->getCurrentRes().Width);

	////TODO: REDO THIS
	if (m_arguments->isFullscreen() || diff >= taskbarHeight * 2 + 10 ||
		(m_arguments->getDesktopRes().Width-25 > m_arguments->getRes().Width && diff <= taskbarHeight-8)) {
		el->setHeight(150);
	}
	if (!m_arguments->isFullscreen() && 
		m_arguments->getDesktopRes().Width-25 > m_arguments->getRes().Width && diff <= taskbarHeight/2) {
		el2->setHeight(140);
		el->setHeight(280);
	}
	else if (!m_arguments->isFullscreen()&&
		m_arguments->getDesktopRes().Width-25 > m_arguments->getRes().Width &&diff <= taskbarHeight + 10) {
		el2->setHeight(100);
		el->setHeight(250);
	}
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
			interaction->setBlockCursor(true);
		}
	};

	m_closeButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		if (SCNEdit::getSCN()) 
			interaction->guiStateLayer().set(GUIState::CloseFile);
	};

	m_exportButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		if (SCNEdit::getSCN()) 
			SCNEdit::exportSCN();
	};

	m_saveButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		//Save data will initalize a warning msgbox before save.
		if (SCNEdit::getSCN()) {	
			if (interaction->guiStateLayer().swap(GUIState::Save, GUIState::Default, GUIState::Hover)) {
			}
			else {
				interaction->guiStateLayer().set(GUIState::Default);
			}
		}
	};
	bool isVertInf = m_arguments->isVertInfo();
	//Section 1 and 2 text edit events
	m_textSection1->OnTextSet = [interaction, isVertInf](UI::CUIBase* base) {
		CScn* scn = SCNEdit::getSCN();
		if (interaction->guiStateLayer().find(GUIState::EditFlags,GUIState::EditAlpha,GUIState::EditHasShading)) {
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
				else if (interaction->guiStateLayer().get() == GUIState::EditHasShading)
					surfi->hasVertexColors = num;
				else
					surfi->flag1 = num;
			}

		}
		else if (interaction->guiStateLayer().find(GUIState::EditShading)) {
			std::string msg = str_split(m_textSection1->getText(), ": ")[1];
			core::array<std::string> split = str_split(msg.c_str(), " ");
			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
			if (split.size() == 4) {
				indexedVec3df_t vert = interaction->getMoveableVert();
				CScnSolid* solid = scn->getSolid(vert.solididx);
				scnSurf_t* surfi = &solid->surfs[vert.surfidx];
				if (surfi->shading) {
					u8* ps = surfi->shading + vert.surf_vertidx * 4;
					for (int i = 0; i < 4; i++) {
						u8 num = std::atoi(split[i].c_str());
						if (num > 255) num = 255;
						surfi->shading[i] = num;
					}
				}
			}
		}
		else if(interaction->guiStateLayer().find(GUIState::EditAll)) {
			core::array<std::string> msg = str_split(m_textSection1->getText(), ",");
			//This will need some proccessing so probably do in another function.
			
		}
		isSection1Set = true;
		if (interaction->guiStateLayer().find(GUIState::EditFlags, GUIState::EditAll) && isSection1Set && isSection2Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}
	
		if (interaction->guiStateLayer().find(GUIState::EditAlpha, GUIState::EditShading, 
			GUIState::EditHasShading) && isSection1Set) {
			resetEditText(isVertInf);
			interaction->setCursorMode(false);
		}
		
	};

	m_textSection2->OnTextSet = [interaction](UI::CUIBase* base) {
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
		else if (interaction->guiStateLayer().find(GUIState::EditAll)) {
			core::array<std::string> msg = str_split(m_textSection1->getText(), ",");
			//This will need some proccessing so probably do in another function.
			
		}
		isSection2Set = true;
		if (isSection1Set && isSection2Set) {
			resetEditText();
			interaction->setCursorMode(false);
		}
	};
	interaction->OnCursorModeEvent([interaction](bool state, bool isRightClick) {
		if (!state && isRightClick) {
			interaction->guiStateLayer().swap(GUIState::Default,
				GUIState::EditAll, GUIState::EditFlags, GUIState::EditShading, GUIState::EditHasShading, 
				GUIState::EditAlpha);
			resetEditText();
		}
	});
	
	//Custom interaction manager events/callbacks
	interaction->OnKeyEvent([interaction, isVertInf](key_pair pair) {

		//Update positition of entity.
		if ((interaction->getKeyState(make_pair(pair.first, pair.second)) &&
			pair.first >= KEY_LEFT && pair.first <= KEY_DOWN) ||
			interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::None))) {
			if (interaction->selTypeLayer().get() == SelectedType::Entity) 
				updateSections(isVertInf);
		}
		//Update UV mode and scale
		if (interaction->getKeyState(make_pair(KEY_KEY_F, KeyAugment::None))) {
			interaction->swapUVMode();
			updateUVInfo();
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_Q, KeyAugment::None))) {
			if (interaction->getUVScalar() > 0.0f) {
				interaction->setUVScalar(interaction->getUVScalar() / 2.0f);
				updateUVInfo();
			}

		}
		if (interaction->getKeyState(make_pair(KEY_KEY_E, KeyAugment::None))) {
			if (interaction->getUVScalar() < 1.0f) {
				interaction->setUVScalar(interaction->getUVScalar() * 2.0f);
				updateUVInfo();
			}
		}
		// help menu oem_2 = ?
		if (interaction->getKeyState(make_pair(KEY_OEM_2, KeyAugment::None))) {
			if (interaction->guiStateLayer().swap(GUIState::Help, GUIState::Hover, GUIState::Default)){ 
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				interaction->setBlockCursor(true);
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_P, KeyAugment::AnyKey))) {
			printSections(isVertInf);
		}

		if (!interaction->selTypeLayer().find(SelectedType::Solid, SelectedType::SolidExtra)) 
			// The next statments require solid.
			return;

		//Open texture popup if t is pressed.
		if (interaction->findKeyState({ make_pair(KEY_KEY_T, KeyAugment::None), 
			make_pair(KEY_KEY_T, KeyAugment::Shift) })) {

			if (interaction->guiStateLayer().swap(GUIState::OpenTexture, GUIState::Hover, GUIState::Default)) {
				getApplication()->getDevice()->getCursorControl()->setVisible(true);
				interaction->resetKeyState();
				interaction->setBlockCursor(true);
			}
		}
		core::array<std::pair<irr::EKEY_CODE, int>> numerickeys;
		CInteractionManager::getNumeric(numerickeys);


		//Proccess Alpha,Shading,Flag editing. TODO: EditAll
		if (interaction->getKeyState(make_pair(KEY_KEY_F, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditAlpha, 
				GUIState::EditShading, GUIState::EditHasShading, GUIState::EditAll))
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
				GUIState::EditShading,GUIState::EditHasShading, GUIState::EditAll))
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
		if (interaction->getKeyState(make_pair(KEY_KEY_L, KeyAugment::None))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags,
				GUIState::EditAlpha, GUIState::EditShading, GUIState::EditAll))
				resetEditText(isVertInf);


			if (interaction->guiStateLayer().swap(GUIState::EditHasShading, GUIState::Default)) {
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				CInteractionManager::activateText(m_textSection1, numerickeys, "Set Has Shading (0-1): ", 1);
				m_textSection2->setText("");
			}
			else if (interaction->guiStateLayer().find(GUIState::EditHasShading)) {
				interaction->setCursorMode(false);
				resetEditText(isVertInf);
			}
		}
		//Should only do this is vert is 1.
		if (interaction->getKeyState(make_pair(KEY_KEY_L, KeyAugment::Ctrl))) {
			CScn* scn = SCNEdit::getSCN();
			solidSelect_t surfdata = interaction->getSurfISelected();
			CScnSolid* solid = scn->getSolid(surfdata.solididx);
			scnSurf_t surf = solid->surfs[surfdata.surfsel];
			if (surf.shading) {
				if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags,
					GUIState::EditAlpha, GUIState::EditHasShading, GUIState::EditAll))
					resetEditText(isVertInf);


				if (interaction->guiStateLayer().swap(GUIState::EditShading, GUIState::Default)) {
					interaction->setCursorMode(true);
					interaction->resetKeyState();
					numerickeys.push_back(std::make_pair(KEY_SPACE, KeyAugment::AnyKey));
					CInteractionManager::activateText(m_textSection1, numerickeys, 
						"Set Color (0-255) (0-255) (0-255) (0-255): ", 15);
					numerickeys.erase(numerickeys.size() - 1);
				}
				else if (interaction->guiStateLayer().find(GUIState::EditShading)) {
					interaction->setCursorMode(false);
					resetEditText(isVertInf);
				}
			}
		}

	});


	interaction->OnCollisionEvent([interaction, isVertInf](core::triangle3df tri, core::vector3df intersection) {
		//Update section if you select a solid/entity/portal
		if (interaction->guiStateLayer().find(GUIState::Default)) {
			if (interaction->selTypeLayer().get() != SelectedType::Empty)
				updateSections(isVertInf);
			else
				m_textSection3a->setText("");
		}
	});

	interaction->OnVertexUpdateEvent([interaction, isVertInf]() {
		if (interaction->guiStateLayer().find(GUIState::Default)) {
			if (interaction->selTypeLayer().get() != SelectedType::Empty)
				//Update the solid vertex pos
				updateSections(isVertInf);
			else
				m_textSection3a->setText("");
		}
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
	ImGui::SetNextWindowPos(ImVec2(size - 160, 15), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(150, 285 - (21 * m_missingItems)), ImGuiCond_Once);

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
	ImGui::SeparatorText("Scn Visibility");
	 CInteractionManager::ToggleButton("Solids", &gui->vis_scn);
	
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	
	bool lightmaps = false;
	CScn* scn = SCNEdit::getSCN();

	if(scn) lightmaps = scn->getLightmap()->hasLightmaps();
	CInteractionManager::ToggleButton("Doors/Others", &gui->vis_doors);
	CInteractionManager::ToggleButton("Portals", &gui->vis_portals);
	CInteractionManager::ToggleButton("Bounding Boxes", &gui->vis_bb);
	CInteractionManager::ToggleButton("Entites", &gui->vis_entities);
	if(lightmaps) CInteractionManager::ToggleButton("Lightmap", &gui->vis_lightmaps);
	if(lightmaps) ImGui::SeparatorText("Save Settings");
	if(lightmaps) CInteractionManager::ToggleButton("Scrape Lightmap", &gui->scrape_lightmaps);
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
	
	if (interaction->guiStateLayer().find(GUIState::Hover))		    drawTooltip();
	else if (interaction->guiStateLayer().find(GUIState::Debug))	openLogger();
	else if (interaction->guiStateLayer().find(GUIState::Save))		saveFile();
	else if (interaction->guiStateLayer().find(GUIState::Help))		helpDialog();
	else if (interaction->guiStateLayer().find(GUIState::CloseFile))	closeFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenScn))		openFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenTexture)) openTextures();
	else if (interaction->guiStateLayer().find(GUIState::Quit))		quit();

	//Bottom bar gui
	if (scn) {
		core::vector3df pos = camera->getPosition();
		s16 cellindx = scn->getSolid(0)->getCellAtPos(pos);
		if (cellindx >= 0) {
			scnRawCell_t cell = scn->getSolid(0)->rawcells[cellindx];
			m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f} | Cell: {}", 
				pos.X, pos.Y, pos.Z, cell.name).c_str());
		}
		else 
			m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f}", pos.X, pos.Y, pos.Z).c_str());
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


void CViewGui::saveFile() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	guiSettings_t* gui = interaction->getGuiSettings();
	if (gui->scrape_lightmaps) {
		
		auto res = pfd::message::message("Save with Scraped Lightmaps", "Notice Lightmaps are Scraped."
			" Will save without lightmaps.").result();

		if (res == pfd::button::ok)
			SCNEdit::saveSCN();
		else
		
		interaction->guiStateLayer().set(GUIState::Default);

	}
	else {
		SCNEdit::saveSCN();
		interaction->guiStateLayer().set(GUIState::Default);

	}
}
void CViewGui::helpDialog() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	auto res= pfd::message::message("Controls and help", "H (ANY SELECTED): Hides element from editor\n"
		"SHIFT+H (SOLID SELECTED): Hides shared and selected surfaces\n"
		"CTRL+H: Unhides hidden elements\n\n"

		"P/CTRL+P/SHIFT+P (ANY SELECTED): Print selected details to console\n"

		"X (SOLID SELECTED): Flips selected texture horizontally\n"
		"V (SOLID SELECTED): Flips selected texture vertically\n\n"

		"T (SOLID SELECTED): Change texture using file dialog.\n\n"

		"Ctrl+T (SOLID SELECTED): Change alpha of selected surface\n"
		"CTRL+F (SOLID SELECTED): Change flags of selected surface\n"
		"CTRL+L (SOLID SELECTED): Change shading of selected surface\n\n"

		"F: Change face uv mode\n"
		"Q: Decrease UV Scalar\n"
		"E: Increase UV Scalar\n\n"

		"R (SOLID SELECTED): Reset uv\n"
		"UP (SOLID SELECTED): Change UV v+\n"
		"DOWN (SOLID SELECTED): Change UV v -\n"
		"LEFT (SOLID SELECTED): Change UV u+\n"
		"RIGHT (SOLID SELECTED): Change UV v-\n\n"

		"CTRL or SHIFT (SOLID SELECTED): Select vertex to move\n\n"

		"CTRL+R (SOLID or ENTITY SELECTED): Reset position\n"
		"SHIFT+UP (SOLID or ENTITY SELECTED): Move X+\n"
		"SHIFT+DOWN (SOLID or ENTITY SELECTED): Move X-\n"
		"SHIFT+LEFT (SOLID or ENTITY SELECTED): Move Z+\n"
		"SHIFT+RIGHT (SOLID or ENTITY SELECTED): Move Z-\n"
		"CTRL+UP (SOLID or ENTITY SELECTED): Move Y+\n"
		"CTRL+DOWN (SOLID/ENTITY SELECTED): Move Y-\n", pfd::choice::ok).result();


	if (res == pfd::button::ok) {
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->setBlockCursor(false);
		interaction->setCursorMode(false);	
	}
	
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

	char path[256] = "";
	sprintf_s(path, "%S", m_arguments->getBaseDirectory());
	pfd::open_file dialog = pfd::open_file::open_file("Open SCN", path, { "SCN Maps","*.scn","All Files", "*" });
	auto res = dialog.result();
	if (!res.empty()) {
		m_arguments->setSCNPath(res[0].c_str());
		m_arguments->setScnLoaded(false);
	}
	interaction->setBlockCursor(false);
	
	interaction->guiStateLayer().set(GUIState::Default);
	dialog.kill();
	if (!m_arguments->getScnLoaded()) {
		//interaction->getLog()->clear();
		CViewManager::getInstance()->getLayer(1)->destroyAllView();
		CViewManager::getInstance()->getLayer(0)->changeView<CViewInit>(m_arguments);

	}
}

void CViewGui::openTextures() {
	CInteractionManager* interaction = CInteractionManager::getInstance();

	CGameObject* selected = interaction->getSelectObj();
	char path[256] = "";
	sprintf_s(path, "%S/textures", m_arguments->getBaseDirectory());
	pfd::open_file dialog = pfd::open_file::open_file("Open Textures", path, { "Images","*.tga *.png *.bmp",
		"All Files", "*" });
	auto res = dialog.result();
	if (!res.empty()) {
		if (selected) 
			selected->getComponent<CScnMeshComponent>()->setTexture(SCNEdit::getSCN(), res[0].c_str());
	}
	interaction->setBlockCursor(false);
	interaction->setCursorMode(false);
	
	dialog.kill();
	interaction->guiStateLayer().set(GUIState::Default);

}
void CViewGui::quit() {
	//Should probably generalize this in SCNEdit as there two instances of this function.
	SCNEdit::proccessQuit();
	delete m_arguments;
	std::exit(0);
}


void CViewGui::updateUVInfo() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	const char* uvmode = interaction->findUVMode(UVMode::Move) ? "Move" : "Resize";
	m_textSection3b->setText(std::format("UV {}. Grid scale: {:.3f}",uvmode, interaction->getUVScalar()).c_str());
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
	CScnSolid* solid = scn->getSolid(surfdata.solididx);
	scnSurf_t surf = solid->surfs[surfdata.surfsel];
	scnSurfParamFrame_t params = solid->paramFrames[surfdata.surfsel];
	scnPlane_t plane = solid->planes[surf.planeidx];

	bool isSelected = interaction->getSelectObj() && interaction->keyAugLayer().get() != KeyAugment::None;

	if (!(isSelected && inVertInf)) {
		info.section1 = format("surf[{}]: | texture = {} | flags = {} {} | alpha = {} | lighmap size = {}x{} "
			"| texture size = {}x{} |",
			surfdata.surfsel, surf.texture, surf.flag1, surf.flag2, surf.alpha, surf.lmsize_h, surf.lmsize_v, 
			surf.height, surf.width);

		info.section2 = format("face vert idx = from {} to {} | plane idx = {} | normal = ({:.1f}, {:.1f},"
			" {:.1f}), d={:.1f} | has shading = {} |",
			surf.vertidxstart, surf.vertidxstart + surf.vertidxlen, surf.planeidx, plane.a, plane.b, 
			plane.c, plane.d, surf.hasVertexColors);
	}
	else {
		indexedVec3df_t vert = interaction->getMoveableVert();
		u32 uvidx = solid->uvidxs[vert.vertidxidx];
		core::vector2df uv = solid->uvpos[uvidx];
		u32 vertidx = solid->vertidxs[vert.vertidxidx];
		scnSurf_t surfv = solid->surfs[vert.surfidx];

		if (surfv.shading && vert.bShared) {
			u8* ps = surfv.shading + vert.surf_vertidx * 4;
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | shading color(rgba) "
				"= {} {} {} {} | shared surfs = {} |",
				vert.vertidxidx, vertidx, vert.surf_vertidx, ps[0], ps[1], ps[2], ps[3], str_join(vert.sharesWith));
		}
		else if (surfv.shading) {
			u8* ps = surfv.shading + vert.surf_vertidx * 4;
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | shading color(rgba) "
				"= {} {} {} {} | no shared | ",
				vert.vertidxidx, vertidx, vert.surf_vertidx, ps[0], ps[1], ps[2], ps[3]);
		}
		else if (vert.bShared) {
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | no vertex color | "
				"shared surfs = {} |",
				vert.vertidxidx, vertidx, vert.surf_vertidx, str_join(vert.sharesWith));
		}
		else {
			info.section1 = format("face vert[{}]: | world vert idx = {} | local vert idx = {} | no vertex color | "
				"no shared |",
				vert.vertidxidx, vertidx, vert.surf_vertidx);
		}
		info.section2 = format("uv index = {} | uv_pos = {} | surf unk = {} {} |", uvidx, vec2_to_str(uv, 3), 
			surf.unk[0], surf.unk[1]);
	}

	if (interaction->getSelectObj()) {
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
		"| normal = ({:.1f}, {:.1f}, {:.1f}), d = {:.1f} |",
		portaldata.portalidx, portal.name, portaldata.cellidx, cell.name, portal.nextcell, 
		nextcell.name, plane.a, plane.b, plane.c, plane.d, portal.unk);
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
		if (i > split && split >= 4) {
			info.section2 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
		}
		else info.section1 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
	}
	
	return info;
}


