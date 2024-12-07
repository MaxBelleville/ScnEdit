#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshComponent.h"
#include "Header/Components/CScnMeshData.h"


CScnMeshComponent::CScnMeshComponent()
{

}

CScnMeshComponent::~CScnMeshComponent()
{

}

void CScnMeshComponent::initComponent()
{
	CEntity* entity = m_gameObject->getEntity();


	// add culling
	CCullingData* culling = entity->addData<CCullingData>();
	culling->Type = CCullingData::BoundingBox;
}

void CScnMeshComponent::setMesh(CScn* scn, CScnArguments* args) {
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->addData<CScnMeshData>(DATA_TYPE_INDEX(CRenderMeshData));
	scnMesh->initMesh(scn, args);
	scnMesh->setVisible(true);

}

void CScnMeshComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnMeshData* scnMesh = entity->getData< CScnMeshData>();
	if(scnMesh)scnMesh->setVisible(true);
}