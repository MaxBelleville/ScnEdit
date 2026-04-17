#ifndef CSCNWRITER_H_
#define CSCNWRITER_H_
#include "include/core/CScn.h"

class CScnWriter
{
private:
	static int writeHeader(std::ofstream* file, CScn* scn);
	static int writeSolids(std::ofstream* file, CScn* scn);
	static int writeEntities(std::ofstream* file, CScn* scn);
	static int writeLightmap(std::ofstream* file, CScn* scn);

public:
	static int write(std::ofstream* file, CScn* scn, bool);
	static int resize(CScn* scn);
};
#endif