#pragma once
#include "include/core/CScn.h"

class CCellBBComponent : public CComponentSystem
{
public:
	CCellBBComponent();

	virtual ~CCellBBComponent();

	virtual void initComponent();

	void setMesh(CSolid* solid, u32 cellindx);

	void updateBB(CSolid* solid, vertBox_t vetsel, bool reset);

	virtual void updateComponent();
};