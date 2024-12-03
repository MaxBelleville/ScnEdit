#include "pch.h"
#if false

using namespace irr;
using namespace core;
using namespace scene;

CSelector::CSelector(CScn* scnin, core::array<SMesh*> scnmeshesin, u32 total, IrrlichtDevice* device)
{
    scn = scnin;
    scnMeshes = scnmeshesin;
    totalSize = total;
    for (int i = 0; i < total; i++) {
        redSurfs.push_back(core::array<u32>(32));
        blueSurfs.push_back(core::array<u32>(32));
    }

    //load textures
    char tex[256];

    sprintf_s(tex, "%s/red.bmp", getBaseDirectory());
    redtex = device->getVideoDriver()->getTexture(tex);

    sprintf_s(tex, "%s/blue.bmp", getBaseDirectory());
    bluetex = device->getVideoDriver()->getTexture(tex);


}
CSelector::~CSelector()
{


}

void CSelector::selectSurf(u32 meshIndx,s32 si, bool bAppend = false)     //s32 because it can be -1
{
    if (si == -1) return;   //if none selected, do nothing

    clearBlueSurfs();

    /*toggle when: bAppend
                 !bAppend && (redSurfs.size()==1 %% redSurfs[0]==si)*/

    if (!bAppend && !(redSurfs[meshIndx].size() == 1 && redSurfs[meshIndx][0] == si))
    {
        clearRedSurfs(); //if not append, clear all surfs first
    }

    //linear_search, so that last surface in array is always the latest selected
    s32 ri = redSurfs[meshIndx].linear_search(si);
    s32 bi = blueSurfs[meshIndx].linear_search(si);

    if (ri > -1 && bi > -1)
        error(1, "CSelector:: Selected surface is in redSurfs and blueSurfs!");

    if (ri > -1) // if it's red, delete it
    {
        remRedSurf(meshIndx,si);
    }
    else    //if not, make it
    {
        makeSurfRed(meshIndx,si);
    }

    //it's easier to just delete all blue, and add them again. It's not pretty but works

    makeSurfBlue(meshIndx,calcSharedSurfaces(meshIndx,redSurfs[meshIndx]));
    //SAY("bAppend=%d\n",bAppend);
    log=SAY("redSurfs:"); for (u32 i = 0; i < redSurfs[meshIndx].size(); i++)  log += SAY(" %d", redSurfs[meshIndx][i]);  log += SAY("\n");
    log+=SAY("blueSurfs:"); for (u32 i = 0; i < blueSurfs[meshIndx].size(); i++)  log += SAY(" %d", blueSurfs[meshIndx][i]);  log += SAY("\n");
    
    clearSelectedVerts();

    makeSelectedVerts(meshIndx);
    
}




/*if (i!=-1)
{

    //add new sel surface, paint it red
    s32 idx = SelSurfs.linear_search(selsurf);
    if (idx > -1) //if surface already red, clear it

    {
        SelSurfs.erase(idx);
        paintSurfacesClear(SelSurfs[idx]);
    }
    else          //if surface is either blue or not selected, make it red
    {
        SelSurfs.push_back(selsurf);
        paintSurfacesRed(selsurf);
    }

    //add new shared surfaces, paint them blue
    array<u32> ss = selectSharedSurfaces(selsurf);
    for (u32 i=0; i<ss.size(); i++)
    {
        //only add to blue surfaces if it is not in red surfaces
        if (SelSurfs.binary_search(ss[i])==-1)
        {
            SharedSurfs.push_back(ss[i]);
            paintSurfacesBlue(ss[i]);
        }
    }
}*/


void CSelector::makeSurfRed(u32 meshIndx,core::array<u32> s) { 
    for (u32 i = 0; i < s.size(); i++) 
        makeSurfRed(meshIndx,s[i]);
}
void CSelector::makeSurfRed(u32 meshIndx, u32 si)
{

    if (redSurfs[meshIndx].linear_search(si) > -1) return;
    if (blueSurfs[meshIndx].linear_search(si) > -1) remBlueSurf(meshIndx,si);

    paint(meshIndx,si, RED);
    redSurfs[meshIndx].push_back(si);
}

void CSelector::makeSurfBlue(u32 meshIndx,core::array<u32> s)
{
    for (u32 i = 0; i < s.size(); i++)
        makeSurfBlue(meshIndx,s[i]);
}
void CSelector::makeSurfBlue(u32 meshIndx,u32 si)
{
    if (blueSurfs[meshIndx].linear_search(si) > -1) return;

    //difference is, if surface is already red, don't make it blue
    if (redSurfs[meshIndx].linear_search(si) > -1) return;

    paint(meshIndx,si, BLUE);

    blueSurfs[meshIndx].push_back(si);
}

//for many surfs, use clearRedSurfs instead
void CSelector::remRedSurf(u32 meshIndx,u32 si)
{
    paint(meshIndx,si, CLEAR_RED);
    redSurfs[meshIndx].erase(redSurfs[meshIndx].linear_search(si));
}
void CSelector::remBlueSurf(u32 meshIndx,u32 si)
{
    paint(meshIndx,si, CLEAR_BLUE);
    blueSurfs[meshIndx].erase(blueSurfs[meshIndx].linear_search(si));
}

void CSelector::clearRedSurfs()  //faster
{
    for (u32 i = 0; i < totalSize; i++) {
        paint(i,redSurfs[i], CLEAR_RED);
        redSurfs[i].set_used(0);
    }
    
}

void CSelector::clearBlueSurfs()
{
    for (u32 i = 0; i < totalSize; i++) {
        paint(i, blueSurfs[i], CLEAR_BLUE);
        blueSurfs[i].set_used(0);
    }
}

void CSelector::clear() {
    clearRedSurfs();
    clearBlueSurfs();
    clearSelectedVerts();
}

void CSelector::clearSelectedVerts()
{
    sharedVerts.set_used(0);
    selVerts.set_used(0);
}

void CSelector::paint(u32 meshIndx, core::array<u32> ss, E_PAINT_TYPE pt)
{
    for (u32 i = 0; i < ss.size(); i++)
        paint(meshIndx,ss[i], pt);
}
void CSelector::paint(u32 meshIndx, u32 si, E_PAINT_TYPE pt)
{

    //SMaterial * mat = &scnMesh->getMeshBuffer(i)->Material;
    if (scnMeshes[meshIndx]->getMeshBufferCount() >= si) {
        CDynamicMeshBuffer* buffer = (CDynamicMeshBuffer*)scnMeshes[meshIndx]->getMeshBuffer(si);

        switch (pt)
        {
        case (RED):
            prevtexRed[si] = buffer->getMaterial().getTexture(1);
            prevtype[si] = buffer->getMaterial().MaterialType;
            buffer->getMaterial().setTexture(1, redtex);
            buffer->getMaterial().MaterialType = EMT_LIGHTMAP;
            break;
        case (BLUE):
            prevtexBlue[si] = buffer->getMaterial().getTexture(1);
            prevtype[si] = buffer->getMaterial().MaterialType;
            buffer->getMaterial().setTexture(1, bluetex);
            buffer->getMaterial().MaterialType = EMT_LIGHTMAP;
            break;
        case (CLEAR_RED):
            buffer->getMaterial().setTexture(1, prevtexRed.at(si)); //this is necessary
            buffer->Material.MaterialType = prevtype.at(si);
            break;
        case (CLEAR_BLUE):
            buffer->getMaterial().setTexture(1, prevtexBlue.at(si)); //this is necessary
            buffer->Material.MaterialType = prevtype.at(si);
            break;
        }
    }
}



//TODO: improve this mess.
void CSelector::makeSelectedVerts(u32 meshIndx) {
    CScnSolid* solid = scn->getSolid(meshIndx);
    u32 swi;
    vertProp_t vp;

    //Red surfaces and shared
    for (u32 i = 0; i < redSurfs[meshIndx].size(); i++) {
        u32 r = redSurfs[meshIndx][i];
        myarray< vertProp_t > vprops;
        vprops.push_back(getSurfVertProps(meshIndx, r));

        while (vprops.iterate(vp)) {
            point3f vert = solid->verts[solid->vertidxs[vp.vertidxidx]];
            i_point3f indexedVert = { vert.x,vert.y,vert.z,vp.vertidxidx,meshIndx,r,vp.surf_vertidx };
            arrayu sharesWith;
            sharesWith.push_back(vp.sharesWith);

            while (sharesWith.iterate(swi)) {
                s32 bi = blueSurfs[meshIndx].binary_search(swi);
                if (bi > -1) {
                    //SAY("Red Surfs[%d]: vertex [%d]: shared with blue surf %d\n", r, vp.surf_vertidx, blueSurfs[bi]);
                    //SAY("- Shared Surfs[%d]: vertex [%d]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.x, vert.y, vert.z);
                    sharedVerts.push_back(indexedVert);
                    break;
                }
                else {
                    // SAY("Red Surfs[%d]: vertex [%d]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.x, vert.y, vert.z);
                    selVerts.push_back(indexedVert);
                }
            }
            if (sharesWith.size() == 0) {
                // SAY("Red Surfs[%d]: vertex [%d]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.x, vert.y, vert.z);
                selVerts.push_back(indexedVert);
            }
        }

    }
    ////Blue surfaces.
    //for (u32 i = 0; i < blueSurfs[meshIndx].size(); i++) {
    //    u32 b = blueSurfs[meshIndx][i];
    //    myarray < vertProp_t > vprops;
    //    vprops.push_back(getSurfVertProps(meshIndx, b));

    //    while (vprops.iterate(vp)) {
    //        point3f vert = solid->verts[solid->vertidxs[vp.vertidxidx]];
    //        i_point3f indexedVert = { vert.x,vert.y,vert.z,vp.vertidxidx,b,meshIndx };
    //        arrayu sharesWith;
    //        sharesWith.push_back(vp.sharesWith);

    //        while (sharesWith.iterate(swi)) {
    //            s32 ri = redSurfs[meshIndx].binary_search(swi);
    //            if (ri > -1) {
    //                bool foundBox = false;//Ensures there are no duplicate shared boxes.
    //                for (u32 i = 0; i < sharedVerts.size(); i++) {
    //                    if (sharedVerts[i].x == vert.x && sharedVerts[i].y == vert.y && sharedVerts[i].z == vert.z) {
    //                        foundBox = true;
    //                        break;
    //                    }
    //                }
    //                if (!foundBox) {
    //                    //SAY("Blue Surfs[%d]: vertex [%d]: shared with red surf %d\n", b, vp.surf_vertidx, redSurfs[bi]);
    //                    //SAY("- Shared Surfs[%d]: vertex [%d]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.x, vert.y, vert.z);
    //                    sharedVerts.push_back(indexedVert);
    //                    break;
    //                }
    //            }
    //            else {
    //                bool foundBox = false;//Ensures there are no duplicate shared boxes.
    //                for (u32 i = 0; i < selVerts.size(); i++) {
    //                    if (selVerts[i].x == vert.x && selVerts[i].y == vert.y && selVerts[i].z == vert.z) {
    //                        foundBox = true;
    //                        break;
    //                    }
    //                }
    //                if (!foundBox) {
    //                    // SAY("Blue Surfs[%d]: vertex [%d]: (x) %.0f (y) %.0f (z) %.0f\n", b, vp.surf_vertidx, vert.x, vert.y, vert.z);
    //                    selVerts.push_back(indexedVert);
    //                }
    //            }
    //        }
    //    }
    //}
}


core::array<vertProp_t> CSelector::getSurfVertProps(u32 meshIndx, u32 si)
//this is very much repeated from calcSharedSurfs, but use for now
{
    CScnSolid* solid = scn->getSolid(meshIndx);
    scnSurf_t* surfi = &solid->surfs[si];

    core::array<u32>* c_uvidxs;
    core::array<u32>* c_surfs;
    core::array<vertProp_t> vprops;



    //run through vertices, to see how many are shared
    bool bShared;

    for (u32 j = 0; j < surfi->vertidxlen; j++)
    {
        //start new index
        vertProp_t vp;

        vp.bShared = false; //assume vertex is not shared
        vp.surf_vertidx = j;    //redundant
        vp.vertidxidx = surfi->vertidxstart + j; //also redundant;

        vp.sharesWith.set_used(0);

        u32 uvidx = solid->uvidxs[surfi->vertidxstart + j]; //uvidx of this vertex


        c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

        for (u32 k = 0; k < c_uvidxs->size(); k++)              //run through all of these uvidxs
        {
            c_surfs = &solid->uvidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx

            //i don't think it ever happened where the same uvidx is called by more than one surface (ie, c_surfs->size() is always 1)
            //although that would make much more sense, mr swat3 dev.
            for (u32 m = 0; m < c_surfs->size(); m++)
            {
                u32 cs = (*c_surfs)[m]; //caller surface index
                if (cs != si)
                {
                    vp.bShared = true; //this means this vertex is shared
                    if (vp.sharesWith.binary_search(cs) == -1) //if not alread added, add
                        vp.sharesWith.push_back(cs);
                }

            } //m
        } //k

        //CONTINUAR: boxes deviam ser desenhadas em todas as superficies! E rectas a unir os pontos se são diferentes no espaço
          //         implementar, em main, retexturing de redsurfs, afecta bluesurfs


        vprops.push_back(vp);
    } //j

    return vprops;

}


core::array<u32> CSelector::calcSharedSurfaces(u32 meshIndx,u32 si)
{

    f32 dx = 1.0, dy = 1.0, dz = 1.0;

    CScnSolid* solid = scn->getSolid(meshIndx);
    scnSurf_t* surfi = &solid->surfs[si];

    core::array<u32>* c_uvidxs;
    core::array<u32>* c_surfs;
    core::array<u32> ss(16); //shared surfaces of si (not including si)


    //run through vertices, to see how many are shared
    bool bShared;

    for (u32 j = 0; j < surfi->vertidxlen; j++)
    {
        bShared = false; //assume vertex is not shared

        u32 uvidx = solid->uvidxs[surfi->vertidxstart + j]; //uvidx of this vertex

        c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

        for (u32 k = 0; k < c_uvidxs->size(); k++)              //run through all of these uvidxs
        {
            c_surfs = &solid->uvidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx

            //i don't think it ever happened where the same uvidx is called by more than one surface (ie, c_surfs->size() is always 1)
            //although that would make much more sense, mr swat3 dev.
            for (u32 m = 0; m < c_surfs->size(); m++)
            {
                u32 cs = (*c_surfs)[m]; //caller surface index
                if (cs != si && ss.linear_search(cs) == -1)
                {
                    bShared = true; //this means this vertex is shared
                    ss.push_back(cs); //add surface to list of shared surfaces
                }

            } //m
        } //k

    } //j

    return ss;
}

core::array<u32> CSelector::calcSharedSurfaces(u32 meshIndx,core::array<u32>& surfs)
{
    core::array<u32> all;
    core::array<u32> ss;

    for (u32 i = 0; i < surfs.size(); i++)
    {
        ss = calcSharedSurfaces(meshIndx,surfs[i]);
        for (u32 j = 0; j < ss.size(); j++)
            if (all.linear_search(ss[j]) == -1 && surfs.linear_search(ss[j]) == -1)
                all.push_back(ss[j]);   //only add, if not already in array, or is not one of the surfs
        //this distinction should not be important, because makeSurfBlue only works in not already red
    }
    //note: calcSharedSurfaces(si) should not return i!

    return all;
}


#endif