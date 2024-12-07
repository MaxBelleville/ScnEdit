#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnCellBBData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnCellBBData();

	virtual ~CScnCellBBData();

	void initMesh(CScn* scn);

};