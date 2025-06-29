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
	CScnCellBBData* cellbb = entity->addData<CScnCellBBData>(DATA_TYPE_INDEX(CRenderMeshData));
	cellbb->initMesh(solid,cellindx);
	cellbb->setVisible(true);

}
void CScnCellBBComponent::updateBB(CScn* scn, indexedVec3df_t vert, bool reset)
{
	CEntity* entity = m_gameObject->getEntity();
	CScnCellBBData* cellbb = entity->getData<CScnCellBBData>();
	if (cellbb) cellbb->updateBB(scn,vert,reset);
}


void CScnCellBBComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnCellBBData* cellbb = entity->getData<CScnCellBBData>();
	if(cellbb) cellbb->setVisible(true);
}