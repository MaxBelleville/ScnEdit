#ifndef CSCN_H_
#define CSCN_H_
#include "CEnt.h"
#include "CSolid.h"
#include "CLightmap.h"

using namespace std;
using namespace irr;
using namespace video;

enum N_TYPE {
	N_UNK1 = 0, N_VERTS, N_UVPOS, N_VERTIDXS, N_PLANES, N_NODES, N_SURFS, N_CELLS, N_UNK2
};

class CScn
{
private:
	void reset();

public:
	header_t* header;
	core::array<CSolid*> solids;
	core::array <CEnt*> ents;
	CLightmap* lmap = new CLightmap();

	inline u32 getVersion() const {
		return header->version;
	}

	//Unsure if I should make not static,these aren't really related to the scn but rather helpful entity contianers.
	inline static core::array<CEnt*> cells;
	inline static core::array<CEnt*> ambients;
	inline static core::array<CEnt*> func_solidref;
	inline static CEnt* swt_start;

	//get cell by index as defined by cell_index field
	static CEnt* getCell(u32 cell_index);
	static CEnt* getAmbientByCell(const char* name);
	static CEnt* getGlobalAmbient();
	static CEnt* getSolidRef(s32 solidref_index);

	CScn();
	~CScn();

	inline core::array<CSolid*> getAllSolids() const
	{
		return solids;
	}

	inline core::array<CEnt*> getAllEnts()
	{
		return ents;
	}

	void collectLocalLightmap();

	//returns pointer to CSolid from index or NULL if none
	inline CSolid* getSolid(u32 idx)
	{
		return (idx < header->n_solids) ? solids[idx] : NULL;
	}

	//returns pointer to CEnt from index or NULL if none
	inline CEnt* getEnt(u32 idx)
	{
		return idx < header->n_ents ? ents[idx] : NULL;
	}

	inline CLightmap* getLightmap()
	{
		return lmap;
	}

	inline u32 getSolidSize(bool bAll) const
	{
		if (!bAll) return 1;
		return header->n_solids;
	}

	inline u32 getTotalEnts() const
	{
		return header->n_ents;
	}

	inline u32 getTotalLights() const
	{
		return header->n_lights;
	}

	inline u32 getLightmapSize() const
	{
		return header->n_extralmaps;
	}
};

#endif
