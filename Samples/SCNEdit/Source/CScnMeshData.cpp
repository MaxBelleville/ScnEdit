#include "pch.h"
#include "SkylichtEngine.h"
#include "CScnMeshData.h"
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
	CMaterial* solid = new CMaterial("solid", "TestShader/TextureColor2Layer.xml");
	CMaterial* transparent = new CMaterial("transparent", "TestShader/TextureColorAlpha.xml");

	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	CScnSolid* meshes = scn->getAllSolids();
	u32 totalSize = scn->getSolidSize(args->isAllMesh());
	CScnLightmap lmap = scn->getLightmap();
	IVideoDriver* driver = getVideoDriver();
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
            bool tmpContains = !cantFind.contains(tmp);
;           if (tmpContains) t = getVideoDriver()->getTexture(tmp);
        
            if (tmpContains && t == 0)cantFind.insert(tmp);
            istga = false;

            //TODO: instead of using the base directory to load the textures consider loading
            //all general textures first then we don't need to load them here
 
            if (t == 0)
            {
                sprintf_s(tmp, "%S/general/%s.bmp", args->getBaseDirectory(), mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t = getVideoDriver()->getTexture(tmp);
                if(tmpContains &&t == 0) cantFind.insert(tmp);
                istga = false;
            }

            if (t == 0)
            {
                sprintf_s(tmp, "./textures/%s.tga", mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t = getVideoDriver()->getTexture(tmp);
                if (tmpContains && t == 0) cantFind.insert(tmp);
                
                istga = true;
            }

            if (t == 0)
            {
                sprintf_s(tmp, "%S/general/%s.tga", args->getBaseDirectory(), mesh->surfs[i].texture);
                tmpContains = !cantFind.contains(tmp);
                if (tmpContains) t =getVideoDriver()->getTexture(tmp);
                if (tmpContains && t == 0) cantFind.insert(tmp);
                istga = true;
            }

	
			CMaterial* material{};
			if (t != 0)
			{

				ITexture* lt = nullptr;
				if (lmap.hasLightmaps()) {
					u16 lmapcell = lmap.getCellIndex(s, i);

					CScnEnt* cellEnt = scn->getCell(lmapcell);
					CScnEnt* ambient = ambient = scn->getGlobalAmbient();
					if (cellEnt) {
						ambient = scn->getAmbientByCell(cellEnt->getField("TargetName"));
					}
					std::string color = "0.01 0.01 0.01";
					if (ambient != NULL) color = ambient->getField("color");

					char* next_token = nullptr;

					 lt = lmap.genLightMapTextures(driver, s, i, color.c_str());

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
					material = new CMaterial(s+ "Surf" + i, "TestShader/TextureColorAlpha.xml");
				}
				else {
					material = new CMaterial(s+ "Surf" + i, "TestShader/TextureColor2Layer.xml");
				}

				material->setTexture(0, t);
				material->setTexture(1, lt);
				material->setUniformTexture("uTexLightmap", lt);
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

			RenderMesh->addMeshBuffer(MeshBuffer, s + "Surf" + i, material);
			MeshBuffer->drop();
		}
	}
	RenderMesh->recalculateBoundingBox();

}