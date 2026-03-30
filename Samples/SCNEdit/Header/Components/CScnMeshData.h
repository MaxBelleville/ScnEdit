#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Header/Base/util.h"
#include <future>
#include <thread>

struct BackupData {
	core::array<S3DVertex2TCoords> vertices;
	core::array<u32> indices;
};

class CScnMeshData : public CRenderMeshData
{
protected:
	u32 solidindx =0;
	core::array<IImage*>lmapImages;
	core::array<BackupData> vis_backup;
	core::array<BackupData> vert_backup;
	core::array<scnProjectionBasis_t> proj_backup;
	core::array<int> hiddensurfs;

public:
	IMeshBuffer *MeshBuffer;
	

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CScn* scn, CScnSolid* solid, CScnArguments*);
	void setLightmapVisible(bool);

	solidSelect_t getSurfaceIndx(CScn* scn, core::triangle3df);

	void select(int si, bool shared);
	void deselect(int si);
	void deselectAll();
	void hide(int si);
	void show();

	void setTexture(CScn* scn, const char* path,int si);
	void updateVert(CScn* scn, indexedVec3df_t& vert, core::vector3df);
	
	void resetVert(CScn* scn, indexedVec3df_t& vert);

	void updatePlane(CScn* scn, int si);
	void updateUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf, int, core::vector2df add);
	void resetUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf);

private: 
	void updateUVSurf(CScnSolid* solid, int si, int uvmode, core::vector2df uvShift);
	void resetUVSurf(CScnSolid* solid, int si);
	void updateMeshUV(CScn* scn, core::array < int > surf);
	void updateMeshVert(int si, int surf_vertidx, core::vector3df pos);

	bool try_load_texture(video::ITexture*& t,
		std::unordered_map<std::string, std::string>& cantFind, const wchar_t* baseDir, const char* texPath, bool&);

};