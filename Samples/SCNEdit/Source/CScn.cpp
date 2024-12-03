#include "pch.h"
#include "CScn.h"
using namespace irr;
using namespace video;
//Resets variables
void CScn::reset()
{
    solids=0;
    ents=0;
    header=0;
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
        log+=(SAY("Getting solid %i/%u...\n",i,header->n_solids-1));
        solids[i].loadSolid(file);
        log += solids[i].getLog();
        if (i < header->n_solids - 1)
        //Sets overall offset of solid by the offset and length
            solids[i+1].offset=solids[i].offset + solids[i].length;

        log+=(SAY("done.\n"));
    }

    loadEntities(file);
    loadLightmap(file);
    return 0;
}

int CScn::loadHeader(ifstream * file)
{
 	log+=(SAY("Getting header..."));

 	//header=(scnHeader_t*) malloc(sizeof(struct scnHeader_t));
    header = new scnHeader_t;

	file->seekg(0);
    read_generic(header,file,sizeof(scnHeader_t));

	if (!str_nequals(header->magic,"NCSM",4))
		error(true,"CScn::loadHeader: Magic doesn't match, not a scn file!");

    if (header->version!=269)
		error(true,"CScn::loadHeader: Wrong scn file version!");

    log+=(SAY("done.\n"));
	return 0;
}


int CScn::loadEntities(std::ifstream * file)
{

    u32 n_ents = header->n_ents;
    log+=(SAY("Getting %u Entities...",n_ents));

    ents = new (std::nothrow) CScnEnt[n_ents];
    if (ents==NULL) {
        error(true,"Error allocating memory for CScnEnt");
    }
    file->seekg(header->ents_offset2);

    u16 keylen;
    u16 vallen;
    CScnEnt * enti;
    char str1[512]{};
    char str2[512]{};

    for (u32 i=0;i<n_ents;i++)
    {
        enti=&ents[i];

        enti->n_fields = read_u32(file);
        enti->srefidx = read_u32(file);

        enti->fields = new (std::nothrow) CScnEnt::field[enti->n_fields];
        if (enti->fields == NULL)
            error(true,"Error allocating memory for %d fields of ent[%d]",enti->n_fields,i);
        log+=(SAY("\r\n"));
        for (u32 n=0;n<enti->n_fields;n++)
        {
            keylen=read_u16(file);
            vallen=read_u16(file);
            if (keylen >= 512 || vallen >= 512)
                error(true,"Ent[%i] has field %i with too large a string",i,n);
            
            enti->entsad.push_back(file->tellg());
            read_generic(enti->fields[n].key,file,keylen);
            read_generic(enti->fields[n].value,file,vallen);
            enti->keylengths.push_back(keylen);
            enti->vallengths.push_back(vallen);
            
            log+=(SAY("(%s) (%s) %i %i\n", enti->fields[n].key, enti->fields[n].value, str_equiv(enti->fields[n].key, "Classname"), str_equiv(enti->fields[n].value, "Cell")));
            //TODO: make sorted according to cell indexs
            if (str_equiv(enti->fields[n].key,"Classname") && str_equiv(enti->fields[n].value,"Cell"))
                cells.push_back(enti);
            if (str_equiv(enti->fields[n].key, "Classname") && str_equiv(enti->fields[n].value, "light_ambient"))
                ambients.push_back(enti);
        }
    }
    log+=(SAY("%u done.\n",n_ents));
    return n_ents;
}

int CScn::loadLightmap(std::ifstream* file) {
    u32 n_lights = header->n_lights;

    log+=(SAY("Getting %u Lightmaps...", n_lights));


    file->seekg(header->lmaps_offset);
    log+=(SAY("Char at %u\n", file->peek()));
    if (file->peek() != EOF) { //Verify that there is a lightmap.
        lmap.loadLightmap(file, solids, header->n_solids, header->n_extralmaps);
    }
    else error(false, "CScn::loadLightmap: No lightmap found");
    return n_lights;
}


CScnEnt * CScn::getCell(u32 idx)
{
    CScnEnt * celli;
    for (u16 i=0; i < cells.size(); i++)
    {
        celli = cells[i];
        const char * val=celli->getField("cell_index");
        if (atoi(val) == idx)
            return celli;
    }
    return NULL;
}

CScnEnt* CScn::getAmbientByCell(const char* name)
{
    CScnEnt* ambienti;
    if (!name) return NULL;

    for (u16 i = 0; i < ambients.size(); i++)
    {
        ambienti = ambients[i];

        const char * val = ambienti->getField("cells");
        if (val) {
            core::array<string> arr = str_split(val, ", ");
            for (int s = 0; s < arr.size(); s++) {
                if (str_equiv(arr[s].c_str(), name))
                    return ambienti;
            }
        }
    }
    return NULL;
}
CScnEnt* CScn::getGlobalAmbient()
{
    CScnEnt* ambienti;
    if (ambients.size() == 1) {
        ambienti = ambients[0];
        return ambienti;
    }
    for (u16 i = 0; i < ambients.size(); i++)
    {
        ambienti = ambients[i];
        const char* val = ambienti->getField("TargetName");
        
        if (val) {
            
            if (str_equiv(val, "global_ambient"))
                return ambienti;
        }

    }
    return NULL;
}











