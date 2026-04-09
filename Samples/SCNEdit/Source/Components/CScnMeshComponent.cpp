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

void CScnMeshComponent::setMesh(CScnSolid* solid, CScnLightmap* lmap, CScnArguments* args) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->addData<CScnMeshData>(DATA_TYPE_INDEX(CRenderMeshData));
	scnMesh->initMesh(solid,lmap, args);
	scnMesh->setVisible(true);

}
void CScnMeshComponent::setLightmapVisible(bool vis) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) scnMesh->setLightmapVisible(vis);
}

core::array<surfaceBox_t> CScnMeshComponent::select(CScnSolid* solid, core::triangle3df tri, bool bAdd) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		return scnMesh->select(solid, tri, bAdd);
	
	return 0;
}

void CScnMeshComponent::deselect(CScnSolid* solid, int si) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->deselect(solid, si);
}

int CScnMeshComponent::getSolidIdx() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		return scnMesh->getSolidIdx();
	return -1;
}


void CScnMeshComponent::deselectAll() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->deselectAll();
}

void CScnMeshComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if(scnMesh)
		scnMesh->setVisible(true);
}

void CScnMeshComponent::hide(CScnSolid* solid,bool bShared) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		scnMesh->hide(solid,bShared);
		scnMesh->deselectAll();
	}
}

void CScnMeshComponent::show() {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		scnMesh->show();
		scnMesh->deselectAll();
	}
}
void CScnMeshComponent::setTexture(CScnSolid* solid, const char* path) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->setTexture(solid,path);
}


void CScnMeshComponent::updateVert(CScnSolid* solid, vertBox_t vertidx, core::vector3df add) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->updateVert(solid, vertidx, add);

}

void CScnMeshComponent::resetVert(CScnSolid* solid, vertBox_t vertidx) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->resetVert(solid,vertidx);
}
void CScnMeshComponent::updateUV(CScnSolid* solid, UVMode mode, core::vector2df shift) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->updateUV(solid,mode,shift);
}
void CScnMeshComponent::resetUV(CScnSolid* solid) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) 
		scnMesh->resetUV(solid);
}