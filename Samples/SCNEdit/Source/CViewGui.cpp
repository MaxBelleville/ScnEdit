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
	if (!m_arguments->isExtrasEnabled()) m_missingItems += 1;
	if (!m_arguments->isPortalEnabled()) m_missingItems += 1;
	if (!m_arguments->isBBEnabled()) m_missingItems += 1;
	bool lightmaps = false;
	if (SCNEdit::getSCN()) lightmaps = SCNEdit::getSCN()->getLightmap()->hasLightmaps();
	if (!m_arguments->isEntityEnabled()) m_missingItems += 1;
	if (!m_arguments->isLightmapEnable()|| !lightmaps) m_missingItems += 3;
	if (!m_arguments->isInternalDebug()) m_missingItems += 3;
}

CViewGui::~CViewGui()
{

}

void CViewGui::onInit()
{
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	CZone* zone = context->getActiveZone();
	CGameObject* gui = zone->createEmptyObject();
	m_canvas = gui->addComponent<CCanvas>();

	CGUIImporter::loadGUI("sedata/gui/SCNEdit.gui", m_canvas);
	float scale = min(m_arguments->getCurrentRes().Width / m_arguments->getDesktopRes().Width, m_arguments->getCurrentRes().Height / m_arguments->getDesktopRes().Width);
	m_canvas->applyScaleGUI(std::clamp(scale, 0.0f, 0.8f));
	m_canvas->setSortDepth(0);

	m_uiContainer = gui->addComponent<UI::CUIContainer>();

	UI::CUIButton* openBtn = addButton("Canvas/Open_Btn");
	CInteractionManager* interaction = CInteractionManager::getInstance();
	openBtn->OnPressed = [camera,interaction](UI::CUIBase* base)
		{
			if (!interaction->setFoundState(GUIState::OpenScn, GUIState::Default, GUIState::Hover))
			{
				camera->setEnable(true);
				ImGuiFileDialog::Instance()->Close();
				interaction->setState(GUIState::Default);
			}
		};
	addTooltip(openBtn, "Open a new .scn mission/map.", "");

	UI::CUIButton* closeBtn = addButton("Canvas/Close_Btn");
	closeBtn->OnPressed = [interaction](UI::CUIBase* base)
		{
			if (SCNEdit::getSCN()) interaction->setState(GUIState::CloseFile);
		};
	addTooltip(closeBtn, "Close current .scn map.", "No map found so does nothing.");
	UI::CUIButton* exportBtn = addButton("Canvas/Export_Btn");
	bool bExtra = m_arguments->isExtrasEnabled();
	exportBtn->OnPressed = [bExtra,interaction](UI::CUIBase* base)
		{
			if (SCNEdit::getSCN()) {
				SCNEdit::exportSCN(bExtra);
				 
			}
		};

	addTooltip(exportBtn, "Export obj,map mtl, 3ds files\nfor use in other programs.", "Nothing to export,\nplease open a .scn map.");
	UI::CUIButton* saveBtn = addButton("Canvas/Save_Btn");
	saveBtn->OnPressed = [interaction](UI::CUIBase* base)
		{
			//Save data
			if (SCNEdit::getSCN()) {
				//m_guiState = GUIState::Export;
				if (interaction->setFoundState(GUIState::Save, GUIState::Default, GUIState::Hover)) {
					static const char* buttons[] = { "Save", "Cancel", NULL };
					m_msgbox = ImGuiAl::MsgBox();
					m_msgbox.Init("Save with Scraped Lightmaps", NULL,
						"Notice Lightmaps are Scraped. Will save without lightmaps.", buttons, false);
				}
				else interaction->setState(GUIState::Default);
			}
		};

	addTooltip(saveBtn, "Save over current .scn file.", "Nothing to save,\nplease open a .scn map");
	UI::CUIButton* quitBtn = addButton("Canvas/Quit_Btn");
	quitBtn->OnPressed = [interaction](UI::CUIBase* base)
	{
		interaction->setState(GUIState::Quit);
	};
	CGUIElement* el = m_canvas->getGUIByPath("Canvas/BottomBar");
	CGUIElement* el2 = m_canvas->getGUIByPath("Canvas/BottomBar/SectionExtra_Layout");
	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	int taskbarHeight = m_arguments->getDesktopRes().Height - workArea.bottom;
	float diff = m_arguments->getDesktopRes().Height - m_arguments->getRes().Height;
	float diffW = m_arguments->getDesktopRes().Width - m_arguments->getRes().Width;
	el->setWidth(m_arguments->getCurrentRes().Width);

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
	m_textSection1 = addTextbox("Canvas/BottomBar/Section1_Rect/Section1_Input");
	m_textSection2 = addTextbox("Canvas/BottomBar/Section2_Rect/Section2_Input");
	m_textSection3a = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect1/Section3_Input1");
	m_textSection3b = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect2/Section3_Input2");
	m_textSection3b-> setText("UV Move, Grid size: 0.01");
	m_textSection3c = addTextbox("Canvas/BottomBar/Section3_Layout/Section3_Rect3/Section3_Input3");
	
	interaction->OnKeyEvent([interaction, camera](key_pair pair) {
		if (interaction->getKeyState(make_pair(KEY_KEY_T, KeyAugment::None))) {
			if (interaction->findSelectedType(SelectedType::Solid, SelectedType::SolidExtra)) {
				if (interaction->setFoundState(GUIState::OpenTexture, GUIState::Hover, GUIState::Default)) {
					getApplication()->getDevice()->getCursorControl()->setVisible(true);
				}
				else {
					camera->setEnable(true);
					ImGuiFileDialog::Instance()->Close();
					getApplication()->getDevice()->getCursorControl()->setVisible(false);
					interaction->setState(GUIState::Default);
				}
			}
		}
		if ((interaction->getKeyState(make_pair(pair.first, pair.second)) && 
			pair.first >= KEY_LEFT && pair.first <= KEY_DOWN) ||
			interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::None))) {
			if (interaction->getSelectedType() == SelectedType::Entity) {
				updateSections();
			}
		}

	});
	interaction->OnCollisionEvent([interaction](core::triangle3df tri, core::vector3df intersection) {
		if (interaction->getSelectedType() != SelectedType::Empty) {
			updateSections();
		}
		else {
			resetSections();
		}
	});
	interaction->OnVertexUpdateEvent([interaction]() {
		updateVertex();
	});

}
UI::CUITextBox* CViewGui::addTextbox(const char* textPath) {
	CGUIElement* element = m_canvas->getGUIByPath(textPath);
	if (element) {
		UI::CUITextBox* textbox = new UI::CUITextBox(m_uiContainer, element);
		CGUIElement* bg = textbox->getElement();
		textbox->setText("");
		textbox->setEditable(false);

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
			return button;
		}
	}
}

void CViewGui::addTooltip(UI::CUIButton* btn, std::string text, std::string altText) {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	btn->OnPointerHover = [interaction,altText,text](float x, float y) {
		
		if (interaction->setFoundState(GUIState::Hover, GUIState::Default)) {
			interaction->setMouse(core::vector2df(x, y));
			if (!altText.empty()) {
				if (SCNEdit::getSCN()) m_tooltip = text;
				else m_tooltip = altText;
			}
			else m_tooltip = text;
		}
	};
	btn->OnPointerOut = [interaction](float x, float y) {
		interaction->setFoundState(GUIState::Default, GUIState::Hover);
	};
}

void CViewGui::onDestroy()
{

}

void CViewGui::onData()
{

}

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


void CViewGui::onPostRender()
{

}

void CViewGui::onGUI()
{
	bool open = false;

	ImGuiWindowFlags window_flags = 2;

	ImGui::SetNextWindowPos(ImVec2(m_arguments->getCurrentRes().Width - 170, 15), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(150, 315 - (22 * m_missingItems)), ImGuiCond_Once);

	if (!ImGui::Begin("SCN Options", &open, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	// BEGIN WINDOW

	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	ImGui::SeparatorText("Scn Visibility");
	ImGui::Checkbox("Solids", &gui->vis_scn);
	std::string s{ "example" };
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	
	bool lightmaps = false;
	if(SCNEdit::getSCN()) lightmaps = SCNEdit::getSCN()->getLightmap()->hasLightmaps();
	if (m_arguments->isExtrasEnabled()) ImGui::Checkbox("Doors/Others", &gui->vis_doors);
	if (m_arguments->isPortalEnabled()) ImGui::Checkbox("Portals", &gui->vis_portals);
	if (m_arguments->isBBEnabled())  ImGui::Checkbox("Bounding Boxes", &gui->vis_bb);
	if (m_arguments->isEntityEnabled()) ImGui::Checkbox("Entites", &gui->vis_entities);
	if (m_arguments->isLightmapEnable()&& lightmaps) ImGui::Checkbox("Lightmap", &gui->vis_lightmaps);
	if (m_arguments->isLightmapEnable()&& lightmaps) ImGui::SeparatorText("Export Settings");
	if (m_arguments->isLightmapEnable() && lightmaps) {
		if (ImGui::Checkbox("Scrape Lightmap", &gui->scrape_lightmaps)) {
			if (gui->scrape_lightmaps) {
				//Msgbox.
			}
		}
	}
	if (m_arguments->isInternalDebug()) ImGui::SeparatorText("Debug");
	if (m_arguments->isInternalDebug()) {
		if (interaction->findState(GUIState::Debug)) {
			if (ImGui::Button("Close Debug", ImVec2(130, 20))) {
				interaction->setState(GUIState::Default);
				camera->setEnable(true);
			}
		}
		else {
			if (ImGui::Button("Open Debug", ImVec2(130, 20))) {
				interaction->setFoundState(GUIState::Debug, GUIState::Hover, GUIState::Default);
				interaction->getLog()->scrollToBottom();
				camera->setEnable(false);
			}
		}
		//Create Combobox
		static int currentPage = interaction->getCurrentLogPage();
		char** list_of_numbers = new char* [interaction->getLogSize()];

		for (int i = 0; i < interaction->getLogSize(); i++) {
			std::string temp = std::to_string(i);         // Convert to string
			list_of_numbers[i] = new char[temp.size() + 1]; // Allocate memory for the C-string
			std::strcpy(list_of_numbers[i], temp.c_str()); // Copy string content
		}

		if (ImGui::Combo("Page", &currentPage, list_of_numbers, interaction->getLogSize())) {
			int prev = interaction->getCurrentLogPage();
			interaction->setLogPage(currentPage);
			if(prev>currentPage)interaction->getLog()->scrollToBottom();
			if (prev<currentPage)interaction->getLog()->scrollToTop();
		}
	}
	ImGui::End();
	
	if (interaction->findState(GUIState::Hover)) drawTooltip();
	else if (interaction->findState(GUIState::Debug)) interaction->getLog()->draw();
	else if (interaction->findState(GUIState::Save)) saveFile();
	else if (interaction->findState(GUIState::CloseFile)) closeFile();
	else if (interaction->findState(GUIState::OpenScn)) openFile();
	else if (interaction->findState(GUIState::OpenTexture)) openTextures();
	else if (interaction->findState(GUIState::Quit)) quit();


	//Bottom bar gui
	if (SCNEdit::getSCN()) {
		core::vector3df pos = camera->getPosition();
		s16 cellindx = SCNEdit::getSCN()->getSolid(0)->getCellAtPos(pos);
		if (cellindx >= 0) {
			scnRawCell_t cell = SCNEdit::getSCN()->getSolid(0)->rawcells[cellindx];
			m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f} | Cell: {}", pos.X, pos.Y, pos.Z, cell.name).c_str());
		}
		else {
			m_textSection3c->setText(format("Pos: X {:.1f} Y {:.1f} Z {:.1f}", pos.X, pos.Y, pos.Z).c_str());
		}
	}
}
void CViewGui::resetSections() {
	m_textSection1->setText("");
	m_textSection2->setText("");
	m_textSection3a->setText("");
}
void CViewGui::updateVertex() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	if (interaction->findSelectedType(SelectedType::SolidExtra, SelectedType::Solid)) {
		if (interaction->getSelectObj()) {
			indexedVec3df_t vert = interaction->getMoveableVert();
			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
			m_textSection3a->setText(format("# Surf: ({}) # Shared: ({}) Vert: {:.0f} {:.0f} {:.0f}", 
				comp->selsurfs.size(), comp->sharedsurfs.size(),
				vert.pos.X, vert.pos.Y, vert.pos.Z).c_str());
		}
	}
}
void CViewGui::updateSections() {
	CScn* scn = SCNEdit::getSCN();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	if (interaction->findSelectedType(SelectedType::SolidExtra,SelectedType::Solid)) {
		std::pair<int,int> surfdata = interaction->getSurfISelected();
		CScnSolid* solid= scn->getSolid(surfdata.first);
		scnSurf_t surf = solid->surfs[surfdata.second];
		scnPlane_t plane = solid->planes[surf.planeidx];
	
		m_textSection1->setText(format("surf[{}]: | texture = {} | flags = {} {} | alpha = {} | lighmap size = {}x{} | texture size = {}x{} |",
			surfdata.second,surf.texture,surf.flag1, surf.flag2, surf.alpha, surf.lmsize_h, surf.lmsize_v, surf.height,surf.width).c_str());
		m_textSection2->setText(format("| verts idx idx = from {} to {} | plane idx = {} | normal = ({:.1f}, {:.1f}, {:.1f}), d={:.1f} | shading = {} |",
			surf.vertidxstart, surf.vertidxstart + surf.vertidxlen, surf.planeidx, plane.a, plane.b, plane.c, plane.d, surf.hasVertexColors).c_str());
		
		if (interaction->getSelectObj()) {
			CScnMeshComponent* comp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
			m_textSection3a->setText(format("# Surfs: ({}) # Shared: ({})", comp->selsurfs.size(), comp->sharedsurfs.size()).c_str());
		}
	}
	if (interaction->getSelectedType() == SelectedType::Portal) {
		m_textSection1->setText("Portal");
		std::pair<int, int> portaldata = interaction->getPortalISelected();
		CScnSolid* solid = scn->getSolid(0);
		scnRawCell_t cell =solid->rawcells[portaldata.first];
		scnPortal_t portal =solid->rawcells[portaldata.first].portals[portaldata.second];
		scnRawCell_t nextcell = solid->rawcells[portal.nextcell];
		scnPlane_t plane = portal.plane;
		m_textSection1->setText(format(" | portal id: {} name: \"{}\" | cell #{}, name: \"{}\" | nextcell id: {}, name: \"{}\" | normal = ({:.1f}, {:.1f}, {:.1f}), d = {:.1f} |",
			portaldata.second, portal.name, portaldata.first, cell.name, portal.nextcell, nextcell.name, plane.a,plane.b,plane.c,plane.d, portal.unk).c_str());
		m_textSection2->setText("");
		m_textSection3a->setText("");
	}
	if (interaction->getSelectedType() == SelectedType::Entity) {
		int entindx = interaction->getEntityISelected();
		CScnEnt* ent = scn->getEnt(entindx);
		int split = ent->n_fields / 2;
		std::string text1 = "";
		std::string text2 = "";
		for (int i = 0; i < ent->n_fields; i++) {
			if (i > split&&split>4) {
				text2 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
			}
			else text1 += format("{}: ({}) | ", ent->fields[i].key, ent->fields[i].value);
		}
		m_textSection1->setText(text1.c_str());
		m_textSection2->setText(text2.c_str());
		m_textSection3a->setText("");
	}
}

void CViewGui::saveFile() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	if (gui->scrape_lightmaps) {
		int selected = m_msgbox.Draw();
		m_msgbox.Open();
		if (selected == 1) 
			SCNEdit::saveSCN(m_arguments->getSCNPath(), m_arguments->isExtrasEnabled());
		if (selected > 0) {
			interaction->setState(GUIState::Default);
		}
	}
	else {
		SCNEdit::saveSCN(m_arguments->getSCNPath(), m_arguments->isExtrasEnabled());
		interaction->setState(GUIState::Default);
	}
}
void CViewGui::quit() {
	CViewManager::getInstance()->releaseAllLayer();
	CViewManager::releaseInstance();
	Sleep(500);	//Make sure everything is loaded before continuing.
	CContext::releaseInstance();
	CImguiManager::releaseInstance();
	if (UI::CUIEventManager::getInstance()) {
		UI::CUIEventManager::releaseInstance();
	}
	CInteractionManager::releaseInstance();
	exit(0);
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
	interaction->setState(GUIState::Default);
}
void CViewGui::drawTooltip() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	core::vector2df mouse = interaction->getMouse();
	ImGui::SetNextWindowPos(ImVec2(mouse.X, 35), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(230, 40), ImGuiCond_Always);
	bool tmp = false;
	if (ImGui::Begin("Tooltip", &tmp, ImGuiWindowFlags_NoDecoration))
	{
		ImGui::Text(m_tooltip.c_str());
		//m_guiState = GUIState::Default;
	}
	ImGui::End();
}
void CViewGui::openFile() {
	
	CInteractionManager* interaction = CInteractionManager::getInstance();
	IGFD::FileDialogConfig config;
	config.path = ".";
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	camera->setEnable(false);
	ImGuiFileDialog::Instance()->OpenDialog("ChooseScnDlg", "Choose Scn File", "Scn file{.scn}", config);
	if (ImGuiFileDialog::Instance()->Display("ChooseScnDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

			m_arguments->setSCNPath(filePathName.c_str());
			m_arguments->setScnLoaded(false);
		}
		camera->setEnable(true);
		// close
		ImGuiFileDialog::Instance()->Close();
		interaction->setState(GUIState::Default);
		if (!m_arguments->getScnLoaded()) {
			interaction->getLog()->clear();
			CViewManager::getInstance()->getLayer(1)->destroyAllView();
			CViewManager::getInstance()->getLayer(0)->changeView<CViewInit>(m_arguments);
		}
	}
}

void CViewGui::openTextures() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	IGFD::FileDialogConfig config;
	config.path = "./textures";
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	camera->setEnable(false);
	ImGuiFileDialog::Instance()->OpenDialog("ChooseTexturesDlg", "Choose Texture File", "Image files{.png,.tga,.bmp}, *", config);
	if (ImGuiFileDialog::Instance()->Display("ChooseTexturesDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			if (interaction->getSelectObj()) {
				interaction->getSelectObj()->getComponent<CScnMeshComponent>()->setTexture(SCNEdit::getSCN(), filePathName.c_str());
			}
			updateSections();
		}
		camera->setEnable(true);
		// close
		ImGuiFileDialog::Instance()->Close();
		interaction->setState(GUIState::Default);

	}
}

