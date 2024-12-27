#include "pch.h"

#include "Header/CViewInteraction.h"
#include "Header/Managers/CInteractionManager.h"
#include "Header/Managers/CContext.h"
#include "Projective/CProjective.h"
#include "Debug/CSceneDebug.h"
#include "Header/SCNEdit.h"


CViewInteraction::CViewInteraction(CScnArguments* args)
{
	m_arguments = args;
	m_rebuildRequired = false;
	m_hideSolids.clear();
	m_hideEntity.clear();
	m_hidePortal.clear();
}

CViewInteraction::~CViewInteraction()
{

}

void CViewInteraction::onInit()
{
	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();
	CScene* scene = context->getScene();
	CZone* zone = context->getActiveZone();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	bool moveable = m_arguments->isVertMoveable();
	interaction->OnKeyEvent([moveable,interaction, camera](key_pair pair) {
		if (!SCNEdit::getSCN()) return;
		if (!interaction->getSelectObj()) return;
		CContext* context = CContext::getInstance();
		CCollisionManager* collisionMgr = context->getCollisionManager();
		if (interaction->getKeyState(make_pair(KEY_KEY_H, KeyAugment::None))) {
			if (interaction->findSelectedType(SelectedType::Entity, SelectedType::Portal)) {
				collisionMgr->removeCollision(interaction->getSelectObj());
				collisionMgr->build(false);
				interaction->getSelectObj()->setVisible(false);
			}
			if (interaction->findSelectedType(SelectedType::Solid, SelectedType::SolidExtra)) {
				interaction->getSelectObj()->getComponent<CScnMeshComponent>()->hide();
				m_hideSolids.push_back(interaction->getSelectObj());
				collisionMgr->removeCollision(interaction->getSelectObj());
				collisionMgr->addComponentCollision(interaction->getSelectObj());
				collisionMgr->build(false);
			}
			if (interaction->getSelectedType() == SelectedType::Entity) {
				m_hideEntity.push_back(interaction->getSelectObj());
			}
			else if (interaction->getSelectedType() == SelectedType::Portal) {
				m_hidePortal.push_back(interaction->getSelectObj());
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_H, KeyAugment::Ctrl))) {
			
			for (int i = 0; i < m_hideEntity.size(); i++) {
				collisionMgr->addBBComponentCollision(m_hideEntity[i]);
				m_hideEntity[i]->setVisible(true);
			}
			for (int i = 0; i < m_hidePortal.size(); i++) {
				collisionMgr->addComponentCollision(m_hidePortal[i]);
				m_hidePortal[i]->setVisible(true);
			}

			for (int i = 0; i < m_hideSolids.size(); i++) {
				m_hideSolids[i]->getComponent<CScnMeshComponent>()->show();
				collisionMgr->removeCollision(m_hideSolids[i]);
				collisionMgr->addComponentCollision(m_hideSolids[i]);
			}
			collisionMgr->build(false);
			m_hideSolids.clear();
			m_hidePortal.clear();
			m_hideEntity.clear();
		}
		if (interaction->getKeyState(make_pair(pair.first, pair.second)) && 
			pair.first >= KEY_LEFT && pair.first <= KEY_DOWN) {
			int posX = 0;
			int posY = 0;
			int posZ = 0;
			if (pair.first == KEY_DOWN || pair.first == KEY_UP) {
				if(pair.second == KeyAugment::Shift) posY = 39 - pair.first;
				else posX = 39 - pair.first;
			}
			else posZ = 38- pair.first;
			if (interaction->getSelectedType() == SelectedType::Entity) {
				if (pair.second == KeyAugment::Shift || 
					pair.second == KeyAugment::None)updateEntityPos(core::vector3df(posX, posY, posZ));
			}
			if (interaction->findSelectedType(SelectedType::Solid, SelectedType::SolidExtra) && moveable) {
				if (pair.second == KeyAugment::Shift || 
					pair.second == KeyAugment::None) { 
					CScnMeshComponent* meshcomp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
					interaction->updateVertPos(meshcomp->updateVert(SCNEdit::getSCN(),interaction->getMoveableVert(), core::vector3df(posX, posY, posZ)));
					interaction->VertexCallback();
					collisionMgr->removeCollision(interaction->getSelectObj());
					collisionMgr->addComponentCollision(interaction->getSelectObj());
					collisionMgr->build(false);
				}
			}
		}
		if (interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::None))) {
			if (interaction->findSelectedType(SelectedType::Solid, SelectedType::SolidExtra)) {
				CScnMeshComponent* meshcomp = interaction->getSelectObj()->getComponent<CScnMeshComponent>();
				interaction->updateVertPos(meshcomp->resetVert(SCNEdit::getSCN(), interaction->getMoveableVert()));
				interaction->VertexCallback();
				collisionMgr->removeCollision(interaction->getSelectObj());
				collisionMgr->addComponentCollision(interaction->getSelectObj());
				collisionMgr->build(false);
			}
			if(interaction->findSelectedType(SelectedType::Entity)) updateEntityPos(core::vector3df(0, 0, 0)); //Reset
		}
	});
	if (SCNEdit::getSCN()) CViewInteraction::checkVisbility();
}

void CViewInteraction::onDestroy()
{
}

void CViewInteraction::updateEntityPos(core::vector3df addpos) {
	CContext* context = CContext::getInstance();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CScnEntityComponent* entcomp = interaction->getSelectObj()->getComponent<CScnEntityComponent>();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	CScnEnt* ent = SCNEdit::getSCN()->getEnt(interaction->getEntityISelected());
	if (addpos != core::vector3df(0, 0, 0)) {
		core::vector3df pos = convert_vec3(ent->getField("origin")) + addpos;
		ent->setField("Origin", format("{} {} {}", pos.X, pos.Y, pos.Z).c_str());
	}
	else {
		ent->setField("Origin", entcomp->getResetPos().c_str());
	}
	entcomp->updateMesh(ent);
	collisionMgr->removeCollision(interaction->getSelectObj());
	collisionMgr->addBBComponentCollision(interaction->getSelectObj());
	collisionMgr->build(false);

}

void CViewInteraction::onUpdate()
{
	CInteractionManager* interaction = CInteractionManager::getInstance();
	core::vector2df mouse = interaction->getMouse();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CScene* scene = context->getScene();
	guiSettings_t* gui = interaction->getGuiSettings();
	CSceneDebug* sceneDebug = CSceneDebug::getInstance();

	if (scene != NULL)
		scene->update();

	// get view ray from camera to mouse position
	const core::recti& vp = getVideoDriver()->getViewPort();
	if (SCNEdit::getSCN()) {
		core::line3df ray = CProjective::getViewRay(
			context->getActiveCamera(),
			mouse.X, mouse.Y,
			vp.getWidth(), vp.getHeight()
		);

		float outBestDistanceSquared = 400 * 100; // hit in 300m
		core::vector3df intersection;
		core::triangle3df triangle;

		CCollisionManager* collisionMgr = context->getCollisionManager();
		if (collisionMgr->hasNodes() && 
			collisionMgr->getCollisionPoint(ray, outBestDistanceSquared, intersection, triangle, interaction->node) 
			&& !getApplication()->getDevice()->getCursorControl()->isVisible())
		{

			CCollisionNode* node = interaction->node;
			if (interaction->isLeftClicked()) deselectAll(node->GameObject);
			//Hover over styling
			if ((str_equals(node->GameObject->getNameA(), "solid") ||
				str_equals(node->GameObject->getNameA(), "solid_extra"))) {
				sceneDebug->addTri(triangle, SColor(255, 255, 0, 255));
			}
			if (str_equals(node->GameObject->getNameA(), "entity")) {
				sceneDebug->addBoudingBox(node->Selector->getBBox(), SColor(255, 0, 0, 0));
			}
			if (str_equals(node->GameObject->getNameA(), "portal")) {
				for (int i = 0; i < node->Triangles.size(); i++) {
					sceneDebug->addTri(node->Triangles[i], SColor(255, 255, 255, 255));
				}
			}
			

			core::vector3df normal = triangle.getNormal();
			normal.normalize();

			// draw decal projection box
			core::aabbox3df box;
			float dist = context->getActiveCamera()->getPosition().getDistanceFrom(intersection) / 30.0f;
			dist = min(max(dist, .3), 6);
			core::vector3df halfBox = core::vector3df(1.0f * 0.5f, 1.0f * 0.5f, 1.0f * 0.5f);
			box.MinEdge = -halfBox * dist;
			box.MaxEdge = halfBox * dist;

			core::quaternion r1;
			r1.rotationFromTo(core::vector3df(0.0f, 1.0f, 0.0f), normal);

			core::quaternion r2;
			r2.fromAngleAxis(0.0f * core::DEGTORAD, core::vector3df(0.0f, 1.0f, 0.0f));

			core::quaternion q = r2 * r1;

			core::matrix4 bbMatrix = q.getMatrix();
			bbMatrix.setTranslation(intersection);


			sceneDebug->addTransformBBox(box, SColor(255, 0, 255, 0), bbMatrix);
			sceneDebug->addTransform(bbMatrix, 2.0f * dist);

			//Read click update interaction manager accordingly.
			if (interaction->isLeftClicked()) {

				CCube* sel =zone->searchObject(L"sel_verts")->getComponent<CCube>();
				CCube* shared = zone->searchObject(L"shared_verts")->getComponent<CCube>();
				resetSolid();
		
				if ((str_equals(node->GameObject->getNameA(), "solid") ||
						str_equals(node->GameObject->getNameA(), "solid_extra")))
				{
					CScnMeshComponent* meshcomp = node->GameObject->getComponent<CScnMeshComponent>();
					std::pair<int, int> surfdata = meshcomp->select(SCNEdit::getSCN(), triangle,
						interaction->getLeftClickAug() == KeyAugment::Ctrl);
				
					interaction->setSurfISelected(surfdata);
					if (surfdata.second != -1) {
						if (str_equals(node->GameObject->getNameA(), "solid"))
							interaction->setSelectedType(SelectedType::Solid);
						else interaction->setSelectedType(SelectedType::SolidExtra);
						
						interaction->setVertData(meshcomp->getVertices(SCNEdit::getSCN()));
						for (int i = 0; i < interaction->getVerts().size(); i++) {
							sel->addPrimitive(interaction->getVerts()[i].pos, core::vector3df(0, 0, 0), core::vector3df(3, 3, 3));
						}
						for (int i = 0; i < interaction->getSharedVerts().size(); i++) {
							shared->addPrimitive(interaction->getSharedVerts()[i].pos, core::vector3df(0, 0, 0), core::vector3df(3, 3, 3));
						}
					}
					else interaction->setSelectedType(SelectedType::Empty);
			
				}

				if (str_equals(node->GameObject->getNameA(), "entity")) {
					int entindx = node->GameObject->getComponent<CScnEntityComponent>()->select();
					interaction->setEntityISelected(entindx);
					if (entindx != -1)interaction->setSelectedType(SelectedType::Entity);
					else interaction->setSelectedType(SelectedType::Empty);

				}
				if (str_equals(node->GameObject->getNameA(), "portal")) {
					std::pair<int, int> portaldata = node->GameObject->getComponent<CScnPortalComponent>()->select();
					interaction->setPortalISelected(portaldata);
					if (portaldata.second != -1)interaction->setSelectedType(SelectedType::Portal);
					else interaction->setSelectedType(SelectedType::Empty);
				}
				interaction->setSelectObj(node->GameObject);
				interaction->CollisionCallback(triangle, intersection);
			}
			getClosestVertex(intersection);
			interaction->resetLeftClick();
		}

		

		if (interaction->isGuiSettingsUpdated()) {
			CViewInteraction::checkVisbility();
			if (m_arguments->isLightmapEnable() && SCNEdit::getSCN()->getLightmap()->hasLightmaps()) {
				zone->searchObject(L"solid")->getComponent<CScnMeshComponent>()->setLightmapVisible(gui->vis_lightmaps);
			}
			interaction->resetPrevGui();
		}
	}
}

void CViewInteraction::onRender()
{
	CContext* context = CContext::getInstance();

	CCamera* camera = context->getActiveCamera();
	CCamera* guiCamera = context->getGUICamera();

	CScene* scene = context->getScene();

	// render scene
	if (camera != NULL && scene != NULL)
	{
		context->getRenderPipeline()->render(NULL, camera, scene->getEntityManager(), core::recti());
	}

	// render GUI
	if (guiCamera != NULL)
	{
		CGraphics2D::getInstance()->render(guiCamera);
	}

}




void CViewInteraction::deselectAll(CGameObject* current) {
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CGameObject* solidObj = zone->searchObject(L"solid");
    if(solidObj !=current)solidObj->getComponent<CScnMeshComponent>()->deselect();
	CContainerObject* group_extra = (CContainerObject*)zone->searchObject(L"group_extra");
	if (group_extra) {
		for (int i = 0; i < group_extra->getChilds()->size(); i++) {
			if (group_extra->getChilds()->at(i) != current)
				group_extra->getChilds()->at(i)->getComponent<CScnMeshComponent>()->deselect();
		}
	}
	CContainerObject* group_entity = (CContainerObject*)zone->searchObject(L"group_entity");
	if (group_entity) {
		for (int i = 0; i < group_entity->getChilds()->size(); i++) {
			if (group_entity->getChilds()->at(i) != current)
				group_entity->getChilds()->at(i)->getComponent<CScnEntityComponent>()->deselect();
		}
	}

	CContainerObject* group_portal = (CContainerObject*)zone->searchObject(L"group_portal");

	if (group_portal) {
		core::array<CGameObject*>objs = group_portal->getArrayChilds(false);
		for (int i = 0; i < objs.size(); i++) {
			if (str_equals(objs[i]->getNameA(), "portal")) {
				if (objs[i] != current)objs[i]->getComponent<CScnPortalComponent>()->deselect();
			}
		}
	}

}

void CViewInteraction::onPostRender()
{

}
void CViewInteraction::updateVisbility(CGameObject* obj, bool state, bool bbCollision) {
	CContext* context = CContext::getInstance();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	if (obj->isVisible() && !state) {
		collisionMgr->removeCollision(obj);
		m_rebuildRequired = true;
	}
	if (!obj->isVisible() && state) {
		if (bbCollision) collisionMgr->addBBComponentCollision(obj);
		else collisionMgr->addComponentCollision(obj);
		m_rebuildRequired = true;
	}

	obj->setVisible(state);

}

void CViewInteraction::checkVisbility()
{
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();

	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	if (gui->vis_scn || gui->vis_doors) {
		for (int i = 0; i < m_hideSolids.size(); i++) {
			m_hideSolids[i]->getComponent<CScnMeshComponent>()->show();
		}

		m_hideSolids.clear();
	}
	if( !gui->vis_scn && interaction->getSelectedType() == SelectedType::Solid) {
		resetSolid();
	}
	if (!gui->vis_doors && interaction->getSelectedType() == SelectedType::SolidExtra) {
		resetSolid();
	}
	updateVisbility(zone->searchObject(L"solid"), gui->vis_scn, false);

	CContainerObject* group_extra = (CContainerObject*)zone->searchObject(L"group_extra");
	if (group_extra) {
		for (int i = 0; i < group_extra->getChilds()->size(); i++) {
			updateVisbility(group_extra->getChilds()->at(i), gui->vis_doors, false);
		}
	}
	CContainerObject* group_portal = (CContainerObject*)zone->searchObject(L"group_portal");

	if (group_portal) {
		core::array<CGameObject*>objs = group_portal->getArrayChilds(false);
		for (int i = 0; i < objs.size(); i++) {
			if (str_equals(objs[i]->getNameA(), "portal")) {
				updateVisbility(objs[i], gui->vis_portals, true);
			}
		}
	}
	CContainerObject* group_bb = (CContainerObject*)zone->searchObject(L"group_bb");
	if (group_bb) {
		for (int i = 0; i < group_bb->getChilds()->size(); i++) {
			updateVisbility(group_bb->getChilds()->at(i), gui->vis_bb, false);
		}
	}
	CContainerObject* group_entity = (CContainerObject*)zone->searchObject(L"group_entity");
	if (group_entity) {
		for (int i = 0; i < group_entity->getChilds()->size(); i++) {
			updateVisbility(group_entity->getChilds()->at(i), gui->vis_entities, true);
		}
	}

	if (m_rebuildRequired) {
		
		if(collisionMgr->hasNodes())collisionMgr->build(false);
		m_rebuildRequired = false;
	}

}
void CViewInteraction::resetSolid() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CCube* sel = zone->searchObject(L"sel_verts")->getComponent<CCube>();
	CCube* shared = zone->searchObject(L"shared_verts")->getComponent<CCube>();
	CSphere* sphere = zone->searchObject(L"vert_over")->getComponent< CSphere>();
	sphere->removeAllEntities();
	sel->removeAllEntities();
	shared->removeAllEntities();
	interaction->resetSelectObj();
	interaction->resetVerts();
}

void CViewInteraction::getClosestVertex(core::vector3df pos) {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	if (interaction->getSharedVerts().size() > 0 || interaction->getVerts().size() > 0) {
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	indexedVec3df_t closest;
	float minDistanceSq = FLT_MAX;
	for (int i = 0; i < interaction->getVerts().size(); i++) {
		core::vector3df point =interaction->getVerts()[i].pos;
		float distanceSq = pos.getDistanceFromSQ(point); // Squared distance
		if (distanceSq < minDistanceSq) {
			minDistanceSq = distanceSq;
			closest = interaction->getVerts()[i];

		}
	}
	for (int i = 0; i < interaction->getSharedVerts().size(); i++) {
		core::vector3df point = interaction->getSharedVerts()[i].pos;
		float distanceSq = pos.getDistanceFromSQ(point); // Squared distance
		if (distanceSq < minDistanceSq) {
			minDistanceSq = distanceSq;
			closest = interaction->getSharedVerts()[i];
		}
	}
	CSphere* sphere= zone->searchObject(L"vert_over")->getComponent< CSphere>();
		sphere->removeAllEntities();
		sphere->addPrimitive(closest.pos, core::vector3df(0, 0, 0), core::vector3df(3, 3, 3));

		if (closest.pos != interaction->getMoveableVert().pos) {
			interaction->setMoveableVert(closest);
			interaction->VertexCallback();
		}
	}
}