#include "pch.h"

#include "Header/Base/CScn.h"

using namespace std;
using namespace irr;
using namespace video;

CScnLightmap::CScnLightmap() :
	current_atlas(NULL),
	hslmaps(NULL){}

CScnLightmap::~CScnLightmap()
{
	omults.clear();
	atlas.clear();

	if(hslmaps) 
		delete hslmaps;
	if (current_atlas) 
		delete current_atlas;
	curr_atlas_pos = core::vector3di(0, 0, 0);
	bitmap.clear();
	atlas_pos.clear();
	hlmaps.clear();
	lumps.clear();
}

int CScnLightmap::loadLightmap(std::ifstream* file, CScnSolid* solids, u32 n_solid, u32 n_extralmaps)
{
    hslmaps = new (std::nothrow) scnSwitchableLMapHeader_t[n_extralmaps];
    offset = file->tellg();
    read_generic(hslmaps, file, sizeof(scnSwitchableLMapHeader_t) * n_extralmaps);

    hl_offset = file->tellg();

    for (u32 i = 0; i < n_solid; i++) {
        scnLMapHeader_t* tmp_hlmap = new (std::nothrow) scnLMapHeader_t[solids[i].n_surfs];

        read_generic(tmp_hlmap, file, sizeof(scnLMapHeader_t) * solids[i].n_surfs);

        hlmaps.push_back(tmp_hlmap);
        if (i == 0) {
            for (u32 j = 0; j < solids[0].n_cells; j++) {
                scnLMapLump_t* tmp_lump = new scnLMapLump_t();
                // Ensure fields are initialized
                tmp_lump->data = nullptr;
                tmp_lump->size = read_u32(file);
                tmp_lump->unk = read_s32(file);

                if (tmp_lump->size > 0) {
                    tmp_lump->data = new s8[tmp_lump->size];
                    read_generic(tmp_lump->data, file, sizeof(s8) * tmp_lump->size);
                } else {
                    // leave data == nullptr, size == 0
                }
                lumps.push_back(tmp_lump);

            }
            lump_offset = file->tellg();
        }
    }

    for (u32 i = 0; i < hlmaps.size(); i++) {
        core::array<f32*> tmp_mults;

        for (u32 j = 0; j < solids[i].n_surfs; j++) {
            scnLMapHeader_t hlmap = hlmaps[i][j];
            f32* tmp_ptr = new f32[4];
            for (int k = 0; k < 4; k++) {
                tmp_ptr[k] = hlmap.uv_mults[k];
            }
            tmp_mults.push_back(tmp_ptr);
        }

        omults.push_back(tmp_mults);
    }
    createBitmaps(solids, n_solid);
    loaded = true;
    return n_extralmaps; // should be the same as number of hlmaps.
}

video::IImage* CScnLightmap::getAtlas(s32 indx) {
	return atlas[indx];
}
core::vector3di CScnLightmap::getAtlasPos(u32 solidi, u32 surfi) {
	u16_pair id = getMasterBitmapId(hlmaps[solidi][surfi]);
	return atlas_pos[id];
}

s8* CScnLightmap::getBitmap(CScnSolid* solids,u32 solididx, u32 surfidx)
{
    if (!solids) {
        os::Printer::log("getBitmap: solids == nullptr", ELL_ERROR);
        return nullptr;
    }
    if (solididx >= (u32)hlmaps.size()) {
        os::Printer::log("getBitmap: solididx out of range (solididx=%u, hlmaps.size=%zu)", ELL_ERROR);
        return nullptr;
    }
    if (!hlmaps[solididx]) {
        os::Printer::log("getBitmap: hlmaps[solididx] == nullptr (solididx=%u)", ELL_ERROR);
        return nullptr;
    }
    if (surfidx >= (u32)solids[solididx].n_surfs) {
        os::Printer::log("getBitmap: surfidx out of range (surfidx=%u, n_surfs=%u)", ELL_ERROR);
        return nullptr;
    }

    scnLMapHeader_t hlmap = hlmaps[solididx][surfidx];

    if (hlmap.cellidx >= (u32)lumps.size()) {
        os::Printer::log("getBitmap: hlmap.cellidx out of range", ELL_ERROR);
        return nullptr;
    }

    scnLMapLump_t* lump = lumps[hlmap.cellidx];
    if (!lump) {
        os::Printer::log("getBitmap: lump pointer is null", ELL_ERROR);
        return nullptr;
    }
    if (lump->size == 0 || lump->data == nullptr) {
        os::Printer::log("getBitmap: lump has no data", ELL_WARNING);
        return nullptr;
    }

    // Validate offset within lump
    if ((u32)hlmap.offset >= lump->size) {
        os::Printer::log("getBitmap: hlmap.offset outside lump size", ELL_ERROR);
        return nullptr;
    }

    s8* start = lump->data + hlmap.offset;
    return start;
}

u16_pair CScnLightmap::getMasterBitmapId(scnLMapHeader_t hlmap)
{
	return make_pair(hlmap.cellidx, hlmap.unk);
}
///Takes the raw lightmap data and converts it into a larger bitmap I can use in the renderer.
void CScnLightmap::createBitmaps(CScnSolid* solids,u32 n_solid)
{
    if (!solids) {
        os::Printer::log("createBitmaps: solids == nullptr", ELL_ERROR);
        return;
    }

    // Make loop bounds defensive: only iterate up to the minimum of provided solids and hlmaps
    u32 safe_max = (u32)hlmaps.size();
    if (n_solid < safe_max) safe_max = n_solid;

    CScnEnt* ambient = CScn::getGlobalAmbient();

    for (u32 i = 0; i < safe_max; i++) {
        if (!hlmaps[i]) {
            os::Printer::log("createBitmaps: skipping null hlmaps[%u]", ELL_WARNING);
            continue;
        }
        u32 surf_count = solids[i].n_surfs;
        for (u32 j = 0; j < surf_count; j++) {
            // bounds checks repeated here to be extra safe
            if (j >= (u32)solids[i].n_surfs) {
                os::Printer::log("createBitmaps: surf index out of range i=%u j=%u", ELL_ERROR);
                break;
            }

            scnLMapHeader_t hlmap = hlmaps[i][j];
            scnSurf_t surf  = solids[i].surfs[j];

            if (hlmap.unk == -1)
                continue; // Surfaces are not lit

            // Obtain bitmap pointer safely
            s8* lbitmap = getBitmap(solids, i, j);
            if (!lbitmap) {
                // log already done by getBitmap, skip this surface
                continue;
            }
            u16 lmapcell = hlmap.cellidx;
			CScnEnt* cellEnt = CScn::getCell(lmapcell);
           
			if (cellEnt&& !ambient) 
				ambient = CScn::getAmbientByCell(cellEnt->getField("TargetName"));

			std::string colorStr = "0.00 0.00 0.00";

			if (ambient) 
				colorStr = ambient->getField("color");

		       
			
			int color[3] = { 0,0,0 };

            core::array<std::string> split = str_split(colorStr.c_str(), " ");
			for (int c = 0; c < 3; c++)
				color[c] = clamp(int(round(std::stof(split[c]) * 255))-10,0,255);

            u16_pair id = getMasterBitmapId(hlmap);

            if (bitmap.find(id)==bitmap.end())
                bitmap[id] = vector<s8>(TEXSIZE * TEXSIZE * 3, 0);

            atlas_pos[id] = curr_atlas_pos;

            if (atlas_pos[id].X == 0 && atlas_pos[id].Y == 0) {
                core::dimension2du size(ATLASSIZE, ATLASSIZE);
                current_atlas= getVideoDriver()->createImage(ECF_R8G8B8, size);
            }

            core::vector2di atpos = core::vector2di(hlmap.pos % TEXSIZE, hlmap.pos / TEXSIZE);
			core::vector2di dim = core::vector2di(surf.lmsize_h, surf.lmsize_v);
			
			//Safety check to make sure it's out out of bounds for the texture size
			if ((atpos.X + dim.X <= TEXSIZE) && (atpos.Y + dim.Y <= TEXSIZE)) {

				for (int y = 0; y < dim.Y; ++y) {
					for (int x = 0; x < dim.X; ++x) {
						// Compute the linear index for bitmap(current atlas point)
						int bitmapIndex = ((atpos.Y + y) * TEXSIZE + (atpos.X + x)) * 3;
						// Compute the linear index for lightmap bitmap
						int lbitmapIndex = (y * dim.X + x) * 3;

						//Fill in all color based on light bitmap and add ambient if not beyond 255.
						for (int c = 0; c < 3; c++) {
							bitmap[id][bitmapIndex+c] = min(lbitmap[lbitmapIndex+c], 255);
							//Add ambiement lighting 
							if (lbitmap[lbitmapIndex+c] >= 0 && lbitmap[lbitmapIndex+c] + color[c] < 255)
								bitmap[id][bitmapIndex+c] += color[c];
						}

					} 
				}
			}

            // ensure current_atlas non-null before copy
            core::dimension2du rawSize(TEXSIZE, TEXSIZE);
            IImage* rawImage = getVideoDriver()->createImageFromData(ECF_R8G8B8, rawSize, bitmap[id].data(), false);
            if (rawImage) {
                if (current_atlas) {
                    rawImage->copyTo(current_atlas, core::vector2di(curr_atlas_pos.X, curr_atlas_pos.Y));
                } else {
                    os::Printer::log("createBitmaps: current_atlas is null, cannot copy", ELL_WARNING);
                }
                rawImage->drop();
            }
			if (curr_atlas_pos.X < ATLASSIZE) 
				curr_atlas_pos.X += TEXSIZE;
	
			if (curr_atlas_pos.X >= ATLASSIZE){
				curr_atlas_pos.X = 0;
				curr_atlas_pos.Y +=TEXSIZE;
			}
			if (curr_atlas_pos.Y >= ATLASSIZE) {
				atlas.push_back(current_atlas);
				curr_atlas_pos.X = 0;
				curr_atlas_pos.Y = 0;
				curr_atlas_pos.Z += 1;
			}

		}
	}

    if (current_atlas)
        atlas.push_back(current_atlas);
}

