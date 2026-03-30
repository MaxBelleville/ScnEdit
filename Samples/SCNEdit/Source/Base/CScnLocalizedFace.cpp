#include "pch.h"
#include "Header/Base/CScnLocalizedFace.h"

using namespace std;
using namespace irr;
using namespace video;

CScnLocalizedFace::~CScnLocalizedFace() {
	for (u32 i = 0; i < verts.size(); i++) {
		for (u32 j = 0; j < verts[i].shared.size(); j++) {
			if (verts[i].shared[j]) {
				delete verts[i].shared[j];
			}
		}
		verts[i].shared.clear();
	}
	verts.clear();
	shared_si.clear();
}

void CScnLocalizedFace::moveVert(u16 localidx, core::vector3df newPos) {
	verts[localidx].pos = newPos;
	for (u32 i = 0; i < verts[localidx].shared.size(); ++i) {
		verts[localidx].shared[i]->pos = newPos;
	}
}

void CScnLocalizedFace::calcVertices(u32 alpha, u16 hasVertexColors, IVertexBuffer* vbuff) {
	S3DVertex2TCoords tVert;
	for (u32 i = 0; i < verts.size(); i++) {
		tVert = getVertice(i);

		tVert.TCoords = verts[i].uv;
		tVert.TCoords2 = verts[i].uv;

		if (lmap.uv_mults) {
			tVert.TCoords2.X = tVert.TCoords2.X * lmap.uv_mults[0] + lmap.uv_mults[2];
			tVert.TCoords2.Y = tVert.TCoords2.Y * lmap.uv_mults[1] + lmap.uv_mults[3];
		}
		tVert.Color.setAlpha(alpha);
		if (hasVertexColors) {
			tVert.Color = video::SColor(verts[i].color[0], verts[i].color[1], verts[i].color[2], verts[i].color[3]);
		}
		vbuff->addVertex(&tVert);
	}
}

void CScnLocalizedFace::calcIndices(IIndexBuffer* ibuff) {
	ibuff->addIndex(0);
	ibuff->addIndex(1);
	ibuff->addIndex(2);

	for (u16 j = 3; j < verts.size(); j++) {
		ibuff->addIndex(j - 1);
		ibuff->addIndex(j);
		ibuff->addIndex(0);
	}
}

void CScnLocalizedFace::projectVertToUv(u16 localidx, u16 width, u16 height) {
	core::vector3df rel = verts[localidx].pos - projection.origin;
	float u_lenSq = projection.u_axis.getLengthSQ();
	float v_lenSq = projection.v_axis.getLengthSQ();

	float u = (u_lenSq > 0) ? (rel.dotProduct(projection.u_axis) / (u_lenSq * width)) : 0;
	float v = (v_lenSq > 0) ? (rel.dotProduct(projection.v_axis) / (v_lenSq * height)) : 0;

}



