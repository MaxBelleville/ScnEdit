#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Decal/CDecals.h"
class CScnMeshComponent : public CComponentSystem
{
public:
	CScnMeshComponent();

	virtual ~CScnMeshComponent();

	virtual void initComponent();

	void setMesh(CScnSolid* solid, CScnLightmap* lmap, CScnArguments*);

	void setLightmapVisible(bool);

	int getSolidIdx();


	core::array<surfaceBox_t> select(CScnSolid* solid, core::triangle3df, bool);
	void deselect(CScnSolid* solid, int si);
	void deselectAll();
	void hide(CScnSolid*,bool bShared);
	void show();
	void setTexture(CScnSolid* solid, const char* path);
	void updateVert(CScnSolid* solid, vertBox_t vertidx, core::vector3df add);
	void resetVert(CScnSolid* solid, vertBox_t vertidx);
	void updateUV(CScnSolid* solid, UVMode mode, core::vector2df uvShift);
	void resetUV(CScnSolid* solid);
	virtual void updateComponent();

};