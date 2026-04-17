#include "pch.h"
#include "include/io/CScnLoader.h"
#include "include/io/CSolidLoader.h"
#include "include/io/CLightmapIO.h"
#include "include/util.h"
using namespace irr;
using namespace video;
CScn* CScnLoader::load(std::ifstream* file)
{
	CScn* scn = new CScn();
	loadHeader(file, scn);
	loadSolids(file, scn);
	loadEntities(file, scn);
	loadLightmap(file, scn);

	scn->collectLocalLightmap();

	return scn;
}

int CScnLoader::loadHeader(std::ifstream* file, CScn* scn)
{
	os::Printer::log("\nGetting header...");

	file->seekg(0);
	scn->header = new header_t;

	read_generic(file, scn->header, sizeof(header_t));

	if (!str_equals_lim(scn->header->magic, "NCSM", 4))
		error(true, "CScn::loadHeader: Magic doesn't match, not a scn file!");

	if (scn->header->version != 269)
		error(true, "CScn::loadHeader: Wrong scn file version!");

	os::Printer::log("\tdone.");

	return 0;
}

int CScnLoader::loadSolids(std::ifstream* file, CScn* scn)
{
	scn->solids.set_used(scn->header->n_solids);

	if (scn->solids.allocated_size() == 0)
		error(true, "Error allocating %u CSolid's", scn->header->n_solids);
	//Sets offset and length of solid

	u32 currentOffset = scn->header->base_solid_offset;

	for (u32 i = 0; i < scn->header->n_solids; i++)
	{
		//Loads solids
		os::Printer::log(std::format("\nGetting solid {}/{}...", i, scn->header->n_solids - 1).c_str());

		scn->solids[i] = CSolidLoader::load(file, i, currentOffset);

		if (i < scn->header->n_solids - 1)
			//Sets overall offset of solid by the offset and length
			currentOffset += scn->solids[i]->length;

		os::Printer::log("\tdone.");
	}
}

int CScnLoader::loadEntities(std::ifstream* file, CScn* scn)
{
	u32 n_ents = scn->header->n_ents;
	os::Printer::log(format("\nGetting {} Entities...", n_ents).c_str());

	scn->ents.set_used(n_ents);
	if (scn->ents.allocated_size() == 0)
		error(true, "Error allocating memory for CEnt");

	file->seekg(scn->header->ents_offset2);

	CScn::cells.clear();
	CScn::ambients.clear();

	for (u32 i = 0; i < n_ents; i++)
	{
		CEnt* enti = new CEnt;

		enti->indx = i;
		read_type<u32>(file, enti->n_fields);
		read_type<u32>(file, enti->srefidx);

		bool hasPos = false;

		enti->fields = new (std::nothrow) CEnt::field[enti->n_fields];
		if (enti->fields == nullptr)
			error(true, "Error allocating memory for %u fields of ent[%u]", enti->n_fields, i);

		u16 keylen = 0, vallen = 0;

		for (u32 n = 0; n < enti->n_fields; n++)
		{
			read_type<u16>(file, keylen);
			read_type<u16>(file, vallen);
			if (keylen >= 512 || vallen >= 512)
				error(true, "Ent[%u] has field %u with too large a string", i, n);

			enti->entsad.push_back(file->tellg());
			read_generic(file, enti->fields[n].key, keylen);
			read_generic(file, enti->fields[n].value, vallen);
			enti->keylengths.push_back(keylen);
			enti->vallengths.push_back(vallen);

			if (str_equiv(enti->fields[n].key, "Classname"))
				os::Printer::log(format("({}) ({})", enti->fields[n].key, enti->fields[n].value).c_str());
			else
				os::Printer::log(format("\t({}) ({})", enti->fields[n].key, enti->fields[n].value).c_str());

			//TODO: make sorted according to cell indexs
			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "swt_start")) {
				if (!scn->swt_start)
					scn->swt_start = enti;
			}

			if (str_equiv(enti->fields[n].key, "Position") && str_equiv(enti->fields[n].value, "0")) {
				if (str_equiv(enti->getField("Classname"), "swt_start"))
					scn->swt_start = enti;
			}

			if (str_equiv(enti->fields[n].key, "Position")) {
				if (str_equiv(enti->getField("Classname"), "swt_start"))
					hasPos = true;
			}

			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "Cell"))
				CScn::cells.push_back(enti);

			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "light_ambient"))
				CScn::ambients.push_back(enti);

			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "func_solidref"))
				CScn::func_solidref.push_back(enti);
		}
		if (!hasPos && str_equiv(enti->getField("Classname"), "swt_start"))
			scn->swt_start = enti;
		scn->ents[i] = enti;
	}
	os::Printer::log("done.");
	return n_ents;
}

int CScnLoader::loadLightmap(std::ifstream* file, CScn* scn) {
	u32 n_lights = scn->header->n_lights;

	os::Printer::log(format("\nGetting {} Lightmaps...", n_lights).c_str());

	file->seekg(scn->header->lmaps_offset);
	os::Printer::log(format("\tlightmap offset at {}", scn->header->lmaps_offset).c_str());
	if (file->peek() != EOF)  //Verify that there is a lightmap.
		scn->lmap = CLightmapIO::load(file, scn->solids, scn->header->n_extralmaps);

	else
		error(false, "CScn::loadLightmap: No lightmap found");
	os::Printer::log("\tdone.");
	return n_lights;
}