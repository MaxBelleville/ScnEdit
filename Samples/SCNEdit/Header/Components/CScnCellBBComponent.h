#pragma once
#include "Header/Base/CScn.h"

class CScnCellBBComponent : public CComponentSystem
{
public:
	CScnCellBBComponent();

	virtual ~CScnCellBBComponent();

	virtual void initComponent();

	void setMesh(CScnSolid* solid, u32 cellindx);

	void updateBB(CScn* scn, indexedVec3df_t vert, bool reset);

	virtual void updateComponent();
};