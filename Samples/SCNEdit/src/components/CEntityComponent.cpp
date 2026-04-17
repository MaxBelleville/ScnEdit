#include "pch.h"
#include "SkylichtEngine.h"
#include "include/components/CEntityComponent.h"
#include "include/components/CEntityData.h"

CEntityComponent::CEntityComponent() {}

CEntityComponent::~CEntityComponent() {}

void CEntityComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CEntityComponent::setMesh(CEnt* ent) {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->addData<CEntityData>(DATA_TYPE_INDEX(CRenderMeshData));
	entData->initMesh(ent);
	entData->setVisible(true);
}

void CEntityComponent::updateMesh(CEnt* ent) {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData) {
		entData->initMesh(ent);
		entData->setVisible(true);
	}
}

int CEntityComponent::select() {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData) {
		if (entData->toggleSelect())
			return entData->getEntityIndx();
	}
	return -1;
}

void CEntityComponent::deselect() {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData)
		entData->deselect();
}

bool CEntityComponent::getSelected() {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData)
		entData->getSelected();
	return false;
}

void CEntityComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData)
		entData->setVisible(true);
}

std::string CEntityComponent::getResetPos() {
	CEntity* entity = m_gameObject->getEntity();
	CEntityData* entData = entity->getData<CEntityData>();
	if (entData)
		return entData->m_origin;
	return "";
}