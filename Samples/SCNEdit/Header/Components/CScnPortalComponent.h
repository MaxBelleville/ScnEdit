#pragma once
#include "Header/Base/CScn.h"

class CScnPortalComponent : public CComponentSystem
{
public:
	CScnPortalComponent();

	virtual ~CScnPortalComponent();

	virtual void initComponent();

	void setMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);

	virtual void updateComponent();

	portalBox_t select();

	void deselect();

	bool getSelected();

};