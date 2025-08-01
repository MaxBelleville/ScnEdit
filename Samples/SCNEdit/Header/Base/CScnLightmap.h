#ifndef CSCNLIGHTMAP_H_
#define CSCNLIGHTMAP_H_
#include "CScnSolid.h"
#include "scntypes.h"
#include "CScnEnt.h"
class CScnLightmap
{
private:
	bool loaded=false;
	
	size_t offset = 0;
	size_t lump_offset =0;
	size_t hl_offset =0;
	core::array <core::array<f32*>> omults;
	
	std::vector<video::IImage*> atlas;
	video::IImage* current_atlas;
	core::vector3di curr_atlas_pos = core::vector3di(0,0,0);
	std::unordered_map<u16_pair, core::vector3di, pair_hash> atlas_pos; // the z value = indx of atlas;
	std::unordered_map<u16_pair,std::vector<s8>, pair_hash> bitmap; // the z value = indx of atlas;
	scnSwitchableLMapHeader_t* hslmaps;
	core::array<scnLMapHeader_t*> hlmaps;
	core::array <scnLMapLump_t*> lumps;

	s8* getBitmap(CScnSolid*, u32, u32);
	void createBitmaps(CScnSolid*,u32);
	u16_pair getMasterBitmapId(scnLMapHeader_t hlmap);


public:
	//constructor - do nothing for now
	CScnLightmap();
	~CScnLightmap();
	int TEXSIZE = 128;
	int ATLASSIZE = 128;

	int loadLightmap(std::ifstream*,  CScnSolid*, u32, u32);
	video::IImage* getAtlas(s32 indx);
	core::vector3di getAtlasPos(u32 solidi, u32 surfi);
	
	inline u16 getCellIndex(u32 solidi, u32 surfi) {
	   return hlmaps[solidi][surfi].cellidx;
    }
	inline void resetMults(u32 solidindx, u32 surfindx) {
		if (hslmaps && loaded) {
			for(int i=0;i<4;i++)
				hlmaps[solidindx][surfindx].uv_mults[i] = omults[solidindx][surfindx][i];
		
		}
	}
	inline f32* getMults(u32 solidindx, u32 surfindx) {
		if(hslmaps&&loaded) return hlmaps[solidindx][surfindx].uv_mults;
		return nullptr;
	};
	inline size_t getLumpOffset() const { return lump_offset; }
	inline size_t getHLOffset() const { return hl_offset; }
	inline size_t getOffset() const { return offset; }
	inline bool hasLightmaps() { return loaded; }
	inline scnLMapHeader_t* getHLmap(int i) { return hlmaps[i]; }
};
#endif