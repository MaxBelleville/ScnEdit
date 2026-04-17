#include "pch.h"
#include "SkylichtEngine.h"
#include "include/components/CScnMeshComponent.h"
#include "include/components/CScnMeshData.h"

CScnMeshComponent::CScnMeshComponent() {}

CScnMeshComponent::~CScnMeshComponent() {}

void CScnMeshComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnMeshComponent::setMesh(CSolid* solid, CLightmap* lmap, CArguments* args) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->addData<CScnMeshData>(DATA_TYPE_INDEX(CRenderMeshData));
	scnMesh->initMesh(solid, lmap, args);
	scnMesh->setVisible(true);
}
void CScnMeshComponent::setLightmapVisible(bool vis) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) scnMesh->setLightmapVisible(vis);
}

core::array<surfaceBox_t> CScnMeshComponent::select(CSolid* solid, core::triangle3df tri, bool bAdd) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		return scnMesh->select(solid, tri, bAdd);

	return 0;
}

void CScnMeshComponent::deselect(CSolid* solid, int si) {
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
	if (scnMesh)
		scnMesh->setVisible(true);
}

void CScnMeshComponent::hide(CSolid* solid, bool bShared) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh) {
		scnMesh->hide(solid, bShared);
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
void CScnMeshComponent::setTexture(CSolid* solid, const char* path) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->setTexture(solid, path);
}

void CScnMeshComponent::updateVert(CSolid* solid, vertBox_t vertidx, core::vector3df add) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->updateVert(solid, vertidx, add);
}

void CScnMeshComponent::resetVert(CSolid* solid, vertBox_t vertidx) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->resetVert(solid, vertidx);
}
void CScnMeshComponent::updateUV(CSolid* solid, UVMode mode, core::vector2df shift) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->updateUV(solid, mode, shift);
}
void CScnMeshComponent::resetUV(CSolid* solid) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData<CScnMeshData>();
	if (scnMesh)
		scnMesh->resetUV(solid);
}