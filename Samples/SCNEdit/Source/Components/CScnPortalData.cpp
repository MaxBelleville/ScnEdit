#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnPortalData.h"


CScnPortalData::CScnPortalData() :
	MeshBuffer(NULL)
{

}

CScnPortalData::~CScnPortalData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CScnPortalData::initMesh(CScnSolid* solid, u32 cellindx, s32 portalIndx)
{
	portaldata = make_pair(cellindx, portalIndx);
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();

	
	core::vector3df start = solid->rawcells[cellindx].portals[portalIndx].bb_verts[0]+0.2; // Removes z-indexing
	core::vector3df end= (solid->rawcells[cellindx].portals[portalIndx].bb_verts[1])-0.2; // at cost of accuracy

	MeshBuffer = new CMeshBuffer<S3DVertex>(driver->getVertexDescriptor(EVT_STANDARD), EIT_16BIT);
	IIndexBuffer* ib = MeshBuffer->getIndexBuffer();
	IVertexBuffer* vb = MeshBuffer->getVertexBuffer();
	// Create vertices
	video::SColor clr(75, 25, 100, 25);

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
		video::S3DVertex(start.X, end.Y, start.Z, 0, 1, 0, clr, 1, 1),
		video::S3DVertex(start.X, end.Y, end.Z, 0, 1, 0, clr, 0, 1),
		video::S3DVertex(end.X, end.Y, end.Z, 0, 1, 0, clr, 0, 0),
		video::S3DVertex(end.X, end.Y, start.Z, 0, 1, 0, clr, 1, 0),

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