#ifndef SCNTYPES_H_
#define SCNTYPES_H_

#include <SkylichtEngine.h>
#include <unordered_set>
#include <optional>
//---------------------------------Start of Original SCN TYPES-----------------------------------

struct header_t
{
	/* address | type| meaning*/
	/*--------------------*/
	/* 0x00 */	char    magic[4];
	/* 0x04 */	u32     version;
	/* 0x08 */	u32     datalen;
	/* 0x0c */	u32     n_ents;
	/* 0x10 */	u32     n_solids;
	/* 0x14 */	u32     base_solid_offset;  //should always be 0x98
	/* 0x18 */	u32     base_solid_length;	//lengths of solid[0], ie, worldspawn
	/* 0x1c */	u32     extra_solid_offset;  //offset to solids[1]
	/* 0x20 */	u32     extra_solid_length;  //length of all other solids
	/* 0x24 */	u32     planes_offset; //Might not be correct
	/* 0x28 */	u32     ents_offset; //binary enity data(spawn objects from engine)
	/* 0x2c */	u32     ents_length; //ent length?
	/* 0x30 */	u32     ents_version; // no idea?
	/* 0x34 */	u32     ents_offset2;//entity stirng data(key and value pairs)
	/* 0x38 */	u32     ents_size;
	/* 0x4c */	u32     n_extralmaps;
	/* 0x40 */	u32     lmaps_offset;   //lightmaps start address
	/* 0x44 */	u32     n_lights;   //number of light entities
	/*zeros follow...*/
};
struct surf_t
{
	//IMPORTANT: keep order, we rely on it to read, 72 bytes
	char texture[32];
	core::vector2df lmoff; // Some sort of shift related to the offset
	u8 flag1, flag2;
	u16 alpha;
	u16 lmsize_h, lmsize_v;
	u16 width, height; //width and height of tex, in pixels
	u32 faceidxstart;
	u16 planeidx;
	u16 faceidxlen;
	u16 hasVertexColors;

	s16 _unk; //possibly padding
	s32 solidref_index;
	s16 _unk2; //possibly padding?
	u8* shading;
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

struct plane_t
{
	f32 a, b, c, d;		//plane equation ax + by + cz + d = 0;
};
struct entField_t
{
	char* key;
	char* value;
};

struct switchLMapHeader_t {
	u32 solid;  //index of the solid it belongs to
	u32 sidx;   //surface index
	u32 unk;    // usually zero? In missiona its always 17. Maybe cell?
	u32 lmsize_h, lmsize_v;
	u32 offset; //ofset into lmap data
};

struct lMapHeader_t {
	u16 pos;    //flat index into 128x128 matrix texture where light bitmap starts
	u16 b;
	u32 offset;
	u16 cellidx; // check
	u16 light_styles; //can be -1
	f32 uv_mults[4];
	//these are w, h, x0, y0 that we need to multiply by a vertex regular uv
	//to get the lightmap uv
};

struct lMapLump_t {
	u32 size;
	s32 compression_type; // might be related to texture flipping?
	s8* data; //byte[size]
};

//Project 3d to uv.
struct projBasis_t {
	core::vector3df u_axis;
	core::vector3df v_axis;
	core::vector3df origin;
};

struct node_t  //16 bytes
{
	s16 plane;  //splitting plane idx
	s8 area;   //? potentially used in determining which are visible via portals/cells an or outside of the map.
	u8 material;

	s16 node1;  //node in front of plane
	s16 node2;  //node behind plane
	s16 nodep;  //parent node

	s16 cell;   //cell index
	s16 specialGeomIdx;   // Index of special geometry (whose name is given in solid.names)
	s16 visframe; //assume it's padding as it's always zero, but it might be used for something else.
};

struct portal_t
{
	char name[32];
	s16 nextcell; //cell idx this portal looks into
	u8 flag1;
	u8 flag2;
	plane_t plane;
	f32 winding;
	s32 n_verts;    //number of verts defining the portal
	core::vector3df bb_verts[2]; //portal bounding box points
	core::vector3df* verts;
};

struct cellData_t //raw cell data bb, surfs and children.
{
	core::vector3df bb_verts[2];
	u16* surfsidxs;
	u16 n_surfs;
	core::array<cellData_t*> children;
	u16 n_children;
};

struct rawCell_t //raw cell means it's the cell read not from the entity list
{
	char name[32];
	s32 n_nodesidxs;
	s32 n_portals;
	s32 n_occluders;
	char skyname[32];
	u16* nodesidxs; //index of nodes
	core::array<portal_t*> portals;
	cellData_t bvh;
	core::array<cellData_t*> leafnode;
	core::array<u16> naivesurfs;
	//there is also more data here
};

// ---------------------------------END OF Original SCN TYPES-----------------------------------
// ---------------------------------Start of Custom Types------------------------------------

struct localVert_t {
	core::vector3df pos;
	core::vector2df uv;

	u32 parent_si;
	u32 localidx;

	u32 uvidx;
	u32 vertidx;
	u32 faceidx;
	bool hasShading;
	u8 color[4];
	core::array<localVert_t*> shared;
};

struct surfaceBox_t
{
	int solididx = 0;
	int si = 0;
	surfaceBox_t(int solidi, int si)
		: solididx(solidi), si(si) {
	}
};

struct vertBox_t {
	int solididx = 0;
	int si = 0;
	int localidx = 0;
	vertBox_t(int solidi, int si, int localidx)
		: solididx(solidi), si(si), localidx(localidx) {
	}
};

struct portalBox_t
{
	int cellidx = 0;
	int portalidx = 0;
	portalBox_t(int ci, int pi)
		: cellidx(ci), portalidx(pi) {
	}
};

typedef std::pair<u16, u16> u16_pair;

enum KeyAugment {
	AnyKey = -1,
	None,
	Shift,
	CtrlShift,
	Ctrl,
};

enum UVMode {
	Move,
	Resize,
	FlipH,
	FlipV
};

struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ h2;
	}
};

typedef std::pair<irr::EKEY_CODE, KeyAugment> key_pair;

typedef std::unordered_set<key_pair, pair_hash> key_map; //key is (key code, augment) value is state.

#endif
