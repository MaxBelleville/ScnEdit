#ifndef MAIN_H_
#define MAIN_H_


#if false
#include "scn.h"
#include "myarray.h"

//Declares more variables
extern irr::scene::ICameraSceneNode * camera;
extern wchar_t * BaseDirectory;

bool loadScnFile(io::path);
bool canChangeLightmap();
void indirectLoad(io::path filename);
bool closeCurrentScnFile();
void selectCurrent(bool bAppend=false);
bool scnRetexture(io::path);
bool scnRetexture_UV(f32 Ushift, f32 Vshift);
bool scnSaveFile();
void debugCells();
void debugCells(scnCellData_t , u16);
void uvgrid_increase();
void uvgrid_decrease();
void toggleUVresize();

void updateBufferUVfromScnSurf(u32 meshIndex,core::array<u32>);
void updateBufferUVfromScnSurf(u32 meshIndex,u32 s);
void selectUVVert(bool bAppend);

//core::std::array<u32> * calcVertsShared_b(u32 i);


//core::std::array<u32> selectSharedSurfaces(u32 i);
//core::std::array<u32> selectAllSharedSurfaces(u32 i);
wchar_t* getBaseDirectory();
void putBoxesInVerts(u32 s);
void putBoxesInSharedVerts(u32 s);
i_point3f getVertexIndex();
void sayProperties();
void lookAtVertex();

void updateIndex(bool increase);
void updateMeshAndNormal(i_point3f);

arrayu getSurfUVIdxs(u32 meshIndex, arrayu);
arrayu getSurfUVIdxs(u32 meshIndex, core::array<u32>);
arrayu getSurfUVIdxs(u32 meshIndex, u32 ss);

void export2obj();
void whereami();

void SetOriginalUV();
void changeAlpha();
void updateAlpha(std::string val, bool data);
void createAlpha();

void changeShading();
void updateShading(std::string val, bool data);
void createShading();
void toggleLightmap();
void changeFlag();
void updateFlag(std::string val, bool data);
void createFlag();

void updateMovementSpeed(f32 increment);
void moveEntity(s32 x,s32 y, s32 z);
void moveVertex(s32 x, s32 y, s32 z);

void addLogFromReciever(core::stringw);
void changePage(bool state,bool increase=false);
void openDebug();
#endif


#endif