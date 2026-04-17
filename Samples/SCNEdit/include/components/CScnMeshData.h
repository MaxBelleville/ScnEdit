#pragma once
#include "include/core/CScn.h"
#include "include/io/CArguments.h"
#include "include/core/CLocalizedFace.h"

struct BackupData {
	core::array<S3DVertex2TCoords> vertices;
	core::array<u32> indices;
};

class CScnMeshData : public CRenderMeshData
{
protected:
	u32 solidindx = 0;
	core::array<IImage*>lmapImages;
	core::array<BackupData> vis_backup;
	core::array<projBasis_t*> proj_backup;
	core::array<int> hiddensurfs;
	core::array<surfaceBox_t> surfsels;

public:
	IMeshBuffer* MeshBuffer;

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CSolid* solid, CLightmap* lmap, CArguments*);
	void setLightmapVisible(bool);

	core::array<surfaceBox_t> select(CSolid* solid, core::triangle3df, bool bAdd);
	void deselect(CSolid* solid, int si);

	void deselectAll();
	void hide(CSolid* solid, bool bShared);
	void show();

	void setTexture(CSolid* solid, const char* path);
	void updateVert(CSolid* solid, vertBox_t vertsel, core::vector3df);

	void resetVert(CSolid* solid, vertBox_t vertsel);

	void updatePlane(CSolid* solid, int si);
	void updateUV(CSolid* solid, UVMode mode, core::vector2df add);
	void resetUV(CSolid* solid);

	inline int getSolidIdx() { return solidindx; };
	inline core::array<surfaceBox_t>* getSurfSelected() { return &surfsels; }

private:
	bool try_load_texture(video::ITexture*& t,
		std::unordered_map<std::string, std::string>& cantFind, const wchar_t* baseDir, const char* texPath, bool&);

	surfaceBox_t collectSurfFromTri(CSolid* solid, core::triangle3df);
	void selectMat(CSolid* solid);
	void deselectMat(CSolid* solid, int si);
};