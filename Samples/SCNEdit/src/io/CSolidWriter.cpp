#include "pch.h"
#include "include/io/CSolidWriter.h"
#include "include/util.h"

int CSolidWriter::write(std::ofstream* file, CSolid* solid)
{
	file->seekp(solid->offset);

	write_type<u32>(file, solid->n_unk1);
	write_type<u32>(file, solid->n_verts);
	write_type<u32>(file, solid->n_uvpos);
	write_type<u32>(file, solid->n_faceidx);
	write_type<u32>(file, solid->n_planes);
	write_type<u32>(file, solid->n_nodes);
	write_type<u32>(file, solid->n_surfs);
	write_type<u32>(file, solid->n_cells);
	write_type<u32>(file, solid->n_names);
	write_type<u32>(file, solid->length);

	writeSurfs(file, solid);
	writeNodes(file, solid);
	writePlanes(file, solid);
	writeVerts(file, solid);
	writeUVPos(file, solid);
	writeVertIdxs(file, solid);
	writeUVIdxs(file, solid);
	writeProjection(file, solid);
	writeCells(file, solid);
	writeNames(file, solid);
}

int CSolidWriter::writeSurfs(std::ofstream* file, CSolid* solid) {
	os::Printer::log(std::format("\Writing Surfaces... {}", solid->n_surfs).c_str());

	for (u16 i = 0; i < solid->n_surfs; i++)
	{
		write_generic(file, solid->surfs[i], 72);   //read the usual 72 first bytes

		if (solid->surfs[i]->hasVertexColors == 1)  //means there are more bytes - the shading or smoothing or whatever we call it
		{
			write_generic(file, solid->surfs[i]->shading, 4 * solid->surfs[i]->faceidxlen);
			//REMEMBER: because shading is initially set to  a random value, we must
			//make sure we only try to draw shading only when more is set to !0 or
			//make constructor to set initial value 0;
		}
		else if (solid->surfs[i]->hasVertexColors != 0)
			error(true, "CSolid: loadSurfs - Unexpected surface[%i].more value - expected 0 or 1 got %i", i, solid->surfs[i]->hasVertexColors);
	}

	os::Printer::log("\t\tdone.");

	return solid->n_surfs;
}

int CSolidWriter::writeNodes(std::ofstream* file, CSolid* solid)
{
	os::Printer::log("\Writing nodes...");
	write_pointers<node_t>(file, solid->tree->nodes, solid->n_nodes);
	os::Printer::log("\t\tdone.");

	return solid->n_nodes;
}
int CSolidWriter::writePlanes(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting planes... {}", solid->n_planes).c_str());
	write_pointers<plane_t>(file, solid->planes, solid->n_planes);
	os::Printer::log("\t\tdone.");
	return solid->n_planes;
}

int CSolidWriter::writeVerts(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting Vertices... {}", solid->n_verts).c_str());
	write_generic(file, solid->verts, sizeof(core::vector3df) * solid->n_verts);
	os::Printer::log("\t\tdone.");
	return solid->n_verts;
}

int CSolidWriter::writeUVPos(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting UV coordinates... {}", solid->n_uvpos).c_str());
	write_generic(file, solid->uvpos, sizeof(core::vector2df) * solid->n_uvpos);
	os::Printer::log("\t\tdone.");
	return solid->n_uvpos;
}
int CSolidWriter::writeVertIdxs(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting Vertex indices... {}", solid->n_faceidx).c_str());
	write_generic(file, solid->vertidxs, sizeof(u32) * solid->n_faceidx);
	os::Printer::log("\t\tdone.");
	return solid->n_faceidx;
}
int CSolidWriter::writeUVIdxs(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting UV indices... {}", solid->n_faceidx).c_str());
	write_generic(file, solid->uvidxs, sizeof(u32) * solid->n_faceidx);
	os::Printer::log("\t\tdone.");
	return solid->n_faceidx;
}
int CSolidWriter::writeProjection(std::ofstream* file, CSolid* solid)
{
	os::Printer::log(std::format("\tWriting projection basis... {}", solid->n_surfs).c_str());
	write_pointers<projBasis_t>(file, solid->projection, solid->n_surfs);
	os::Printer::log("\t\tdone.");
	return solid->n_surfs;
}

int CSolidWriter::writeCells(std::ofstream* file, CSolid* solid)
{
	os::Printer::log("\tWriting Cells...");

	//loop with something about the number of cells
	for (u32 i = 0; i < solid->n_cells; i++)
	{
		write_generic(file, solid->rawcells[i]->name, 32);
		write_type<s32>(file, solid->rawcells[i]->n_nodesidxs);
		write_type<s32>(file, solid->rawcells[i]->n_portals);
		write_type<s32>(file, solid->rawcells[i]->n_occluders);
		write_generic(file, solid->rawcells[i]->skyname, 32);

		os::Printer::log(std::format("\t\tcell[{}]: {} {} {} {} {}",
			i, solid->rawcells[i]->name, solid->rawcells[i]->n_nodesidxs, solid->rawcells[i]->n_portals,
			solid->rawcells[i]->n_occluders, solid->rawcells[i]->skyname).c_str());

		write_generic(file, solid->rawcells[i]->nodesidxs, 2 * solid->rawcells[i]->n_nodesidxs);

		os::Printer::log(std::format("\Writing {} portals... ", solid->rawcells[i]->n_portals).c_str());
		for (s32 j = 0; j < solid->rawcells[i]->n_portals; j++)
		{
			writePortal(file, solid->rawcells[i]->portals[j]);
		}
		os::Printer::log("\t\tdone.");
		os::Printer::log("\Writing cell data... ");
		writeCellData(file, solid->rawcells[i], &(solid->rawcells[i]->bvh));
		os::Printer::log("\t\tdone.");
	}
	os::Printer::log("\t\tdone.");

	return solid->n_cells;
}

int CSolidWriter::writePortal(std::ofstream* file, portal_t* portal)
{
	write_generic(file, portal->name, 32);
	os::Printer::log(std::format("\t\Writing portal with {} names...", strlen(portal->name)).c_str());

	write_type<s16>(file, portal->nextcell);
	write_type<u8>(file, portal->flag1);
	write_type<u8>(file, portal->flag2);
	write_generic(file, &(portal->plane), sizeof(plane_t));
	write_type<f32>(file, portal->winding);
	write_type<s32>(file, portal->n_verts);
	write_generic(file, portal->bb_verts, sizeof(core::vector3df) * 2);

	write_generic(file, portal->verts, sizeof(core::vector3df) * portal->n_verts);

	os::Printer::log("\t\t\tdone writing portal. ");
	return 0;
}

int CSolidWriter::writeCellData(std::ofstream* file, rawCell_t* raw, cellData_t* celldata) {
	int n = 0;
	write_generic(file, celldata->bb_verts, sizeof(core::vector3df) * 2);
	write_type<u16>(file, celldata->n_children);
	write_type<u16>(file, celldata->n_surfs);
	if (celldata->n_surfs > 0) {
		for (u16 i = 0; i < celldata->n_surfs; i++)
			write_type<u16>(file, celldata->surfsidxs[i]);
	}
	if (celldata->n_children > 0) {
		for (s32 j = 0; j < celldata->n_children; j++) {
			n += writeCellData(file, raw, celldata->children[j]) + 1;
		}
	}
	return n;
}
int CSolidWriter::writeNames(std::ofstream* file, CSolid* solid) {
	os::Printer::log("\tWriting Names...");
	for (int i = 0; i < solid->n_names; i++)
		write_generic(file, solid->names[i], 32);

	os::Printer::log("\t\tdone.");

	return solid->n_names;
}

int CSolidWriter::resize(CSolid* solid) {
	u32 new_len = 40; // The 10 u32 header variables (n_unk...length)
	new_len += calcSizeFace(solid);
	new_len += calcSizeCells(solid);
	new_len += solid->n_names * 32;
	solid->length = new_len;
	return new_len;
}

int CSolidWriter::calcSizeFace(CSolid* solid) {
	int new_len = 0;
	for (u32 i = 0; i < solid->n_surfs; i++) {
		new_len += 72; // base surf_t
		if (solid->surfs[i]->hasVertexColors == 1) {
			new_len += (4 * solid->surfs[i]->faceidxlen);
		}
	}

	// 2. Nodes, Planes, Verts, UVPos, Idxs
	new_len += solid->n_nodes * sizeof(node_t);
	new_len += solid->n_planes * sizeof(plane_t);
	new_len += solid->n_verts * sizeof(core::vector3df);
	new_len += solid->n_uvpos * sizeof(core::vector2df);
	new_len += solid->n_faceidx * sizeof(u32);
	new_len += solid->n_faceidx * sizeof(u32);
	new_len += solid->n_surfs * sizeof(projBasis_t);
	return new_len;
}

int CSolidWriter::calcSizeCells(CSolid* solid) {
	int new_len = 0;
	for (u32 i = 0; i < solid->n_cells; i++) {
		new_len += 76; // name(32) + n_nodes(4) + n_portals(4) + n_occ(4) + sky(32)
		new_len += (solid->rawcells[i]->n_nodesidxs * 2); // u16 array

		for (s32 j = 0; j < solid->rawcells[i]->n_portals; j++) {
			new_len += 32; // name
			new_len += 2;  // nextcell
			new_len += 1;  // flag1
			new_len += 1;  // flag2
			new_len += sizeof(plane_t);
			new_len += 4;  // winding (f32)
			new_len += 4;  // n_verts (s32)
			new_len += 24; // bb_verts (2 * vector3df)
			new_len += (solid->rawcells[i]->portals[j]->n_verts * 12); // verts (vector3df)
		}
		new_len += calcSizeCellData(&solid->rawcells[i]->bvh);
	}
	return new_len;
}

int CSolidWriter::calcSizeCellData(cellData_t* data) {
	u32 new_len = 0;
	new_len += 24; // bb_verts (2 * vector3df)
	new_len += 2;  // n_children
	new_len += 2;  // n_surfs
	new_len += (data->n_surfs * 2); // surfsidxs (u16)

	for (u32 i = 0; i < data->n_children; i++) {
		new_len += calcSizeCellData(data->children[i]);
	}
	return new_len;
}