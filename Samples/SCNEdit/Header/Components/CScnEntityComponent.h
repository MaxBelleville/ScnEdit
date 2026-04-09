#pragma once
#include "Header/Base/CScn.h"

class CScnEntityComponent : public CComponentSystem
{
public:
	CScnEntityComponent();

	virtual ~CScnEntityComponent();

	virtual void initComponent();

	void setMesh(CScnEnt*);

	void updateMesh(CScnEnt*);

	virtual void updateComponent();

	int select();

	void deselect();

	bool getSelected();

	std::string getResetPos();
};