#pragma once
#include "Header/Base/CScn.h"

class CScnEntityComponent : public CComponentSystem
{
public:
	CScnEntityComponent();

	virtual ~CScnEntityComponent();

	virtual void initComponent();

	void setMesh(CScn*);

	virtual void updateComponent();
};