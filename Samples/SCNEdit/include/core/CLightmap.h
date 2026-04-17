#ifndef CLIGHTMAP_H_
#define CLIGHTMAP_H_
#include "CSolid.h"
#include "CEnt.h"
class CLightmap
{
private:
	bool loaded = false;

	core::array <core::array<f32*>> omults;

	std::vector<video::IImage*> atlas;
	video::IImage* current_atlas;
	core::vector3di curr_atlas_pos = core::vector3di(0, 0, 0);
	std::unordered_map<u16_pair, core::vector3di, pair_hash> atlas_pos; // the z value = indx of atlas;
	std::unordered_map<u16_pair, std::vector<s8>, pair_hash> bitmap; // the z value = indx of atlas;

	s8* getBitmap(core::array<CSolid*>, u32, u32);

	inline u16_pair getMasterBitmapId(lMapHeader_t hlmap)
	{
		return std::make_pair(hlmap.cellidx, hlmap.light_styles);
	}

public:
	//constructor - do nothing for now
	CLightmap();
	~CLightmap();

	void buildLightmap(core::array<CSolid*> solids);

	int TEXSIZE = 128;
	int ATLASSIZE = 128;
	switchLMapHeader_t* hslmaps;
	core::array<lMapHeader_t*> hlmaps;
	core::array <lMapLump_t*> lumps;

	inline video::IImage* getAtlas(s32 indx) {
		return atlas[indx];
	}
	inline core::vector3di getAtlasPos(u32 solidi, u32 surfi) {
		u16_pair id = getMasterBitmapId(hlmaps[solidi][surfi]);
		return atlas_pos[id];
	}
	void createBitmaps(core::array<CSolid*>);

	inline u16 getCellIndex(u32 solidi, u32 surfi) {
		return hlmaps[solidi][surfi].cellidx;
	}
	inline void resetMults(u32 solidindx, u32 surfindx) {
		if (hslmaps && loaded) {
			for (int i = 0; i < 4; i++)
				hlmaps[solidindx][surfindx].uv_mults[i] = omults[solidindx][surfindx][i];
		}
	}
	inline f32* getMults(u32 solidindx, u32 surfindx) {
		if (hslmaps && loaded) return hlmaps[solidindx][surfindx].uv_mults;
		return nullptr;
	};
	inline bool hasLightmaps() { return loaded; }
	inline lMapHeader_t* getHLmap(u32 solidindx, u32 surfindx) { return &hlmaps[solidindx][surfindx]; }
	inline switchLMapHeader_t* getHSLmap(u32 extralight) { return &hslmaps[extralight]; }
};
#endif