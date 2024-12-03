#include "pch.h"
#if false
#include "scnmesh.h"
#include "util.h"
#include "main.h"
#include <set>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//each surface is a meshbuffer, and is added in turn to the SMesh that is the solid(0)
//the problem with this is that vertices are repeated, how can we fix this, by grabbing from a pool?
//TODO: think

//CONTINUE: make MeshBuffer SSharedMeshBuffer (which allows you to share vertices.

core::array <core::array<E_MATERIAL_TYPE>> basemtArr;
core::array <core::array<video::ITexture*>> baseLightmapArr;
core::stringw logs;

core::array<SMesh*> scnCreateMesh(IrrlichtDevice *device, CScn* scn, bool bLoadAll, bool bloadLightmap)
{
    core::stringw log=L"";
    CScnSolid* meshes = scn->getAllSolids();
    u32 totalSize = scn->getSolidSize(bLoadAll);
    CScnLightmap lmap = scn->getLightmap();
    std::set<std::string> cantFind({});
    log+=SAY("Creating mesh...\n");

    core::array<SMesh*> scnMeshes;

    scnMeshes.clear();

    //video::S3DVertex tVert;
    //u8 * ps;
   
    for (u32 s = 0; s < totalSize; s++) {
        SMesh* scnMesh = new SMesh();
        core::array<E_MATERIAL_TYPE> basemt(256);
        core::array<video::ITexture*> baseLightmap;
        CScnSolid* mesh = &meshes[s];
        video::ITexture* t = 0;
        core::array<u32> is;
        u16 alpha;

        char tmp[128];

        scnMesh->BoundingBox.reset(mesh->getVertice(0).Pos);
        scnMesh->BoundingBox.reset(mesh->verts[0].x, mesh->verts[0].y, mesh->verts[0].z);
        basemt.clear();
  
        //#include "pvs0.txt"
        //u32 vstart;
        //u16 istart;
     
        for (u32 i = 0; i < mesh->n_surfs; i++)
        {
            //if (is.binary_search(i) == -1)
              //  continue;
            //if (i==8)
              //  continue;
            E_MATERIAL_TYPE mt = EMT_SOLID;

            
            CDynamicMeshBuffer* buffer = new CDynamicMeshBuffer(EVT_2TCOORDS,EIT_16BIT);

            //vstart=mesh->surfs[i].vertidxstart;
            //u32 vlen=mesh->surfs[i].vertidxlen;

            alpha = mesh->surfs[i].alpha;
            //ps = mesh->surfs[i].shading;


            bool istga;
            
            sprintf_s(tmp, "./textures/%s.bmp", mesh->surfs[i].texture);
            t = 0;
            bool tmpContains = !cantFind.contains(tmp);
;           if (tmpContains) t = device->getVideoDriver()->getTexture(tmp);
        
            if (tmpContains && t == 0)cantFind.insert(tmp);
            istga = false;

            //TODO: instead of using the base directory to load the textures consider loading
            //all general textures first then we don't need to load them here
 
            if (t == 0)
            {
                sprintf_s(tmp, "%S/general/%s.bmp", BaseDirectory, mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t = device->getVideoDriver()->getTexture(tmp);
                if(tmpContains &&t == 0) cantFind.insert(tmp);
                istga = false;
            }

            if (t == 0)
            {
                sprintf_s(tmp, "./textures/%s.tga", mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t = device->getVideoDriver()->getTexture(tmp);
                if (tmpContains && t == 0) cantFind.insert(tmp);
                
                istga = true;
            }

            if (t == 0)
            {
                sprintf_s(tmp, "%S/general/%s.tga", BaseDirectory, mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t = device->getVideoDriver()->getTexture(tmp);
                if (tmpContains && t == 0) cantFind.insert(tmp);
                istga = true;
            }
            if (t != 0)
            {

                //buffer->Material.Textures[0]=t;

                ITexture* lt = nullptr;
                if (lmap.hasLightmaps()) {
                    u16 lmapcell = lmap.getCellIndex(s, i);

                    CScnEnt* cellEnt = scn->getCell(lmapcell);
                    CScnEnt* ambient = ambient = scn->getGlobalAmbient();
                    if (cellEnt) {
                        ambient = scn->getAmbientByCell(cellEnt->getField("TargetName"));
                    }
                    std::string color = "0.01 0.01 0.01";
                    if (ambient != NULL) color = ambient->getField("color");

                    char* next_token = nullptr;

                    if (bloadLightmap) lt = lmap.genLightMapTextures(device->getVideoDriver(), s, i, color.c_str());
  
                }
                // buffer->setHardwareMappingHint(EHM_DYNAMIC);
                buffer->Material.setTexture(0, t);

                if (bloadLightmap) buffer->Material.setTexture(1, lt);
                if (bloadLightmap) baseLightmap.push_back(lt);
                buffer->Material.Wireframe = false;
                buffer->Material.setFlag(EMF_LIGHTING, false);
                buffer->Material.setFlag(EMF_BACK_FACE_CULLING, true);
                if (istga)
                {
                    if (alpha == 255) {
                        mt = bloadLightmap ? video::EMT_LIGHTMAP_ALPHA_REF : video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
                    }
                    else
                        SAY("Texture of surface[%d] is tga and alpha=%i", i, alpha);
                }
                else
                {
                    //solid must be used because irrlicht doesn't display transparencies
                    //over transparencies properly (even if alpha=255, ie, fully opaque)
                    if (alpha == 255) {
                        mt = bloadLightmap ? video::EMT_LIGHTMAP : video::EMT_SOLID;
                    }
                    else
                        mt = video::EMT_TRANSPARENT_VERTEX_ALPHA;
                }

                //DEBUG test
                //mt = video::EMT_SOLID;

                buffer->Material.MaterialType = mt;
                
            }
            
            basemt.push_back(mt);
            scene::IVertexBuffer &vbuff = buffer->getVertexBuffer();
            scene::IIndexBuffer& ibuff = buffer->getIndexBuffer();
            vbuff.set_used(0);
            ibuff.set_used(0);
          
            mesh->calcSurfVertices(i, lmap.getMults(s, i), vbuff);
            mesh->calcSurfIndices(i,ibuff);
            buffer->setHardwareMappingHint(EHM_STATIC);
            ibuff.setHardwareMappingHint(EHM_STATIC);
            vbuff.setHardwareMappingHint(EHM_STATIC);
            /*
            buffer->Indices.push_back(0);
            buffer->Indices.push_back(1);
            buffer->Indices.push_back(2);
            for (u16 j=2;j<vlen-1;j++)
            {
                buffer->Indices.push_back(j);
                buffer->Indices.push_back(j+1);
                buffer->Indices.push_back(0);
            }

            for (int k=0;k<vlen;k++)
            {
                //tVert=Vertices[mesh->vertidxs[vstart+k]];
                tVert = mesh->getVertice(mesh->vertidxs[vstart+k]);
                tVert.TCoords.X=mesh->uvpos[mesh->uvidxs[vstart+k]].u;
                tVert.TCoords.Y=mesh->uvpos[mesh->uvidxs[vstart+k]].v;

                if (alpha!=255)
                    tVert.Color=video::SColor(alpha,255,255,255);

                if (mesh->surfs[i].more)
                {
                    tVert.Color = video::SColor(ps[3],ps[0],ps[1],ps[2]);
                    ps+=4;
                }

                buffer->Vertices.push_back(tVert);
            }*/

            for (s32 l = 0; l < mesh->surfs[i].vertidxlen; ++l)
            {
                scnMesh->BoundingBox.addInternalPoint(vbuff[l].Pos);
            };
            //buffer->append(lbuffer);
            scnMesh->addMeshBuffer(buffer);
            buffer->drop();
      
        }
        scnMeshes.push_back(scnMesh);
        basemtArr.push_back(basemt);
        baseLightmapArr.push_back(baseLightmap);
    }
    log += SAY("Done.\n");
    logs = log;
    return scnMeshes;
}


E_MATERIAL_TYPE getBaseEMT(u32 meshIndx, u32 i)
{
    return basemtArr[meshIndx][i];
}

ITexture* getBaseLightmap(u32 meshIndx, u32 i) {
    return baseLightmapArr[meshIndx][i];
}

core::stringw getScnMeshLog(){
    return logs;
}
#endif