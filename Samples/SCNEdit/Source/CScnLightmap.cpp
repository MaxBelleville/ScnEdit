#include "pch.h"
#include "CScnLightmap.h"
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


ITexture* CScnLightmap::genLightMapTextures(IVideoDriver* driver, u32 solidi, u32 surfi, const char* color)
{
    int ar = BASE_AMBIENT_LIGHT,ag = BASE_AMBIENT_LIGHT, ab = BASE_AMBIENT_LIGHT;

    if (color) {
        core::array<std::string> strArray = str_split(color, " ");
        ar = min((int)(atof(strArray[0].c_str()) * 255)+ BASE_AMBIENT_LIGHT,255);
        ag = min((int)(atof(strArray[1].c_str()) * 255)+ BASE_AMBIENT_LIGHT,255);
        ab = min((int)(atof(strArray[2].c_str()) * 255)+ BASE_AMBIENT_LIGHT,255);
       // SAY("RGB %s %u %u %u\n", strArray[0].c_str(), ar, ag, ab);
    }
    // CScnEnt* ambientEnt = getGlobalAmbient();
    if (image == nullptr)
        image = driver->createImage(video::ECF_A8R8G8B8, core::dimension2du(TEXSIZE, TEXSIZE));
    u16_pair id = getMasterBitmapId(hlmaps[solidi][surfi]);
    for (u32 y = 0; y < TEXSIZE; y++) {
        for (u32 x = 0; x < TEXSIZE; x++) {
            vector<u8> rgb = bitmap[id][y][x];
            rgb[0] = min(rgb[0] + ar, 255);
            rgb[1] = min(rgb[1] + ag, 255);
            rgb[2] = min(rgb[2] + ab, 255);
            image->setPixel(x, y, SColor(255, rgb[0], rgb[1], rgb[2]));
        }
    }
    return driver->addTexture(("lmt"+to_string(solidi)+"-"+to_string(surfi)).c_str(), image);
}





u8_3DArr CScnLightmap::getBitmap(CScnSolid* solids,u32 solididx, u32 surfidx)
{
    scnLMapHeader_t hlmap = hlmaps[solididx][surfidx];
    scnSurf_t surf = solids[solididx].surfs[surfidx];
    scnLMapLump_t* lump = lumps[hlmap.cellidx];
    u32 w = surf.lmsize_h;
    u32 h = surf.lmsize_v;
    s8* start = lump->data + hlmap.offset;
    u8_3DArr array = u8_3DArr(h, u8_2DArr(w, vector<u8>(3, 0)));
    for (u32 y = 0; y < h; y++) {
        for (u32 x = 0; x < w; x++) {
            u32 index = (y * w + x) * 3;
            array[y][x][0] = start[index];
            array[y][x][1] = start[index+1];
            array[y][x][2] = start[index+2];
        }
    }
    return array;
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
            if (bitmap.find(id)==bitmap.end()) {
                bitmap[id] = u8_3DArr(TEXSIZE, u8_2DArr(TEXSIZE, vector<u8>(3, 0)));
            }
            u8_3DArr lbitmap= getBitmap(solids,i, j);
            u32 x0 = hlmap.pos % TEXSIZE;
            u32 y0 = hlmap.pos / TEXSIZE;
            scnSurf_t surf = solids[i].surfs[j];
            u32 w = surf.lmsize_h;
            u32 h = surf.lmsize_v;
            if ((x0 + w <= TEXSIZE) && (y0 + h <= TEXSIZE)) {
                for (int y = 0; y < h; ++y) {
                    for (int x = 0; x < w; ++x) {
                        for (int c = 0; c < 3; ++c) {
                            bitmap[id][y0 + y][x0 + x][c] = lbitmap[y][x][c];
                        }
                    }
                   
                }
            }
        }
    }
}

CScnLightmap::CScnLightmap()
{
    
}