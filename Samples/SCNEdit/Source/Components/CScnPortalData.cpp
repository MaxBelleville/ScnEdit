#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnPortalData.h"


CScnPortalData::CScnPortalData() :
	MeshBuffer(NULL){}

CScnPortalData::~CScnPortalData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CScnPortalData::initMesh(CScnSolid* solid, u32 cellindx, s32 portalIndx)
{
	portaldata = portalSelect_t(cellindx, portalIndx);
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();

	core::vector3df start = solid->rawcells[cellindx].portals[portalIndx].bb_verts[0];
	core::vector3df end= solid->rawcells[cellindx].portals[portalIndx].bb_verts[1]; 
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

void CScnPortalData::select() {

	RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColor.xml");

}

void CScnPortalData::deselect() {
	
	if (!str_equals("BuiltIn/Shader/Basic/VertexColorAlpha.xml", RenderMesh->Materials[0]->getShaderPath())) {
		RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColorAlpha.xml");
	}

}