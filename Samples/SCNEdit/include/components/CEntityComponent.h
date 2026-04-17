#pragma once
#include "include/core/CScn.h"

class CEntityComponent : public CComponentSystem
{
public:
	CEntityComponent();

	virtual ~CEntityComponent();

	virtual void initComponent();

	void setMesh(CEnt*);

	void updateMesh(CEnt*);

	virtual void updateComponent();

	int select();

	void deselect();

	bool getSelected();

	std::string getResetPos();
};