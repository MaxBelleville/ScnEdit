//To understand this file you need to learn export.h
//util.h and irrlicht.h
#include "pch.h"
#include "export.h"

void scnExportObj(CScn * scn, const char * name)
{
        u32 i;
        char tmp[16];
        char objname[256];
        char mtlname[256];
        sprintf_s(objname, "%s.obj", name);
        sprintf_s(mtlname, "%s.mtl", name);

        SAY("\nBuilding obj and mtl file...");
        FILE* obj;
        fopen_s(&obj, objname, "wt");
        if (obj == NULL)
            error(true, "Can't write to file %s\n", obj);
        FILE* mtl;
        fopen_s(&mtl, mtlname, "wt");
        if (mtl == NULL)
            error(true, "Can't write to file %s\n", obj);

        fprintf(obj, "mtllib %s\n", mtlname);

        u32 nvp = 0; //number of vertices in previous solid
        u32 nuvp = 0; //number of uvpos in previous solid

        for (u32 idx = 0; idx < scn->header->n_solids; idx++)
        {
            CScnSolid* solid = scn->getSolid(idx);

            u32 n_surfs = solid->n_surfs;
            u32 n_verts = solid->n_verts;
            u32 n_uvpos = solid->n_uvpos;

            sprintf_s(tmp, "g solid%03u\n", idx);
            fprintf(obj, tmp);
            for (i = 0; i < n_surfs; i++)
            {
                if (solid->surfs[i].alpha <= 1)   //don't draw transparent solids
                    continue;

                fprintf(obj, "usemtl %s\n", solid->surfs[i].texture);
                fprintf(obj, "f ");
                u32 vstart = solid->surfs[i].vertidxstart;

                for (u16 j = 0; j < solid->surfs[i].vertidxlen; j++)
                {
                    fprintf(obj, "%u/%u ", solid->vertidxs[vstart + j] + 1 + nvp, solid->uvidxs[vstart + j] + 1 + nuvp);
                }
                fprintf(obj, "\n");
            }

            for (i = 0; i < n_verts; i++) {
                point3f* verti = &solid->verts[i];
                fprintf(obj, "v %f %f %f\n", verti->x, verti->y, verti->z);
                nvp++;
            }

            for (i = 0; i < n_uvpos; i++) {
                point2f* uvi = &solid->uvpos[i];
                fprintf(obj, "vt %f %f \n", uvi->u, -uvi->v);
                nuvp++;
            }

            for (i = 0; i < solid->textures.size(); i++)
            {

                fprintf(mtl, "newmtl %s\n", solid->textures[i].c_str());
                fprintf(mtl, "map_Kd textures/%s.bmp\n", solid->textures[i].c_str());
            }

        }

        fclose(obj);
        fclose(mtl);
        SAY("done.\n");
}

void scnExport3ds(CScnSolid* mesh, u32 totalSize, const char* name)
{
    SAY("Building 3ds file...");


    char fname[128];
    sprintf_s(fname, "%s.3ds", name);
    FILE* f3ds = NULL;
    fopen_s(&f3ds, fname, "wt");
    if (f3ds == NULL)
        error(true, "Can't write to file %s\n", f3ds);

    for (u32 s = 0; s < totalSize; s++) {
        CScnSolid* scn = &mesh[s];
        u16 id;
        u32 size;
        u16 n_verts = scn->n_verts;
        u32 verts_size = n_verts * sizeof(point3f);

        u16 n_surfs = scn->n_surfs;
        u16 n_polys = 0;
        for (int i = 0; i < n_surfs; i++)
            n_polys += (scn->surfs[i].vertidxlen - 2);
        u32 polys_size = n_polys * 8;


        u32 len = 0;
        u8* data;
        u8* p;     //byte pointer

        len += 2 + 4; //main chunk header
        len += 2 + 4; //edit3ds chunk header
        len += 2 + 4; //edit_oject chunk header
        len += 2 + 4; //OBJ_TRIMESH chunk header
        len += verts_size + 2 + 2 + 4;       //vertex list header + data
        len += polys_size + 2 + 2 + 4;

        data = (u8*)malloc(len);
        p = data;
        id = MAIN3DS;
        size = len;

        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;

        id = EDIT3DS;
        size -= 6;

        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;

        id = EDIT_OBJECT;
        size -= 6;

        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;

        id = OBJ_TRIMESH;
        size -= 6;

        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;

        id = TRI_VERTEXL;
        size = verts_size + 2 + 6;

        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;
        memcpy(p, &n_verts, 2); p += 2;
        memcpy(p, scn->verts, verts_size); p += verts_size;

        id = TRI_FACEL1;
        size = polys_size + 2 + 6;
        memcpy(p, &id, 2); p += 2;
        memcpy(p, &size, 4); p += 4;
        memcpy(p, &n_polys, 2); p += 2;

        for (int s = 0; s < n_surfs; s++)
        {

            scnSurf_t* surfi = &scn->surfs[s];
            u16 vstart = surfi->vertidxstart;
            u16 idx;
            u16 vis = 6;
            idx = vstart;
            memcpy(p, &idx, 2); p += 2;
            idx = vstart + 1;
            memcpy(p, &idx, 2); p += 2;
            idx = vstart + 2;
            memcpy(p, &idx, 2); p += 2;
            memcpy(p, &vis, 2); p += 2;

            for (u16 j = 3; j < surfi->vertidxlen; j++)
            {
                idx = vstart + j - 1;
                memcpy(p, &idx, 2); p += 2;
                idx = vstart + j;
                memcpy(p, &idx, 2); p += 2;
                idx = vstart;
                memcpy(p, &idx, 2); p += 2;
                memcpy(p, &vis, 2); p += 2;
            }
        }
        fwrite(data, 1, len, f3ds);
    }
    fclose(f3ds);
    SAY("done.\n");
}

/*point3f crossProduct(point3f a,point3f b)
{
    point3f res;
    res.x = (a.y)*(b.z) - (a.z)*(b.y);
    res.y = (a.z)*(b.x) - (a.x)*(b.z);
    res.z = (a.x)*(b.y) - (a.y)*(b.x);
    return res;
}

point3f subtract(point3f a, point3f b)
{
    point3f res;
    res.x = a.x-b.x;
    res.y = a.y-b.y;
    res.z = a.z-b.z;
    return res;
}
point3f add(point3f a, point3f b)
{
    point3f res;
    res.x = a.x+b.x;
    res.y = a.y+b.y;
    res.z = a.z+b.z;
    return res;
}*/
/*point3f normalize(point3f a)
{
    f32 mod = sqrt((a.x)*(a.x) + (a.y)*(a.y) + (a.z)*(a.z));
    point3f res;
    if (mod == 0.0)
    {
        res.x = 0.0;
        res.y = 0.0;
        res.z = 0.0;
        error(true,"normalize mod is zero!");
    }
    else
    {
        res.x = a.x/mod;
        res.y = a.y/mod;
        res.z = a.z/mod;
    }
    return res;

}*/


/*point3f getNormalFrom3pts(point3f a,point3f b,point3f c)
{
    return normalize(crossProduct(subtract(b,a),subtract(c,a)));
}*/


using namespace irr;
using namespace core;

void writeMapLine(FILE * map, vector3df a,vector3df b,vector3df c,vector3df t1,vector3df t2, const char * texname=0)
{

    //FIX: fix direction of vertex, i just put - so it works
    fprintf(map,"( %.2f %.2f %.2f ) ",a.X,a.Z,a.Y);
    fprintf(map,"( %.2f %.2f %.2f ) ",b.X,b.Z,b.Y);
    fprintf(map,"( %.2f %.2f %.2f ) ",c.X,c.Z,c.Y);
    if (texname)
        fprintf(map,"%s ",texname);
    else
        fprintf(map,"NONE ");
    fprintf(map,"[ %.0f %.0f %.0f 0 ] [ %.0f %.0f %.0f 0 ] 0 1 1 \n",
    t1.X,t1.Z,t1.Y,t2.X,t2.Z,t2.Y);
}

vector3df vec2irrvec(point3f a)
{
    return vector3df(a.x,a.y,a.z);
}
void scnExportMap(CScn * scn, const char * name)
{

    // added to give feedback about export and match others -SJ
    SAY("Building map file...");

    char fname[128];
    sprintf_s(fname,"%s.map",name);
    FILE* map;
    fopen_s(&map, fname,"wt");
    fprintf(map,
    "{\n"
    "\"classname\" \"worldspawn\"\n"
    "\"mapversion\" \"220\"\n"
    "\"wad\" \"wads\\sample.wad\"\n");

    vector3df a,b,c,dp,r1,r2,normal,lastr1,zero(0,0,0);
    for (u32 idx = 0; idx < scn->header->n_solids; idx++)
    {
        CScnSolid* solid = scn->getSolid(idx);

        for (u32 i = 0; i < solid->n_surfs; i++)
        {
            scnSurf_t* surfi = &solid->surfs[i];
            u32 idxs[3]{};
            idxs[0] = surfi->vertidxstart;
            idxs[1] = surfi->vertidxstart + 1;
            a = vec2irrvec(solid->verts[solid->vertidxs[idxs[0]]]);
            b = vec2irrvec(solid->verts[solid->vertidxs[idxs[1]]]);

            f32 width = 0.5;
            scnPlane_t* plane = &solid->planes[surfi->planeidx];
            normal = vector3df(plane->a, plane->b, plane->c);
            dp = normal * width;

            /*
            if (!((dp.X == 0 && dp.Y == 0 && dp.Z!=0) ||
                  (dp.X == 0 && dp.Y != 0 && dp.Z==0) ||
                  (dp.X != 0 && dp.Y == 0 && dp.Z==0)))
                continue;*/

            r1 = (b - a);
            r2 = r1.crossProduct(normal);
            c = a + r2;
            r1.normalize();
            r2.normalize();

            fprintf(map, "{\n");
            writeMapLine(map, c, b, a, r1, r2, surfi->texture);                                          //plane A
            writeMapLine(map, a - dp, b - dp, c - dp, r1, r2, surfi->texture); //plane B

            lastr1 = zero;
            u32 j = 0;
            do
            {
                idxs[0] = surfi->vertidxstart + j;

                j++;
                if (j == surfi->vertidxlen) j = 0;

                idxs[1] = surfi->vertidxstart + j;

                a = vec2irrvec(solid->verts[solid->vertidxs[idxs[0]]]);
                b = vec2irrvec(solid->verts[solid->vertidxs[idxs[1]]]);
                c = b - dp;
                r1 = (b - a);
                if (lastr1 != zero)
                    if (r1.crossProduct(lastr1).equals(zero)) //if lastr1 and r1 are collinear skip this plane (we already have it)
                        continue;

                r2 = r1.crossProduct(normal);
                r1.normalize();
                r2.normalize();
                writeMapLine(map, c, b, a, r1, r2); //plane C...

                lastr1 = r1;


            } while (j > 0);

            fprintf(map, "}\n");
        }
    }
    //SAY("Exported %d surfaces.\n",ns);
    fprintf(map,"}\n");

    //export entities
    CScnEnt * ent;
    u32 nents = scn->getTotalEnts();
    for (u32 i=0; i<nents; ++i)
    {
        ent = scn->getEnt(i);

    // No Entities printed to file
    // No idea how to access the entity array - SJ
    //fprintf(map,"I an entity %i\n");

    }


    fclose(map);

    SAY("done.\n\nAll done exporting. This process is not perfect so expect geometry glitches\n");
}



