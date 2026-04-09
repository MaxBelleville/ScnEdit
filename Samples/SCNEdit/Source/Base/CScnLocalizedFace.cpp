#include "pch.h"
#include "Header/Base/CScnLocalizedFace.h"

using namespace std;
using namespace irr;
using namespace video;

CScnLocalizedFace::CScnLocalizedFace() {
	si = 0;
	hlmap = 0;
	flipX = false;
	flipY = false;
}

CScnLocalizedFace::~CScnLocalizedFace() {
	//Might be able to get away with not doing this and just using clear, but dangling pointers are scary
	for (u32 i = 0; i < verts.size(); i++) {
		for (u32 j = 0; j < verts[i].shared.size(); j++) {
			localizedVertex_t* peer = verts[i].shared[i];
			if (peer) {
				s32 idx = peer->shared.linear_search(&verts[i]);
				if (idx != -1) {
					peer->shared.erase(idx);
				}
			}
		}
	}

	verts.clear();
	shared.clear();
}




void CScnLocalizedFace::calcVertices(u32 alpha, u16 hasVertexColors, IVertexBuffer* vbuff) {
	S3DVertex2TCoords tVert;
	for (u32 i = 0; i < verts.size(); i++) {
		tVert = getS3DVert(i);

		tVert.TCoords = verts[i].uv;
		tVert.TCoords2 = verts[i].uv;

		if (hlmap->uv_mults) {
			tVert.TCoords2.X = tVert.TCoords2.X * hlmap->uv_mults[0] + hlmap->uv_mults[2];
			tVert.TCoords2.Y = tVert.TCoords2.Y * hlmap->uv_mults[1] + hlmap->uv_mults[3];
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

core::vector2df CScnLocalizedFace::projectVertToUv(u16 localidx,scnProjectionBasis_t proj,u16 width, u16 height) {
	core::vector3df rel = verts[localidx].pos - proj.origin;
	float u_lenSq = proj.u_axis.getLengthSQ();
	float v_lenSq = proj.v_axis.getLengthSQ();

	float u = (u_lenSq > 0) ? (rel.dotProduct(proj.u_axis) / (u_lenSq * width)) : 0;
	float v = (v_lenSq > 0) ? (rel.dotProduct(proj.v_axis) / (v_lenSq * height)) : 0;
	return core::vector2df(u,v);
}

core::vector3df CScnLocalizedFace::getVertCenter() {
	core::vector3df center(0, 0, 0);
	for (u16 j = 0; j < verts.size(); j++) {
		center += verts[j].pos;
	}
	int totalVerts = verts.size();
	if (totalVerts > 0) {
		center /= static_cast<f32>(totalVerts);
	}
	return center;
}

std::pair<int,float> CScnLocalizedFace::getNearestVert(core::vector3df pos) {
	int nearestIdx = -1;
	float minDistSq = FLT_MAX;
	for (int i = 0; i < verts.size(); i++) {
		core::vector3df point = verts[i].pos;
		float distanceSq = pos.getDistanceFromSQ(point); 

		if (distanceSq < minDistSq) {
			minDistSq = distanceSq;
			nearestIdx = i;
		}
	}
	return std::make_pair(nearestIdx,minDistSq);
}


bool CScnLocalizedFace::isVertShared(u16 localidx) {
	for (u32 i = 0; i < verts[localidx].shared.size(); i++) {
		localizedVertex_t* peer = verts[localidx].shared[i];
		if (peer) {
			if (shared.linear_search(peer->parent_si) == -1)
				return true;
		}
	}
	return false;
}
int CScnLocalizedFace::findVertFromGlobal(u32 faceidx) {
	for (u32 i = 0; i < verts.size(); i++) {
		if (verts[i].faceidx == faceidx) 
			return i;
	}
	return -1;
}


