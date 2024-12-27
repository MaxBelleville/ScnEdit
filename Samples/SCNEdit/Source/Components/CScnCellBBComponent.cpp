#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnCellBBComponent.h"
#include "Header/Components/CScnCellBBData.h"


CScnCellBBComponent::CScnCellBBComponent()
{

}

CScnCellBBComponent::~CScnCellBBComponent()
{

}

void CScnCellBBComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();


	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnCellBBComponent::setMesh(CScnSolid* solid, u32 cellindx)
{
	CEntity* entity = m_gameObject->getEntity();
	CScnCellBBData* portals = entity->addData<CScnCellBBData>(DATA_TYPE_INDEX(CRenderMeshData));
	portals->initMesh(solid,cellindx);
	portals->setVisible(true);

}

void CScnCellBBComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnCellBBData* portals = entity->getData<CScnCellBBData>();
	if(portals) portals->setVisible(true);
}