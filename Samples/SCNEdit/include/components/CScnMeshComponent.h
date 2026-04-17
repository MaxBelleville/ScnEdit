#pragma once
#include "include/core/CScn.h"
#include "include/io/CArguments.h"
#include "Decal/CDecals.h"
class CScnMeshComponent : public CComponentSystem
{
public:
	CScnMeshComponent();

	virtual ~CScnMeshComponent();

	virtual void initComponent();

	void setMesh(CSolid* solid, CLightmap* lmap, CArguments*);

	void setLightmapVisible(bool);

	int getSolidIdx();

	core::array<surfaceBox_t> select(CSolid* solid, core::triangle3df, bool);
	void deselect(CSolid* solid, int si);
	void deselectAll();
	void hide(CSolid*, bool bShared);
	void show();
	void setTexture(CSolid* solid, const char* path);
	void updateVert(CSolid* solid, vertBox_t vertidx, core::vector3df add);
	void resetVert(CSolid* solid, vertBox_t vertidx);
	void updateUV(CSolid* solid, UVMode mode, core::vector2df uvShift);
	void resetUV(CSolid* solid);
	virtual void updateComponent();
};