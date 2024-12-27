#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Decal/CDecals.h"
class CScnMeshComponent : public CComponentSystem
{
public:
	core::array<int> selsurfs;
	core::array<int> sharedsurfs;
public:
	CScnMeshComponent();

	virtual ~CScnMeshComponent();

	virtual void initComponent();

	void setMesh(CScn*, CScnSolid*, CScnArguments*);

	void setLightmapVisible(bool);
	std::pair<int, int> select(CScn* scn, core::triangle3df, bool);
	void deselect();
	void hide();
	void show();
	void setTexture(CScn* scn, const char* path);
	indexedVec3df_t updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df add);
	indexedVec3df_t resetVert(CScn* scn, indexedVec3df_t vert);
	core::array<vertProp_t> getSurfVertProps(CScn* scn, int si);
	indexed_vertices getVertices(CScn* scn);
	virtual void updateComponent();

};