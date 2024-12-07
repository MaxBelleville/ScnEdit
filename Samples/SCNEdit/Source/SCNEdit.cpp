#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/SCNEdit.h"


//This class loads in cmd line arguments, sets up view manager and loads in scn from f
using namespace std;

void prepareApplication(const std::vector<std::string>& argv, SIrrlichtCreationParameters* param) {
	getApplication()->showDebugConsole();
	CScnArguments* options = new CScnArguments(argv);
	SCNEdit* app = new SCNEdit(options);
	param->DriverType = options->getDriverType();
	if (options->getDriverType() != EDT_DIRECT3D11) {
		param->AntiAlias = 0;
	}
	param->Fullscreen = options->isFullscreen();
	param->WindowSize = options->isFullscreen() ? options->getDesktopRes() : options->getRes();
	param->Borderless = options->isBorderless();
	param->Resizeable = options->isResizeable();


	getApplication()->registerAppEvent("SWAT3 SCN editor", app);
}

SCNEdit::SCNEdit(CScnArguments* options)
{
	arguments = options;
	CContext::createGetInstance();
	CViewManager::createGetInstance()->initViewLayer(1);
	CLightmapper::createGetInstance();

}

SCNEdit::~SCNEdit()
{
	delete arguments;

	CViewManager::releaseInstance();
	CContext::releaseInstance();
	
}

void SCNEdit::onInitApp()
{
	CViewManager::getInstance()->getLayer(0)->pushView<CViewInit>(arguments);
	//CBaseApp* app = getApplication();
	//app->getDevice()->setGammaRamp(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	//app->getFileSystem()->addFileArchive(app->getBuiltInPath("sedata.res"), true, true, io::EFAT_ZIP);

	////printf("%S", general.c_str());
	//
	//app->getDevice()->setWindowCaption(L"SWAT3 SCN editor");;
	//

	//// load basic shader
	//CShaderManager* shaderMgr = CShaderManager::getInstance();
	//shaderMgr->initExtremlyBasicShader();

	//// load font
	//CGlyphFreetype* freetypeFont = CGlyphFreetype::getInstance();
	//freetypeFont->initFont("Segoe UI Light", "BuiltIn/Fonts/segoeui/segoeuil.ttf");

	//// create a Scene

	//m_scene = new CScene();


	//// create a Zone in Scene
	//CZone* zone = m_scene->createZone();


	//// camera 2D
	//CGameObject* guiCameraObject = zone->createEmptyObject();
	//m_guiCamera = guiCameraObject->addComponent<CCamera>();
	//m_guiCamera->setProjectionType(CCamera::OrthoUI);

	//// camera 3D
	//CGameObject* camObj = zone->createEmptyObject();
	//camObj->addComponent<CCamera>();


	//CEditorCamera * editorCam = camObj->addComponent<CEditorCamera>();
	//editorCam->setControlStyle(CEditorCamera::EControlStyle::FPS);
	//editorCam->setMoveSpeed(7.0f);
	//editorCam->setZoomSpeed(7.0f);
	//CFpsMoveCamera* fpsMoveCam = camObj->addComponent<CFpsMoveCamera>();
	//fpsMoveCam->setMoveSpeed(75.0f);
	//fpsMoveCam->setShiftSpeed(2.5f);
	//
	//m_camera = camObj->getComponent<CCamera>();

	//m_camera->setPosition(core::vector3df(0.0f, 0.0f, 0.0f));
	//m_camera->setFOV(arguments->getFov());
	//m_camera->setFarValue(arguments->getViewDist() * 100.0f);
	//m_camera->lookAt(core::vector3df(-1.0f, 0.0f, 0.0f), core::vector3df(0.0f, 1.0f, 0.0f));

	//io::path filename = arguments->getSCN();
	//if (!filename.empty())
	//	SCNEdit::loadScnFile(filename);


	//CGameObject* sky = zone->createEmptyObject();
	//m_skyBox = sky->addComponent<CSkyBox>();
	//CScnEnt* ent = scn->getCell(0);
	//const char* skyName = ent->getField("SkyboxName");
	//if (skyName) {

	//	core::array<ITexture*> textures = get_skybox(skyName);
	//	if (textures.size() == 6) {
	//		m_skyBox->setTextures(textures.pointer());
	//	}
	//}
	//sky->setStatic(true);
	//
	//if (scn) {
	//	CGameObject* scnObj = zone->createEmptyObject();
	//	CScnMeshComponent* mesh = scnObj->addComponent<CScnMeshComponent>();
	//	mesh->setMesh(scn, arguments);
	//	scnObj->setStatic(true);
	//	m_meshes.push_back(scnObj);

	//	if (arguments->isPortal()) {
	//		CGameObject* portalObj = zone->createEmptyObject();
	//		CScnPortalComponent* mesh = portalObj->addComponent<CScnPortalComponent>();
	//		mesh->setMesh(scn);
	//		scnObj->setStatic(true);

	//		m_meshes.push_back(portalObj);
	//	}
	//	if (arguments->isBBVisible()) {
	//		CGameObject* bbObj = zone->createEmptyObject();
	//		CScnCellBBComponent* mesh = bbObj->addComponent<CScnCellBBComponent>();
	//		mesh->setMesh(scn);
	//		scnObj->setStatic(true);

	//		m_meshes.push_back(bbObj);
	//	}
	//	if (arguments->isEntityVisible()) {
	//		CGameObject* enitites = zone->createEmptyObject();
	//		CScnEntityComponent* mesh = enitites->addComponent<CScnEntityComponent>();
	//		mesh->setMesh(scn);
	//		scnObj->setStatic(true);

	//		m_meshes.push_back(enitites);
	//	}

	//}
	//
	//// Rendering
	//u32 w = app->getWidth();
	//u32 h = app->getHeight();

	//m_rendering = new CDeferredSimpleRP();
	//m_rendering->initRender(w, h);
	//m_rendering->enableUpdateEntity(true);

	//CForwardRP* m_forwardRP = new CForwardRP(false);
	//m_forwardRP->initRender(w, h);
	//m_forwardRP->enableUpdateEntity(false);

	//m_rendering->setNextPipeLine(m_forwardRP);
}

void SCNEdit::onUpdate()
{
	//m_scene->update();
	CViewManager::getInstance()->update();
}

void SCNEdit::onRender()
{

	//m_rendering->render(NULL, m_camera, m_scene->getEntityManager(), core::recti());
	CViewManager::getInstance()->render();
	//CGraphics2D::getInstance()->render(m_guiCamera);
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
	// end application
	delete this;
}


bool SCNEdit::loadScnFile(io::path fname) {
	CBaseApp* app = getApplication();
	s32 pos = fname.findLastChar("\\/", 2); //search for \ or /
	if (pos > -1) {
		//Changes working directory based on the postion
		app->getFileSystem()->changeWorkingDirectoryTo((fname.subString(0, pos - 1)));
		//fname=fname.subString(pos+1,fname.size()-pos);
		//clears path
		deletePathFromFilename(fname);
	}
	io::path bakname;
	//removes any extentions related to scn and bak files
	cutFilenameExtension(bakname, fname);
	//Adds .bak
	bakname += ".bak";
	ifstream scnFile(fname.c_str(), ios::in | ios::binary);

	if (scnFile.is_open()) {
		if (!CopyFile_(fname.c_str(), bakname.c_str()))
			error(true, "can't copy from %s to %s", fname.c_str(), bakname.c_str());
	}
	if (!scnFile.is_open()) {
		error(true, "Can't open %s, and or it doesn't exist", fname.c_str());
	}
	else {
		scn= new CScn(&scnFile); //sets scn to file
	}
	//Closes file
	scnFile.close();

	//Allows to set data to file
	output = new ofstream(fname.c_str(), ios::in | ios::binary | ios::ate);

	return false;
}
