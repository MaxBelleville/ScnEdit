#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshComponent.h"
#include "Header/Components/CScnMeshData.h"


CScnMeshComponent::CScnMeshComponent()
{

}

CScnMeshComponent::~CScnMeshComponent()
{

}

void CScnMeshComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnMeshComponent::setMesh(CScn* scn, CScnSolid* solid, CScnArguments* args) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->addData<CScnMeshData>(DATA_TYPE_INDEX(CRenderMeshData));
	scnMesh->initMesh(scn,solid, args);
	scnMesh->setVisible(true);

}
void CScnMeshComponent::setLightmapVisible(bool vis) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) scnMesh->setLightmapVisible(vis);
}

std::pair<int, int> CScnMeshComponent::select(CScn* scn, core::triangle3df tri, bool bAdd) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		std::pair<int, int> scndata = scnMesh->getSurfaceIndx(scn, tri);
		int indx = selsurfs.linear_search(scndata.second);
		//If found surf has already been selected, remove and change to prev, return if none selected.
		scnMesh->deselectAll();
		if (indx> -1) {
			if (bAdd)selsurfs.erase(indx);
			else selsurfs.clear();

			if (selsurfs.size() > 0) scndata = make_pair(scndata.first, selsurfs.getLast());
			else return make_pair(-1, -1);
		}
		else {
			//Surf hasn't been selected, add or set value depending on bAdd.
			if (!bAdd&& selsurfs.size() >0) {
				selsurfs.clear();
				selsurfs.push_back(scndata.second);
				
			}
			else {
				selsurfs.push_back(scndata.second);
			}
		}
		//calculate shared using selsurf array.
		sharedsurfs = scnMesh->getSharedSurface(scn, selsurfs);
		
		for (int i = 0; i < selsurfs.size(); i++) { // Red
			if(selsurfs[i]>=0)scnMesh->select(selsurfs[i],false);
		}
		for (int i = 0; i < sharedsurfs.size(); i++) { //Blue
			if (sharedsurfs[i] >= 0)scnMesh->select(sharedsurfs[i],true);
		}
		return scndata;
	}
	return make_pair(-1, -1); //Doubt this will even trigger.
}
void CScnMeshComponent::deselect() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) scnMesh->deselectAll();
}

void CScnMeshComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if(scnMesh)scnMesh->setVisible(true);
}

void CScnMeshComponent::hide() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		for (int i = 0; i < selsurfs.size(); i++) {
			scnMesh->hide(selsurfs[i]);
		}
		scnMesh->deselectAll();
		selsurfs.clear();
	}
}

void CScnMeshComponent::show() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		scnMesh->show();
		scnMesh->deselectAll();
		selsurfs.clear();
	}
}
void CScnMeshComponent::setTexture(CScn* scn, const char* path) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		for (int i = 0; i < selsurfs.size(); i++) {
			scnMesh->setTexture(scn,path, selsurfs[i]);
		}
	}
}
core::array<vertProp_t> CScnMeshComponent::getSurfVertProps(CScn* scn, int si) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		return scnMesh->getSurfVertProps(scn, si);
	}
}
indexed_vertices CScnMeshComponent::getVertices(CScn* scn) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		return scnMesh->getVertices(scn, selsurfs,sharedsurfs);
	}
}

indexedVec3df_t CScnMeshComponent::updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df add) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
	   return scnMesh->updateVert(scn, vert, add);
	}
}

indexedVec3df_t CScnMeshComponent::resetVert(CScn* scn, indexedVec3df_t vert) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		return scnMesh->resetVert(scn, vert);
	}
}