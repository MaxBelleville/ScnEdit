#pragma once
#include "Header/Base/CScn.h"

class CScnPortalComponent : public CComponentSystem
{
public:
	CScnPortalComponent();

	virtual ~CScnPortalComponent();

	virtual void initComponent();

	void setMesh(CScn*);

	virtual void updateComponent();
};