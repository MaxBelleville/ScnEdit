#include "pch.h"
#include "Header/Base/CScnLightmap.h"
using namespace std;
using namespace irr;
using namespace video;



CScnLightmap::~CScnLightmap()
{

}

int CScnLightmap::loadLightmap(std::ifstream* file, CScnSolid* solids, u32 n_solid, u32 n_extralmaps)
{
	hslmaps = new (std::nothrow) scnSwitchableLMapHeader_t[n_extralmaps];
	offset = file->tellg();
	read_generic(hslmaps, file, sizeof(scnSwitchableLMapHeader_t) * n_extralmaps);
	for (u32 i = 0; i < n_solid; i++) {
		scnLMapHeader_t* tmp_hlmap = new (std::nothrow) scnLMapHeader_t[solids[i].n_surfs];

		read_generic(tmp_hlmap, file, sizeof(scnLMapHeader_t) * solids[i].n_surfs);
		hlmaps.push_back(tmp_hlmap);
		if (i == 0) {
			for (u32 j = 0; j < solids[0].n_cells; j++) {
				scnLMapLump_t* tmp_lump = new scnLMapLump_t;
				tmp_lump->size = read_u32(file);
				tmp_lump->unk = read_s32(file);
				if (tmp_lump->size > 0) {
					tmp_lump->data = new s8[tmp_lump->size];
					read_generic(tmp_lump->data, file, sizeof(s8)* tmp_lump->size);
				}
				lumps.push_back(tmp_lump);
			}
		}
	}
	for (u32 i = 0; i < hlmaps.size(); i++) {
		core::array<f32*> tmp_mults;
		for (u32 j = 0; j < solids[i].n_surfs; j++) {
			scnLMapHeader_t hlmap = hlmaps[i][j];
			tmp_mults.push_back(hlmap.uv_mults);
		}
		omults.push_back(tmp_mults);
	}

	createBitmaps(solids, n_solid);
	return 0; // should be the same as number of hlmaps.
}


video::IImage* CScnLightmap::genLightMapTextures(u32 solidi, u32 surfi, const char* color)
{
	u16_pair id = getMasterBitmapId(hlmaps[solidi][surfi]);
	core::dimension2du size = core::dimension2du(TEXSIZE, TEXSIZE);
	return getVideoDriver()->createImageFromData(ECF_R8G8B8,size,bitmap[id].data());
}




s8* CScnLightmap::getBitmap(CScnSolid* solids,u32 solididx, u32 surfidx)
{
	scnLMapHeader_t hlmap = hlmaps[solididx][surfidx];
	scnSurf_t surf = solids[solididx].surfs[surfidx];
	scnLMapLump_t* lump = lumps[hlmap.cellidx];
	u32 w = surf.lmsize_h;
	u32 h = surf.lmsize_v;
	s8* start = lump->data + hlmap.offset;
 

	return start;
}

u16_pair CScnLightmap::getMasterBitmapId(scnLMapHeader_t hlmap)
{
	return make_pair(hlmap.cellidx, hlmap.unk);
}

void CScnLightmap::createBitmaps(CScnSolid* solids,u32 n_solid)
{
	for (u32 i = 0; i < hlmaps.size(); i++) {
		for (u32 j = 0; j < solids[i].n_surfs; j++) {
			scnLMapHeader_t hlmap = hlmaps[i][j];
			if (hlmap.unk == -1)
				continue; // Surfaces are not lit

			auto id = getMasterBitmapId(hlmap);
			scnSurf_t surf = solids[i].surfs[j];

			u32 w = surf.lmsize_h;
			u32 h = surf.lmsize_v;
			if (bitmap.find(id)==bitmap.end()) {
				bitmap[id] = vector<u8>(TEXSIZE * TEXSIZE * 3, 0);
			}
			s8* lbitmap= getBitmap(solids,i, j);
			u32 x0 = hlmap.pos % TEXSIZE;
			u32 y0 = hlmap.pos / TEXSIZE;
		  
			if ((x0 + w <= TEXSIZE) && (y0 + h <= TEXSIZE)) {
				for (int y = 0; y < h; ++y) {
					for (int x = 0; x < w; ++x) {
							// Compute the linear index for bitmap
							int bitmapIndex = ((y0 + y) * TEXSIZE + (x0 + x)) * 3;
							// Compute the linear index for lbitmap
							int lbitmapIndex = (y * w + x) * 3;
							bitmap[id][bitmapIndex] = min(lbitmap[lbitmapIndex],255);
							bitmap[id][bitmapIndex+1] = min(lbitmap[lbitmapIndex+1], 255);
							bitmap[id][bitmapIndex+2] = min(lbitmap[lbitmapIndex+2], 255);
					   
					} 
				}
			}
		}
	}
}

CScnLightmap::CScnLightmap()
{

}