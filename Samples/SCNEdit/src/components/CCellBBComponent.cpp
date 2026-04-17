#include "pch.h"
#include "SkylichtEngine.h"
#include "include/components/CCellBBComponent.h"
#include "include/components/CCellBBData.h"

CCellBBComponent::CCellBBComponent() {}

CCellBBComponent::~CCellBBComponent() {}

void CCellBBComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();

	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CCellBBComponent::setMesh(CSolid* solid, u32 cellindx)
{
	CEntity* entity = m_gameObject->getEntity();
	CCellBBData* cellbb = entity->addData<CCellBBData>(DATA_TYPE_INDEX(CRenderMeshData));
	cellbb->initMesh(solid, cellindx);
	cellbb->setVisible(true);
}
void CCellBBComponent::updateBB(CSolid* solid, vertBox_t vertsel, bool reset)
{
	CEntity* entity = m_gameObject->getEntity();
	CCellBBData* cellbb = entity->getData<CCellBBData>();
	if (cellbb)
		cellbb->updateBB(solid, vertsel, reset);
}

void CCellBBComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CCellBBData* cellbb = entity->getData<CCellBBData>();
	if (cellbb)
		cellbb->setVisible(true);
}