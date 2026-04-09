#include "pch.h"
#include "Header/Base/CScnSolid.h"

using namespace std;
using namespace irr;
using namespace video;

//default constructor
CScnSolid::CScnSolid()
{
	offset = 0;
	length = 0;
	//setting pointers to zero
	rawcells = 0;
	uvidxs = 0;
	vertidxs = 0;
	uvpos = 0;
	verts = 0;
	planes = 0;
	tree = 0;
	surfs = 0;
	surfsad = 0;
	uvposad = 0;

}

//Deconstructor for solids
CScnSolid::~CScnSolid()
{
	textures.clear();
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
				for (s32 j = 0; j < rawcells[i].n_portals; j++) {
					if (rawcells[i].portals[j].verts)
						delete[] rawcells[i].portals[j].verts;
				}

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
	if (projection)
		delete[] projection;
	if (surfs)
	{
		//Deletes surface shading then surfaces
		for (u32 i = 0; i < n_surfs; i++) {
			if (surfs[i].hasVertexColors == 1)
				delete[] surfs[i].shading;
		}

		delete [] surfs;
	}
	//Deletes uv postion caller
   if (uvpos_caller)
	  delete [] uvpos_caller;
   if (vertpos_caller)
	   delete[] vertpos_caller;
	//Deletes uv id caller
   if (faceidxs_caller)
	  delete [] faceidxs_caller;

}

int CScnSolid::loadSolid(std::ifstream * file, u32 indx)
{
	solididx = indx;
	//file offset
	file->seekg(offset);
	os::Printer::log(format("\toffset: {}",offset).c_str());
	//reads file for information in 32 bits
	//all loading up number of...
	if(firstVal==NULL) 
		firstVal = true;
	
	n_unk1=read_u32(file);
	n_verts=read_u32(file);
	n_uvpos = read_u32(file);
	n_faceidx=read_u32(file);
	n_planes=read_u32(file);
	n_nodes=read_u32(file);
	n_surfs=read_u32(file);
	n_cells=read_u32(file);
	n_names=read_u32(file);
	lengthsad = file->tellg();
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
	loadProjection(file);
	loadCells(file);
	loadNames(file);
	
	//Gives utid-unique texture name
	s32 n_texs = calcUniqueTexturesNames(file);
	os::Printer::log(format("\t{} unique textures",n_texs).c_str());

	//if number of textures is not equal to the texture size
	if (n_texs != textures.size())
		error(true,"loadSolid: Number of unique textures and texture array size doesn't match");
	//Thats good for the enivoroment
	buildBackTree();
	extractSurfaces();

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
		rawcells[i].n_occluders=read_s32(file);
		//reads sky name in file 32 bits
		read_generic(rawcells[i].skyname,file,32);
		os::Printer::log(format("\t\tcell[{}]: {} {} {} {} {}", 
			i, rawcells[i].name, rawcells[i].n_nodesidxs, rawcells[i].n_portals, 
			rawcells[i].n_occluders, rawcells[i].skyname).c_str());

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
//
int CScnSolid::loadNames(std::ifstream* file) {
	os::Printer::log("\tGetting Names...");
	for (int i = 0; i < n_names; i++) {
		char name[32];
		read_generic(name, file, 32);
		names.push_back(name);
	}
	return n_names;
	os::Printer::log("\t\tdone.");
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
			celldata->surfsidxs[i] = read_u16(file);   
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
			n+=loadCellData(rawcell,&(celldata->children[j]), file)+1;
			
	}
	else 
		rawcell->leafnode.push_back(celldata);
		
   
	return n;
}

int CScnSolid::loadPortal(scnPortal_t * portal, std::ifstream * file)
{
	//reads portal name
	read_generic(portal->name,file,32);
	os::Printer::log(format("\t\tReading portal with {} names...", strlen(portal->name)).c_str());
	//reads next portal cell
	portal->nextcell=read_s32(file);
	//reads the plane in the file in the size of the plane
	read_generic(&(portal->plane),file,sizeof(scnPlane_t));
	//read portals unk what ever that means
	portal->winding=read_f32(file);
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

int CScnSolid::loadProjection(std::ifstream * file)
{
	os::Printer::log("\t\tReading paramertization frames/lightmap uv bias for surfaces...");
	projsad = file->tellg();
	projection = new scnProjectionBasis_t[n_surfs];
	read_generic(projection, file, sizeof(scnProjectionBasis_t)*n_surfs);
	
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
	os::Printer::log(format("\tGetting UV coordinates indices... {}", n_faceidx).c_str());
	//creates new uv indexes
	uvidxs = new u32[n_faceidx];   //allocate   n_uvidxs=n_vertidxs

	uvidxsad = file->tellg();
	//reads uv indexes in file

	read_generic(uvidxs,file,sizeof(u32)* n_faceidx);
	os::Printer::log("\t\tdone.");

	return n_faceidx;

}

int CScnSolid::loadVertIdxs(std::ifstream * file)
{
	os::Printer::log(format("\tGetting Vertex indices... {}", n_faceidx).c_str());
	vertidxs = new u32[n_faceidx];   //allocate
	vertidxsad = file->tellg();
	read_generic(vertidxs,file,sizeof(u32)* n_faceidx); //read all, should work

	os::Printer::log("\t\tdone.");

	return n_faceidx;
}

int CScnSolid::loadUVPos(std::ifstream * file)
{
	os::Printer::log(format("\tGetting UV coordinates... {}", n_uvpos).c_str());
	uvpos = new core::vector2df[n_uvpos];   //allocate
	uvposad=file->tellg();
	read_generic(uvpos,file,sizeof(core::vector2df)*n_uvpos); //read all, should work

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
	int64_t start_node = file->tellg();
	tree = new CScnBSPTree(n_nodes);

	if (tree->loadNodes(file) == -1)
		error(true,"loadNodes: Error reading nodes from solid,n_nodes < 1");

	int64_t end_node = file->tellg();

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
		int64_t t = file->tellg();
	
		if (surfs[i].hasVertexColors==1)  //means there are more bytes - the shading or smoothing or whatever we call it
		{
			surfs[i].shading = new u8[4*surfs[i].faceidxlen];     //allocate
			read_generic(surfs[i].shading,file,4*surfs[i].faceidxlen);
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

void CScnSolid::buildBackTree()
{
	os::Printer::log("\tBuilding back tree...");
	uvpos_caller = new core::array<u32>[n_faceidx];
	vertpos_caller = new core::array<u32>[n_faceidx];

	for (u32 i = 0; i < n_faceidx; i++) {
		uvpos_caller[uvidxs[i]].push_back(i);
		vertpos_caller[vertidxs[i]].push_back(i);
	}

	faceidxs_caller = new core::array<u32>[n_faceidx];
	for (u32 i = 0; i < n_surfs; i++) {
		for (u32 j = 0; j < surfs[i].faceidxlen; j++)
			faceidxs_caller[surfs[i].faceidxstart + j].push_back(i);
	}
	os::Printer::log("\t\tdone.");
}

scnCellData_t* CScnSolid::getBBFromSurf(u16 surfindx,scnCellData_t * celldata) {
	scnCellData_t* founddata;
	for (u32 s = 0; s < celldata->n_surfs; s++) {
		if (celldata->surfsidxs[s] == surfindx) 
			return celldata;
	}
	for (u32 c = 0; c < celldata->n_children; c++) {
		founddata =getBBFromSurf(surfindx, &celldata->children[c]);
		if (founddata) 
			return founddata;
	}
	return nullptr;
}

void CScnSolid::extractSurfaces() {
	os::Printer::log("\tExtracting surfaces into internal...");
	for (u32 i = 0; i < n_surfs; i++) {
		CScnLocalizedFace face;
		face.si = i;
		// don't call set_used here; reserve capacity instead
		face.verts.set_used(0);
		face.verts.reallocate(surfs[i].faceidxlen); // if reallocate is available, otherwise rely on push_back

		for (u32 j = 0; j < surfs[i].faceidxlen; j++) {
			u32 faceidx = surfs[i].faceidxstart + j;

			// bounds checks
			
			u32 vindex = vertidxs[faceidx];
			u32 uindex = uvidxs[faceidx];

			localizedVertex_t vert;
			vert.localidx = j;
			vert.pos = verts[vindex];
			vert.uv = uvpos[uindex];
			vert.vertidx = vindex;
			vert.uvidx = uindex;
			vert.faceidx = faceidx;
			vert.parent_si = i;
			vert.hasShading = false;

			if (surfs[i].hasVertexColors && surfs[i].shading) {
				size_t shadingSize = 4 * (size_t)surfs[i].faceidxlen;
				size_t off = (size_t)j * 4;
				if (off + 4 <= shadingSize) {
					vert.hasShading = true;
					memcpy(vert.color, &surfs[i].shading[off], 4);
				}
				else {
					os::Printer::log("color failed");
				}
			}

			face.verts.push_back(vert);
		}
		local_faces.push_back(face);
	}

	for (u32 i = 0; i < local_faces.size(); i++) {
		for (u32 j = 0; j < local_faces[i].verts.size(); j++) {
			// Get a pointer to the current localized vertex
			localizedVertex_t* vA = &local_faces[i].verts[j];

			// 1. Get all global face indices sharing this UV/Pos point
			core::array<u32>* peer_face_indices = &uvpos_caller[vA->uvidx];

			for (u32 k = 0; k < peer_face_indices->size(); k++) {
				u32 peer_fidx = (*peer_face_indices)[k]; // Use 'k', not 'i'!

				// Skip if this is the same global index we are currently processing
				if (peer_fidx == vA->faceidx) continue;

				// 2. Find which surface(s) own this peer face index
				core::array<u32>* peer_surfs = &faceidxs_caller[peer_fidx];

				for (u32 s = 0; s < peer_surfs->size(); s++) {
					u32 other_si = (*peer_surfs)[s]; // Use 's', not 'j'!

					if (other_si != local_faces[i].si) {
						// 3. Search the other surface for the peer_fidx
						// (NOT vA->faceidx, which only exists on the current surface)
						int other_local_idx = local_faces[other_si].findVertFromGlobal(peer_fidx);

						if (other_local_idx != -1) {
							// Link the pointers
							localizedVertex_t* vB = &local_faces[other_si].verts[other_local_idx];
							vA->shared.push_back(vB);
						}

						// Track neighbor surface ID at the face level
						if (local_faces[i].shared.linear_search(other_si) == -1)
							local_faces[i].shared.push_back(other_si);
					}
				}
			}
		}
	}
	os::Printer::log("\t\tdone.");

}

void CScnSolid::rebuildSurfaces() {
	for (u32 i = 0; i < local_faces.size(); i++) {
		for (u32 j = 0; j < local_faces[i].verts.size(); j++) {
			localizedVertex_t* vA = &local_faces[i].verts[j];

			verts[vA->vertidx] = vA->pos;
			if (local_faces[i].flipX || local_faces[i].flipY) {


				//assume flip means that vert 1(Top Left) uvidx and vert 2(To Right) uvidx are swapped
				//only do for shared verts
				bool wasSwapped = false;
				for (u32 k = 0; k < local_faces[i].verts.size(); k++) {
					localizedVertex_t* vB = &local_faces[i].verts[k];

					if (vA->uv.getDistanceFromSQ(uvpos[vB->uvidx]) < 0.0001) {
						//swap uvidx
						uvidxs[vA->faceidx] = vB->uvidx;
						uvidxs[vB->faceidx] = vA->uvidx;
						wasSwapped = true;
						break;
					}
				}
			}
			if (vA->shared.empty()) {
				uvpos[vA->uvidx] = vA->uv;
			}
		}
		if (local_faces[i].flipX) local_faces[i].flipX = false;
		if (local_faces[i].flipY) local_faces[i].flipY = false;
	}
}