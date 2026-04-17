#ifndef CSOLID_H_
#define CSOLID_H_
#include "CBSPTree.h"
#include "CLocalizedFace.h"
class CSolid
{
private:

	cellData_t* getBBFromSurf(u16 surfindx, cellData_t* cell_data);

	//const static defColor = video::SColor(128,255,255,255);
public:
	u32 n_unk1, n_verts, n_uvpos, n_faceidx, n_planes, n_nodes, n_surfs, n_cells, n_names;
	u32 offset;
	u32 length;
	u32 solididx;

	CBSPTree* tree;

	core::array<surf_t*> surfs;
	core::array<projBasis_t*> projection;
	core::array<plane_t*> planes;

	core::array<char*> names;
	u32* vertidxs;
	u32* uvidxs;

	core::vector3df* verts;
	core::vector2df* uvpos;

	core::array <rawCell_t*> rawcells;
	bool firstVal;
	//all texture names in the solid, each only once
	core::array<std::string> textures;

	core::array<CLocalizedFace> local_faces;

	//constructor - do nothing for now
	CSolid();
	~CSolid();
	int calcUniqueTexturesNames();
	//load from file
	void buildBackTree();
	void extractSurfaces();
	void rebuildSurfaces();

	core::array<u32>* uvpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
	//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j
	core::array<u32>* vertpos_caller;  //for uvpos[i], this array contains the index of uvidxs that points to it,
	//ie, uvpos[uvidx[uvpos_caller[i][j]]] = uvpos[i], for all j

	core::array<u32>* faceidxs_caller; //same idea. Also, for scn files, each uvidx is pointed by only one surface,
	//so, really no point in using an array<u32>[i], could use u32[i]

	inline s16 getCellAtPos(core::vector3df pos) const {
		s16 nodeindx = tree->findNodePos(planes, pos);
		return tree->nodes[nodeindx]->cell;
	}

	inline node_t* getNodeAtPos(core::vector3df pos) const {
		s16 nodeindx = tree->findNodePos(planes, pos);
		return tree->nodes[nodeindx];
	}

	inline cellData_t* getBBFromSurf(u16 surfindx, u16 cellIndx) {
		return getBBFromSurf(surfindx, &rawcells[cellIndx]->bvh);
	}
};

#endif
