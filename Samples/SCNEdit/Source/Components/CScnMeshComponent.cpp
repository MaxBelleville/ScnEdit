#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshComponent.h"
#include "Header/Components/CScnMeshData.h"


CScnMeshComponent::CScnMeshComponent(){}

CScnMeshComponent::~CScnMeshComponent(){}

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
	solididx = solid->solididx;
	scnMesh->setVisible(true);

}
void CScnMeshComponent::setLightmapVisible(bool vis) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) scnMesh->setLightmapVisible(vis);
}

solidSelect_t CScnMeshComponent::select(CScn* scn, core::triangle3df tri, bool bAdd) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		solidSelect_t scndata = scnMesh->getSurfaceIndx(scn, tri);
		
		int indx = selsurfs.linear_search(scndata.surfsel);
		//If found surf has already been selected, remove and change to prev, return if none selected.
		scnMesh->deselectAll();
		if (indx> -1) {
			if (bAdd)
				selsurfs.erase(indx);
			else 
				selsurfs.clear();

			if (selsurfs.size() > 0) 
				scndata = solidSelect_t(scndata.solididx, selsurfs.getLast());
			else 
				return solidSelect_t(-1, -1);
		}
		else {
			//Surf hasn't been selected, add or set value depending on bAdd.
			if (!bAdd&& selsurfs.size() >0) {
				selsurfs.clear();
				selsurfs.push_back(scndata.surfsel);	
			}
			else 
				selsurfs.push_back(scndata.surfsel);
		}
		//calculate shared using selsurf array.
		sharedsurfs = scnMesh->getUVSharedSurface(scn, selsurfs);
		
		for (int i = 0; i < selsurfs.size(); i++) { // Red
			if(selsurfs[i]>=0)
				scnMesh->select(selsurfs[i],false);
		}
		for (int i = 0; i < sharedsurfs.size(); i++) { //Blue
			if (sharedsurfs[i] >= 0)
				scnMesh->select(sharedsurfs[i],true);
		}
		return scndata;
	}
	
	return solidSelect_t(-1, -1); //Doubt this will even trigger.
}
void CScnMeshComponent::deselect() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->deselectAll();
	selsurfs.clear();
}

void CScnMeshComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if(scnMesh)
		scnMesh->setVisible(true);
}

void CScnMeshComponent::hide(bool shared) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		for (int i = 0; i < selsurfs.size(); i++) 
			scnMesh->hide(selsurfs[i]);

		if (shared) {
			for (int i = 0; i < sharedsurfs.size(); i++)
				scnMesh->hide(sharedsurfs[i]);
		}

		scnMesh->deselectAll();
		selsurfs.clear();
		sharedsurfs.clear();
	}
}

void CScnMeshComponent::show() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		scnMesh->show();
		scnMesh->deselectAll();
		selsurfs.clear();
		sharedsurfs.clear();
	}
}
void CScnMeshComponent::setTexture(CScn* scn, const char* path) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		for (int i = 0; i < selsurfs.size(); i++) 
			scnMesh->setTexture(scn,path, selsurfs[i]);
	}
}
core::array<vertProp_t> CScnMeshComponent::getSurfVertProps(CScn* scn, int si) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		return scnMesh->getSurfVertProps(scn, si);
}
indexed_vertices CScnMeshComponent::getVertices(CScn* scn) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		return scnMesh->getVertices(scn, selsurfs,sharedsurfs);
}


indexedVec3df_t CScnMeshComponent::updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df add) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		return scnMesh->updateVert(scn, vert, add);

}

indexedVec3df_t CScnMeshComponent::resetVert(CScn* scn, indexedVec3df_t vert) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		return scnMesh->resetVert(scn, vert);
}
void CScnMeshComponent::updateUV(CScn* scn, int resize, core::vector2df shift) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->updateUV(scn, selsurfs, sharedsurfs, resize,shift);
}
void CScnMeshComponent::resetUV(CScn* scn) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->resetUV(scn, selsurfs,sharedsurfs);
}