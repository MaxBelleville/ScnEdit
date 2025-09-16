#ifndef CSCNSOLID_H_
#define CSCNSOLID_H_
#include "CScnBSPTree.h"
#include "scntypes.h"
#include "util.h"
class CScnSolid
{
private:

	int loadSurfs(std::ifstream *);
	int loadNodes(std::ifstream *);
	int loadPlanes(std::ifstream *);
	int loadVerts(std::ifstream *);
	int loadUVPos(std::ifstream *);
	int loadVertIdxs(std::ifstream *);
	int loadUVIdxs(std::ifstream *);
	int loadParamFrames(std::ifstream *);
	int loadCells(std::ifstream *);
	int loadNames(std::ifstream* file);
	int loadPortal(scnPortal_t *, std::ifstream *);
	int loadCellData(scnRawCell_t *,scnCellData_t * cell_data,std::ifstream * file);
	scnCellData_t* getBBFromSurf(u16 surfindx, scnCellData_t* cell_data);
	int calcUniqueTexturesNames(std::ifstream *);
	void buildBackTree();


//const static defColor = video::SColor(128,255,255,255);
public:
	u32 n_unk1, n_verts, n_uvpos, n_vertidxs, n_planes, n_nodes, n_surfs, n_cells, n_names;
	u32 offset;
	u32 length;
	u32 solididx;
	int64_t* surfsad;  //array of offset in scn file to each surface i
	int64_t uvposad;    //offset in file to start of uvpos array
	int64_t paramsad;   //offset in file to start of param frame array
	int64_t vertssad; //ofset for vertices
	int64_t planessad; //ofset for planes
	int64_t lengthsad;
	scnSurf_t * surfs;
	scnSurfParamFrame_t* paramFrames;
	scnPlane_t * planes;
	core::vector3df	* verts;
	core::vector2df * uvpos;
	core::array<const char*> names;
	u32 * vertidxs;
	u32 * uvidxs;
	CScnBSPTree * tree;
	scnRawCell_t * rawcells;
	bool firstVal;
	//all texture names in the solid, each only once
	std::vector<std::string> textures;
	
	//constructor - do nothing for now
	CScnSolid ();
	~CScnSolid();

	//load from file
	int loadSolid(std::ifstream*,u32);
	s16 getCellAtPos(core::vector3df pos) const;
	scnCellData_t* getBBFromSurf(u16 surfindx, u16 cellindx);
	scnNode_t getNodeAtPos(core::vector3df pos) const;
	

#ifdef __IRRLICHT_H_INCLUDED__
	//function only returns vertice from array
	//so it doesn't include surface specifics like color, tcoords or normal
	//irrlicht specific
	inline video::S3DVertex2TCoords getVertice(u32 idx)
	{
		video::S3DVertex2TCoords vert = video::S3DVertex2TCoords(
					verts[idx].X,verts[idx].Y,verts[idx].Z,        //coords
					1,1,1,video::SColor(255,255,255,255),0,1,0,1);
		return vert;
	}
	void calcSurfVertices(u32 surfidx, f32*, scene::IVertexBuffer*, u16);
	void calcSurfIndices(u32 surfidx, scene::IIndexBuffer*);
	core::array<u32> * uvpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
									//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j
	core::array<u32>* vertpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
	//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j

	core::array<u32> * uvvertidxs_caller; //same phylosophy. Also, for scn files, each uvidx is pointed by only one surface,
										//so, really no point in using an array<u32>[i], could use u32[i]



#else
	vector<u16> calcSurfIndices(u32 surfidx);



#endif
};

#endif
