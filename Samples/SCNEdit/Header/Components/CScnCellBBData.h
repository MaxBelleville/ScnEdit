#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"



class CScnCellBBData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

	core::array<core::vector3df*> backup_bb;
	int cellindx= -1;
public:
	CScnCellBBData();

	virtual ~CScnCellBBData();

	void initMesh(CScnSolid* solid, u32 cellindx);

	void updateBB(CScn* scn, indexedVec3df_t vert, bool);
	int getIndexFromCellBB(CScnSolid* solid, scnCellData_t* celldata);
};