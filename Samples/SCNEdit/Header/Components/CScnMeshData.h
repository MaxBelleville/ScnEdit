#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Header/Base/CScnLocalizedFace.h"
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
	core::array<scnProjectionBasis_t> proj_backup;
	core::array<int> hiddensurfs;
	core::array<surfaceBox_t> surfsels;

public:
	IMeshBuffer *MeshBuffer;
	

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CScnSolid* solid,CScnLightmap* lmap, CScnArguments*);
	void setLightmapVisible(bool);

	core::array<surfaceBox_t> select(CScnSolid* solid, core::triangle3df, bool bAdd);
	void deselect(CScnSolid* solid, int si);

	void deselectAll();
	void hide(CScnSolid* solid,bool bShared);
	void show();

	void setTexture(CScnSolid* solid, const char* path);
	void updateVert(CScnSolid* solid, vertBox_t vertsel, core::vector3df);
	
	void resetVert(CScnSolid* solid, vertBox_t vertsel);

	void updatePlane(CScnSolid* solid, int si);
	void updateUV(CScnSolid* solid, UVMode mode, core::vector2df add);
	void resetUV(CScnSolid* solid);

	inline int getSolidIdx() { return solidindx; };
	inline core::array<surfaceBox_t>* getSurfSelected() { return &surfsels; }

private: 
	bool try_load_texture(video::ITexture*& t,
		std::unordered_map<std::string, std::string>& cantFind, const wchar_t* baseDir, const char* texPath, bool&);


	surfaceBox_t collectSurfFromTri(CScnSolid* solid, core::triangle3df);
	void selectMat(CScnSolid* solid);
	void deselectMat(CScnSolid* solid, int si);


};