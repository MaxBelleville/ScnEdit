#ifndef SCNTYPES_H_
#define SCNTYPES_H_

#include <SkylichtEngine.h>
//---------------------------------Start of Original SCN TYPES-----------------------------------

struct tempLight_t
{
	core::vector3df color;
	f32 radius;
	core::vector3df origin;
};

struct scnHeader_t
{
/* address | type| meaning*/
/*--------------------*/
/* 0x00 */	char    magic[4];
/* 0x04 */	u32     version;
/* 0x08 */	u32     datalen;
/* 0x0c */	u32     n_ents;
/* 0x10 */	u32     n_solids;
/* 0x14 */	u32     solid0_offset;  //should always be 0x98
/* 0x18 */	u32     solid0_length;	//lengths of solid[0], ie, worldspawn
/* 0x1c */	u32     solids_offset;  //offset to solids[1]
/* 0x20 */	u32     solids_length;    //length of all other solids
/* 0x24 */	u32     unk2;
/* 0x28 */	u32     ents_offset;
/* 0x2c */	u32     unk3;
/* 0x30 */	u32     unk4;
/* 0x34 */	u32     ents_offset2;
/* 0x38 */	u32     ents_size;
/* 0x4c */	u32     n_extralmaps;
/* 0x40 */	u32     lmaps_offset;   //lightmaps start address
/* 0x44 */	u32     n_lights;   //number of light entities
/*zeros follow...*/

};
struct scnSurf_t
{
	//IMPORTANT: keep order, we rely on it to read
	char texture[32];
	f32 unk[2];
	u8 flag1, flag2;
	u16 alpha;
	u16 lmsize_h, lmsize_v;
	u16 width, height; //width and height of tex, in pixels
	u32 vertidxstart;
	u16 planeidx;
	u16 vertidxlen;
	u16 hasVertexColors;
	char stuff2[10];
	u8 * shading;
//	char extra[?]	??
};

//#surface flags :
//#   1st Byte
//#       b0
//#       b1
//#       b2
//#       b3
//#       b4 - light backsides(also set when smooth is on)
//#       b5 - don't receive shadows
//#       b6 -
//#       b7 -
//#       b8 -
//
//#   2nd Byte
//#       b0
//#       b1
//#       b2
//#       b3
//#       b4
//#       b5
//#       b6 - no player clip ?
//#       b7 - water / mist - no clip ?
//#       b8 - non shootable


struct scnPlane_t
{
	f32 a,b,c,d;		//plane equation ax + by + cz + d = 0;
};
struct scnEntField_t
{
	char *key;
	char *value;
};

enum EScnMaterial
{
	ESM_DEFAULT     = 0x00,
	ESM_LIQUID      = 0x10,
	ESM_MUD         = 0x20,
	ESM_GRAVEL      = 0x30,
	ESM_PLASTER     = 0x40,
	ESM_CARPET      = 0x50,
	ESM_GLASS       = 0x60,
	ESM_WOOD        = 0x70,
	ESM_CREAKWOOD   = 0x80,
	ESM_BRICK       = 0x90,
	ESM_SHEETMETAL  = 0xA0,
	ESM_STEEL       = 0xB0
};

struct scnSwitchableLMapHeader_t {
	u32 solid;  //index of the solid it belongs to
	u32 sidx;   //surface index
	u32 unk;    // usually zero? In missiona its always 17. Maybe cell?
	u32 lmsize_h, lmsize_v; 
	u32 offset; //ofset into lmap data
};

struct scnLMapHeader_t {
	u16 pos;    //flat index into 128x128 matrix texture where light bitmap starts
	u16 b;     
	u32 offset;
	u16 cellidx; // check
	u16 unk; //can be -1
	f32 uv_mults[4]; 
	//these are w, h, x0, y0 that we need to multiply by a vertex regular uv
	//to get the lightmap uv
};

struct scnLMapLump_t {
	u32 size;
	s32 unk;
	s8* data; //byte[size]
};

struct scnNode_t  //16 bytes
{
	s16 plane;  //splitting plane idx
	s8 area;   //?
	u8 material;

	s16 node1;  //node in front of plane
	s16 node2;  //node behind plane
	s16 nodep;  //parent node

	s16 cell;   //cell index
	s16 specialGeomIdx;   // Index of special geometry (whose name is given in solid.names)
	s16 unk2;
};

struct scnPortal_t
{
	char name[32];
	s32 nextcell; //cell idx this portal looks into
	scnPlane_t plane;
	f32 unk;        //float ?
	s32 n_verts;    //number of verts defining the portal
	core::vector3df bb_verts[2]; //portal bounding box points
	core::vector3df * verts;
};

struct scnCellData_t //raw cell data bb, surfs and children.
{
	core::vector3df bb_verts[2];
	u16* surfsidxs;
	u16 n_surfs;
	scnCellData_t* children;
	int64_t bbsad;
	u16 n_children;

};

struct scnRawCell_t //raw cell means it's the cell read not from the entity list
{
	char name[32];
	s32 n_nodesidxs;
	s32 n_portals;
	s32 n3; //?
	char skyname[32];
	u16 * nodesidxs; //index of nodes
	scnPortal_t * portals;
	scnCellData_t bvh;
	core::array<scnCellData_t *> leafnode;
	core::array<u16> naivesurfs;
	//there is also more data here
};
struct vertProp_t
{
	u32 vertidxidx;
	u32 surf_vertidx;
	bool bShared;
	core::array<u32> sharesWith;
};

//---------------------------------END OF Original SCN TYPES-----------------------------------
//---------------------------------Start of Custom Types------------------------------------
struct indexedVec3df_t
{
	core::vector3df pos;
	u32 vertindx = 0;
	u32 solidindx = 0;
	u32 surfindx = 0;
	u32 surf_vertindx = 0;
};

//Simplify the 3d array type call so I don't nee to refer to it as vector<vectory<vector<u8>>>
typedef std::vector<std::vector<std::vector<u8>>> u8_3DArr;
//Simplify the 3d array type call so I don't nee to refer to it as array<array<u8>>
typedef std::vector<std::vector<u8>> u8_2DArr;

typedef std::pair<u16, u16> u16_pair;

enum KeyAugment {
	CtrlShift,
	Shift,
	Ctrl,
	None
};
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};

typedef std::pair<irr::EKEY_CODE, KeyAugment> key_pair;

typedef std::unordered_map<key_pair, bool, pair_hash> key_map; //key is (key code, augment) value is state.

typedef std::pair<core::array<indexedVec3df_t>, core::array<indexedVec3df_t>> indexed_vertices;

#endif
