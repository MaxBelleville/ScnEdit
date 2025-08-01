#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/SCNEdit.h"
#include "Header/Managers/CContext.h"
#include "Header/Managers/CViewManager.h"
#include "CImguiManager.h"
#include "Header/CViewInit.h"
#include "Header/Managers/CInteractionManager.h"
#include "UserInterface/CUIEventManager.h"
#include <filesystem>
#include <chrono>


using namespace std;
///Prepares scn arguments singleton and initalizes the application (will create win32 window with these parameters)
void prepareApplication(const std::vector<std::string>& argv, SIrrlichtCreationParameters* param) {
	
	CScnArguments* options = new CScnArguments(argv);
	if(options->isDebugEnabled())getApplication()->showDebugConsole();

	SCNEdit* app = new SCNEdit(options);
	param->DriverType = options->getDriverType();
	if (options->getDriverType() != EDT_DIRECT3D11) 
		param->AntiAlias = 0;
	param->Fullscreen = options->isFullscreen();
	param->WindowSize = options->getCurrentRes();
	param->Borderless = options->isBorderless();
	param->Resizeable = options->isResizeable();
	
	getApplication()->registerAppEvent("SWAT3 SCN editor", app);
}

SCNEdit::SCNEdit(CScnArguments* options)
{
	m_arguments = options;


	CViewManager::createGetInstance()->initViewLayer(2);
}

SCNEdit::~SCNEdit()
{
	proccessQuit();
	getApplication()->unRegisterAppEvent(this);
	delete m_arguments;
}

void SCNEdit::onInitApp()
{

	CContext::createGetInstance();

	//Create essential managers.
	CImguiManager::createGetInstance();
	CInteractionManager::createGetInstance();
	UI::CUIEventManager::createGetInstance();


	CBaseApp* app = getApplication();
	os::Printer::Logger->setLogLevel(ELL_DEBUG);
	//Reprint data so inline console also reads it.
	os::Printer::log(format("Irrlicht Engine version {}",app->getDevice()->getVersion()).c_str(), ELL_NONE);
	os::Printer::log(app->getDriver()->getName(), ELL_NONE);
	os::Printer::log(format("Driver Vendor: {}",app->getDriver()->getVendorInfo().c_str()).c_str(), ELL_NONE);
	os::Printer::log(format("Shader Lang: {}",app->getDriver()->getDriverType() == EDT_DIRECT3D11? "HLSL" :"GLSL").c_str(), ELL_NONE);
	//Load in sedata which contains all the resources needed for the app including font and shaders. 
	app->getFileSystem()->addFileArchive(app->getBuiltInPath("sedata.res"), true, true, io::EFAT_ZIP);

	app->getDevice()->setWindowCaption(L"SWAT3 SCN editor");;
	
	// load basic shader
	CShaderManager* shaderMgr = CShaderManager::getInstance();
	shaderMgr->initExtremlyBasicShader();

	// load font
	CGlyphFreetype* freetypeFont = CGlyphFreetype::getInstance();
	freetypeFont->initFont("Segoe UI Light", "BuiltIn/Fonts/segoeui/segoeuil.ttf");

	//Shift the view manager layer to be init which will load in the actual scn and more.
	CViewManager::getInstance()->getLayer(0)->pushView<CViewInit>(m_arguments);
}

void SCNEdit::onUpdate()
{
	//m_scene->update();
	CViewManager::getInstance()->update();
}

void SCNEdit::onRender()
{

	CViewManager::getInstance()->render();
}

void SCNEdit::onPostRender()
{
	CViewManager::getInstance()->postRender();
}

bool SCNEdit::onBack()
{
	// on back key press
	// return TRUE will run default by OS (Mobile)
	// return FALSE will cancel BACK FUNCTION by OS (Mobile)
	return CViewManager::getInstance()->onBack();
}

void SCNEdit::onResize(int w, int h)
{
	// on window size changed
	if (CContext::getInstance() != NULL)
		CContext::getInstance()->resize(w, h);
}

void SCNEdit::onResume()
{
	// resume application
	CViewManager::getInstance()->onResume();
}

void SCNEdit::onPause()
{
	// pause application
	CViewManager::getInstance()->onPause();
}


void SCNEdit::onQuitApp()
{
	delete this;
}

///Release instance data and close the file.
void SCNEdit::proccessQuit() {
	if (CInteractionManager::getInstance()) CInteractionManager::releaseInstance();
	Sleep(250);
	CViewManager::getInstance()->getLayer(1)->destroyAllView();
	CViewManager::getInstance()->getLayer(0)->destroyAllView();
	CViewManager::getInstance()->releaseAllLayer();
	if (CViewManager::getInstance()) CViewManager::releaseInstance();
	if (UI::CUIEventManager::getInstance()) UI::CUIEventManager::releaseInstance();
	if (CImguiManager::getInstance()) CImguiManager::releaseInstance();
	closeScnFile();
}

bool SCNEdit::loadScnFile(io::path fname) {

	CBaseApp* app = getApplication();
	if (scn) 
		closeScnFile();
	s32 pos = fname.findLastChar("\\/", 2); //search for \ or /

	if (pos > -1) {
		//Changes working directory based on the postion
		app->getFileSystem()->changeWorkingDirectoryTo((fname.subString(0, pos - 1)));
		//fname=fname.subString(pos+1,fname.size()-pos);
		//clears path
		deletePathFromFilename(fname);
	}

	//Creates the backupfile if not found. Important as without this it will crash.
	std::filesystem::path backupDir = "backups";
	if (!std::filesystem::exists(backupDir)) {
		std::filesystem::create_directory(backupDir);
	}

	ifstream scnFile(fname.c_str(), ios::in | ios::binary);

	if (!scnFile.is_open()) {
		error(true, "Can't open {}, and or it doesn't exist", fname.c_str());
	}
	else {
		scn= new CScn(&scnFile); //sets scn to file
	}

	//Closes file
	scnFile.close();

	//Allows to set data to file
	output = new ofstream(fname.c_str(), ios::in | ios::binary | ios::ate);
	outputPath = fname;
	return false;
}

bool SCNEdit::saveSCN() {
	//Gets solids
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t * gui=  interaction->getGuiSettings();
	CScnLightmap* lmap = scn->getLightmap();


	io::path bakname;
	//removes any extentions related to scn and bak files
	cutFilenameExtension(bakname, outputPath);

	//Get time of when saved and store into backup
	auto now = floor<std::chrono::seconds>(std::chrono::system_clock::now());
	bakname += format("_{:%Y-%m-%d_%H-%M-%S}.scn", now).c_str();
	io::path backupPath = "backups/";
	backupPath += bakname.c_str();

	//Record texture names
	//if file is open
	if (output && output->is_open()) {
		//Save backup before do anything
		if (!copy_file(outputPath.c_str(), backupPath.c_str())) {
			error(true, "can't copy from %s to %s", outputPath.c_str(), backupPath.c_str());
		}

		for (u32 s = 0; s < scn->getSolidSize(true); s++) {
			CScnSolid* solid = scn->getSolid(s);
			//save textures to file
			for (u32 i = 0; i < solid->n_surfs; i++) {
				output->seekp(solid->surfsad[i]);
				write_generic(&solid->surfs[i], output, 72);

			}

			//save planes
			output->seekp(solid->planessad);
			u32 len = (solid->n_planes) * sizeof(scnPlane_t);
			output->write((char*)(solid->planes), len);

			//save vertices
			output->seekp(solid->vertssad);
			len = (solid->n_verts) * sizeof(core::vector3df);
			output->write((char*)(solid->verts), len);


			//save uvpos
			output->seekp(solid->uvposad);
			len = (solid->n_uvpos) * sizeof(core::vector2df);
			output->write((char*)(solid->uvpos), len);

			//Save param frame
			output->seekp(solid->paramsad);
			len = (solid->n_surfs) * sizeof(scnSurfParamFrame_t);
			output->write((char*)(solid->paramFrames), len);


			//save bb data
			for (u32 c = 0; c < solid->n_cells; c++) {
				for (u32 l = 0; l < solid->rawcells[c].leafnode.size(); l++) {
					scnCellData_t* celldata = solid->rawcells[c].leafnode[l];
					output->seekp(celldata->bbsad);
					u32 len = 2 * sizeof(core::vector3df);
					output->write((char*)celldata->bb_verts, len);
				}
			}
		}
		//save entity data
		for (u32 e = 0; e < scn->getTotalEnts(); e++) {
			CScnEnt* ent = scn->getEnt(e);
			for (u32 f = 0; f < ent->n_fields; f++)
			{
				CScnEnt::field* fi = &ent->fields[f];
				output->seekp(ent->entsad[f]);
				write_generic(fi->key, output, ent->keylengths[f]);
				write_generic(fi->value, output, ent->vallengths[f]);
			}
		}
		//scrap lightmap?
		if (gui->scrape_lightmaps && scn->getLightmap()->hasLightmaps()) {
			size_t pos = scn->getLightmap()->getOffset();
			output->close();
			filesystem::resize_file(outputPath.c_str(), pos);
		}

		os::Printer::log("Changes saved to file.");
	
		return true;
	}
	else
		//Displays error if file isn't open
		error(false, "Output file stream is not open! Make sure it's not read only.");
	return 0;
}