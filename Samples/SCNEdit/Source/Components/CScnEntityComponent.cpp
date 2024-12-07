#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnEntityComponent.h"
#include "Header/Components/CScnEntityData.h"


CScnEntityComponent::CScnEntityComponent()
{

}

CScnEntityComponent::~CScnEntityComponent()
{

}

void CScnEntityComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();


	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnEntityComponent::setMesh(CScn* scn) {
	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* portals = entity->addData<CScnEntityData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(scn);
	portals->setVisible(true);

}

void CScnEntityComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* portals = entity->getData<CScnEntityData>();
	if(portals) portals->setVisible(true);
}