#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnPortalData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnPortalData();

	virtual ~CScnPortalData();

	void initMesh(CScn* scn);

};