#ifndef CSOLIDWRITER_H_
#define CSOLIDWRITER_H_
#include "include/core/CSolid.h"

class CSolidWriter
{
private:
	static int writeSurfs(std::ofstream*, CSolid*);
	static int writeNodes(std::ofstream*, CSolid*);
	static int writePlanes(std::ofstream*, CSolid*);
	static int writeVerts(std::ofstream*, CSolid*);
	static int writeUVPos(std::ofstream*, CSolid*);
	static int writeVertIdxs(std::ofstream*, CSolid*);
	static int writeUVIdxs(std::ofstream*, CSolid*);
	static int writeProjection(std::ofstream*, CSolid*);
	static int writeCells(std::ofstream*, CSolid*);
	static int writeNames(std::ofstream* file, CSolid* solid);
	static int writePortal(std::ofstream* file, portal_t*);
	static int writeCellData(std::ofstream* file, rawCell_t* raw, cellData_t* cell_data);

	static int calcSizeFace(CSolid* solid);//Surf nodes planes vert uvs vertidx uvidx
	static int calcSizeCells(CSolid* solid); //Cells and portals
	static int calcSizeCellData(cellData_t* cell_data);

public:
	static int write(std::ofstream* file, CSolid*);
	static int resize(CSolid* solid);
};
#endif