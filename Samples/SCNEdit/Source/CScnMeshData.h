#pragma once
#include "CScn.h"
#include "CScnArguments.h"
#include "util.h"
#include <set>

class CScnMeshData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CScn* scn, CScnArguments*);
};