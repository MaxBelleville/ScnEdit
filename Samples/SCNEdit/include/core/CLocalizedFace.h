#ifndef CLOCALIZEDFACE_H_
#define CLOCALIZEDFACE_H_
#include "include/core/scntypes.h"

class CLocalizedFace
{
public:
	u32 si;
	core::array<localVert_t> verts;
	lMapHeader_t* hlmap;

	core::array<u32> shared;
	CLocalizedFace();
	~CLocalizedFace();

	void calcVertices(u32 alpha, u16 hasVertexColors, IVertexBuffer* vbuff);
	void calcIndices(IIndexBuffer* ibuff);
	core::vector2df projectVertToUv(u16 localidx, projBasis_t* proj, u16 width, u16 height);
	bool isVertShared(u16 localidx);

	core::vector3df getVertCenter();
	std::pair<int, float> getNearestVert(core::vector3df pos);

	int findVertFromGlobal(u32 faceidx);
	inline video::S3DVertex2TCoords getS3DVert(u32 localidx)
	{
		localVert_t local_vert = verts[localidx];
		video::S3DVertex2TCoords vert = video::S3DVertex2TCoords(
			local_vert.pos.X, local_vert.pos.Y, local_vert.pos.Z,
			1, 1, 1, video::SColor(255, 255, 255, 255), 0, 1, 0, 1);
		return vert;
	};
};

#endif
