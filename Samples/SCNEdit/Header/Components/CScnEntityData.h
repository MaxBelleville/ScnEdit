#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnEntityData : public CRenderMeshData
{
protected:
	int m_indx;
	bool m_firstLoad=false;
public:
	IMeshBuffer *MeshBuffer;
	std::string m_origin = "";

public:
	CScnEntityData();

	virtual ~CScnEntityData();

	void initMesh(CScnEnt* ent);

	void select();

	void deselect();

	inline int getEntityIndx() {
		return m_indx;
	}

};