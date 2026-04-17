#pragma once
#include "include/core/CScn.h"

class CPortalComponent : public CComponentSystem
{
public:
	CPortalComponent();

	virtual ~CPortalComponent();

	virtual void initComponent();

	void setMesh(CSolid* solid, u32 cellindx, s32 portalindx);

	virtual void updateComponent();

	portalBox_t select();

	void deselect();

	bool getSelected();
};