#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Header/Base/util.h"
#include <set>
#include <future>
#include <thread>

struct BackupData {
	core::array<S3DVertex2TCoords> vertices;
	core::array<u32> indices;
};

class CScnMeshData : public CRenderMeshData
{
protected:
	u32 solidindx;
	core::array<IImage*>lmapImages;
	core::array<BackupData> vis_backup;
	core::array<BackupData> vert_backup;
	core::array<int> hiddensurfs;
	CMesh* RenderMeshBackup;
public:
	IMeshBuffer *MeshBuffer;
	

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CScn* scn, CScnSolid* solid, CScnArguments*);
	void setLightmapVisible(bool);
	std::pair<int, int> getSurfaceIndx(CScn* scn, core::triangle3df);
	core::array<int> getSharedSurface(CScn* scn, core::array<int>sindices);
	core::array<int> getSharedSurface(CScn* scn, int si);
	core::array<vertProp_t> getSurfVertProps(CScn* scn, core::array<int>sindices);
	core::array<vertProp_t> getSurfVertProps(CScn* scn, int si);
	void select(int si, bool bShared);
	void deselect(int si);
	void deselectAll();
	void hide(int si);
	void show();
	void setTexture(CScn* scn, const char* path,int si);
	indexedVec3df_t updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df);
	indexedVec3df_t resetVert(CScn* scn, indexedVec3df_t vert);
	indexed_vertices getVertices(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf);
};