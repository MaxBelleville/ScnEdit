#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnPortalComponent.h"
#include "Header/Components/CScnPortalData.h"


CScnPortalComponent::CScnPortalComponent()
{

}

CScnPortalComponent::~CScnPortalComponent()
{

}

void CScnPortalComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();


	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnPortalComponent::setMesh(CScn* scn) {
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->addData<CScnPortalData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(scn);
	portals->setVisible(true);

}

void CScnPortalComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->getData<CScnPortalData>();
	if(portals) portals->setVisible(true);
}