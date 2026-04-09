#ifndef CSCNSOLID_H_
#define CSCNSOLID_H_
#include "CScnBSPTree.h"
#include "CScnLocalizedFace.h"
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
	int loadProjection(std::ifstream *);
	int loadCells(std::ifstream *);
	int loadNames(std::ifstream* file);
	int loadPortal(scnPortal_t *, std::ifstream *);
	int loadCellData(scnRawCell_t *,scnCellData_t * cell_data,std::ifstream * file);




	int calcUniqueTexturesNames(std::ifstream *);
	void buildBackTree();


	scnCellData_t* getBBFromSurf(u16 surfindx, scnCellData_t* cell_data);

//const static defColor = video::SColor(128,255,255,255);
public:
	u32 n_unk1, n_verts, n_uvpos, n_faceidx, n_planes, n_nodes, n_surfs, n_cells, n_names;
	u32 offset;
	u32 length;
	u32 solididx;
	int64_t* surfsad;  //array of offset in scn file to each surface i
	int64_t uvposad;    //offset in file to start of uvpos array
	int64_t projsad;   //offset in file to start of param frame array
	int64_t vertssad; //ofset for vertices
	int64_t uvidxsad; //offset for uvidxs 
	int64_t vertidxsad; //offset for vertidxs

	int64_t planessad; //ofset for planes
	int64_t lengthsad;
	scnSurf_t * surfs;
	scnProjectionBasis_t* projection;
	scnPlane_t * planes;
	core::vector3df	* verts;
	core::vector2df * uvpos;
	core::array<std::string> names;
	u32 * vertidxs;
	u32 * uvidxs;
	CScnBSPTree * tree;
	scnRawCell_t * rawcells;
	bool firstVal;
	//all texture names in the solid, each only once
	std::vector<std::string> textures;
	
	core::array<CScnLocalizedFace> local_faces; 

	//constructor - do nothing for now
	CScnSolid ();
	~CScnSolid();

	//load from file
	int loadSolid(std::ifstream*,u32);

	void extractSurfaces();
	void rebuildSurfaces();

	core::array<u32>* uvpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
	//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j
	core::array<u32>* vertpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
	//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j

	core::array<u32>* faceidxs_caller; //same idea. Also, for scn files, each uvidx is pointed by only one surface,
	//so, really no point in using an array<u32>[i], could use u32[i]

	inline s16 getCellAtPos(core::vector3df pos) const {
		s16 nodeindx = tree->findNodePos(pos);
		return tree->nodes[nodeindx].cell;
	}

	inline scnNode_t getNodeAtPos(core::vector3df pos) const {
		s16 nodeindx = tree->findNodePos(pos);
		return tree->nodes[nodeindx];
	}

	inline scnCellData_t* getBBFromSurf(u16 surfindx, u16 cellIndx) {
		return getBBFromSurf(surfindx, &rawcells[cellIndx].bvh);
	}

};

#endif
