#include "pch.h"
#include "include/io/CScnWriter.h"
#include "include/io/CSolidWriter.h"
#include "include/io/CLightmapIO.h"
#include "include/util.h"

int CScnWriter::write(std::ofstream* file, CScn* scn, bool scrapeLightmap)
{
	writeHeader(file, scn);
	writeSolids(file, scn);
	writeEntities(file, scn);
	if (!scrapeLightmap) writeLightmap(file, scn);
}

int CScnWriter::writeHeader(std::ofstream* file, CScn* scn)
{
	os::Printer::log("\nWriting header...");

	file->seekp(0);
	write_generic(file, scn->header, sizeof(header_t));

	os::Printer::log("\tdone.");

	return 0;
}

int CScnWriter::writeSolids(std::ofstream* file, CScn* scn)
{
	for (u32 i = 0; i < scn->header->n_solids; i++)
	{
		os::Printer::log(std::format("\nWriting solid {}/{}...", i, scn->header->n_solids - 1).c_str());
		CSolidWriter::write(file, scn->solids[i]);
		os::Printer::log("\tdone.");
	}
}

int CScnWriter::writeEntities(std::ofstream* file, CScn* scn)
{
	u32 n_ents = scn->header->n_ents;
	os::Printer::log(format("\nWriting {} Entities...", n_ents).c_str());

	if (scn->ents.allocated_size() == 0)
		error(true, "Error allocating memory for CEnt");

	file->seekp(scn->header->ents_offset2);

	for (u32 i = 0; i < n_ents; i++)
	{
		CEnt* enti = scn->ents[i];
		write_type<u32>(file, enti->n_fields);
		write_type<u32>(file, enti->srefidx);

		bool hasPos = false;

		for (u32 n = 0; n < enti->n_fields; n++)
		{
			write_type<u16>(file, enti->keylengths[n]);
			write_type<u16>(file, enti->vallengths[n]);
			if (enti->keylengths[n] >= 512 || enti->vallengths[n] >= 512)
				error(true, "Ent[%u] has field %u with too large a string", i, n);

			write_generic(file, enti->fields[n].key, enti->keylengths[n]);
			write_generic(file, enti->fields[n].value, enti->vallengths[n]);
		}
	}
	os::Printer::log("done.");
	return n_ents;
}

int CScnWriter::writeLightmap(std::ofstream* file, CScn* scn) {
	u32 n_lights = scn->header->n_lights;

	os::Printer::log(format("\nWriting {} Lightmaps...", n_lights).c_str());

	file->seekp(scn->header->lmaps_offset);
	os::Printer::log(format("\tlightmap offset at {}", scn->header->lmaps_offset).c_str());
	if (scn->lmap)
		CLightmapIO::write(file, scn->lmap, scn->solids, scn->header->n_extralmaps);
	else
		error(false, "CScn::writeLightmap: No lightmap found");
	os::Printer::log("\tdone.");
	return n_lights;
}

int CScnWriter::resize(CScn* scn) {
	os::Printer::log("Resizing scn file");
	u32 start_offset = scn->header->base_solid_offset;
	u32 current_offset = start_offset;
	u32 old_total_solids_len = scn->header->base_solid_length + scn->header->extra_solid_length;
	for (u32 i = 0; i < scn->solids.size(); i++) {
		scn->solids[i]->offset = current_offset;
		current_offset += CSolidWriter::resize(scn->solids[i]);
		if (i == 0) scn->header->base_solid_length = scn->solids[i]->length;
		else if (i == 1) {
			scn->header->extra_solid_offset = scn->solids[i]->offset;
			//Probably shouldn't need to even set length for extras cus it shouldnt change
			//But just to be safe.
			scn->header->extra_solid_length = scn->solids[i]->length;
		}
		else scn->header->extra_solid_length += scn->solids[i]->length;
	}

	u32 new_total_solids_len = current_offset - start_offset;
	s32 delta = (s32)new_total_solids_len - (s32)old_total_solids_len;

	scn->header->ents_offset += delta;
	scn->header->ents_offset2 += delta;
	scn->header->lmaps_offset += delta;
	os::Printer::log(format("\tDone - file diff {}",delta).c_str());
	return current_offset;

}
