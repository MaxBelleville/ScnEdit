#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnEntityData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnEntityData();

	virtual ~CScnEntityData();

	void initMesh(CScn* scn);

};