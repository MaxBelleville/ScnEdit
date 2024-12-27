#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnEntityData.h"



CScnEntityData::CScnEntityData() :
	MeshBuffer(NULL)
{

}

CScnEntityData::~CScnEntityData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CScnEntityData::initMesh(CScnEnt* ent)
{
	m_indx = ent->indx;
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();

	ITexture* otexture = driver->getTexture("entity.png");
	ITexture* texture = otexture;

	const char* originStr = ent->getField("origin");
	const char* className = ent->getField("classname");
	core::vector3df scale = core::vector3df(3.f, 3.f, 3.f);
	char path[128] = { "" };
	char prevPath[128] = { "" };

		strcpy_s(path, className);
		strcat_s(path, +".png");
		if (strcmp(path, prevPath) != 0) {//Bit of optomization so not reloading textures
			if (getIrrlichtDevice()->getFileSystem()->existFile(path))
				texture = driver->getTexture(path);
			else texture = otexture;
			strcpy_s(prevPath, path);
		}

		if (str_equiv(className, "waypointnode")) {
			scale = core::vector3df(5.0f, 5.f, 5.f);
		}
		else if (str_equiv(className, "light_spot"))
		{
			scale = core::vector3df(2.6f, 5.f, 2.6f);
		}
		else if (str_equiv(className, "light_ambient")) {
			scale = core::vector3df(5.0f, 5.f, 5.f);
		}
		core::vector3df og_start = convert_vec3(originStr);
		if (!m_firstLoad) {
			m_origin = originStr;
		}
		core::vector3df start = og_start;
		core::vector3df end = start + scale;
		start -= scale / 2;
		CMaterial* material = new CMaterial(originStr, m_firstLoad? "TextureColor2Layer.xml":"TextureColor2LayerAlpha.xml");
		material->setBackfaceCulling(true);


		
		material->setTexture(0, texture);
		material->setUniform("uLightMapEnabled", 0.0f);
		material->setUniform("uSelected", 0.0f);
		material->setUniform4("uSelectedColor", SColor(0, 0, 0, 0));
		material->updateShaderParams();
		
		MeshBuffer = new CMeshBuffer<S3DVertex>(driver->getVertexDescriptor(EVT_STANDARD), EIT_16BIT);
		IIndexBuffer* ib = MeshBuffer->getIndexBuffer();
		IVertexBuffer* vb = MeshBuffer->getVertexBuffer();
		// Create vertices
		video::SColor clr(255, 255, 255, 255);

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
		// add cube mesh buffer to mesh
		RenderMesh->addMeshBuffer(MeshBuffer, originStr, material);

		MeshBuffer->drop();

		// recalc bbox for culling
		RenderMesh->recalculateBoundingBox();

		// remeber set static mesh buffer to optimize (it will stored on GPU)
		RenderMesh->setHardwareMappingHint(EHM_STATIC);
		m_firstLoad = true;
}


void CScnEntityData::select() {

	
	RenderMesh->Materials[0]->changeShader("TextureColor2Layer.xml");
	RenderMesh->Materials[0]->setUniform("uLightMapEnabled", 0.0f);
	RenderMesh->Materials[0]->setUniform("uSelected", 0.0f);
	RenderMesh->Materials[0]->setUniform4("uSelectedColor", SColor(0, 0, 0, 0));
	RenderMesh->Materials[0]->updateShaderParams();


}

void CScnEntityData::deselect() {
;	if (!str_equals("TextureColor2LayerAlpha.xml", RenderMesh->Materials[0]->getShaderPath())) {
		RenderMesh->Materials[0]->changeShader("TextureColor2LayerAlpha.xml");

	}

}
