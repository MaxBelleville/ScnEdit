#ifndef CSOLIDLOADER_H_
#define CSOLIDLOADER_H_

#include "include/core/CSolid.h"

class CSolidLoader
{
private:
	static int loadSurfs(std::ifstream*, CSolid*);
	static int loadNodes(std::ifstream*, CSolid*);
	static int loadPlanes(std::ifstream*, CSolid*);
	static int loadVerts(std::ifstream*, CSolid*);
	static int loadUVPos(std::ifstream*, CSolid*);
	static int loadVertIdxs(std::ifstream*, CSolid*);
	static int loadUVIdxs(std::ifstream*, CSolid*);
	static int loadProjection(std::ifstream*, CSolid*);
	static int loadCells(std::ifstream*, CSolid*);
	static int loadNames(std::ifstream* file, CSolid* solid);
	static int loadPortal(std::ifstream* file, portal_t*);
	static int loadCellData(std::ifstream* file, rawCell_t* raw, cellData_t* cell_data);

public:
	static CSolid* load(std::ifstream* file, u32 solididx, u32);
};
#endif