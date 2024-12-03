#include "pch.h"
#include "SkylichtEngine.h"

#include "SCNEdit.h"
#include "SkyBox/CSkyBox.h"
#include "Lightmapper/CLightmapper.h"


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

SCNEdit::SCNEdit(CScnArguments* options):
	m_rendering(NULL),
	m_camera(NULL),
	m_guiCamera(NULL),
	m_scene(NULL),
	m_bakeSHLighting(true)
{
	arguments = options;

}

SCNEdit::~SCNEdit()
{
	delete arguments;

}

void SCNEdit::onInitApp()
{
	CBaseApp* app = getApplication();
	app->getDevice()->setGammaRamp(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	app->getFileSystem()->addFileArchive(app->getBuiltInPath("sedata.res"), true, true, io::EFAT_ZIP);

	//printf("%S", general.c_str());
	
	app->getDevice()->setWindowCaption(L"SWAT3 SCN editor");;
	io::path filename = arguments->getSCN();
	if (!filename.empty())
		SCNEdit::loadScnFile(filename);

	// load "BuiltIn.zip" to read files inside it
	app->getFileSystem()->addFileArchive(app->getBuiltInPath("BuiltIn.zip"), false, false);
	app->getFileSystem()->addFileArchive(app->getBuiltInPath("Common.zip"), false, false);


	// load basic shader
	CShaderManager* shaderMgr = CShaderManager::getInstance();
	shaderMgr->initBasicShader();
	shaderMgr->initSGDeferredShader();
	// load font
	CGlyphFreetype* freetypeFont = CGlyphFreetype::getInstance();
	freetypeFont->initFont("Segoe UI Light", "BuiltIn/Fonts/segoeui/segoeuil.ttf");

	// create a Scene

	m_scene = new CScene();


	// create a Zone in Scene
	CZone* zone = m_scene->createZone();


	// camera 2D
	CGameObject* guiCameraObject = zone->createEmptyObject();
	m_guiCamera = guiCameraObject->addComponent<CCamera>();
	m_guiCamera->setProjectionType(CCamera::OrthoUI);

	// camera 3D
	CGameObject* camObj = zone->createEmptyObject();
	camObj->addComponent<CCamera>();
	camObj->addComponent<CEditorCamera>()->setMoveSpeed(10.0f);
	camObj->addComponent<CFpsMoveCamera>()->setMoveSpeed(50.0f);

	m_camera = camObj->getComponent<CCamera>();
	m_camera->setPosition(core::vector3df(0.0f, 0.0f, 0.0f));
	m_camera->setFOV(60.0);
	m_camera->lookAt(core::vector3df(-1.0f, 0.0f, 0.0f), core::vector3df(0.0f, 1.0f, 0.0f));


	const char* textures[6] = { "textures/Skyboxes/a_Porch_bk.png", "textures/Skyboxes/a_Porch_dn.png",
		"textures/Skyboxes/a_Porch_fr.png", "textures/Skyboxes/a_Porch_lf.png",
		"textures/Skyboxes/a_Porch_rt.png","textures/Skyboxes/a_Porch_up.png"
	};

	CSkyBox* skyBox = zone->createEmptyObject()->addComponent<CSkyBox>();
	skyBox->setTextures(textures);

	CGameObject* scnObj = zone->createEmptyObject();
	CScnMeshComponent* mesh = scnObj->addComponent<CScnMeshComponent>();
	scnObj->setStatic(true);
;	if (SCNEdit::getSCN()) {
		mesh->setMesh(SCNEdit::getSCN(), arguments);
		scnObj->getTransformEuler()->setPosition(core::vector3df(-1.0f, 0.0f, 0.0f));
	}

	m_meshes.push_back(scnObj);
	// Reflection probe

	// Rendering
	u32 w = app->getWidth();
	u32 h = app->getHeight();

	m_rendering = new CDeferredRP();
	m_rendering->initRender(w, h);
	m_rendering->enableUpdateEntity(true);

	CForwardRP* m_forwardRP = new CForwardRP(false);
	m_forwardRP->initRender(w, h);
	m_forwardRP->enableUpdateEntity(false);

	m_rendering->setNextPipeLine(m_forwardRP);
}

void SCNEdit::onUpdate()
{
	m_scene->update();
}

void SCNEdit::onRender()
{

	m_rendering->render(NULL, m_camera, m_scene->getEntityManager(), core::recti());

	CGraphics2D::getInstance()->render(m_guiCamera);
}

void SCNEdit::onPostRender()
{
	
}

bool SCNEdit::onBack()
{
	return true;
}

void SCNEdit::onResize(int w, int h)
{
	if (m_rendering != NULL)
		m_rendering->resize(w, h);
}

void SCNEdit::onResume()
{
	
}

void SCNEdit::onPause()
{

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
