#include "pch.h"
#include "Header/CViewInit.h"
#include "Header/CViewInteraction.h"
#include "Header/CViewGui.h"
#include "SkySun/CSkySun.h"
#include "Header/Managers/CContext.h"
#include "Header/Managers/CInteractionManager.h"
#include "CImguiManager.h"

CViewInit::CViewInit(CScnArguments* args) :
	m_initState(CViewInit::Start),
	m_camera(NULL),
	m_font(NULL),
	m_guiCamera(NULL),
	m_guiObject(NULL),
	m_skyBox(NULL),
	m_skyObject(NULL),
	m_textInfo(NULL)
{
	m_start = os::Timer::getRealTime();
	
	m_arguments = args;
}

CViewInit::~CViewInit()
{
}

io::path CViewInit::getBuiltInPath(const char* name)
{
	return getApplication()->getBuiltInPath(name);
}

void CViewInit::onInit()
{
	CScene* prevScene = CContext::getInstance()->getScene();
	if(prevScene) CContext::getInstance()->releaseScene();
	CScene* scene = CContext::getInstance()->initScene();

	CZone* zone = scene->createZone();

	m_guiObject = zone->createEmptyObject();
	CCanvas* canvas = m_guiObject->addComponent<CCanvas>();

	m_font = new CGlyphFont();
	m_font->setFont("Segoe UI Light", 25);

	// create text
	m_textInfo = canvas->createText(m_font);
	m_textInfo->setTextAlign(EGUIHorizontalAlign::Center, EGUIVerticalAlign::Middle);
	m_textInfo->setText("Starting...");

	// crate gui camera
	CGameObject* guiCameraObject = zone->createEmptyObject();
	m_guiCamera = guiCameraObject->addComponent<CCamera>();
	m_guiCamera->setProjectionType(CCamera::OrthoUI);
	CContext::getInstance()->setGUICamera(m_guiCamera);
}

void CViewInit::initScene()
{
	CBaseApp* app = getApplication();

	CContext* context = CContext::getInstance();

	CScene* scene = context->getScene();
	CZone* zone = scene->getZone(0);

	// camera
	CGameObject* camObj = zone->createEmptyObject();

	m_camera = camObj->addComponent<CCamera>();
	CEditorCamera * editorCam = camObj->addComponent<CEditorCamera>();
	editorCam->setControlStyle(CEditorCamera::EControlStyle::FPS);
	editorCam->setMoveSpeed(8.0f);
	editorCam->setRotateSpeed(24.0f);
	editorCam->setZoomSpeed(8.0f);
	editorCam->setInvert(m_arguments->isMouseInvert());
	CFpsMoveCamera* fpsMoveCam = camObj->addComponent<CFpsMoveCamera>();
	fpsMoveCam->setMoveSpeed(150.0f);
	fpsMoveCam->setShiftSpeed(2.5f);

	
	m_camera->setFOV(m_arguments->getFov());
	m_camera->setFarValue(m_arguments->getViewDist() * 100.0f);
	m_camera->setPosition(core::vector3df(0.0f, 4.0f, 0.0f));
	m_camera->lookAt(core::vector3df(0.0f, 4.0f, 1.0f), core::vector3df(0.0f, 1.0f, 0.0f));
	
	// gui camera
	CGameObject* guiCameraObj = zone->createEmptyObject();
	guiCameraObj->addComponent<CCamera>();
	CCamera* guiCamera = guiCameraObj->getComponent<CCamera>();
	guiCamera->setProjectionType(CCamera::OrthoUI);
	// sky
	CSkySun* skySun = zone->createEmptyObject()->addComponent<CSkySun>();

	// sky
	m_skyObject = zone->createEmptyObject();
	m_skyObject->setName("skybox");
	m_skyBox = m_skyObject->addComponent<CSkyBox>();
	zone->registerObjectInSearchList(m_skyObject);
	m_skyObject->setVisible(false);
	//m_skyObject->setStatic(true);
	// save to context


	initShapeCollection<CCube>(SColor(255, 255, 0, 0), "surf_verts");
	initShapeCollection<CCube>(SColor(255, 255, 0, 255), "shared_verts");
	initShapeCollection<CCube>(SColor(255, 0, 0, 0), "hover_vert");
	initShapeCollection<CCube>(SColor(255, 255, 255, 255), "sel_vert");

	context->initSimpleRenderPipeline(app->getWidth(), app->getHeight());
	context->setActiveZone(zone);
	context->setActiveCamera(m_camera);
	context->setGUICamera(guiCamera);
}
template<class T>
void CViewInit::initShapeCollection(SColor color, const char* name) {
	CContext* context = CContext::getInstance();

	CScene* scene = context->getScene();
	CZone* zone = scene->getZone(0);
	CGameObject* shapeObj = zone->createEmptyObject();
	T* shape = shapeObj->addComponent<T>();
	shape->setColor(color);
	shape->removeAllEntities();
	shapeObj->setName(name); //hover over 
	zone->registerObjectInSearchList(shapeObj);
}

void CViewInit::buildScnComponents() {
	
	CScn* scn = SCNEdit::getSCN();
	
	CContext* context = CContext::getInstance();

	CScene* scene = context->getScene();
	CZone* zone = scene->getZone(0);
	CScnSolid* solid = scn->getSolid(0);
	

	const char* skyName ="";
	for (int i = 0; solid->n_cells; i++) {
		if (!str_equiv(solid->rawcells[i].skyname,"")) {
			skyName = solid->rawcells[i].skyname;
			break;
		}
	}
	if (skyName) {
		core::array<ITexture*> textures = get_skybox(skyName);
		if (textures.size() == 6) {
			m_skyObject->setVisible(true);
			m_skyBox->setTextures(textures.pointer());
		}
	}
	
	//Structure of scn components goes as follows
	//Zone->Body
	//Body->Solid
	//Body->ExtraGroup->Extra(doors etc)
	//Body->PortalGroup->PortalCellGroup->Portals
	//Body->EntityGroup->Entities.

	//Define body, solid group and solid.

	CContainerObject* body = zone->createContainerObject();
	body->setName("body");
	zone->registerObjectInSearchList(body);

	CGameObject* scnObj = body->createEmptyObject();
	scnObj->setName("solid");
	zone->registerObjectInSearchList(scnObj);
	CScnMeshComponent* mesh = scnObj->addComponent<CScnMeshComponent>();
	mesh->setMesh(scn, solid, m_arguments);
	//scnObj->setStatic(true);
	context->getCollisionManager()->addComponentCollision(scnObj);

	//Define components, objects and groups needed in extras
	CContainerObject* extraGroup = body->createContainerObject();
	extraGroup->setName("group_extra");
	if (m_arguments->isExtrasEnabled()) {
		for (u32 s = 1; s < scn->getSolidSize(true); s++) {
			CScnSolid* solid = scn->getSolid(s);

			CGameObject* extra = extraGroup->createEmptyObject();
			extra->setName("solid_extra");

			CScnMeshComponent* mesh = extra->addComponent<CScnMeshComponent>();
			mesh->setMesh(scn, solid, m_arguments);
		//	scnObj->setStatic(true);
			context->getCollisionManager()->addComponentCollision(extra);
		}
	}
	zone->registerObjectInSearchList(extraGroup);
	CContainerObject* decalsGroup = body->createContainerObject();
	decalsGroup->setName("group_decals");
	if (m_arguments->isDecalEnabled()) {
		scene->update();
		scene->getEntityManager()->update();
		context->getCollisionManager()->build();
		for (u32 i = 0; i < scn->getTotalEnts(); i++) {
			CScnEnt* ent = scn->getEnt(i);
			const char* originStr = ent->getField("origin");
			const char* className = ent->getField("classname");
			if (str_equals(className, "infodecal")) {

				const char* rotation = ent->getField("angle");
				float rot = 0.0f;
				if (rotation) rot = stof(rotation) * (core::PI / 180);
		
				core::array<ITexture*> textures = get_decals(ent->getField("Sprite"));
				if (textures.size() > 0) {
					CGameObject* obj = decalsGroup->createEmptyObject();
					obj->setName("decals");
					CDecals* decal = obj->addComponent<CDecals>();
					decal->setTexture(textures[0]);
					decal->addDecal(
						convert_vec3(originStr),
						core::vector3df(10.f, 7.0f, 10.f),
						core::vector3df(0.0f, 1.0f, 0.0f),
						0.0,
						0.0f,
						0.1f);

					decal->bake(context->getCollisionManager());
					decalsGroup->registerObjectInSearchList(obj);
				}
				
			}
		}
	}
	zone->registerObjectInSearchList(decalsGroup);
	
	//Define components, objects and groups needed in portals
	CContainerObject* portalGroup = body->createContainerObject();
	portalGroup->setName("group_portal");
	if (m_arguments->isPortalEnabled()) {
		for (u32 s = 0; s < scn->getSolidSize(m_arguments->isExtrasEnabled()); s++) { //Should only exist when solid= 0 but you never know
			CScnSolid* solid = scn->getSolid(s);
			for (u32 c = 0; c < solid->n_cells; c++) {
				CContainerObject* portalCellGroup = portalGroup->createContainerObject();
				portalCellGroup->setName("cellgroup_portal");

				for (u32 p = 0; p < solid->rawcells[c].n_portals; p++) {

					CGameObject* portalObj = portalCellGroup->createEmptyObject();
					portalObj->setName("portal");

					CScnPortalComponent* mesh = portalObj->addComponent<CScnPortalComponent>();
					mesh->setMesh(solid, c, p);
					portalObj->setStatic(true);
					context->getCollisionManager()->addBBComponentCollision(portalObj);
				}
				portalGroup->registerObjectInSearchList(portalCellGroup);
			}
		}
	}
	zone->registerObjectInSearchList(portalGroup);
	//Define components, objects and groups needed in bounding boxes
	CContainerObject* bbGroup = body->createContainerObject();
	bbGroup->setName("group_bb");
	if (m_arguments->isBBEnabled()) {
		for (u32 s = 0; s < scn->getSolidSize(m_arguments->isExtrasEnabled()); s++) {//Should only exist when solid= 0 but you never know
			CScnSolid* solid = scn->getSolid(s);
			for (u32 c = 0; c < solid->n_cells; c++) {

				CGameObject* bbObj = bbGroup->createEmptyObject();
				bbObj->setName("bb");

				CScnCellBBComponent* mesh = bbObj->addComponent<CScnCellBBComponent>();
				mesh->setMesh(solid, c);
				bbObj->setStatic(true);
				//context->getCollisionManager()->addBBComponentCollision(bbObj);

			}
		}
	}
	zone->registerObjectInSearchList(bbGroup);
	//Define components, objects and groups needed in entities
	CContainerObject* entityGroup = body->createContainerObject();
	entityGroup->setName("group_entity");
	if (m_arguments->isEntityEnabled()) {
		
		for (u32 i = 0; i < scn->getTotalEnts(); i++) {
			CScnEnt* ent = scn->getEnt(i);
			const char* originStr = ent->getField("origin");
			const char* className = ent->getField("classname");

			if (originStr && className && !str_equiv(className, "Door")) {
				CGameObject* entity = entityGroup->createEmptyObject();
				entity->setName("entity");
				CScnEntityComponent* mesh = entity->addComponent<CScnEntityComponent>();
				mesh->setMesh(ent);
				entity->setStatic(true);
				context->getCollisionManager()->addBBComponentCollision(entity);
			}
		}
	
	}
	zone->registerObjectInSearchList(entityGroup);
 
	
}

void CViewInit::onDestroy()
{

	m_guiObject->remove();
	delete m_font;
}

void CViewInit::onUpdate()
{
	CContext* context = CContext::getInstance();
	
	switch (m_initState)
	{
	case CViewInit::Start:
	{
		io::IFileSystem* fileSystem = getApplication()->getFileSystem();
		m_textInfo->setText("Loading Swat Files and Assets...");


		if (m_arguments->hasError()) {
			m_textInfo->setText("Error... check console.");
			m_initState = CViewInit::Error;
		}
		else m_initState = CViewInit::ReadScn;

	}
	break;
	case CViewInit::ReadScn:
	{

		io::path filename = m_arguments->getSCNPath();
		if (!filename.empty()) {
			SCNEdit::loadScnFile(filename);
			m_arguments->setScnLoaded(true);
			m_textInfo->setText("Initializing Mission Scene...");
		}
		else {
			//No file found...
			m_textInfo->setText("Initializing Empty Scene...");
		}
		m_initState = CViewInit::InitScene;
		
	}
	break;
	case CViewInit::InitScene:
	{
		initScene();
		if (SCNEdit::getSCN()) {
			m_initState = CViewInit::BuildScnComponents;
			m_textInfo->setText("Building SCN Mesh...");
		}
		else {
			
			m_initState = CViewInit::Finished;
			m_textInfo->setText("Completed Init... Updating Viewport");
		}
	}
	break;
	case CViewInit::BuildScnComponents:
	{
		buildScnComponents();
		m_initState = CViewInit::Finished;
		m_textInfo->setText("Completed Build... Updating Viewport");
		Sleep(1500);
	}
	break;

	case CViewInit::Error:
	{
		
		os::Printer::log(m_arguments->getErrorText(), irr::ELL_ERROR);
		
		system("pause");
		CInteractionManager::releaseInstance();
		CImguiManager::releaseInstance();
		CViewManager::getInstance()->getLayer(1)->destroyAllView();
		CViewManager::getInstance()->getLayer(0)->destroyAllView();
		CViewManager::getInstance()->releaseAllLayer();
		CViewManager::releaseInstance();
		if (UI::CUIEventManager::getInstance()) {
			UI::CUIEventManager::releaseInstance();
		}
		
		delete m_arguments;
		exit(1337);
	}
	break;
	default:
	{
		CScene* scene = context->getScene();
		if (scene != NULL) {
			scene->update();
			scene->getEntityManager()->update();
			if(SCNEdit::getSCN())context->getCollisionManager()->build();
		}

		os::Printer::log(format("Loaded everything in: {:3f} seconds", (os::Timer::getRealTime() - m_start) / 1000.0).c_str());
		CViewManager::getInstance()->getLayer(0)->changeView<CViewInteraction>(m_arguments);
		CViewManager::getInstance()->getLayer(1)->pushView<CViewGui>(m_arguments);
	}
	break;
	}
}

void CViewInit::onRender()
{
	CCamera* guiCamera = CContext::getInstance()->getGUICamera();
	CGraphics2D::getInstance()->render(guiCamera);
}

