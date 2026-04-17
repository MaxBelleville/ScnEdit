#ifndef CLIGHTMAPIO_H_
#define CLIGHTMAPIO_H_

#include "include/core/CLightmap.h"
#include "include/core/CSolid.h"

class CLightmapIO
{
private:
	static int writeSwitchable(std::ofstream*, CLightmap*, u32);
	static int loadSwitchable(std::ifstream*, CLightmap*, u32);
	static int loadLumps(std::ifstream*, CLightmap*, u32);
	static int writeLumps(std::ofstream*, CLightmap*, u32);
public:
	static CLightmap* load(std::ifstream*, core::array<CSolid*>, u32);
	static int write(std::ofstream*, CLightmap*, core::array<CSolid*> solids, u32);
};
#endif