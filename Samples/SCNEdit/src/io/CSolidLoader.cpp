#include "pch.h"
#include "include/io/CSolidLoader.h"
#include "include/util.h"

CSolid* CSolidLoader::load(std::ifstream* file, u32 solididx, u32 offset) {
	CSolid* solid = new CSolid();
	solid->solididx = solididx;
	//file offset
	solid->offset = offset;
	file->seekg(solid->offset);
	os::Printer::log(std::format("\toffset: {}", solid->offset).c_str());
	//reads file for information in 32 bits
	//all loading up number of...
	if (solid->firstVal == NULL)
		solid->firstVal = true;

	//load solid header
	read_type<u32>(file, solid->n_unk1);
	read_type<u32>(file, solid->n_verts);
	read_type<u32>(file, solid->n_uvpos);
	read_type<u32>(file, solid->n_faceidx);
	read_type<u32>(file, solid->n_planes);
	read_type<u32>(file, solid->n_nodes);
	read_type<u32>(file, solid->n_surfs);
	read_type<u32>(file, solid->n_cells);
	read_type<u32>(file, solid->n_names);
	read_type<u32>(file, solid->length);

	//288 bits of variables in the scn file
	//must be in this order
	loadSurfs(file, solid);
	loadNodes(file, solid);
	loadPlanes(file, solid);
	loadVerts(file, solid);
	loadUVPos(file, solid);
	loadVertIdxs(file, solid);
	loadUVIdxs(file, solid);
	loadProjection(file, solid);
	loadCells(file, solid);
	loadNames(file, solid);

	int n_texs = solid->calcUniqueTexturesNames();
	os::Printer::log(std::format("\t{} unique textures", n_texs).c_str());

	//if number of textures is not equal to the texture size
	if (n_texs != solid->textures.size())
		error(true, "loadSolid: Number of unique textures and texture array size doesn't match");

	solid->buildBackTree();
	solid->extractSurfaces();
	
	return solid;
}

int CSolidLoader::loadSurfs(std::ifstream* file, CSolid* solid) {
	os::Printer::log(std::format("\tGetting Surfaces... {}", solid->n_surfs).c_str());

	solid->surfs.set_used(solid->n_surfs);

	u16 i;
	for (i = 0; i < solid->n_surfs; i++)
	{
		solid->surfs[i] = new surf_t;
		read_generic(file, solid->surfs[i], 72);   //read the usual 72 first bytes

		if (solid->surfs[i]->hasVertexColors == 1)  //means there are more bytes - the shading or smoothing or whatever we call it
		{
			solid->surfs[i]->shading = new u8[4 * solid->surfs[i]->faceidxlen]; //allocate
			read_generic(file, solid->surfs[i]->shading, 4 * solid->surfs[i]->faceidxlen);
			//REMEMBER: because shading is initially set to  a random value, we must
			//make sure we only try to draw shading only when more is set to !0 or
			//make constructor to set initial value 0;
		}
		else if (solid->surfs[i]->hasVertexColors != 0)
			error(true, "CSolid: loadSurfs - Unexpected surface[%i].more value - expected 0 or 1 got %i", i, solid->surfs[i]->hasVertexColors);
	}

	os::Printer::log("\t\tdone.");

	return i;
}

int CSolidLoader::loadNodes(std::ifstream* file, CSolid* solid)
{
	os::Printer::log("\tReading nodes...");

	solid->tree = new CBSPTree();
	read_pointers<node_t>(file, solid->tree->nodes, solid->n_nodes);
	os::Printer::log("\t\tdone.");

	return solid->n_nodes;
}
int CSolidLoader::loadPlanes(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting planes... {}", solid->n_planes).c_str());
	read_pointers<plane_t>(file, solid->planes, solid->n_planes);
	os::Printer::log("\t\tdone.");
	return solid->n_planes;
}

int CSolidLoader::loadVerts(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting Vertices... {}", solid->n_verts).c_str());
	solid->verts = new core::vector3df[solid->n_verts];   //allocate
	read_generic(file, solid->verts, sizeof(core::vector3df) * solid->n_verts);
	os::Printer::log("\t\tdone.");
	return solid->n_verts;
}

int CSolidLoader::loadUVPos(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting UV coordinates... {}", solid->n_uvpos).c_str());
	solid->uvpos = new core::vector2df[solid->n_uvpos];   //allocate
	read_generic(file, solid->uvpos, sizeof(core::vector2df) * solid->n_uvpos);
	os::Printer::log("\t\tdone.");
	return solid->n_uvpos;
}
int CSolidLoader::loadVertIdxs(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting Vertex indices... {}", solid->n_faceidx).c_str());
	solid->vertidxs = new u32[solid->n_faceidx];   //allocate
	read_generic(file, solid->vertidxs, sizeof(u32) * solid->n_faceidx);
	os::Printer::log("\t\tdone.");
	return solid->n_faceidx;
}
int CSolidLoader::loadUVIdxs(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting UV indices... {}", solid->n_faceidx).c_str());
	solid->uvidxs = new u32[solid->n_faceidx];   //allocate
	read_generic(file, solid->uvidxs, sizeof(u32) * solid->n_faceidx);
	os::Printer::log("\t\tdone.");
	return solid->n_faceidx;
}
int CSolidLoader::loadProjection(std::ifstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tGetting projection basis... {}", solid->n_surfs).c_str());
	read_pointers<projBasis_t>(file, solid->projection, solid->n_surfs);
	os::Printer::log("\t\tdone.");
	return solid->n_surfs;
}

int CSolidLoader::loadCells(std::ifstream* file, CSolid* solid)
{
	os::Printer::log("\tGetting Cells...");
	//sets raw cells to number of cells
	solid->rawcells.set_used(solid->n_cells);
	//loop with something about the number of cells
	for (u32 i = 0; i < solid->n_cells; i++)
	{   //sets nodeid and portals to 0 for each cell
		solid->rawcells[i] = new rawCell_t;
		solid->rawcells[i]->nodesidxs = 0;
		solid->rawcells[i]->portals.clear();

		read_generic(file, solid->rawcells[i]->name, 32);
		read_type<s32>(file, solid->rawcells[i]->n_nodesidxs);
		read_type<s32>(file, solid->rawcells[i]->n_portals);
		read_type<s32>(file, solid->rawcells[i]->n_occluders);
		read_generic(file, solid->rawcells[i]->skyname, 32);

		os::Printer::log(std::format("\t\tcell[{}]: {} {} {} {} {}",
			i, solid->rawcells[i]->name, solid->rawcells[i]->n_nodesidxs, solid->rawcells[i]->n_portals,
			solid->rawcells[i]->n_occluders, solid->rawcells[i]->skyname).c_str());

		//creates new node id
		solid->rawcells[i]->nodesidxs = new u16[solid->rawcells[i]->n_nodesidxs];
		read_generic(file, solid->rawcells[i]->nodesidxs, 2 * solid->rawcells[i]->n_nodesidxs);

		solid->rawcells[i]->portals.set_used(solid->rawcells[i]->n_portals);
		os::Printer::log(std::format("\tReading {} portals... ", solid->rawcells[i]->n_portals).c_str());
		for (s32 j = 0; j < solid->rawcells[i]->n_portals; j++)
		{
			solid->rawcells[i]->portals[j] = new portal_t;
			loadPortal(file, solid->rawcells[i]->portals[j]);
		}
		os::Printer::log("\t\tdone.");
		os::Printer::log("\tReading cell data... ");
		loadCellData(file, solid->rawcells[i], &(solid->rawcells[i]->bvh));
		os::Printer::log("\t\tdone.");
	}
	os::Printer::log("\t\tdone.");

	return solid->n_cells;
}

int CSolidLoader::loadPortal(std::ifstream* file, portal_t* portal)
{
	read_generic(file, portal->name, 32);
	os::Printer::log(std::format("\t\tReading portal with {} names...", strlen(portal->name)).c_str());

	read_type<s16>(file, portal->nextcell);
	read_type<u8>(file, portal->flag1);
	read_type<u8>(file, portal->flag2);
	read_generic(file, &(portal->plane), sizeof(plane_t));
	read_type<f32>(file, portal->winding);
	read_type<s32>(file, portal->n_verts);
	read_generic(file, portal->bb_verts, sizeof(core::vector3df) * 2);

	//creates new portal vertices
	portal->verts = new core::vector3df[portal->n_verts];
	read_generic(file, portal->verts, sizeof(core::vector3df) * portal->n_verts);

	os::Printer::log("\t\t\tdone reading portal. ");
	return 0;
}

int CSolidLoader::loadCellData(std::ifstream* file, rawCell_t* raw, cellData_t* celldata) {
	int n = 0;
	read_generic(file, celldata->bb_verts, sizeof(core::vector3df) * 2);
	read_type<u16>(file, celldata->n_children);
	read_type<u16>(file, celldata->n_surfs);
	if (celldata->n_surfs > 0) {
		celldata->surfsidxs = new u16[celldata->n_surfs];
		for (u16 i = 0; i < celldata->n_surfs; i++)
		{
			read_type<u16>(file, celldata->surfsidxs[i]);
			bool found = false;
			for (u16 j = 0; j < raw->naivesurfs.size(); j++) {
				if (raw->naivesurfs[j] == celldata->surfsidxs[i])//Add if exists
					found = true;
			}
			if (!found) raw->naivesurfs.push_back(celldata->surfsidxs[i]);
		}
	}
	if (celldata->n_children > 0) {
		celldata->children.set_used(celldata->n_children);
		for (s32 j = 0; j < celldata->n_children; j++) {
			celldata->children[j] = new cellData_t;
			n += loadCellData(file, raw, celldata->children[j]) + 1;
		}
	}
	else
		raw->leafnode.push_back(celldata);

	return n;
}
int CSolidLoader::loadNames(std::ifstream* file, CSolid* solid) {
	os::Printer::log("\tGetting Names...");
	for (int i = 0; i < solid->n_names; i++) {
		char name[32] = "";
		read_generic(file, name, 32);
		solid->names.push_back(name);
	}
	os::Printer::log("\t\tdone.");

	return solid->n_names;
}