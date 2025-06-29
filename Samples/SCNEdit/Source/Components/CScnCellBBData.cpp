#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnCellBBData.h"

CScnCellBBData::CScnCellBBData() :
	MeshBuffer(NULL)
{

}

CScnCellBBData::~CScnCellBBData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CScnCellBBData::initMesh(CScnSolid* solid, u32 currCellindx)
{
	
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();
	cellindx = currCellindx;

	for (u32 l = 0; l < solid->rawcells[cellindx].leafnode.size(); l++) {//TODO Abstract
		scnCellData_t* leaf = solid->rawcells[cellindx].leafnode[l];
		core::vector3df start = leaf->bb_verts[0]; // Removes z-indexing
		core::vector3df end= leaf->bb_verts[1]; // at cost of accuracy
		backup_bb.push_back(new core::vector3df[2]{ start,end });
		MeshBuffer = new CMeshBuffer<S3DVertex>(driver->getVertexDescriptor(EVT_STANDARD), EIT_16BIT);
		IIndexBuffer* ib = MeshBuffer->getIndexBuffer();
		IVertexBuffer* vb = MeshBuffer->getVertexBuffer();
		// Create vertices
		video::SColor clr(255, 0, 0, 0);

		vb->reallocate(12);

		video::S3DVertex Vertices[] = {
			// back
			video::S3DVertex(start.X, start.Y, start.Z, 0, 0, -1, clr, 0, 1),
			video::S3DVertex(end.X, start.Y, start.Z, 0, 0, -1, clr, 1, 1),
			video::S3DVertex(end.X, end.Y, start.Z, 0, 0, -1, clr, 1, 0),
			video::S3DVertex(start.X, end.Y, start.Z, 0, 0, -1, clr, 0, 0),

			// front
			video::S3DVertex(start.X, start.Y, end.Z, 0, 0, 1, clr, 1, 1),
			video::S3DVertex(end.X, start.Y, end.Z, 0, 0, 1, clr, 0, 1),
			video::S3DVertex(end.X, end.Y, end.Z, 0, 0, 1, clr, 0, 0),
			video::S3DVertex(start.X, end.Y, end.Z, 0, 0, 1, clr, 1, 0),

			// bottom
			video::S3DVertex(start.X, start.Y, start.Z, 0, -1, 0, clr, 0, 1),
			video::S3DVertex(start.X, start.Y, end.Z, 0, -1, 0, clr, 1, 1),
			video::S3DVertex(end.X, start.Y, end.Z, 0, -1, 0, clr, 1, 0),
			video::S3DVertex(end.X, start.Y, start.Z, 0, -1, 0, clr, 0, 0),

			// top
			video::S3DVertex(start.X, end.Y, start.Z, 0, 1, 0, clr, 0, 1),
			video::S3DVertex(start.X, end.Y, end.Z, 0, 1, 0, clr, 1, 1),
			video::S3DVertex(end.X, end.Y, end.Z, 0, 1, 0, clr, 1, 0),
			video::S3DVertex(end.X, end.Y, start.Z, 0, 1, 0, clr, 0, 0),

			// left
			video::S3DVertex(end.X, start.Y, start.Z, 1, 0, 0, clr, 0, 1),
			video::S3DVertex(end.X, start.Y, end.Z, 1, 0, 0, clr, 1, 1),
			video::S3DVertex(end.X, end.Y, end.Z, 1, 0, 0, clr, 1, 0),
			video::S3DVertex(end.X, end.Y, start.Z, 1, 0, 0, clr, 0, 0),

			// right
			video::S3DVertex(start.X, start.Y, start.Z, -1, 0, 0, clr, 1, 1),
			video::S3DVertex(start.X, start.Y, end.Z, -1, 0, 0, clr, 0, 1),
			video::S3DVertex(start.X, end.Y, end.Z, -1, 0, 0, clr, 0, 0),
			video::S3DVertex(start.X, end.Y, start.Z, -1, 0, 0, clr, 1, 0),
		};

		for (u32 i = 0; i < 24; ++i)
		{
					
			vb->addVertex(&Vertices[i]);
		}

		// cube mesh
		// Create indices
		const u16 u[36] =
		{
			// back
			0,2,1,
			0,3,2,

			// front
			4,5,6,
			4,6,7,

			// bottom
			8,10,9,
			8,11,10,

			// top
			12,13,14,
			12,14,15,

			// left
			16,18,17,
			16,19,18,

			// right
			20,21,22,
			20,22,23
		};

		ib->set_used(36);

		for (u32 i = 0; i < 36; ++i)
			ib->setIndex(i, u[i]);

		// recalc bbox
		MeshBuffer->recalculateBoundingBox();
		MeshBuffer->getMaterial().Wireframe = true;
		// add cube mesh buffer to mesh
		RenderMesh->addMeshBuffer(MeshBuffer, "cellbb", nullptr);
				
		MeshBuffer->drop();

	}

	// recalc bbox for culling
	RenderMesh->recalculateBoundingBox();

	// remeber set static mesh buffer to optimize (it will stored on GPU)
	RenderMesh->setHardwareMappingHint(EHM_STATIC);

}

int CScnCellBBData::getIndexFromCellBB(CScnSolid* solid, scnCellData_t* celldata) {
	for (u32 l = 0; l < solid->rawcells[cellindx].leafnode.size(); l++) {//TODO Abstract
		scnCellData_t* leaf = solid->rawcells[cellindx].leafnode[l];
		if (leaf == celldata) {
			return l;
		}
	}
	return -1;
}

void CScnCellBBData::updateBB(CScn* scn, indexedVec3df_t vert , bool reset) {
	CScnSolid* solid= scn->getSolid(vert.solidindx);
	scnCellData_t* celldata;
	celldata = solid->getBBFromSurf(vert.surfindx, cellindx);
	
	if (celldata) {
		core::vector3df min = celldata->bb_verts[0];
		core::vector3df max = celldata->bb_verts[1];
		int indx = getIndexFromCellBB(solid, celldata);
		if (vert.pos.X >= min.X && vert.pos.X <= max.X &&
			vert.pos.Y >= min.Y && vert.pos.Y <= max.Y &&
			vert.pos.Z >= min.Z && vert.pos.Z <= max.Z) {
			//In the case vert smaller then bounding box.
			core::vector3df minVert = core::vector3df(FLT_MAX, FLT_MAX, FLT_MAX);
			core::vector3df maxVert = core::vector3df(FLT_MIN, FLT_MIN, FLT_MIN);
			bool flatX = true;
			bool flatY = true;
			bool flatZ = true;
		
			scnSurf_t* surfi = &solid->surfs[vert.surfindx];
			for (int v = 0; v < surfi->vertidxlen; v++) {
				u32 vertidx = solid->vertidxs[surfi->vertidxstart + v];
				core::vector3df compareVert = solid->verts[vertidx];
				minVert.X = min(minVert.X, compareVert.X);
				minVert.Y = min(minVert.Y, compareVert.Y);
				minVert.Z = min(minVert.Z, compareVert.Z);

				maxVert.X = max(maxVert.X, compareVert.X);
				maxVert.Y = max(maxVert.Y, compareVert.Y);
				maxVert.Z = max(maxVert.Z, compareVert.Z);
			}
			//1. If diference is 1.5ish and is flat.
			if (!reset) {
				if (maxVert.X < max.X && maxVert.X >= max.X - 1.1) {
					max.X = maxVert.X;
				}
				if (maxVert.Y < max.Y && maxVert.Y >= max.Y - 1.1) max.Y = maxVert.Y;
				if (maxVert.Z < max.Z && maxVert.Z >= max.Z - 1.1) max.Z = maxVert.Z;

				if (minVert.X > min.X && minVert.X <= min.X + 1.1) min.X = minVert.X;
				if (minVert.Y > min.Y && minVert.Y <= min.Y + 1.1) min.Y = minVert.Y;
				if (minVert.Z > min.Z && minVert.Z <= min.Z + 1.1) min.Z = minVert.Z;
			}
			//2. If reset check if contains backup_bb.
			if (reset && indx != -1) {
				if (minVert.X >= backup_bb[indx][0].X && minVert.X <= backup_bb[indx][1].X &&
					minVert.Y >= backup_bb[indx][0].Y && minVert.Y <= backup_bb[indx][1].Y &&
					minVert.Z >= backup_bb[indx][0].Z && minVert.Z <= backup_bb[indx][1].Z) {
					min = backup_bb[indx][0];
				}
				
				if (maxVert.X >= backup_bb[indx][0].X && maxVert.X <= backup_bb[indx][1].X &&
					maxVert.Y >= backup_bb[indx][0].Y && maxVert.Y <= backup_bb[indx][1].Y &&
					maxVert.Z >= backup_bb[indx][0].Z && maxVert.Z <= backup_bb[indx][1].Z) {
					max = backup_bb[indx][1];
				}
			}
		
		}
		else {
			//In the cases of vert being beyond bounding box.
			if (vert.pos.X < min.X) min.X = vert.pos.X;
			if (vert.pos.Y < min.Y) min.Y = vert.pos.Y;
			if (vert.pos.Z < min.Z) min.Z = vert.pos.Z;

			if (vert.pos.X > max.X) max.X = vert.pos.X;
			if (vert.pos.Y > max.Y) max.Y = vert.pos.Y;
			if (vert.pos.Z > max.Z) max.Z = vert.pos.Z;
		}
		celldata->bb_verts[0] = min;
		celldata->bb_verts[1] = max;
	}
	//TODO Visually update the bb
}
