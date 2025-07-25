#include "pch.h"
#include "Header/CViewGui.h"

#include "Graphics2D/CGUIImporter.h"
#include "Header/Managers/CContext.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
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
		m_missingItems += 3;
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
	CCamera* camera = context->getActiveCamera();
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
			button->addMotion(UI::EMotionEvent::PointerDown, bg, new UI::CColorMotion(SColor(255, 125, 125, 175)))->setTime(0.0f, 50.0f);
			button->addMotion(UI::EMotionEvent::PointerHover, bg, new UI::CColorMotion(SColor(255, 175, 175, 225)))->setTime(0.0f, 50.0f);
			button->addMotion(UI::EMotionEvent::PointerUp, bg, new UI::CColorMotion(SColor(255, 175, 175, 225)))->setTime(0.0f, 50.0f);
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
	
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();

	//UI Button events
	m_quitButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		interaction->guiStateLayer().set(GUIState::Quit);
	};

	m_openButton->OnPressed = [camera, interaction](UI::CUIBase* base)
	{
		if (!interaction->guiStateLayer().swap(GUIState::OpenScn, GUIState::Default, GUIState::Hover))
		{
			camera->setEnable(true);
			camera->updateComponent();
			ImGui::CloseCurrentPopup();
			interaction->guiStateLayer().set(GUIState::Default);
			interaction->infoModeLayer().set(GUIInfoMode::Basic);
		}
	};

	m_closeButton->OnPressed = [interaction](UI::CUIBase* base)
	{
		if (SCNEdit::getSCN()) interaction->guiStateLayer().set(GUIState::CloseFile);
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
				static const char* buttons[] = { "Save", "Cancel", NULL };
				m_msgbox = ImGuiAl::MsgBox();
				m_msgbox.Init("Save with Scraped Lightmaps", NULL,
					"Notice Lightmaps are Scraped. Will save without lightmaps.", buttons, false);
			}
			else {
				interaction->guiStateLayer().set(GUIState::Default);
				interaction->infoModeLayer().set(GUIInfoMode::Basic);
			}
		}
	};

	//Section 1 and 2 text edit events
	m_textSection1->OnTextSet = [interaction](UI::CUIBase* base) {
		if (interaction->guiStateLayer().find(GUIState::EditFlags,GUIState::EditAlpha, GUIState::EditShading)) {
			//Uses the ": " in the input to proccess the number and update surf
			std::string msg = str_split(m_textSection1->getText(), ": ")[1];
			u8 num = std::atoi(msg.c_str());
			//This section is repeated maybe I can improve it so it's shared between textsections?
			CScn* scn = SCNEdit::getSCN();
			solidSelect_t surfdata = interaction->getSurfISelected();
			CScnSolid* solid = scn->getSolid(surfdata.solididx);
			scnSurf_t* surfi =&solid->surfs[surfdata.surfsel];

;			if (interaction->guiStateLayer().get() == GUIState::EditAlpha) 
				surfi->alpha = num;
			else if (interaction->guiStateLayer().get() == GUIState::EditShading)
				surfi->hasVertexColors = num;
			else 
				surfi->flag1 = num;
		}
		else if(interaction->guiStateLayer().find(GUIState::EditAll)) {
			core::array<std::string> msg = str_split(m_textSection1->getText(), ",");
			//This will need some proccessing so probably do in another function.
			
		}
		isSection1Set = true;
		if (interaction->guiStateLayer().find(GUIState::EditFlags, GUIState::EditAll) && isSection1Set && isSection2Set) {
			resetEditText();
			interaction->setCursorMode(false);
		}
		
		if (interaction->guiStateLayer().find(GUIState::EditAlpha, GUIState::EditShading) && isSection1Set) {
			resetEditText();
			interaction->setCursorMode(false);
		}
		
	};

	m_textSection2->OnTextSet = [interaction](UI::CUIBase* base) {
		if (interaction->guiStateLayer().find(GUIState::EditFlags)) {
			std::string msg = str_split(m_textSection2->getText(), ": ")[1];
			u8 flag2 = std::atoi(msg.c_str());
			CScn* scn = SCNEdit::getSCN();
			solidSelect_t surfdata = interaction->getSurfISelected();
			CScnSolid* solid = scn->getSolid(surfdata.solididx);
			scnSurf_t* surfi = &solid->surfs[surfdata.surfsel];

			surfi->flag2 = flag2;
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
				GUIState::EditAll, GUIState::EditFlags, GUIState::EditShading, GUIState::EditAlpha);
			interaction->infoModeLayer().set(GUIInfoMode::Basic);
			os::Printer::log("Resetting edit on cursor change");
			resetEditText();
		}
	});

	//Custom interaction manager events/callbacks
	interaction->OnKeyEvent([interaction, camera](key_pair pair) {

		//Update positition of entity.
		if ((interaction->getKeyState(make_pair(pair.first, pair.second)) &&
			pair.first >= KEY_LEFT && pair.first <= KEY_DOWN) ||
			interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::None))) {
			if (interaction->selTypeLayer().get() == SelectedType::Entity) 
				updateSections();
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
			}
			else {
				ImGui::CloseCurrentPopup();
				getApplication()->getDevice()->getCursorControl()->setVisible(false);
				interaction->guiStateLayer().set(GUIState::Default);
				interaction->infoModeLayer().set(GUIInfoMode::Basic);
			}
		}

		if (!interaction->selTypeLayer().find(SelectedType::Solid, SelectedType::SolidExtra)) // The next statments require solid.
			return;

		//Open texture popup if t is pressed.
		if (interaction->findKeyState({ make_pair(KEY_KEY_T, KeyAugment::None), make_pair(KEY_KEY_T, KeyAugment::Shift) })) {
			if (interaction->guiStateLayer().swap(GUIState::OpenTexture, GUIState::Hover, GUIState::Default)) {
				getApplication()->getDevice()->getCursorControl()->setVisible(true);
				interaction->resetKeyState();
			}
			else {
				ImGui::CloseCurrentPopup();
				getApplication()->getDevice()->getCursorControl()->setVisible(false);
				interaction->guiStateLayer().set(GUIState::Default);
				interaction->infoModeLayer().set(GUIInfoMode::Basic);
			}
		}
		core::array<std::pair<irr::EKEY_CODE, int>> numerickeys;
		CInteractionManager::getNumeric(numerickeys);


		//Proccess Alpha,Shading,Flag editing. TODO: EditAll
		if (interaction->getKeyState(make_pair(KEY_KEY_F, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditAlpha, GUIState::EditShading, GUIState::EditAll)) 
				resetEditText();
			os::Printer::log("Hello there");
			if (interaction->guiStateLayer().swap(GUIState::EditFlags, GUIState::Default)) {
				interaction->infoModeLayer().set(GUIInfoMode::Edit);
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				CInteractionManager::activateText(m_textSection1, numerickeys, "Set Flag 1 (0 - 999): ", 3);
				CInteractionManager::activateText(m_textSection2, numerickeys, "Set Flag 2 (0 - 999): ", 3);
			}
			else if (interaction->guiStateLayer().find(GUIState::EditFlags)) {
				interaction->setCursorMode(false);
				resetEditText();
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_T, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags, GUIState::EditShading, GUIState::EditAll))
				resetEditText();
			
			
			if (interaction->guiStateLayer().swap(GUIState::EditAlpha, GUIState::Default)) {
				interaction->infoModeLayer().set(GUIInfoMode::Edit);
				interaction->setCursorMode(true);
				interaction->resetKeyState();
				CInteractionManager::activateText(m_textSection1, numerickeys, "Set Alpha (0 - 255): ", 3);
			}
			else if (interaction->guiStateLayer().find(GUIState::EditAlpha)) {
				interaction->setCursorMode(false);
				resetEditText();
			}
		}
		/*if (interaction->getKeyState(make_pair(KEY_KEY_L, KeyAugment::Ctrl))) {
			if (interaction->guiStateLayer().swap(GUIState::Default, GUIState::EditFlags, GUIState::EditAlpha, GUIState::EditAll)) 
				resetEditText();

			if (interaction->guiStateLayer().swap(GUIState::EditShading, GUIState::Default)) 
				CInteractionManager::activateText(m_textSection1, CInteractionManager::getNumeric(), "Set Shading (0 - 999): ", 3);
			else if (interaction->guiStateLayer().find(GUIState::EditShading))
				resetEditText();
		}*/

	});


	interaction->OnCollisionEvent([interaction](core::triangle3df tri, core::vector3df intersection) {
		//Update section if you select a solid/entity/portal
		if (interaction->selTypeLayer().get() != SelectedType::Empty) 
			updateSections();
		else 
			m_textSection3a->setText("");
	});

	interaction->OnVertexUpdateEvent([interaction]() {
		//Update the solid vertex pos
		updateSections();
	});

}
void CViewGui::resetEditText() {	
	isSection1Set = false; 
	isSection2Set = false;
	CInteractionManager* interaction = CInteractionManager::getInstance();
	interaction->guiStateLayer().set(GUIState::Default);
	interaction->infoModeLayer().set(GUIInfoMode::Basic);
	CInteractionManager::resetText(m_textSection1,512);
	CInteractionManager::resetText(m_textSection2, 512);
	updateSections();
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
	ImGui::SetNextWindowSize(ImVec2(150, 315 - (21 * m_missingItems)), ImGuiCond_Once);

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
	std::string s{ "example" };
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	
	bool lightmaps = false;

	if(SCNEdit::getSCN()) lightmaps = SCNEdit::getSCN()->getLightmap()->hasLightmaps();
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
			if (ImGui::Button("Close Debug", ImVec2(130, 20))) {
				interaction->guiStateLayer().set(GUIState::Default);
				camera->setEnable(true);
			}
		}
		else {
			if (ImGui::Button("Open Debug", ImVec2(130, 20))) {
				interaction->guiStateLayer().swap(GUIState::Debug, GUIState::Hover, GUIState::Default);
				interaction->getLog()->scrollToBottom();
				camera->setEnable(false);
			}
		}
		//Create Combobox
		static int currentPage = interaction->getCurrentLogPage();
		char** list_of_numbers = new char* [interaction->getLogSize()];
		
		//Should hopefully copy all the log information into debug popup.
		for (int i = 0; i < interaction->getLogSize(); i++) {
			std::string temp = std::to_string(i);         // Convert to string
			list_of_numbers[i] = new char[temp.size() + 1]; // Allocate memory for the C-string
			std::strcpy(list_of_numbers[i], temp.c_str()); // Copy string content
		}
		//Proccess pages for combo box.
		if (ImGui::Combo("Page", &currentPage, list_of_numbers, interaction->getLogSize())) {
			int prev = interaction->getCurrentLogPage();
			interaction->setLogPage(currentPage);
			if(prev>currentPage)interaction->getLog()->scrollToBottom();
			if (prev<currentPage)interaction->getLog()->scrollToTop();
		}
	}
	ImGui::End();
	
	if (interaction->guiStateLayer().find(GUIState::Hover))		    drawTooltip();
	else if (interaction->guiStateLayer().find(GUIState::Debug))		interaction->getLog()->draw();
	else if (interaction->guiStateLayer().find(GUIState::Save))		saveFile();
	else if (interaction->guiStateLayer().find(GUIState::Help))		helpDialog();
	else if (interaction->guiStateLayer().find(GUIState::CloseFile))	closeFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenScn))		openFile();
	else if (interaction->guiStateLayer().find(GUIState::OpenTexture)) openTextures();
	else if (interaction->guiStateLayer().find(GUIState::Quit))		quit();

	//Bottom bar gui
	if (SCNEdit::getSCN()) {
		core::vector3df pos = camera->getPosition();
		s16 cellindx = SCNEdit::getSCN()->getSolid(0)->getCellAtPos(pos);
		if (cellindx >= 0) {
			scnRawCell_t cell = SCNEdit::getSCN()->getSolid(0)->rawcells[cellindx];
			m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f} | Cell: {}", pos.X, pos.Y, pos.Z, cell.name).c_str());
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

void CViewGui::saveFile() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	if (gui->scrape_lightmaps) {
		int selected = m_msgbox.Draw();
		m_msgbox.Open();
		interaction->infoModeLayer().set(GUIInfoMode::Dialog);
		if (selected == 1) 
			SCNEdit::saveSCN();
		if (selected > 0) {
			interaction->guiStateLayer().set(GUIState::Default);
			interaction->infoModeLayer().set(GUIInfoMode::Basic);
		}
	}
	else {
		SCNEdit::saveSCN();
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->infoModeLayer().set(GUIInfoMode::Basic);
	}
}
void CViewGui::helpDialog() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	static const char* buttons[] = { "Ok", NULL };
	m_msgbox = ImGuiAl::MsgBox();
	m_msgbox.Init("Controls and help", NULL,
		"H (ANY SELECTED): Hides element from editor\n"
		"SHIFT+H (SOLID SELECTED): Hides shared and selected surfaces\n"
		"CTRL+H: Unhides hidden elements\n\n"

		"X (SOLID SELECTED & -flip): Flips selected texture horizontally\n"
		"V (SOLID SELECTED & -flip): Flips selected texture vertically\n\n"

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
		"CTRL+DOWN (SOLID/ENTITY SELECTED): Move Y-\n", buttons, false);

	float size = getApplication()->getDriver()->getScreenSize().Width;
	float sizeH = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(size/2-260, sizeH/2-260), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(475, 480), ImGuiCond_Always);
	int selected = m_msgbox.Draw();
	

	if (selected > 0) {
		ImGui::CloseCurrentPopup();
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->infoModeLayer().set(GUIInfoMode::Basic);
		interaction->setCursorMode(false);	
	}
	else if(interaction->infoModeLayer().swap(GUIInfoMode::Dialog,GUIInfoMode::Basic)) 
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
	interaction->infoModeLayer().set(GUIInfoMode::Basic);
}

void CViewGui::openFile() {
	
	CInteractionManager* interaction = CInteractionManager::getInstance();
	IGFD::FileDialogConfig config;
	config.path = ".";
	CContext* context = CContext::getInstance();
	float w = getApplication()->getDriver()->getScreenSize().Width;
	float h = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(15, 60), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(w / 1.5, h / 1.5), ImGuiCond_FirstUseEver);
	interaction->infoModeLayer().set(GUIInfoMode::Dialog);
	ImGuiFileDialog::Instance()->OpenDialog("ChooseScnDlg", "Choose Scn File", "Scn file{.scn}", config);
	if (ImGuiFileDialog::Instance()->Display("ChooseScnDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			m_arguments->setSCNPath(filePathName.c_str());
			m_arguments->setScnLoaded(false);
		}

		ImGuiFileDialog::Instance()->Close();
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->infoModeLayer().set(GUIInfoMode::Basic);
		if (!m_arguments->getScnLoaded()) {
			//interaction->getLog()->clear();
			CViewManager::getInstance()->getLayer(1)->destroyAllView();
			CViewManager::getInstance()->getLayer(0)->changeView<CViewInit>(m_arguments);
			
		}
	}
}

void CViewGui::openTextures() {
	//IMGUI dialog setup and other variable collecting.
	CInteractionManager* interaction = CInteractionManager::getInstance();
	IGFD::FileDialogConfig config;
	config.path = "./textures";
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();

	//Set window pos
	float w = getApplication()->getDriver()->getScreenSize().Width;
	float h = getApplication()->getDriver()->getScreenSize().Height;
	ImGui::SetNextWindowPos(ImVec2(15, 60), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(w / 1.5, h / 1.5), ImGuiCond_FirstUseEver);

	CGameObject* selected = interaction->getSelectObj();
	interaction->infoModeLayer().set(GUIInfoMode::Dialog);
	//Open dialog when finished proccess texture and update text.
	ImGuiFileDialog::Instance()->OpenDialog("ChooseTexturesDlg", "Choose Texture File", "Image files{.png,.tga,.bmp}, *", config);
	if (ImGuiFileDialog::Instance()->Display("ChooseTexturesDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			if (selected)
				selected->getComponent<CScnMeshComponent>()->setTexture(SCNEdit::getSCN(), filePathName.c_str());
			updateSections();
		}
		// close
		ImGuiFileDialog::Instance()->Close();
		interaction->guiStateLayer().set(GUIState::Default);
		interaction->infoModeLayer().set(GUIInfoMode::Basic);
		interaction->setCursorMode(false);
	}
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

/// Splits the data into solid info, portal info, entity info sections and updates it.
void CViewGui::updateSections() {
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	if (interaction->guiStateLayer().find(GUIState::Default)) {
		if (interaction->selTypeLayer().find(SelectedType::SolidExtra, SelectedType::Solid)) 
			updateSolidInfo();
		if (interaction->selTypeLayer().get() == SelectedType::Portal) 
			updatePortalInfo();
		if (interaction->selTypeLayer().get() == SelectedType::Entity)
			updateEntityInfo();
	}
}

void CViewGui::updateSolidInfo() {
	//Get values from singletons for solids.
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	solidSelect_t surfdata = interaction->getSurfISelected();
	CScnSolid* solid = scn->getSolid(surfdata.solididx);
	scnSurf_t surf = solid->surfs[surfdata.surfsel];
	scnSurfParamFrame_t params = solid->paramFrames[surfdata.surfsel];
	scnPlane_t plane = solid->planes[surf.planeidx];

	//Display surface details
	if (interaction->guiStateLayer().find(GUIState::Default)) {
		m_textSection1->setText(format("surf[{}]: | texture = {} | flags = {} {} | alpha = {} | lighmap size = {}x{} | texture size = {}x{} |",
			surfdata.surfsel, surf.texture, surf.flag1, surf.flag2, surf.alpha, surf.lmsize_h, surf.lmsize_v, surf.height, surf.width).c_str());

		m_textSection2->setText(format("| verts idx idx = from {} to {} | plane idx = {} | normal = ({:.1f}, {:.1f}, {:.1f}), d={:.1f} | has shading = {} |",
			surf.vertidxstart, surf.vertidxstart + surf.vertidxlen, surf.planeidx, plane.a, plane.b, plane.c, plane.d, surf.hasVertexColors).c_str());
	
		//os::Printer::log(format("U_AX {} V_AX {} ORG {}", vec3_to_str(params.u_axis,2), vec3_to_str(params.v_axis,2), vec3_to_str(params.origin,2)).c_str());
	}

	//Check internal surface/vert data in the mesh and display it.
	if (interaction->getSelectObj()) {
		CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
		const char* mode = interaction->keyAugLayer().get() == KeyAugment::None ? "Hover" : "Select";
		indexedVec3df_t vert = interaction->getMoveableVert();
		m_textSection3a->setText(format("Surfs: ({}), Shared: ({}) {} V: {}",
			comp->selsurfs.size(), comp->sharedsurfs.size(), mode,vec3_to_str(vert.pos,0)).c_str());

		core::vector3df local = vert.pos - params.origin;
		core::vector2df uv = core::vector2df(local.dotProduct(params.u_axis), local.dotProduct(params.v_axis));
	//	os::Printer::log(format("Local {} UV {}", vec3_to_str(local, 2), vec2_to_str(uv, 2)).c_str());
	}

}

void CViewGui::updatePortalInfo() {
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	m_textSection1->setText("Portal");
	portalSelect_t portaldata = interaction->getPortalISelected();
	CScnSolid* solid = scn->getSolid(0);
	scnRawCell_t cell = solid->rawcells[portaldata.cellidx];
	scnPortal_t portal = solid->rawcells[portaldata.cellidx].portals[portaldata.portalidx];
	scnRawCell_t nextcell = solid->rawcells[portal.nextcell];
	scnPlane_t plane = portal.plane;

	m_textSection1->setText(format(" | portal id: {} name: \"{}\" | cell #{}, name: \"{}\" | nextcell id: {}, name: \"{}\" "
		"| normal = ({:.1f}, {:.1f}, {:.1f}), d = {:.1f} |",
		portaldata.portalidx, portal.name, portaldata.cellidx, cell.name, portal.nextcell, 
		nextcell.name, plane.a, plane.b, plane.c, plane.d, portal.unk).c_str());
	m_textSection2->setText("");
	m_textSection3a->setText("");
}

void CViewGui::updateEntityInfo() {
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	int entindx = interaction->getEntityISelected();
	CScnEnt* ent = scn->getEnt(entindx);
	int split = ent->n_fields / 2;

	std::string text1 = "";
	std::string text2 = "";
	for (int i = 0; i < ent->n_fields; i++) {
		if (i > split && split > 4) {
			text2 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
		}
		else text1 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
	}
	m_textSection1->setText(text1.c_str());
	m_textSection2->setText(text2.c_str());
	m_textSection3a->setText("");
}


