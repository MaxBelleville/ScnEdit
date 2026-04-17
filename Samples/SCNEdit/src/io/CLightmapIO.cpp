#include "pch.h"
#include "include/io/CLightmapIO.h"
#include "include/util.h"
CLightmap* CLightmapIO::load(std::ifstream* file, core::array<CSolid*> solids, u32 n_extralmaps)
{
	CLightmap* lmap = new CLightmap();
	loadSwitchable(file, lmap, n_extralmaps);

	for (u32 i = 0; i < solids.size(); i++) {
		os::Printer::log(std::format("\tGetting lightmap headers for solid {}/{}...", i, solids.size() - 1).c_str());
		lMapHeader_t* tmp_hlmap = new (std::nothrow) lMapHeader_t[solids[i]->n_surfs];
		read_generic(file, tmp_hlmap, sizeof(lMapHeader_t) * solids[i]->n_surfs);
		lmap->hlmaps.push_back(tmp_hlmap);
		if (i == 0)
			loadLumps(file, lmap, solids[i]->n_cells);
	}
	lmap->buildLightmap(solids);

	return lmap;
}

int CLightmapIO::write(std::ofstream* file, CLightmap* lmap, core::array<CSolid*> solids, u32 n_extralmaps)
{
	writeSwitchable(file, lmap, n_extralmaps);
	for (u32 i = 0; i < solids.size(); i++) {
		os::Printer::log(std::format("\Writing lightmap headers for solid {}/{}", i, solids.size() - 1).c_str());
		write_generic(file, lmap->hlmaps[i], sizeof(lMapHeader_t) * solids[i]->n_surfs);
		if (i == 0)
			writeLumps(file, lmap, solids[i]->n_cells);
	}
}

int CLightmapIO::loadSwitchable(std::ifstream* file, CLightmap* lmap, u32 n_extralmaps)
{
	os::Printer::log(std::format("\tGetting {} switchable lightmaps...", n_extralmaps).c_str());
	lmap->hslmaps = new (std::nothrow) switchLMapHeader_t[n_extralmaps];
	read_generic(file, lmap->hslmaps, sizeof(switchLMapHeader_t) * n_extralmaps);
	os::Printer::log("\t\tdone.");
}

int CLightmapIO::writeSwitchable(std::ofstream* file, CLightmap* lmap, u32 n_extralmaps)
{
	os::Printer::log(std::format("\Writing {} switchable lightmaps...", n_extralmaps).c_str());
	write_generic(file, lmap->hslmaps, sizeof(switchLMapHeader_t) * n_extralmaps);
	os::Printer::log("\t\tdone.");
}

int CLightmapIO::loadLumps(std::ifstream* file, CLightmap* lmap, u32 n_cells)
{
	os::Printer::log(std::format("\tGetting {} lightmap lumps...", n_cells).c_str());
	for (u32 j = 0; j < n_cells; j++) {
		lMapLump_t* tmp_lump = new lMapLump_t;
		// Ensure fields are initialized
		tmp_lump->data = nullptr;
		read_type(file, tmp_lump->size);
		read_type(file, tmp_lump->compression_type);
		if (tmp_lump->size > 0) {
			tmp_lump->data = new s8[tmp_lump->size];
			read_generic(file, tmp_lump->data, sizeof(s8) * tmp_lump->size);
		}
		else {
			// leave data == nullptr, size == 0
		}
		lmap->lumps.push_back(tmp_lump);
	}
	os::Printer::log("\t\tdone.");
}

int CLightmapIO::writeLumps(std::ofstream* file, CLightmap* lmap, u32 n_cells)
{
	os::Printer::log(std::format("\Writing {} lightmap lumps...", n_cells).c_str());
	for (u32 j = 0; j < n_cells; j++) {
		write_type(file, lmap->lumps[j]->size);
		write_type(file, lmap->lumps[j]->compression_type);
		if (lmap->lumps[j]->size > 0) {
			write_generic(file, lmap->lumps[j]->data, sizeof(s8) * lmap->lumps[j]->size);
		}
	}
	os::Printer::log("\t\tdone.");
}