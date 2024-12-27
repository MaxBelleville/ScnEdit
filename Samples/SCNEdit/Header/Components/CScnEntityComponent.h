#pragma once
#include "Header/Base/CScn.h"

class CScnEntityComponent : public CComponentSystem
{
private:
bool selected = false;
public:
	CScnEntityComponent();

	virtual ~CScnEntityComponent();

	virtual void initComponent();

	void setMesh(CScnEnt*);

	void updateMesh(CScnEnt*);

	virtual void updateComponent();

	int select();

	void deselect();

	bool isSelected() { return selected; }

	std::string getResetPos();
};