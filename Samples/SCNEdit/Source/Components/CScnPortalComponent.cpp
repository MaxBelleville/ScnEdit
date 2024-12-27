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

void CScnPortalComponent::setMesh(CScnSolid* solid, u32 cellindx, s32 portalIndx){
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->addData<CScnPortalData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(solid,cellindx, portalIndx);
	portals->setVisible(true);

}


std::pair<u32, s32> CScnPortalComponent::select() {

	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portalData = entity->getData<CScnPortalData>();
	if (portalData) {
		selected ? portalData->deselect() : portalData->select();
		selected = !selected;
		if (selected) return portalData->portaldata;
	}
	return make_pair(0, -1);
}

void CScnPortalComponent::deselect() {

	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portalData = entity->getData<CScnPortalData>();
	if (portalData) {
		portalData->deselect();
		selected = false;
	}
}


void CScnPortalComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnPortalData* portals = entity->getData<CScnPortalData>();
	if(portals) portals->setVisible(true);
}