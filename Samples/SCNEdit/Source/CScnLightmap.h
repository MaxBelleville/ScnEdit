#ifndef CSCNLIGHTMAP_H_
#define CSCNLIGHTMAP_H_
#include "CScnSolid.h"
#include "scntypes.h"
#include "util.h"
class CScnLightmap
{
private:
    bool CHECKS = true;
    int TEXSIZE = 128;
    int BASE_AMBIENT_LIGHT = 40;
    size_t offset = 0;
    core::array <core::array<f32*>> omults;
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
    video::IImage* image = nullptr;
    std::unordered_map<u16_pair, u8_3DArr, pair_hash> bitmap;
    scnSwitchableLMapHeader_t* hslmaps;
    core::array<scnLMapHeader_t*> hlmaps;
    core::array <scnLMapLump_t*> lumps;
    u8_3DArr getBitmap(CScnSolid*, u32, u32);
    void createBitmaps(CScnSolid*,u32);
    u16_pair getMasterBitmapId(scnLMapHeader_t hlmap);

    //Do something related to changing the textures.




public:
    //constructor - do nothing for now
    CScnLightmap();
    ~CScnLightmap();
    int loadLightmap(std::ifstream*,  CScnSolid*, u32, u32);
    video::ITexture* genLightMapTextures(video::IVideoDriver* driver, u32 solidi, u32 surfi, const char*);
   u16 getCellIndex(u32 solidi, u32 surfi) {
       return hlmaps[solidi][surfi].cellidx;
   }

    f32* getMults(u32 solidindx, u32 surfindx) {
        if(hslmaps) return hlmaps[solidindx][surfindx].uv_mults;
        return nullptr;
    };
    size_t getOffset() const { return offset; }
    bool hasLightmaps() { return hslmaps; }
};
#endif