#ifndef CSCNLOCALIZEDFACE_H_
#define CSCNLOCALIZEDFACE_H_
#include "scntypes.h"
#include "util.h"

class CScnLocalizedFace
{
public:
	u16 si;
	scnPlane_t plane;
	scnProjectionBasis_t projection;
	core::array<localizedVertex_t> verts;
	scnLMapHeader_t lmap;

	core::array<u16> shared_si;

	~CScnLocalizedFace();

	void moveVert(u16 localidx, core::vector3df newPos);
	void buildVerts();


	void calcVertices(u32 alpha, u16 hasVertexColors, IVertexBuffer* vbuff);
	void calcIndices(IIndexBuffer* ibuff);
	void projectVertToUv(u16 localidx, u16 width, u16 height);

	inline video::S3DVertex2TCoords getVertice(u32 localidx)
	{
		localizedVertex_t local_vert = verts[localidx];
		video::S3DVertex2TCoords vert = video::S3DVertex2TCoords(
			local_vert.pos.X, local_vert.pos.Y, local_vert.pos.Z,      
			1, 1, 1, video::SColor(255, 255, 255, 255), 0, 1, 0, 1);
		return vert;
	}

		inline core::array<localizedVertex_t*> getSharedVerts(u16 localidx) {
		verts[localidx].shared
	};
};

#endif
