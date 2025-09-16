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
	CContext* context = CContext::getInstance();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	collisionMgr->clear();
	m_hidePortal.clear();
	m_hideEntity.clear();
	m_hideSolids.clear();
	m_rebuildRequired = false;

}

void CViewInteraction::onInit()
{
	//On the start set the camera pos according to swat start orgin.
	//Also get the needed values and singletons used later.


	CContext* context = CContext::getInstance();
	CCamera* camera = context->getActiveCamera();

	CInteractionManager* interaction = CInteractionManager::getInstance();
	CScn* scn = SCNEdit::getSCN();

	if (scn) {
		if (!scn->swt_start) 
			camera->setPosition(core::vector3df(0.0f, 4.0f, 0.0f));
		else {
			const char* str = scn->swt_start->getField("Origin");

			core::vector3df tmp = convert_vec3(str) + core::vector3df(0,4,0);
			camera->setPosition(tmp);

		}
	}


	interaction->OnKeyEvent([interaction](key_pair pair) {
		//If no scn or no selected item then ignore key input for interactions.
		CScn* scn = SCNEdit::getSCN();
		if (!scn) return;
	
		//Only really care when key is down
		if (!interaction->getKeyState(make_pair(pair.first, pair.second)))
			return;


		//Get singletons and other values 
		CContext* context = CContext::getInstance();
		CCamera* camera = context->getActiveCamera();
		CZone* zone = context->getActiveZone();
		CCollisionManager* collisionMgr = context->getCollisionManager();
		CGameObject* selected = interaction->getSelectObj();
		core::vector3df pos = core::vector3df(0);
	
		// If ctrl + h pressed then unhide all items. (Goes through each item and sets visible and adds collision
		if (interaction->getKeyState(make_pair(KEY_KEY_H, KeyAugment::Ctrl))) {
			for (int i = 0; i < m_hideEntity.size(); i++) {
				m_hideEntity[i]->setVisible(true);
				collisionMgr->addBBComponentCollision(m_hideEntity[i]);
			}

			for (int i = 0; i < m_hidePortal.size(); i++) {
				m_hidePortal[i]->setVisible(true);
				collisionMgr->addComponentCollision(m_hidePortal[i]);
			}

			for (int i = 0; i < m_hideSolids.size(); i++) {
				m_hideSolids[i]->getComponent<CScnMeshComponent>()->show();
				collisionMgr->removeCollision(m_hideSolids[i]);
				collisionMgr->addComponentCollision(m_hideSolids[i]);
			}

			collisionMgr->build(false);
			m_hideSolids.clear(); m_hidePortal.clear(); m_hideEntity.clear();
			interaction->resetKeyState();
		}
		if (!interaction->getSelectObj()) return;


		// If h or shift h pressed then hide the currently selected item(remove collision as well)
		if (interaction->findKeyState({ make_pair(KEY_KEY_H, KeyAugment::None),make_pair(KEY_KEY_H, KeyAugment::Shift)})) {
			//Hide entity and portal
			if (interaction->selTypeLayer().find(SelectedType::Entity, SelectedType::Portal)) {
				selected->setVisible(false);

				collisionMgr->removeCollision(selected);
				collisionMgr->build(false);
			}
			//Hide solid and extras
			if (interaction->selTypeLayer().find(SelectedType::Solid, SelectedType::SolidExtra)) {
				
				selected->getComponent<CScnMeshComponent>()->hide(pair.second == KeyAugment::Shift);
				m_hideSolids.push_back(selected);
				
			
				//The only way to update the solid surf is currnetly to remove the collision, then readd it (with hidden section)
				collisionMgr->removeCollision(selected);
				collisionMgr->addComponentCollision(selected);
				collisionMgr->build(false);
				
				//Reset solid data/vertices and update gui callback.
				interaction->UICallback();
				interaction->resetLeftClick();
				deselectAll(NULL);
				resetSolid();

			}
			//add portal and entity to hideEntity list.
			if (interaction->selTypeLayer().get() == SelectedType::Entity)
				m_hideEntity.push_back(selected);
			else if (interaction->selTypeLayer().get() == SelectedType::Portal)
				m_hidePortal.push_back(selected);
		}
	
		//Update pos based on arrow keys and agument.

		if (pair.first == KEY_DOWN || pair.first == KEY_UP) {
			if (pair.second == KeyAugment::Ctrl) pos.Y = 39 - pair.first;
			else pos.X = 39 - pair.first;
		}
		else if (pair.first == KEY_RIGHT || pair.first == KEY_LEFT) pos.Z = 38 - pair.first;

		//Since Key left = 37, key up = 38, key right = 39 and key down = 40 can get all arrow keys
		//And update position for entity(reason why shift or ctrl is that it's consistent with moving vert.
		if (interaction->selTypeLayer().get() == SelectedType::Entity) {
			if (pair.first >= KEY_LEFT && pair.first <= KEY_DOWN) {
				if (interaction->keyAugLayer().find(KeyAugment::Ctrl, KeyAugment::Shift))
					updateEntityPos(pos);
			}
			//Reset entity position.
			if (interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::Ctrl)))
				updateEntityPos(core::vector3df(0)); //Reset
		}

		//Make it so the next portion is solid only interactions (simplifies compares)
		if (!interaction->selTypeLayer().find(SelectedType::Solid, SelectedType::SolidExtra))
			return;

		
		CScnMeshComponent* meshcomp = selected->getComponent<CScnMeshComponent>();
		UVMode uvmode = interaction->getUVMode();
		float uvscale = interaction->getUVScalar();
		
		//Move or reset uv.
		if(pair.first >= KEY_LEFT && pair.first <= KEY_DOWN && pair.second == KeyAugment::None)
			meshcomp->updateUV(scn, uvmode, core::vector2df(pos.Z, pos.X) * uvscale);

		if (interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::None)))
			meshcomp->resetUV(scn);

		//Move solid vertex and then update bounding box
		if (pair.first >= KEY_LEFT && pair.first <= KEY_DOWN && interaction->keyAugLayer().find(KeyAugment::Ctrl, KeyAugment::Shift)) {
			if (interaction->hasMoveableVert()) {
				meshcomp->updateVert(scn, interaction->getMoveableVert(), pos);
				moveVertNBounds(false);

			}
		}
		//Reset vert (only works for current game when you save you cant reset)
		//Might change in future to have a history using imgui.ini or something idk.
		if (interaction->getKeyState(make_pair(KEY_KEY_R, KeyAugment::Ctrl))) {
			if (interaction->hasMoveableVert()) {
				meshcomp->resetVert(scn, interaction->getMoveableVert());
				moveVertNBounds(true);

			}
		}

		if (interaction->getKeyState(make_pair(KEY_KEY_X, KeyAugment::None))) 
			meshcomp->updateUV(scn,UVMode::FlipH,core::vector2df(0,0));
		
		if (interaction->getKeyState(make_pair(KEY_KEY_V, KeyAugment::None))) 
			meshcomp->updateUV(scn, UVMode::FlipV, core::vector2df(0, 0));

	});
	if (scn) CViewInteraction::updateVisbility();
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
	CCamera* active = context->getActiveCamera();
	CCollisionManager* collisionMgr = context->getCollisionManager();

	if (scene != NULL)
		scene->update();

	CScn* scn = SCNEdit::getSCN();

	// get view ray from camera to mouse position
	const core::recti& vp = getVideoDriver()->getViewPort();
	if (scn) {
		core::line3df ray = CProjective::getViewRay(active,mouse.X, mouse.Y,vp.getWidth(), vp.getHeight());

		float outBestDistanceSquared = 500 * 100; // hit in 500m
		core::vector3df intersection;
		core::triangle3df triangle;

		if (collisionMgr->hasNodes() && 
			collisionMgr->getCollisionPoint(ray, outBestDistanceSquared, intersection, triangle, interaction->node) 
			&& !interaction->getCursorMode()){

			CCollisionNode* node = interaction->node;
			CGameObject* highlighted = node->GameObject;
			if (interaction->isLeftClicked()) deselectAll(highlighted);

			//Hover over styling
			if ((str_equals(highlighted->getNameA(), "solid") ||
				str_equals(highlighted->getNameA(), "solid_extra"))) {
				sceneDebug->addTri(triangle, SColor(255, 255, 0, 255));
			}
			if (str_equals(highlighted->getNameA(), "entity")) {
				sceneDebug->addBoudingBox(node->Selector->getBBox(), SColor(255, 0, 0, 0));
			}
			if (str_equals(highlighted->getNameA(), "portal")) {
				for (int i = 0; i < node->Triangles.size(); i++) {
					sceneDebug->addTri(node->Triangles[i], SColor(255, 255, 255, 255));
				}
			}

			core::vector3df normal = triangle.getNormal();
			normal.normalize();

			// draw projection box will draw white box and xyz lines
			core::aabbox3df box;
			float dist = context->getActiveCamera()->getPosition().getDistanceFrom(intersection) / 30.0f;
			dist = min(max(dist, .3), 6);
			core::vector3df halfBox = core::vector3df(1.0f * 0.5f);
			box.MinEdge = -halfBox * dist;
			box.MaxEdge = halfBox * dist;

			core::quaternion r1;
			r1.rotationFromTo(core::vector3df(0.0f, 1.0f, 0.0f), normal);

			core::quaternion r2;
			r2.fromAngleAxis(0.0f * core::DEGTORAD, core::vector3df(0.0f, 1.0f, 0.0f));

			core::quaternion q = r2 * r1;

			core::matrix4 bbMatrix = q.getMatrix();
			bbMatrix.setTranslation(intersection);

			core::matrix4 bbMatrix2;
			bbMatrix2.setTranslation(intersection);
			sceneDebug->addTransformBBox(box, SColor(255, 0, 255, 0), bbMatrix);
			
			sceneDebug->addTransform(bbMatrix2, 3.0f * dist);
			core::vector3df start = intersection;
			core::vector3df end = intersection + (normal * (3.0f * dist));
			sceneDebug->addLine(core::line3df(start, end), SColor(255, 200, 0, 200)); // purple

			//Read click update interaction manager accordingly.
			if (interaction->isLeftClicked()) {
				
				resetSolid();
				
				//When click and solid is highlighted get the mesh component and select it. 
				if ((str_equals(highlighted->getNameA(), "solid") || str_equals(highlighted->getNameA(), "solid_extra"))){
					CScnMeshComponent* meshcomp = highlighted->getComponent<CScnMeshComponent>();
					solidSelect_t surfdata = meshcomp->select(scn, triangle,
						interaction->getLeftClickAug() == KeyAugment::Shift);
			
					//Set the surf indexed selected.
					interaction->setSurfISelected(surfdata);
					if (surfdata.surfsel != -1) {
						if (str_equals(highlighted->getNameA(), "solid"))
							interaction->selTypeLayer().set(SelectedType::Solid);
						else 
							interaction->selTypeLayer().set(SelectedType::SolidExtra);
						
						//Set vert and draw cubes based on vert data.
						interaction->setVertData(meshcomp->getVertices(scn));
						updateSurfaceVertCube();
					}
					else 
						interaction->selTypeLayer().set(SelectedType::Empty);
				}

				else if (str_equals(highlighted->getNameA(), "entity")) {
					int entindx = highlighted->getComponent<CScnEntityComponent>()->select();
					interaction->setEntityISelected(entindx);
					if (entindx != -1)
						interaction->selTypeLayer().set(SelectedType::Entity);
					else 
						interaction->selTypeLayer().set(SelectedType::Empty);

				}
				else if (str_equals(highlighted->getNameA(), "portal")) {
					portalSelect_t portaldata = highlighted->getComponent<CScnPortalComponent>()->select();
					interaction->setPortalISelected(portaldata);
					if (portaldata.portalidx != -1)
						interaction->selTypeLayer().set(SelectedType::Portal);
					else 
						interaction->selTypeLayer().set(SelectedType::Empty);
				}

				interaction->setSelectObj(highlighted);
				interaction->UICallback();
				interaction->resetLeftClick();
			}

			
			//Update the nearest or selected vert depending on moveable vert
			if (interaction->selTypeLayer().find(SelectedType::Solid,SelectedType::SolidExtra)) {
				
				if (interaction->keyAugLayer().get() == KeyAugment::None)
					updateNearestVert(intersection);
				else if (interaction->keyAugLayer().find(KeyAugment::Ctrl, KeyAugment::Shift))
					updateSelectedVert(intersection);
			}

		}

		//When you update the imgui settings, update the visiblity according to settings, then update the lightmap.
		//Also reset the prev gui data(used in ini file).
		if (interaction->isGuiSettingsUpdated()) {
			CViewInteraction::updateVisbility();

			if (scn->getLightmap()->hasLightmaps()) {
				zone->searchObject(L"solid")->getComponent<CScnMeshComponent>()->setLightmapVisible(gui->vis_lightmaps);
				CContainerObject* group_extra = (CContainerObject*)zone->searchObject(L"group_extra");

				if (group_extra) {
					for (int i = 0; i < group_extra->getChilds()->size(); i++) 
						group_extra->getChilds()->at(i)->getComponent<CScnMeshComponent>()->setLightmapVisible(gui->vis_lightmaps);
				}
			}
			interaction->resetPrevGui();
		}
	}
}

void CViewInteraction::onDestroy() {}

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
void CViewInteraction::moveVertNBounds(bool reset) {
	CContext* context = CContext::getInstance();

	CInteractionManager* interaction = CInteractionManager::getInstance();

	CGameObject* selected = interaction->getSelectObj();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	indexedVec3df_t moveable = interaction->getMoveableVert();
	CZone* zone = context->getActiveZone();

	CScn* scn = SCNEdit::getSCN();

	interaction->updateVertPos();
	interaction->UICallback();
	collisionMgr->removeCollision(selected);
	collisionMgr->addComponentCollision(selected);
	collisionMgr->build(false);

	CCube* selvert = zone->searchObject(L"sel_vert")->getComponent< CCube>();
	selvert->removeAllEntities();
	selvert->addPrimitive(moveable.pos, core::vector3df(0), core::vector3df(2.6));


	if (moveable.solididx == 0) {
		CContainerObject* group_bb = (CContainerObject*)zone->searchObject(L"group_bb");
		if (group_bb) {
			for (int i = 0; i < group_bb->getChilds()->size(); i++) {
				CScnCellBBComponent* cellbb = group_bb->getChilds()->at(i)->getComponent<CScnCellBBComponent>();
				cellbb->updateBB(scn, moveable, reset);
			}
		}
	}

}

void CViewInteraction::updateEntityPos(core::vector3df addpos) {
	//Get entity variables and singletons
	CContext* context = CContext::getInstance();
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CGameObject* selected = interaction->getSelectObj();
	int entitySelected = interaction->getEntityISelected();
	CScnEntityComponent* entcomp = selected->getComponent<CScnEntityComponent>();
	CCollisionManager* collisionMgr = context->getCollisionManager();
	CScnEnt* ent = SCNEdit::getSCN()->getEnt(entitySelected);

	//Set entity position based on add pos + origin.
	if (addpos != core::vector3df(0)) {
		core::vector3df pos = convert_vec3(ent->getField("origin")) + addpos;
		ent->setField("Origin", format("{} {} {}", pos.X, pos.Y, pos.Z).c_str());
	}
	else
		ent->setField("Origin", entcomp->getResetPos().c_str());

	//Update mesh and rebuild collision.
	entcomp->updateMesh(ent);
	collisionMgr->removeCollision(selected);
	collisionMgr->addBBComponentCollision(selected);
	collisionMgr->build(false);

}

void CViewInteraction::deselectAll(CGameObject* current) {
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();

	//Deselect solid
	CGameObject* solidObj = zone->searchObject(L"solid");
    if(solidObj !=current)solidObj->getComponent<CScnMeshComponent>()->deselect();

	//Deselect solid extra
	CContainerObject* group_extra = (CContainerObject*)zone->searchObject(L"group_extra");
	if (group_extra) {
		for (int i = 0; i < group_extra->getChilds()->size(); i++) {
			if (group_extra->getChilds()->at(i) != current)
				group_extra->getChilds()->at(i)->getComponent<CScnMeshComponent>()->deselect();
		}
	}

	//Delselect entity
	CContainerObject* group_entity = (CContainerObject*)zone->searchObject(L"group_entity");
	if (group_entity) {
		for (int i = 0; i < group_entity->getChilds()->size(); i++) {
			if (group_entity->getChilds()->at(i) != current)
				group_entity->getChilds()->at(i)->getComponent<CScnEntityComponent>()->deselect();
		}
	}

	//Deselect portal
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

void CViewInteraction::onPostRender(){}

void CViewInteraction::updateObjectVisbility(CGameObject* obj, bool state, bool bbCollision) {
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

void CViewInteraction::updateVisbility()
{
	CInteractionManager* interaction = CInteractionManager::getInstance();
	guiSettings_t* gui = interaction->getGuiSettings();
	guiSettings_t* prevgui = interaction->getPrevGuiSettings();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CCollisionManager* collisionMgr = context->getCollisionManager();

	//Given solid or extra selected show all hidden elements for simplicity sake
	if (( gui->vis_scn && gui->vis_scn != prevgui->vis_scn) || (gui->vis_doors && gui->vis_doors != prevgui->vis_doors)) {
		for (int i = 0; i < m_hideSolids.size(); i++) {
			m_hideSolids[i]->getComponent<CScnMeshComponent>()->show();
			collisionMgr->removeCollision(m_hideSolids[i]);
			collisionMgr->addComponentCollision(m_hideSolids[i]);
			m_rebuildRequired = true;
		}
		m_hideSolids.clear();
	}
	//Reset solid verts and selected boxes.
	if ((!gui->vis_scn && interaction->selTypeLayer().get() == SelectedType::Solid) ||
		(!gui->vis_doors && interaction->selTypeLayer().get() == SelectedType::SolidExtra)) {
		deselectAll(NULL);
		resetSolid();
		interaction->UICallback();
	}

	if ((!gui->vis_entities && interaction->selTypeLayer().get() == SelectedType::Entity) ||
		(!gui->vis_portals && interaction->selTypeLayer().get() == SelectedType::Portal)) {
		deselectAll(NULL);
		interaction->selTypeLayer().set(SelectedType::Empty);
		interaction->UICallback();
	}

	//Update solid based on imgui
	updateObjectVisbility(zone->searchObject(L"solid"), gui->vis_scn, false);

	//Update extra based on imgui
	CContainerObject* group_extra = (CContainerObject*)zone->searchObject(L"group_extra");
	if (group_extra) {
		for (int i = 0; i < group_extra->getChilds()->size(); i++) 
			updateObjectVisbility(group_extra->getChilds()->at(i), gui->vis_doors, false);
	}
	
	//Update portal based on imgui
	CContainerObject* group_portal = (CContainerObject*)zone->searchObject(L"group_portal");
	if (group_portal) {
		core::array<CGameObject*>objs = group_portal->getArrayChilds(false);
		for (int i = 0; i < objs.size(); i++) {
			if (str_equals(objs[i]->getNameA(), "portal"))
				updateObjectVisbility(objs[i], gui->vis_portals, true);
		}
	}
	//Update bounding box based on imgui
	CContainerObject* group_bb = (CContainerObject*)zone->searchObject(L"group_bb");
	if (group_bb) {
		for (int i = 0; i < group_bb->getChilds()->size(); i++) 
			group_bb->getChilds()->at(i)->setVisible(gui->vis_bb);
	}
	
	//Update entity based on imgui
	CContainerObject* group_entity = (CContainerObject*)zone->searchObject(L"group_entity");
	if (group_entity) {
		for (int i = 0; i < group_entity->getChilds()->size(); i++) 
			updateObjectVisbility(group_entity->getChilds()->at(i), gui->vis_entities, true);
	}

	//If needed rebuild the collision.
	if (m_rebuildRequired) {
		interaction->selTypeLayer().set(SelectedType::Empty);
		if(collisionMgr->hasNodes())
			collisionMgr->build(false);
		m_rebuildRequired = false;
	}

}
void CViewInteraction::resetSolid() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	//Could probably use a for loop here :)
	CCube* selvert = zone->searchObject(L"sel_vert")->getComponent<CCube>();
	CCube* surf = zone->searchObject(L"surf_verts")->getComponent<CCube>();
	CCube* shared = zone->searchObject(L"shared_verts")->getComponent<CCube>();
	CCube* hoververt = zone->searchObject(L"hover_vert")->getComponent< CCube>();

	interaction->selTypeLayer().set(SelectedType::Empty);
	hoververt->removeAllEntities();
	selvert->removeAllEntities();
	surf->removeAllEntities();
	shared->removeAllEntities();
	interaction->resetSelected();
	interaction->resetVerts();
}
void CViewInteraction::updateSurfaceVertCube() {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CCube* shared = zone->searchObject(L"shared_verts")->getComponent<CCube>();
	CCube* surf = zone->searchObject(L"surf_verts")->getComponent<CCube>();
	surf->removeAllEntities();
	shared->removeAllEntities();
	for (int i = 0; i < interaction->getVerts().size(); i++)
		surf->addPrimitive(interaction->getVerts()[i].pos, core::vector3df(0), core::vector3df(2.4));

	for (int i = 0; i < interaction->getSharedVerts().size(); i++)
		shared->addPrimitive(interaction->getSharedVerts()[i].pos, core::vector3df(0), core::vector3df(2.4));

}

void CViewInteraction::updateNearestVert(core::vector3df pos) {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CScn* scn = SCNEdit::getSCN();
	CCube* hoververt = zone->searchObject(L"hover_vert")->getComponent< CCube>();
	CCube* selvert = zone->searchObject(L"sel_vert")->getComponent< CCube>();
	//Make sure that surf and shared vert exists.
	if (interaction->getSharedVerts().size() > 0 || interaction->getVerts().size() > 0) {
		indexedVec3df_t closest;
		float minDistSq = FLT_MAX;
		
		//Gets the nearest Vert for shared surf and surf vert.
		getNearestDistVert(pos, interaction->getVerts(), closest, minDistSq);
		getNearestDistVert(pos, interaction->getSharedVerts(), closest, minDistSq);


		//Resused code..
		if (!interaction->hasMoveableVert()) {
			hoververt->removeAllEntities();
			hoververt->addPrimitive(closest.pos, core::vector3df(0), core::vector3df(2.6));

			interaction->setMoveableVert(closest);
			interaction->UICallback();
		}
		//If closest has changed update the moveable vert and proccess the vertex callback.
		else if (closest.faceidx != interaction->getMoveableVert().faceidx) {
			hoververt->removeAllEntities();
			hoververt->addPrimitive(closest.pos, core::vector3df(0), core::vector3df(2.6));

			interaction->setMoveableVert(closest);
			interaction->UICallback();
		}


		if (selvert->getEntityCount() > 0) {
			updateSurfaceVertCube();
			interaction->UICallback();
			selvert->removeAllEntities();
			hoververt->removeAllEntities();
			hoververt->addPrimitive(closest.pos, core::vector3df(0), core::vector3df(2.6));
		}
		
	}
}
///Loop through each vert get the squared distance and compare/update current min.
void CViewInteraction::getNearestDistVert(core::vector3df pos, core::array<indexedVec3df_t> verts,indexedVec3df_t& closest, float& minDistSq) {
	for (int i = 0; i < verts.size(); i++) {
		core::vector3df point = verts[i].pos;
		float distanceSq = pos.getDistanceFromSQ(point); // Squared distance

		if (distanceSq < minDistSq) {
			minDistSq = distanceSq;
			closest = verts[i];
		}
	}
}

void CViewInteraction::updateSelectedVert(core::vector3df pos) {
	CInteractionManager* interaction = CInteractionManager::getInstance();
	CContext* context = CContext::getInstance();
	CZone* zone = context->getActiveZone();
	CCube* hoververt = zone->searchObject(L"hover_vert")->getComponent< CCube>();
	CCube* selvert = zone->searchObject(L"sel_vert")->getComponent< CCube>();
	if (interaction->hasMoveableVert()) {
		indexedVec3df_t moveable = interaction->getMoveableVert();

		if (hoververt->getEntityCount() > 0) {
			updateSurfaceVertCube();
			interaction->UICallback();
			hoververt->removeAllEntities();
			selvert->removeAllEntities();
			selvert->addPrimitive(moveable.pos, core::vector3df(0), core::vector3df(2.6));
		}

	}
	else {
		updateNearestVert(pos);
	}
	
}
