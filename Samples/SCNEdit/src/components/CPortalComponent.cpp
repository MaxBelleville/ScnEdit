#include "pch.h"
#include "SkylichtEngine.h"
#include "include/components/CPortalComponent.h"
#include "include/components/CPortalData.h"

CPortalComponent::CPortalComponent() {}

CPortalComponent::~CPortalComponent() {}

void CPortalComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CPortalComponent::setMesh(CSolid* solid, u32 cellindx, s32 portalIndx) {
	CEntity* entity = m_gameObject->getEntity();
	CPortalData* portals = entity->addData<CPortalData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(solid, cellindx, portalIndx);
	portals->setVisible(true);
}

portalBox_t CPortalComponent::select() {
	CEntity* entity = m_gameObject->getEntity();
	CPortalData* portalData = entity->getData<CPortalData>();
	if (portalData) {
		if (portalData->toggleSelect())
			return portalData->getPortalData();
	}
	return portalBox_t(0, -1);
}

void CPortalComponent::deselect() {
	CEntity* entity = m_gameObject->getEntity();
	CPortalData* portalData = entity->getData<CPortalData>();
	if (portalData)
		portalData->deselect();
}

bool CPortalComponent::getSelected() {
	CEntity* entity = m_gameObject->getEntity();
	CPortalData* portalData = entity->getData<CPortalData>();
	if (portalData)
		portalData->getSelected();
	return false;
}

void CPortalComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CPortalData* portals = entity->getData<CPortalData>();
	if (portals) portals->setVisible(true);
}