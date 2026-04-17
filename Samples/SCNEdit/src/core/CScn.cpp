#include "pch.h"
#include "include/core/CScn.h"
using namespace irr;
using namespace video;
//Resets variables
void CScn::reset()
{
	solids.clear();
	ents.clear();
	header = 0;
	cells.clear();
	ambients.clear();
	swt_start = nullptr;
}

//default constructor
CScn::CScn()
{
	reset();
}

//destructor (Removes solids and entities
CScn::~CScn()
{
	solids.clear();
	ents.clear();

	if (header)
		delete header;

	if (lmap) {
		delete lmap;
		lmap = new CLightmap();
	}
}

void CScn::collectLocalLightmap() {
	for (u32 i = 0; i < header->n_solids; i++) {
		for (u32 j = 0; j < solids[i]->local_faces.size(); j++) {
			if (lmap->hasLightmaps())
				solids[i]->local_faces[j].hlmap = lmap->getHLmap(i, j);
		}
	}
}

CEnt* CScn::getCell(u32 idx)
{
	CEnt* celli;
	for (u16 i = 0; i < CScn::cells.size(); i++)
	{
		celli = CScn::cells[i];
		const char* val = celli->getField("cell_index");
		if (atoi(val) == idx)
			return celli;
	}
	return nullptr;
}

CEnt* CScn::getAmbientByCell(const char* name)
{
	CEnt* ambienti;
	if (!name)
		return nullptr;

	for (u16 i = 0; i < CScn::ambients.size(); i++)
	{
		ambienti = CScn::ambients[i];

		const char* val = ambienti->getField("cells");
		if (val) {
			core::array<string> arr = str_split(val, ", ");
			for (int s = 0; s < arr.size(); s++) {
				if (str_equiv(arr[s].c_str(), name))
					return ambienti;
			}
		}
	}
	return nullptr;
}

CEnt* CScn::getSolidRef(s32 solidref_index)
{
	CEnt* solidrefi;
	for (u16 i = 0; i < CScn::func_solidref.size(); i++)
	{
		solidrefi = CScn::func_solidref[i];
		const char* val = solidrefi->getField("solidref_index");
		if (val) {
			if (atoi(val) == solidref_index) {
				return solidrefi;
			}
		}
	}
	return nullptr;
}

CEnt* CScn::getGlobalAmbient()
{
	CEnt* ambienti;
	if (CScn::ambients.size() == 1) {
		ambienti = CScn::ambients[0];
		return ambienti;
	}
	for (u16 i = 0; i < CScn::ambients.size(); i++)
	{
		ambienti = CScn::ambients[i];
		const char* val = ambienti->getField("TargetName");

		if (val) {
			if (str_equiv(val, "global_ambient")) {
				os::Printer::log("Found global ambient");
				return ambienti;
			}
		}
	}
	return nullptr;
}