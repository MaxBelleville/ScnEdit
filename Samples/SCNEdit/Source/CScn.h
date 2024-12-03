#ifndef CSCN_H_
#define CSCN_H_
#include "CScnEnt.h"
#include "scntypes.h"
#include "CScnSolid.h"
#include "util.h"
#include "CScnLightmap.h"

using namespace std;
using namespace irr;
using namespace video;

enum N_TYPE{
    N_UNK1=0, N_VERTS, N_UVPOS, N_VERTIDXS, N_PLANES, N_NODES, N_SURFS, N_CELLS, N_UNK2
};

class CScn
{
private:
    void reset();
    CScnEnt * ents;
    CScnLightmap lmap;
    int loadFile(std::ifstream * file);
    int loadHeader(std::ifstream * file);
    int loadEntities(std::ifstream * file);
    int loadLightmap(std::ifstream* file);
    core::stringw log;

public:
    scnHeader_t * header;
    CScnSolid * solids;

    u32 getVersion() const {
        return header->version;}

    vector<CScnEnt*> cells;
    vector<CScnEnt*> ambients;

    //get cell by index as defined by cell_index field
    CScnEnt * getCell(u32 cell_index);
    CScnEnt* getAmbientByCell(const char* name);
    CScnEnt* getGlobalAmbient();

    CScn ();
    CScn (std::ifstream *);
    ~CScn ();

    CScnSolid* getAllSolids() const
    {
        return solids;
    }

    CScnEnt* getAllEnts()
    {
        return ents;
    }

    //returns pointer to CScnSolid from index or NULL if none
    CScnSolid * getSolid(u32 idx)
    {
        if (idx < header->n_solids)
            return &solids[idx];
        else
            return NULL;
    }

    //returns pointer to CScnEnt from index or NULL if none
    CScnEnt * getEnt(u32 idx)
    {
        if (idx < header->n_ents)
            return &ents[idx];
        else
            return NULL;
    }

    CScnLightmap getLightmap()
    {
        return lmap;
    }

    u32 getSolidSize(bool bAll) const
    {
        if (!bAll) return 1;
        return header->n_solids;
    }

    u32 getTotalEnts() const
    {
        return header->n_ents;
    }

    u32 getTotalLights() const
    {
        return header->n_lights;
    }

    core::stringw getLog() {
        return log;
    }
};

#endif
