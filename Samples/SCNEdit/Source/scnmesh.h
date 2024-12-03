
#include <irrlicht.h>
#include "scn.h"

using namespace irr;

core::array<scene::SMesh*> scnCreateMesh(IrrlichtDevice* device, CScn* scn, bool bLoadAll, bool bloadLightmap);
video::E_MATERIAL_TYPE getBaseEMT(u32 meshIndx, u32 i);
video::ITexture* getBaseLightmap(u32 meshIndx, u32 i);
core::stringw getScnMeshLog();