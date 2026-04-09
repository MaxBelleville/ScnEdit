#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnPortalComponent.h"
#include "Header/Components/CScnPortalData.h"


CScnPortalComponent::CScnPortalComponent(){}

CScnPortalComponent::~CScnPortalComponent(){}

void CScnPortalComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnPortalComponent::setMesh(CScnSolid* solid, u32 cellindx, s32 portalIndx){
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->addData<CScnPortalData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(solid,cellindx, portalIndx);
	portals->setVisible(true);

}

portalBox_t CScnPortalComponent::select() {
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portalData = entity->getData<CScnPortalData>();
	if (portalData) {
		if (portalData->toggleSelect())
			return portalData->getPortalData();
	}
	return portalBox_t(0, -1);
}

void CScnPortalComponent::deselect() {
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portalData = entity->getData<CScnPortalData>();
	if (portalData) 
		portalData->deselect();
}



bool CScnPortalComponent::getSelected() {
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portalData = entity->getData<CScnPortalData>();
	if (portalData)
		portalData->getSelected();
	return false;
}

void CScnPortalComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->getData<CScnPortalData>();
	if(portals) portals->setVisible(true);
}