//this is a storage for no longer used functions i might need
//don't actually compile
#include "pch.h"
#if false

void selectUVVert(bool bAppend)
{
    CScnSolid * solid = scn->getSolid(0);

    if (!node || !scnMesh)
        return;

    core::line3d<f32> line;
    ISceneCollisionManager * scmg=device->getSceneManager()->getSceneCollisionManager();

	line = scmg->getRayFromScreenCoordinates ( device->getCursorControl()->getPosition());
	//line.start = camera->getPosition();
	//line.end = line.start +
      //  (camera->getTarget() - line.start).normalize() * 1000.0f;

    core::vector3df intersection;
    selected = scmg->getCollisionPoint(line, selector, intersection, tri);
    SAY("intersection: %f %f %f\n",intersection.X,intersection.Y,intersection.Z);

    //now we are interested in the intersection point. See if it's close to any of the boxes
    for (u32 i=0; i<sel->boxes.size(); i++)
    {
        line3df diff = line3df(intersection,sel->boxes[i].getCenter());
        if (diff.getLengthSQ()<10.0)
            SAY("Point inside box %d\n",i);
    }

    sel->boxes[i].


}
}

array<u32> CSelector::calcSharedVerts(u32 i)
//TODO: improve performance
{

    bool bShared = false;
    f32 dx=1.0, dy=1.0, dz=1.0;


    CScnSolid * solid = scn->getSolid(0);
    scnSurf_t * surfi = &solid->surfs[i];

    array<u32> * callers;
    array<u32> allcallers(16);

    boxes.set_used(0);

    //run through vertices, to see how many are shared
    for (u32 j=0; j<surfi->vertidxlen; j++)
    {
        u32 uvpos_idx = solid->uvidxs[surfi->vertidxstart + j];

        callers = &solid->uvpos_caller[uvpos_idx];  //array with the indices of surfaces that have this uvidx
        if (callers->size() == 1) continue;     //if only one calls, then it is not shared

        bShared = true;

        for (u32 k=0; k<callers->size(); k++)
        {
            u32 callers_uvidx = (*callers)[k];

            //add these callers to the allcallers list, except for uvpos_idx itself
            if (callers_uvidx != uvpos_idx)
            {
                allcallers.push_back(callers_uvidx)
                //draw boxes around the shared vertices
                point3f vert = solid->verts[solid->vertidxs[(*callers)[k]]];
                boxes.push_back(aabbox3df(vert.x-dx,vert.y-dy,vert.z-dz,vert.x+dx,vert.y+dy,vert.z+dz));
            }
        }
    }

   if (bShared)
   {
      SAY("\t shares %d vertices with other surfaces\n",allcallers->size());
  }
  return allcallers;
}

SMeshBuffer * getMeshBufferFrom3points(vector3df pa, vector3df pb, vector3df pc)
{
    u32 selbufferi = -1;
    SMeshBuffer * buf;
    for (u32 i=0; i<scnMesh->getMeshBufferCount(); i++)
    {
        buf = (SMeshBuffer*)scnMesh->getMeshBuffer(i);
        S3DVertex * vertices = (S3DVertex*)buffer->getVertices();

        bool foundpa=0; bool foundpb=0; bool foundpc=0;
        for (u32 j=0; j<buf->getVertexCount(); j++)
        {
            if (!foundpa)
            {
                if (vertices[j].Pos.equals(pa))
                    foundpa=1;
            }
            if (!foundpb)
            {
                if (vertices[j].Pos.equals(pb))
                    foundpb=1;
            }
            if (!foundpc)
            {
                if (vertices[j].Pos.equals(pc))
                    foundpc=1;
            }
            if (foundpa && foundpb && foundpc) //if found all vertices
            {
                selbufferi=i;
                break;
            }
        }
        if (selbufferi>-1)
            break;
    }
    if (selbufferi==-1)
        error(true,"Selected meshbuffer not found!");

    return buf;

}

void paintMeshBufferRed(SMeshBuffer * buf)
{

    char redtex[256];
    sprintf(redtex,"%s/red.bmp",BaseDirectory);
    if (buffer)
    {
        buffer->Material.MaterialType=prevmt;

        prevmt=buffer->Material.MaterialType;
    }

    //selbuffer = (SMeshBuffer*)scnMesh->getMeshBuffer(selbufferi);
    buffer->Material.setTexture(1,device->getVideoDriver()->getTexture(redtex));
    buffer->Material.MaterialType=EMT_LIGHTMAP;

}

void paintSurfacesClear(u32 si)
{
    ((SMeshBuffer*)scnMesh->getMeshBuffer(si))->Material.MaterialType=getBaseEMT(si);
}
void paintSurfacesClear(array<u32> ss)
{
    for (u32 i=0;i<ss.size(); i++)
        paintSurfacesClear(ss[i]);
}

void paintSurfacesRed(u32 si)
{

    char redtex[256];
    sprintf(redtex,"%s/red.bmp",BaseDirectory);

    SMeshBuffer * buffer = (SMeshBuffer*)scnMesh->getMeshBuffer(si); //for some reason bufferi = surfi + 1!
    buffer->Material.setTexture(1,device->getVideoDriver()->getTexture(redtex));
    buffer->Material.MaterialType=EMT_LIGHTMAP;

}

void paintSurfacesRed(array<u32> ss)
{
    for (u32 i=0;i<ss.size(); i++)
        paintSurfacesRed(ss[i]);
}

void paintSurfacesBlue(u32 si)
{
    char bluetex[256];
    sprintf(bluetex,"%s/blue.bmp",BaseDirectory);

    SMeshBuffer * buffer = (SMeshBuffer*)scnMesh->getMeshBuffer(si); //for some reason bufferi = surfi + 1!
    buffer->Material.setTexture(1,device->getVideoDriver()->getTexture(bluetex));
    buffer->Material.MaterialType=EMT_LIGHTMAP;

}


void paintSurfacesBlue(array<u32> ss)
{

    for (u32 i=0;i<ss.size(); i++)
        paintSurfacesBlue(ss[i]);
}

array<u32> build_uvidxs_i(CScnSolid * solid,scnSurf_t * surfi)
{
   array<u32> a(surfi->vertidxlen);
   for (u32 i=0; i<surfi->vertidxlen; i++)
      a.push_back(solid->uvidxs[surfi->vertidxstart+i]);

   return a;
}

array<u32> calcVertsShared(u32 i)
//TODO: improve performance
{
   if (i==-1)
      return false;

   bool bShared = false;
   array <u32> idxsi, idxsj,idxscommon(32),_idxscommon(32);

   CScnSolid * solid = scn->getSolid(0);
   scnSurf_t * surfi = &solid->surfs[i];

   idxsi = build_uvidxs_i(solid,&solid->surfs[i]);

   for (u32 j=0; j<solid->n_surfs; j++)
   {
      if (j!=i)
      {
         idxsj = build_uvidxs_i(solid,&solid->surfs[j]);
         for (u32 jj=0; jj<idxsj.size(); jj++)
         {
            u32 ii=idxsi.linear_search(idxsj[jj]);
            //linear_search because we don't want to sort the array and miss the positions ii
            if (ii>-1)
            {
               bShared = true;
               idxscommon.push_back(idxsj[jj]);
               _idxscommon.push_back(ii);
            }
         }
      }
   }

   if (bShared)
   {
      SAY("WARNING: ******************************************************\n");
      SAY("Surface %d shared vertices with other surfaces. Changing the UV coordinates will affect all other textures.\n",i);
      SAY("Vertices shared: ");
      for (u32 k=0; k<idxscommon.size();k++)
         SAY("%d (%d)",idxscommon[k], _idxscommon[k]);

      SAY("\n***************************************************************\n");
   }

   //draw those points!
   //_idxscommon represent the indexes (from vertidxstart to vertidxlen) in idxsi that are shared with other
   //draw something in the corresponding vertices
   f32 dx=10, dy=10, dz=10;
   for (u32 k=0; k<_idxscommon.size(); k++)
   {
      point3f vert = solid->verts[solid->vertidxs[surfi->vertidxstart + _idxscommon[k]]];
      boxes[nboxes] = aabbox3df(vert.x-dx,vert.y-dy,vert.z-dz,vert.x+dx,vert.y+dy,vert.z+dz);
      nboxes++;
   }


   return idxscommon;
}

array<u32> * calcVertsShared_b(u32 i)
//TODO: improve performance
{
    if (i==-1)
        return false;

    bool bShared = false;
    f32 dx=1.0, dy=1.0, dz=1.0;


    CScnSolid * solid = scn->getSolid(0);
    scnSurf_t * surfi = &solid->surfs[i];


    array<u32> * callers;

    nboxes = 0;
    for (u32 j=0; j<surfi->vertidxlen; j++)
    {
        u32 uvpos_idx = solid->uvidxs[surfi->vertidxstart + j];

        callers = &solid->uvpos_caller[uvpos_idx];
        if (callers->size() == 1) continue;

        bShared = true;

        for (u32 k=0; k<callers->size(); k++)
        {
            point3f vert = solid->verts[solid->vertidxs[(*callers)[k]]];
            boxes[nboxes] = aabbox3df(vert.x-dx,vert.y-dy,vert.z-dz,vert.x+dx,vert.y+dy,vert.z+dz);
            nboxes++;
        }
    }

   if (bShared)
   {
      SAY("\t shares %d vertices with other surfaces\n",callers->size());
  }
  return callers;
}

array<u32> selectSharedSurfaces(u32 si)
{

    CScnSolid * solid = scn->getSolid(0);
    scnSurf_t * surfi = &solid->surfs[si];

    array<u32> * uvidxs_idxs = calcVertsShared_b(si);
    array<u32> * s;
    array<u32> ss(16);

//    ss.push_back(si);

    for (u32 i=0; i<uvidxs_idxs->size(); i++)
    {
        s = &solid->uvidxs_caller[(*uvidxs_idxs)[i]];
        for (u32 j=0; j< s->size(); j++)
        {
            u32 ns = (*s)[j];
            if (ns != si) ss.push_back(ns);
        }
    }

    SAY("\t%d -> ",si);
    for (u32 i=0; i<ss.size(); i++)
        SAY("%d ",ss[i]);
    SAY("\n");

    return ss;


}

array<u32> selectAllSharedSurfaces(u32 si)
{
    array<u32> ss(32);
    array<u32> ssi;

    bool bContinue;

    u32 idx=0;

    ss.push_back(si);
    s32 len1;
    do
    {
        ssi = selectSharedSurfaces(ss[idx]);
        len1 = ssi.size();

        idx++;

        bContinue = false;
        for (u32 i=0;i<ssi.size();i++)
        {
            if (ss.linear_search(ssi[i])==-1) //if not found, add
            //  linear_search so we don't mess the order with sorting
            {
                bContinue = true;
                ss.push_back(ssi[i]);
            }
        }

    } while(bContinue);
    s32 len2 = ss.size();

    SAY("\t%d *** > ",si);
    for (u32 i=0; i<ss.size(); i++)
        SAY("%d ",ss[i]);
    SAY("\n");
    return ss;
}


#endif
