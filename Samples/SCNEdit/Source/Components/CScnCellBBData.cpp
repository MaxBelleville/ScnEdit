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
			scnSurf_t* surfi = &solid->surfs[vert.surfidx];

			core::vector3df minVert = core::vector3df(FLT_MAX, FLT_MAX, FLT_MAX);
			core::vector3df maxVert = -core::vector3df(FLT_MAX, FLT_MAX, FLT_MAX);
			
			const core::vector3df& bbMin = backup_bb[indx][0];
			const core::vector3df& bbMax = backup_bb[indx][1];


			bool allInside = true;
			bool allInBackup = true;

			for (int f = 0; f < surfi->faceidxlen; f++) {
				u32 vertidx = solid->vertidxs[surfi->faceidxstart + f];
				const core::vector3df& compareVert = solid->verts[vertidx];

				minVert = min(minVert, compareVert);
				maxVert = max(maxVert, compareVert);

				if (compareVert.X < min.X || compareVert.X > max.X ||
					compareVert.Y < min.Y || compareVert.Y > max.Y ||
					compareVert.Z < min.Z || compareVert.Z > max.Z) {
					allInside = false;
				}

				if (compareVert.X < bbMin.X || compareVert.X > bbMax.X ||
					compareVert.Y < bbMin.Y || compareVert.Y > bbMax.Y ||
					compareVert.Z < bbMin.Z || compareVert.Z > bbMax.Z) {
					allInBackup = false;

				}

			}

			if (!reset) {
				//In the case vert smaller then bounding box.
				
				if (allInside) {
					const float minSize = 0.1f;
					// Shrink -X
					if (min.X < bbMin.X && minVert.X > min.X && (max.X - minVert.X) >= minSize)
						min.X = minVert.X;
					// Shrink +X
					if (max.X > bbMax.X && maxVert.X < max.X && (maxVert.X - min.X) >= minSize)
						max.X = maxVert.X;

					// Shrink -Y
					if (min.Y < bbMin.Y && minVert.Y > min.Y && (max.Y - minVert.Y) >= minSize)
						min.Y = minVert.Y;
					// Shrink +Y
					if (max.Y > bbMax.Y  && maxVert.Y < max.Y && (maxVert.Y - min.Y) >= minSize)
						max.Y = maxVert.Y;

					// Shrink -Z
					if (min.Z < bbMin.Z && minVert.Z > min.Z && (max.Z - minVert.Z) >= minSize)
						min.Z = minVert.Z;
					// Shrink +Z
					if (max.Z > bbMax.Z &&maxVert.Z < max.Z && (maxVert.Z - min.Z) >= minSize)
						max.Z = maxVert.Z;
				}

				
			}
			//If reset check if contains backup_bb.
			if (reset && indx != -1 && allInBackup) {
				min = backup_bb[indx][0];
				max = backup_bb[indx][1];
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

		if(indx != -1)updateMeshBB(scn, celldata, indx);
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