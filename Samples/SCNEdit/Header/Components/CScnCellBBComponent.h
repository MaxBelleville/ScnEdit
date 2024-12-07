#pragma once
#include "Header/Base/CScn.h"

class CScnCellBBComponent : public CComponentSystem
{
public:
	CScnCellBBComponent();

	virtual ~CScnCellBBComponent();

	virtual void initComponent();

	void setMesh(CScn*);

	virtual void updateComponent();
};