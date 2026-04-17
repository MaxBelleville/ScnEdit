#pragma once
#include "include/core/CScn.h"

class CCellBBData : public CRenderMeshData
{
public:
	IMeshBuffer* MeshBuffer;

	core::array<core::vector3df*> backup_bb;
	int cellindx = -1;
public:
	CCellBBData();

	virtual ~CCellBBData();

	void initMesh(CSolid* solid, u32 cellindx);

	void updateBB(CSolid* solid, vertBox_t vertsel, bool);
	void updateMeshBB(cellData_t* celldata, int leafindx);
	int getIndexFromCellBB(CSolid* solid, cellData_t* celldata);
};