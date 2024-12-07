#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshData.h"
#include <set>


CScnMeshData::CScnMeshData() :
	MeshBuffer(NULL)
{

}

CScnMeshData::~CScnMeshData()
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();
}

void CScnMeshData::initMesh(CScn* scn, CScnArguments* args)
{

	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();
	CScnSolid* meshes = scn->getAllSolids();
	u32 totalSize = scn->getSolidSize(args->isAllMesh());
	CScnLightmap lmap = scn->getLightmap();

	std::set<std::string> cantFind({});
	for (u32 s = 0; s < totalSize; s++) {
		CScnSolid* mesh = &meshes[s];
		video::ITexture* t = 0;
		core::array<u32> is;
		u16 alpha;
		char tmp[128];
		for (u32 i = 0; i < mesh->n_surfs; i++)
		{
			E_MATERIAL_TYPE mt = EMT_SOLID;
			MeshBuffer = new CMeshBuffer<S3DVertex2TCoords>(driver->getVertexDescriptor(EVT_2TCOORDS), EIT_16BIT);
			alpha = mesh->surfs[i].alpha;
			bool istga;
  
			sprintf_s(tmp, "./textures/%s.bmp", mesh->surfs[i].texture);
			t = 0;
			bool tmpContains = cantFind.find(tmp) != cantFind.end();
;           if (tmpContains) t = convert_image(tmp);
		
			if (tmpContains && t == 0)cantFind.insert(tmp);
			istga = false;

			//TODO: instead of using the base directory to load the textures consider loading
			//all general textures first then we don't need to load them here
 
			if (t == 0)
			{
				sprintf_s(tmp, "%S/general/%s.bmp", args->getBaseDirectory(), mesh->surfs[i].texture);
				tmpContains = cantFind.find(tmp) != cantFind.end();
				if (tmpContains) t = convert_image(tmp);
				if(tmpContains &&t == 0) cantFind.insert(tmp);
				istga = false;
			}

			if (t == 0)
			{
				sprintf_s(tmp, "./textures/%s.tga", mesh->surfs[i].texture);
				tmpContains = cantFind.find(tmp) != cantFind.end();
				if (tmpContains) t = convert_image(tmp);
				if (tmpContains && t == 0) cantFind.insert(tmp);
				
				istga = true;
			}

			if (t == 0)
			{
				sprintf_s(tmp, "%S/general/%s.tga", args->getBaseDirectory(), mesh->surfs[i].texture);
				tmpContains = cantFind.find(tmp) != cantFind.end();
				if (tmpContains) t = convert_image(tmp);
				if (tmpContains && t == 0) cantFind.insert(tmp);
				istga = true;
			}

	
			CMaterial* material{};
			if (t != 0)
			{
	
				ITexture* lt = nullptr;
				if (lmap.hasLightmaps() && args->isLightmapEnable()) {
					u16 lmapcell = lmap.getCellIndex(s, i);

					CScnEnt* cellEnt = scn->getCell(lmapcell);
					CScnEnt* ambient = ambient = scn->getGlobalAmbient();
					if (cellEnt) {
						ambient = scn->getAmbientByCell(cellEnt->getField("TargetName"));
					}
					std::string color = "0.01 0.01 0.01";
					if (ambient != NULL) color = ambient->getField("color");

					char* next_token = nullptr;
					
					IImage* image = lmap.genLightMapTextures(s,i,color.c_str());
					if (image) {
						lt= driver->addTexture(("lmt" + to_string(s) + "-" + to_string(i)).c_str(), image);
						image->drop();
					}
				}

				bool isTransparent = false;
				if (istga)
				{
					if (alpha == 255) {
						isTransparent = true;
					}
					else
						SAY("Texture of surface[%d] is tga and alpha=%i", i, alpha);
				}
				else
				{
					//solid must be used because irrlicht doesn't display transparencies
					//over transparencies properly (even if alpha=255, ie, fully opaque)
					if (alpha == 255) {
						isTransparent = false;
					}
					else
						isTransparent = true;
				}
				
				if (isTransparent) {
					material = new CMaterial(s+ "-" + i, "BuiltIn/Shader/Basic/TextureColorAlpha.xml");
				}
				else {
					material = new CMaterial(s+ "-" + i, "BuiltIn/Shader/Basic/TextureColor2Layer.xml");
				}

				material->setTexture(0, t);
				if (args->isLightmapEnable()) {
					material->setTexture(1, lt);
					material->setUniformTexture("uTexLightmap", lt);
				}
				
				material->setBackfaceCulling(true);
		
				//DEBUG test
				//mt = video::EMT_SOLID;

			}

			IIndexBuffer* ibuff = MeshBuffer->getIndexBuffer();
			IVertexBuffer* vbuff = MeshBuffer->getVertexBuffer();
			vbuff->set_used(0);
			ibuff->set_used(0);

			mesh->calcSurfVertices(i, lmap.getMults(s, i), vbuff, args->getAlpha());
			mesh->calcSurfIndices(i, ibuff);
			
			RenderMesh->addMeshBuffer(MeshBuffer, s + "-" + i, material);
			
			MeshBuffer->drop();
		}
		
	}
	RenderMesh->recalculateBoundingBox();
	RenderMesh->setHardwareMappingHint(EHM_STATIC);

}

