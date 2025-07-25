#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnPortalData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;
	portalSelect_t portaldata = portalSelect_t(-1,-1);
public:
	CScnPortalData();

	virtual ~CScnPortalData();

	void initMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);

	void select();

	void deselect();


};