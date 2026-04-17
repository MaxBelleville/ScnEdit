#ifndef CSCNLOADER_H_
#define CSCNLOADER_H_

#include "include/core/CScn.h"

class CScnLoader
{
private:
	static int loadHeader(std::ifstream* file, CScn* scn);
	static int loadSolids(std::ifstream* file, CScn* scn);
	static int loadEntities(std::ifstream* file, CScn* scn);
	static int loadLightmap(std::ifstream* file, CScn* scn);
public:
	static CScn* load(std::ifstream* file);
};
#endif