#pragma once
#include "Header/Base/CScn.h"

class CScnPortalComponent : public CComponentSystem
{
private:
	bool selected = false;
public:
	CScnPortalComponent();

	virtual ~CScnPortalComponent();

	virtual void initComponent();

	void setMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);

	virtual void updateComponent();

	std::pair<u32, s32> select();

	void deselect();
};