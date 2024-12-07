#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"

class CScnMeshComponent : public CComponentSystem
{
public:
	CScnMeshComponent();

	virtual ~CScnMeshComponent();

	virtual void initComponent();

	void setMesh(CScn*, CScnArguments*);

	virtual void updateComponent();
};