#pragma once
#include "Header/Base/CScn.h"

class CScnCellBBComponent : public CComponentSystem
{
public:
	CScnCellBBComponent();

	virtual ~CScnCellBBComponent();

	virtual void initComponent();

	void setMesh(CScnSolid* solid, u32 cellindx);

	virtual void updateComponent();
};