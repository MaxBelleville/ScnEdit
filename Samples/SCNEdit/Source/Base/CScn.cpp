#include "pch.h"
#include "Header/Base/CScn.h"
using namespace irr;
using namespace video;
//Resets variables
void CScn::reset()
{
	solids=0;
	ents=0;
	header=0;
	cells.clear();
	ambients.clear();
	swt_start = nullptr;
}

//default constructor
CScn::CScn ()
{
	reset();
}

//constructor that reads scn file
CScn::CScn(std::ifstream * file)
{
	reset();
	loadFile(file);
}

//destructor (Removes solids and entities
CScn::~CScn()
{
	if (solids)
		delete [] solids;

	if (header)
		delete header;

	if (ents)
		delete [] ents;
 
	if (lmap) {
		delete lmap;
		lmap = new CScnLightmap();
	}
}

int CScn::loadFile(std::ifstream * file)
{
	//IMPORTANT: load must be in this order so we don't have to store addresses
	loadHeader(file);
	//Loads solids
	solids = (CScnSolid*) new (std::nothrow) CScnSolid[header->n_solids];

	if (!solids)
		error(true,"Error allocating %u CScnSolid's", header->n_solids);
	//Sets offset and length of solid
	solids[0].offset=header->solid0_offset;
	solids[0].length=header->solid0_length;

	for (u32 i=0; i < header->n_solids; i++)
	{
		//Loads solids
		os::Printer::log(format("\nGetting solid {}/{}...",i,header->n_solids-1).c_str());
		solids[i].loadSolid(file,i);
		if (i < header->n_solids - 1)
		//Sets overall offset of solid by the offset and length
			solids[i+1].offset=solids[i].offset + solids[i].length;

		os::Printer::log("\tdone.");
	}

	loadEntities(file);
	loadLightmap(file);
	return 0;
}

int CScn::loadHeader(ifstream * file)
{
	os::Printer::log("\nGetting header...");

	header = new scnHeader_t;

	file->seekg(0);
	read_generic(header,file,sizeof(scnHeader_t));

	if (!str_equals_lim(header->magic,"NCSM",4))
		error(true,"CScn::loadHeader: Magic doesn't match, not a scn file!");

	if (header->version!=269)
		error(true,"CScn::loadHeader: Wrong scn file version!");

	os::Printer::log("\tdone.");
	return 0;
}


int CScn::loadEntities(std::ifstream * file)
{

	u32 n_ents = header->n_ents;
	os::Printer::log(format("\nGetting {} Entities...",n_ents).c_str());

	ents = new (std::nothrow) CScnEnt[n_ents];
	if (ents==NULL) 
		error(true,"Error allocating memory for CScnEnt");
	
	file->seekg(header->ents_offset2);

	u16 keylen, vallen;
	CScnEnt * enti;
	char str1[512] = {},str2[512] = {};
	CScn::cells.clear();
	CScn::ambients.clear();
	

	for (u32 i=0;i<n_ents;i++)
	{
		enti=&ents[i];
		enti->indx = i;
		enti->n_fields = read_u32(file);
		enti->srefidx = read_u32(file);
		bool hasPos = false;
		enti->fields = new (std::nothrow) CScnEnt::field[enti->n_fields];
		if (enti->fields == nullptr)
			error(true,"Error allocating memory for %u fields of ent[%u]",enti->n_fields,i);

		for (u32 n=0;n<enti->n_fields;n++)
		{
			keylen=read_u16(file);
			vallen=read_u16(file);
			if (keylen >= 512 || vallen >= 512)
				error(true,"Ent[%u] has field %u with too large a string",i,n);
			
			enti->entsad.push_back(file->tellg());
			read_generic(enti->fields[n].key,file,keylen);
			read_generic(enti->fields[n].value,file,vallen);
			enti->keylengths.push_back(keylen);
			enti->vallengths.push_back(vallen);
			
			if(str_equiv(enti->fields[n].key, "Classname")) 
				os::Printer::log(format("({}) ({})",enti->fields[n].key, enti->fields[n].value).c_str());
			else 
				os::Printer::log(format("\t({}) ({})",enti->fields[n].key,enti->fields[n].value).c_str());

			//TODO: make sorted according to cell indexs
			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "swt_start")) {
				if (!swt_start) 
					swt_start = enti;
			}

			if (str_equiv(enti->fields[n].key, "Position") && str_equiv(enti->fields[n].value, "0")) {
				if(str_equiv(enti->getField("Classname"),"swt_start")) 
					swt_start = enti;
			}

			if (str_equiv(enti->fields[n].key, "Position")) {
				if (str_equiv(enti->getField("Classname"), "swt_start")) 
					hasPos = true;
			}

			if (str_equiv(enti->fields[n].key,"Classname") && str_equiv(enti->fields[n].value,"Cell"))
				CScn::cells.push_back(enti);

			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "light_ambient"))
				CScn::ambients.push_back(enti);

			if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "func_solidref"))
				CScn::func_solidref.push_back(enti);
		}
		if (!hasPos && str_equiv(enti->getField("Classname"), "swt_start")) 
			swt_start = enti;
		
	}
	os::Printer::log("done.");
	return n_ents;
}

int CScn::loadLightmap(std::ifstream* file) {
	u32 n_lights = header->n_lights;

	os::Printer::log(format("\nGetting {} Lightmaps...", n_lights).c_str());

	file->seekg(header->lmaps_offset);
	os::Printer::log(format("\tlightmap offset at {}", file->peek()).c_str());
	if (file->peek() != EOF)  //Verify that there is a lightmap.
		lmap->loadLightmap(file, solids, header->n_solids, header->n_extralmaps);
	
	else 
		error(false, "CScn::loadLightmap: No lightmap found");
	os::Printer::log("\tdone.");
	return n_lights;
}


CScnEnt * CScn::getCell(u32 idx)
{
	CScnEnt * celli;
	for (u16 i=0; i < CScn::cells.size(); i++)
	{
		celli = CScn::cells[i];
		const char * val=celli->getField("cell_index");
		if (atoi(val) == idx)
			return celli;
	}
	return nullptr;
}

CScnEnt* CScn::getAmbientByCell(const char* name)
{
	CScnEnt* ambienti;
	if (!name) 
		return nullptr;

	for (u16 i = 0; i < CScn::ambients.size(); i++)
	{
		ambienti = CScn::ambients[i];

		const char * val = ambienti->getField("cells");
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

CScnEnt* CScn::getSolidRef(s32 solidref_index)
{
	CScnEnt* solidrefi;
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


CScnEnt* CScn::getGlobalAmbient()
{

	CScnEnt* ambienti;
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
const char* CScn::getMaterialName(u8 material) {
	u8 materialType = material & 0xF0;

	switch (materialType) {
	case ESM_DEFAULT:    return "Default";
	case ESM_LIQUID:     return "Liquid";
	case ESM_MUD:        return "Mud";
	case ESM_GRAVEL:     return "Gravel";
	case ESM_PLASTER:    return "Plaster";
	case ESM_CARPET:     return "Carpet";
	case ESM_GLASS:      return "Glass";
	case ESM_WOOD:       return "Wood";
	case ESM_CREAKWOOD:  return "Creakwood";
	case ESM_BRICK:      return "Brick";
	case ESM_SHEETMETAL: return "Sheet Metal";
	case ESM_STEEL:      return "Steel";
	default:             return "Unknown";
	}
}











