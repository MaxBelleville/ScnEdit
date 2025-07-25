#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnEntityData.h"



CScnEntityData::CScnEntityData() :
	MeshBuffer(NULL){}

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
	char path[128] = { "" },prevPath[128] = { "" };

	strcpy_s(path, className);
	strcat_s(path, + ".png");
	if (strcmp(path, prevPath) != 0) {//Bit of optomization so not reloading textures
		texture = (getIrrlichtDevice()->getFileSystem()->existFile(path)) ? driver->getTexture(path) : otexture;
		strcpy_s(prevPath, path);
	}

	if (str_equiv(className, "waypointnode")) 
		scale = core::vector3df(5.0f, 5.f, 5.f);
	else if (str_equiv(className, "light_spot"))
		scale = core::vector3df(2.6f, 5.f, 2.6f);
	else if (str_equiv(className, "light_ambient")) 
		scale = core::vector3df(5.0f, 5.f, 5.f);

	core::vector3df start = convert_vec3(originStr);
	if (!m_firstLoad) 
		m_origin = originStr;

	core::vector3df end = start + scale;
	start -= scale / 2;
	CMaterial* material = new CMaterial(originStr, m_firstLoad? "TextureColor.xml":"TextureColorAlpha.xml");
	material->setBackfaceCulling(true);
	material->setTexture(0, texture);
	// Create vertices
	video::SColor clr(255, 255, 255, 255);
	MeshBuffer = generate_cube_mesh_buff(start, end, clr);
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
	RenderMesh->Materials[0]->changeShader("TextureColor.xml");
}

void CScnEntityData::deselect() {
;	if (!str_equals("TextureColor.xml", RenderMesh->Materials[0]->getShaderPath())) 
		RenderMesh->Materials[0]->changeShader("TextureColorAlpha.xml");

}
