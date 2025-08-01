#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnCellBBData.h"

CScnCellBBData::CScnCellBBData() :
	MeshBuffer(NULL){}

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
		// Create vertices
		video::SColor clr(255, 0, 0, 0);

		IMeshBuffer* MeshBuffer =generate_cube_mesh_buff(start,end,clr);

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
	for (u32 l = 0; l < solid->rawcells[cellindx].leafnode.size(); l++) {
		scnCellData_t* leaf = solid->rawcells[cellindx].leafnode[l];
		if (leaf == celldata)
			return l;
	}
	return -1;
}

void CScnCellBBData::updateBB(CScn* scn, indexedVec3df_t vert , bool reset) {
	CScnSolid* solid= scn->getSolid(vert.solididx);
	scnCellData_t* celldata;
	celldata = solid->getBBFromSurf(vert.surfidx, cellindx);
	//This math may be wrong lol
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
		
			scnSurf_t* surfi = &solid->surfs[vert.surfidx];
			for (int v = 0; v < surfi->vertidxlen; v++) {
				u32 vertidx = solid->vertidxs[surfi->vertidxstart + v];
				core::vector3df compareVert = solid->verts[vertidx];

				minVert = min(minVert, compareVert);
				maxVert =max(maxVert, compareVert);
			}
			//1. If diference is less then or equal 1.5ish and is flat, set current bb to maxVert.
			if (!reset) {
				if (maxVert.X < max.X && maxVert.X >= max.X - 1.1) 
					max.X = maxVert.X;
				
				if (maxVert.Y < max.Y && maxVert.Y >= max.Y - 1.1) 
					max.Y = maxVert.Y;

				if (maxVert.Z < max.Z && maxVert.Z >= max.Z - 1.1) 
					max.Z = maxVert.Z;

				if (minVert.X > min.X && minVert.X <= min.X + 1.1) 
					min.X = minVert.X;

				if (minVert.Y > min.Y && minVert.Y <= min.Y + 1.1) 
					min.Y = minVert.Y;

				if (minVert.Z > min.Z && minVert.Z <= min.Z + 1.1) 
					min.Z = minVert.Z;
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
			if (vert.pos.X < min.X) 
				min.X = vert.pos.X;
			if (vert.pos.Y < min.Y) 
				min.Y = vert.pos.Y;
			if (vert.pos.Z < min.Z) 
				min.Z = vert.pos.Z;

			if (vert.pos.X > max.X) 
				max.X = vert.pos.X;
			if (vert.pos.Y > max.Y) 
				max.Y = vert.pos.Y;
			if (vert.pos.Z > max.Z) 
				max.Z = vert.pos.Z;
		}

		celldata->bb_verts[0] = min;
		celldata->bb_verts[1] = max;
		updateMeshBB(scn, celldata, indx);
	}

}
void CScnCellBBData::updateMeshBB(CScn* scn, scnCellData_t* celldata, int leafindx) {
	core::vector3df start = celldata->bb_verts[0]; // Removes z-indexing
	core::vector3df end = celldata->bb_verts[1]; // at cost of accuracy
	video::SColor clr(255, 0, 0, 0);

	IMeshBuffer* MeshBuffer = generate_cube_mesh_buff(start, end, clr);

	MeshBuffer->getMaterial().Wireframe = true;
	RenderMesh->replaceMeshBuffer(leafindx, MeshBuffer);
	MeshBuffer->drop();

	// Update bounding box of full mesh
	RenderMesh->recalculateBoundingBox();
}