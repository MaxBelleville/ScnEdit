#include "pch.h"
#include "Header/Base/CScnSolid.h"

using namespace std;
using namespace irr;
using namespace video;
//Deconstructor for solids
CScnSolid::~CScnSolid()
{
	//pointers must be set to zero initially otherwise errors
	if (rawcells)
	{
		for (u32 i=0; i < n_cells; i++)
		{
			//Deletes cells node id
			if (rawcells[i].nodesidxs)
				delete [] rawcells[i].nodesidxs;
			//Deletes cell portal vertices and portals
			if (rawcells[i].portals)
			{
				for (s32 j=0; j < rawcells[i].n_portals; j++)
					if (rawcells[i].portals[j].verts)
						delete [] rawcells[i].portals[j].verts;

				delete [] rawcells[i].portals;
			}
		}
		//Deletes cells
		delete [] rawcells;
	}
	//Deletes uv id
	if (uvidxs)
		delete [] uvidxs;
	//Deletes vertex id
	if (vertidxs)
		delete [] vertidxs;
	//Deletes uvpos
	if (uvpos)
		delete [] uvpos;
	if (ouvpos)
		delete [] ouvpos;
	//Deletes vertices
	if (verts)
		delete [] verts;
	//Deletes planes
	if (planes)
		delete [] planes;
	//Stop destroying trees...
	//Deletes trees
	if (tree)
		delete tree;
	//Deletes something about surfaces
	if (surfsad)
		delete [] surfsad;

	if (surfs)
	{
		//Deletes surface shading then surfaces
		for (u32 i=0;i < n_surfs ;i++)
			if (surfs[i].hasVertexColors==1)
				delete [] surfs[i].shading;

		delete [] surfs;
	}
	//Deletes uv postion caller
   if (uvpos_caller)
	  delete [] uvpos_caller;
	//Deletes uv id caller
   if (uvidxs_caller)
	  delete [] uvidxs_caller;

}

int CScnSolid::loadSolid(std::ifstream * file, u32 indx)
{
	solididx = indx;
	//file offset
	file->seekg(offset);
	os::Printer::log(format("\toffset: {}",offset).c_str());
	//reads file for information in 32 bits
	//all loading up number of...
	if(firstVal==NULL) {
	firstVal = true;
	}
	n_unk1=read_u32(file);
	n_verts=read_u32(file);
	n_uvpos = read_u32(file);
	n_vertidxs=read_u32(file);
	n_planes=read_u32(file);
	n_nodes=read_u32(file);
	n_surfs=read_u32(file);
	n_cells=read_u32(file);
	n_unk2=read_u32(file);
	//Ooh look at this one its seperated...
	length=read_u32(file);
	//288 bits of variables in the scn file
	//must be in this order
	loadSurfs(file);
	loadNodes(file);
	loadPlanes(file);
	loadVerts(file);
	loadUVPos(file);
	loadVertIdxs(file);
	loadUVIdxs(file);
	loadUnk(file);
	loadCells(file);
	//Gives utid-unique texture name
	s32 n_texs = calcUniqueTexturesNames(file);
	os::Printer::log(format("\t{} unique textures",n_texs).c_str());
	//if number of textures is not equal to the texture size
	if (n_texs != textures.size())
		error(true,"loadSolid: Number of unique textures and texture array size doesn't match");
	//Thats good for the enivoroment
	buildBackTree();

	return 1;
}
//The rest of this code just deals with loading different parts of the solid
int CScnSolid::loadCells(std::ifstream * file)
{
	os::Printer::log("\tGetting Cells...");
	//sets raw cells to number of cells
	rawcells = new scnRawCell_t[n_cells];
	//loop with something about the number of cells
	for (u32 i=0; i < n_cells; i++)
	{   //sets nodeid and portals to 0 for each cell
		rawcells[i].nodesidxs=0;
		rawcells[i].portals=0;

		//reads cell names in file 32 bits
		read_generic(rawcells[i].name,file,32);
		//reads number of node ids
		rawcells[i].n_nodesidxs=read_s32(file);
		//reads number of portals
		rawcells[i].n_portals=read_s32(file);
		//reads unknow variable
		rawcells[i].n3=read_s32(file);
		//reads sky name in file 32 bits
		read_generic(rawcells[i].skyname,file,32);
		os::Printer::log(format("\t\tcell[{}]: {} {} {} {} {}", 
			i, rawcells[i].name, rawcells[i].n_nodesidxs, rawcells[i].n_portals, rawcells[i].n3, rawcells[i].skyname).c_str());

		//creates new node id
		rawcells[i].nodesidxs = new u16[rawcells[i].n_nodesidxs];
		//reads node id in file in a byte amount of double the number of node ids
		read_generic(rawcells[i].nodesidxs,file,2 * rawcells[i].n_nodesidxs);
		//creates new portals
		rawcells[i].portals = new scnPortal_t[rawcells[i].n_portals];
		os::Printer::log(format("\tReading {} portals... ", rawcells[i].n_portals).c_str());
		for (s32 j=0; j < rawcells[i].n_portals; j++)
		{
			//sets portal vertices to 0
			rawcells[i].portals[j].verts=0;
			loadPortal(&(rawcells[i].portals[j]),file);
		}
		os::Printer::log("\t\tdone.");
		os::Printer::log("\tReading cell data... ");
		loadCellData(&rawcells[i], &(rawcells[i].bvh), file);
		os::Printer::log("\t\tdone.");
	}


   os::Printer::log("\t\tdone.");
   tree->n_cells = n_cells;
   tree->rawcells = rawcells;
	return n_cells;
}
//read bboxes and surface indices
int CScnSolid::loadCellData(scnRawCell_t* rawcell,scnCellData_t * celldata,std::ifstream * file)
{
	int n = 0;
	//sets read postion double the size of core::vector3df and puts it in seekg
	celldata->bbsad = file->tellg();
	read_generic(celldata->bb_verts, file, sizeof(core::vector3df) * 2);
	//reads file
	celldata->n_children=read_u16(file);
	//reads number of surfaces
	celldata->n_surfs=read_u16(file);
	if (celldata->n_surfs > 0) {
		celldata->surfsidxs = new u16[celldata->n_surfs];
		for (u16 i = 0; i < celldata->n_surfs; i++)
		{
			celldata->surfsidxs[i] = read_u16(file);   //read the usual 72 first bytes
			bool found = false;
			for (u16 j = 0; j < rawcell->naivesurfs.size(); j++) {
				if (rawcell->naivesurfs[j] == celldata->surfsidxs[i])//Add if exists
					found = true;
			}
			if (!found) rawcell->naivesurfs.push_back(celldata->surfsidxs[i]);
		}
	}
	if (celldata->n_children > 0) {
		celldata->children=new scnCellData_t[celldata->n_children];
		for (s32 j = 0; j < celldata->n_children; j++)
		{
			n+=loadCellData(rawcell,&(celldata->children[j]), file)+1;
			  
		}
	}
	else {
		rawcell->leafnode.push_back(celldata);

	}
		
   
	return n;
}
//IMPORTANT Figure out why he is hiding the fact he is reading the portals by commenting out the say commands
int CScnSolid::loadPortal(scnPortal_t * portal, std::ifstream * file)
{

	//reads portal name
	read_generic(portal->name,file,32);
	os::Printer::log(format("\t\tReading portal with {} names...", strlen(portal->name)).c_str());
	//os::Printer::log("{} ", portal->name);
	//reads next portal cell
	portal->nextcell=read_s32(file);
	//os::Printer::log("{} ",portal->nextcell);
	//reads the plane in the file in the size of the plane
	read_generic(&(portal->plane),file,sizeof(scnPlane_t));
	//read portals unk what ever that means
	portal->unk=read_f32(file);
	//os::Printer::log("(%.1f %.1f %.1f %.1f %.1f)",portal->plane.a, portal->plane.b, portal->plane.c, portal->plane.d, portal->unk);
	//reads the portals number of vertices
	portal->n_verts=read_s32(file);
	//reads the portals bb_verices in the file in double the size of core::vector3df
	read_generic(portal->bb_verts,file,sizeof(core::vector3df)*2);
	//creates new portal vertices
	portal->verts = new core::vector3df[portal->n_verts];
	//reads the portal vertices in file in the size of core::vector3df times the number of portal vertices
	read_generic(portal->verts, file, sizeof(core::vector3df) * portal->n_verts);

	os::Printer::log("\t\t\tdone reading portal. ");
	return 0;
}

int CScnSolid::loadUnk(std::ifstream * file)
{
	os::Printer::log("\tSkipping Unk lump...");

	//About this lump only know it's 9 floats per surface
	file->seekg(36*n_surfs,ios::cur);

	os::Printer::log("\t\tdone.");
	return 0;
}


int CScnSolid::calcUniqueTexturesNames(std::ifstream * file)
{
	int ret=0;
	bool found=false;
	//loop for amount of surfaces
	for (u32 i=0; i < n_surfs; i++)
	{
		//sets texture name to surface texture
		string texname(surfs[i].texture);
		found=false;
		//loop for texture sizes
		for (int j=0; i < textures.size(); i++)
		{
			//checks if textures is equal to texture names
			if (textures[j]==texname)
			{
				found=true;
				break;
			}
		}
		//if not found
		if (!found)
		{
			//create textures with texture name
			textures.push_back(texname);
			ret++;
		}
	}
	return ret;

}

int CScnSolid::loadUVIdxs(std::ifstream * file)
{
	u32 nvui = n_vertidxs;
	os::Printer::log(format("\tGetting UV coordinates indices... {}", nvui).c_str());
	//creates new uv indexes
	uvidxs = new u32[nvui];   //allocate   n_uvidxs=n_vertidxs
	//reads uv indexes in file
	read_generic(uvidxs,file,sizeof(u32)*nvui);
	os::Printer::log("\t\tdone.");

	return nvui;

}

void CScnSolid::buildBackTree()
{
	//sets number of uv indexes to number of vertex indexes
	u32 n_uvidxs = n_vertidxs;

	uvpos_caller = new core::array<u32>[n_uvpos];
	//build backtree
	for (u32 i=0; i< n_uvidxs ;i++)
		uvpos_caller[uvidxs[i]].push_back(i);

	uvidxs_caller = new core::array<u32>[n_uvidxs];
	for (u32 i=0; i< n_surfs; i++)
		for (u32 j=0; j< surfs[i].vertidxlen; j++)
			uvidxs_caller[surfs[i].vertidxstart + j].push_back(i);

	// TESTS to make sure this is right ----------------------------
	/*bool bPassed = true;

	for (u32 i=0; i<n_uvpos; i++)
	{
		for (u32 j=0; j<uvpos_caller[i].size();j++)
		{
			core::vector2df uvposa = uvpos[uvidxs[uvpos_caller[i][j]]];
			if ((uvposa.X != uvpos[i].X) || (uvposa.Y != uvpos[i].Y))
			{
				bPassed = false;
				break;
			}
		}
	}

	for (u32 i=0; i<n_uvidxs; i++)
	{
		for (u32 j=0; j<uvidxs_caller[i].size();j++)
		{
			scnSurf_t * surfi = &surfs[uvidxs_caller[i][j]];
			bool bContains=false;
			for (u32 k=0; k<surfi->vertidxlen; k++)
			{
				if ((surfi->vertidxstart + k) == i)
				{
					bContains = true;
					break;
				}
			}
			if (!bContains)
			{
				bPassed = false;
				break;
			}
		}
	}

	if (!bPassed) error(true, "buildBackTree() failed, tests not passed");
	*/

}


int CScnSolid::loadVertIdxs(std::ifstream * file)
{
	os::Printer::log(format("\tGetting Vertex indices... {}", n_vertidxs).c_str());
	vertidxs = new u32[n_vertidxs];   //allocate
	read_generic(vertidxs,file,sizeof(u32)*n_vertidxs); //read all, should work

	os::Printer::log("\t\tdone.");

	return n_vertidxs;
}

int CScnSolid::loadUVPos(std::ifstream * file)
{
	os::Printer::log(format("\tGetting UV coordinates... {}", n_uvpos).c_str());
	uvpos = new core::vector2df[n_uvpos];   //allocate
	ouvpos = new core::vector2df[n_uvpos];
	uvposad=file->tellg();
	read_generic(uvpos,file,sizeof(core::vector2df)*n_uvpos); //read all, should work
	for(u16 i=0; i< n_uvpos;i++){
	ouvpos[i].X = uvpos[i].X;
	ouvpos[i].Y = uvpos[i].Y;
	}
	os::Printer::log("\t\tdone.");

	return n_uvpos;
}

int CScnSolid::loadVerts(std::ifstream* file)
{
	os::Printer::log(format("\tGetting Vertices... {}", n_verts).c_str());
	verts = new core::vector3df[n_verts];   //allocate
	vertssad = file->tellg();
	read_generic(verts, file, sizeof(core::vector3df) * n_verts); //read all, should work
	os::Printer::log("\t\tdone.");

	return n_verts;

}

int CScnSolid::loadPlanes(std::ifstream * file)
{
	os::Printer::log(format("\tGetting planes... {}", n_planes).c_str());
	planes = new scnPlane_t[n_planes];   //allocate
	planessad = file->tellg();
	tree->planes = planes; //IMPORTANT
							//TODO: make better

	read_generic(planes,file,sizeof(scnPlane_t)*n_planes); //read all, should work
	os::Printer::log("\t\tdone.");

	return n_planes;

}

int CScnSolid::loadNodes(std::ifstream * file)
{
	os::Printer::log("\tReading nodes...");
	//file->seek(n[N_NODES]*16,true);

	tree = new CScnBSPTree(n_nodes);

	if (tree->loadNodes(file) == -1)
		error(true,"loadNodes: Error reading nodes from solid,n_nodes < 1");

	os::Printer::log("\t\tdone.");

	return 0;

}

int CScnSolid::loadSurfs(std::ifstream * file)
{
	os::Printer::log(format("\tGetting Surfaces... {}", n_surfs).c_str());

	surfs = new scnSurf_t[n_surfs];
	surfsad = new int64_t[n_surfs];
	u16 i;
	for (i=0;i < n_surfs ;i++)
	{
		surfsad[i]=file->tellg();

		read_generic(&surfs[i],file,72);   //read the usual 72 first bytes

		if (surfs[i].hasVertexColors==1)  //means there are more bytes - the shading or smoothing or whatever we call it
		{
			surfs[i].shading = new u8[4*surfs[i].vertidxlen];     //allocate
			read_generic(surfs[i].shading,file,4*surfs[i].vertidxlen);
			//REMEMBER: because shading is initially set to  a random value, we must
			//make sure we only try to draw shading only when more is set to !0 or
			//make constructor to set initial value 0;
		}
		else if (surfs[i].hasVertexColors !=0)
			error(true,"CScnSolid: loadSurfs - Unexpected surface[{}].more value - expected 0 or 1",i);
	}
	os::Printer::log("\t\tdone.");
	return i;
}

s16 CScnSolid::getCellAtPos(core::vector3df pos) const {
   s16 nodeindx= tree->findNodePos(pos);
   return tree->nodes[nodeindx].cell;
}



scnCellData_t* CScnSolid::getBBFromSurf(u16 surfindx,u16 cellIndx) {
	return getBBFromSurf(surfindx,&rawcells[cellIndx].bvh);
}
scnCellData_t* CScnSolid::getBBFromSurf(u16 surfindx,scnCellData_t * celldata) {
	scnCellData_t* founddata;
	for (u32 s = 0; s < celldata->n_surfs; s++) {
		if (celldata->surfsidxs[s] == surfindx) {
			return celldata;
		}
	}
	for (u32 c = 0; c < celldata->n_children; c++) {
		founddata =getBBFromSurf(surfindx, &celldata->children[c]);
		if (founddata) return founddata;
	}
	return nullptr;
}



//default constructor
CScnSolid::CScnSolid()
{
	offset=0;
	length=0;
	//TODO: do something
	textures.clear();

	//setting pointers to zero
	rawcells=0;
	uvidxs=0;
	vertidxs=0;
	uvpos=0;
	ouvpos=0;
	verts=0;
	planes=0;
	tree=0;
	surfs=0;
	surfsad=0;
	uvposad=0;

}

#ifdef __IRRLICHT_H_INCLUDED__
//return array of surface vertices with alpha, shading and uv information
//irr_specific
void CScnSolid::calcSurfVertices(u32 surfidx, f32* mults, IVertexBuffer* vbuff, u16 arg_alpha)
{

	scnSurf_t * surfi = &surfs[surfidx];

	S3DVertex2TCoords tVert;

	u8 * ps= surfi->shading;

	for (u32 i=0; i < surfi->vertidxlen; i++)
	{
		tVert = getVertice(vertidxs[surfi->vertidxstart+i]);
		  
		tVert.TCoords.X = uvpos[uvidxs[surfi->vertidxstart + i]].X;
		tVert.TCoords.Y = uvpos[uvidxs[surfi->vertidxstart + i]].Y;

		tVert.TCoords2.X = uvpos[uvidxs[surfi->vertidxstart + i]].X;
		tVert.TCoords2.Y = uvpos[uvidxs[surfi->vertidxstart + i]].Y;
	
		if(mults) {
			tVert.TCoords2.X = tVert.TCoords2.X * mults[0] + mults[2];
			tVert.TCoords2.Y = tVert.TCoords2.Y * mults[1] + mults[3];
		}

		u32 a = surfi->alpha;

		if (a!=255 && arg_alpha > 0) a = arg_alpha; //override with l
		//if (a!=255) //removed this
		tVert.Color.setAlpha(a);
		if (surfi->hasVertexColors)
		{
			//if there is shading, we find that ps[3] always equals surfi->alpha
			//we could use this to make fun things like make vertices of the same surface have different transparencies
			tVert.Color = video::SColor(ps[3],ps[0],ps[1],ps[2]);
			ps+=4;
		}
		vbuff->addVertex(&tVert);
	}
	//debug
	//check if there are 3 colinear vertices
	/*

	a = v[1].Pos - v[0].Pos;
	b = v[2].Pos - v[1].Pos;
	cp = a.crossProduct(b);
	if (cp == zero)
		os::Printer::log("first 3 verts are colinear");

	a = v[surfi->vertidxlen-3].Pos - v[surfi->vertidxlen-2].Pos;
	b = v[surfi->vertidxlen-2].Pos - v[surfi->vertidxlen-1].Pos;
	cp = a.crossProduct(b);
	if (cp == zero)
		os::Printer::log("last 3 verts are colinear");*/

	//core::vector3df a,b,cp,zero(0,0,0);
	//for (u32 j=0; j<surfi->vertidxlen; j++)
	//{
	//    u32 idxs[3];
	//    idxs[0] = j;
	//    idxs[1] = j+1;
	//    idxs[2] = j+2;
	//    if (j == surfi->vertidxlen-2)
	//    {
	//        idxs[2] = 0;
	//    }
	//    else if (j == surfi->vertidxlen-1)
	//    {
	//        idxs[1] = 0;
	//        idxs[2] = 1;
	//    }

	//    a = v[idxs[2]].Pos-v[idxs[0]].Pos;
	//    b = v[idxs[1]].Pos-v[idxs[0]].Pos;
	//    cp = a.crossProduct(b);
	//    //TODO: THIS
	//    //if (cp == zero)
	//        //os::Printer::log("Surf {}: Verts {} {} {} are colinear",surfidx,idxs[0],idxs[1],idxs[2]);
	//}



}

#endif

#ifdef __IRRLICHT_H_INCLUDED__
void CScnSolid::calcSurfIndices(u32 surfidx, IIndexBuffer* ibuff)
{

	scnSurf_t * surfi = &surfs[surfidx];

	//we pass the vertices always in the order 0,1,2; 2,3,0; 3,4,0; ...
	//                  (swat and irrlicht are counter-clockwise)

	ibuff->addIndex(0);
	ibuff->addIndex(1);
	ibuff->addIndex(2);

	//each surface is a new meshbuffer - because they may have different textures
	//draw mesh like a triangle fan - first three vertices define a triangle,
	//from then, each new one defines a tringle with the last and the origin (0)


	for (u16 j=3; j<surfi->vertidxlen ;j++)
	{
		ibuff->addIndex(j-1);
		ibuff->addIndex(j);
		ibuff->addIndex(0);
	}


}

#else
vector<u16> CScnSolid::calcSurfIndices(u32 surfidx)
{
	vector<u16> idxs;
	scnSurf_t * surfi = &surfs[surfidx];

	idxs.push_back(0);
	idxs.push_back(1);
	idxs.push_back(2);

	//each surface is a new meshbuffer - because they may have different textures
	//draw mesh like a triangle fan - first three vertices define a triangle,
	//from then, each new one defines a tringle with the last and the origin (0)

	for (u16 j=3; j<surfi->vertidxlen ;j++)
	{
		idxs.push_back(j-1);
		idxs.push_back(j);
		idxs.push_back(0);
	}
	return idxs;

}
#endif
