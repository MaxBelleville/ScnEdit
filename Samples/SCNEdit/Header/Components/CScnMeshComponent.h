#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Decal/CDecals.h"
class CScnMeshComponent : public CComponentSystem
{
public:
	core::array<int> selsurfs;
	core::array<int> sharedsurfs;
	int solididx;
public:
	CScnMeshComponent();

	virtual ~CScnMeshComponent();

	virtual void initComponent();

	void setMesh(CScn*, CScnSolid*, CScnArguments*);

	void setLightmapVisible(bool);
	solidSelect_t select(CScn* scn, core::triangle3df, bool);
	void deselect();
	void hide(bool shared);
	void show();
	void setTexture(CScn* scn, const char* path);
	indexedVec3df_t updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df add);
	indexedVec3df_t resetVert(CScn* scn, indexedVec3df_t vert);
	core::array<vertProp_t> getSurfVertProps(CScn* scn, int si);
	indexed_vertices getVertices(CScn* scn);
	void updateUV(CScn* scn, int uvmode, core::vector2df uvShift);
	void resetUV(CScn* scn);
	virtual void updateComponent();

};