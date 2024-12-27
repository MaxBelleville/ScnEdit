#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnPortalData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;
	std::pair<u32, s32> portaldata;
public:
	CScnPortalData();

	virtual ~CScnPortalData();

	void initMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);


	void select();

	void deselect();


};