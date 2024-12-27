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

void CScnEntityComponent::setMesh(CScnEnt* ent) {

	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->addData<CScnEntityData>(DATA_TYPE_INDEX(CRenderMeshData));
	entData->initMesh(ent);
	entData->setVisible(true);
}

void CScnEntityComponent::updateMesh(CScnEnt* ent) {

	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->getData<CScnEntityData>();
	if (entData) {
		entData->initMesh(ent);
		entData->setVisible(true);
	}
}



int CScnEntityComponent::select() {

	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->getData<CScnEntityData>();
	if (entData) {
		selected ? entData->deselect() : entData->select();
		selected = !selected;
		if(selected) return entData->getEntityIndx();
	}
	return -1;
}

void CScnEntityComponent::deselect() {

	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->getData<CScnEntityData>();
	if (entData){
		entData->deselect();
		selected = false;
	}
}


void CScnEntityComponent::updateComponent()
{
	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->getData<CScnEntityData>();

	if(entData) entData->setVisible(true);
}

std::string CScnEntityComponent::getResetPos() {

	CEntity* entity = m_gameObject->getEntity();
	CScnEntityData* entData = entity->getData<CScnEntityData>();
	if (entData) { return entData->m_origin;  }
	return "";
}