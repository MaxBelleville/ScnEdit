#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"

class CScnCellBBData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnCellBBData();

	virtual ~CScnCellBBData();

	void initMesh(CScnSolid* solid, u32 cellindx);

};