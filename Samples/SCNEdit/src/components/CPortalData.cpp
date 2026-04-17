#include "pch.h"
#include "SkylichtEngine.h"
#include "include/components/CPortalData.h"
#include "include/util.h"

CPortalData::CPortalData() :
	MeshBuffer(NULL) {
}

CPortalData::~CPortalData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CPortalData::initMesh(CSolid* solid, u32 cellindx, s32 portalIndx)
{
	portaldata = portalBox_t(cellindx, portalIndx);
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();

	core::vector3df start = solid->rawcells[cellindx]->portals[portalIndx]->bb_verts[0];
	core::vector3df end = solid->rawcells[cellindx]->portals[portalIndx]->bb_verts[1];
	// Create vertices
	video::SColor clr(125, 25, 100, 25);

	MeshBuffer = generate_cube_mesh_buff(start, end, clr);
	CMaterial* material = new CMaterial("portal", "BuiltIn/Shader/Basic/VertexColorAlpha.xml");
	material->setBackfaceCulling(true);
	// add cube mesh buffer to mesh
	RenderMesh->addMeshBuffer(MeshBuffer, "portal", material);
	MeshBuffer->drop();

	// recalc bbox for culling
	RenderMesh->recalculateBoundingBox();

	// remeber set static mesh buffer to optimize (it will stored on GPU)
	RenderMesh->setHardwareMappingHint(EHM_STATIC);
}