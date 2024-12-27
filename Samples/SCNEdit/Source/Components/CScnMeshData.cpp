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

void CScnMeshData::initMesh(CScn* scn,CScnSolid* mesh, CScnArguments* args)
{

	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();
	CScnLightmap* lmap = scn->getLightmap();
	
	solidindx = mesh->solididx;
	std::set<std::string> cantFind({});

		video::ITexture* t = 0;
		ITexture* lt{};
		core::array<u32> is;
		s32 currAtlas = -1;
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
			bool tmpContains =!cantFind.contains(tmp);
;           if (tmpContains) t = convert_image(tmp);
		
			if (tmpContains && t == 0)cantFind.insert(tmp);
			istga = false;

			//TODO: instead of using the base directory to load the textures consider loading
			//all general textures first then we don't need to load them here
 
			if (t == 0)
			{
				sprintf_s(tmp, "%S/general/%s.bmp", args->getBaseDirectory(), mesh->surfs[i].texture);
				tmpContains = !cantFind.contains(tmp);
				if (tmpContains) t = convert_image(tmp);
				if(tmpContains &&t == 0) cantFind.insert(tmp);
				istga = false;
			}

			if (t == 0)
			{
				sprintf_s(tmp, "./textures/%s.tga", mesh->surfs[i].texture);
				tmpContains = !cantFind.contains(tmp);
				if (tmpContains) t = convert_image(tmp);
				if (tmpContains && t == 0) cantFind.insert(tmp);
				
				istga = true;
			}

			if (t == 0)
			{
				sprintf_s(tmp, "%S/general/%s.tga", args->getBaseDirectory(), mesh->surfs[i].texture);
				tmpContains = !cantFind.contains(tmp);
				if (tmpContains) t = convert_image(tmp);
				if (tmpContains && t == 0) cantFind.insert(tmp);
				istga = true;
			}

			CMaterial* material = new CMaterial((to_string(mesh->solididx) + "-" + to_string(i)).c_str(), "TextureColor2Layer.xml");
			if (t != 0)
			{
				if (lmap->hasLightmaps() && args && args->isLightmapEnable()) {
					//TOOD: change color according to ambient ent like previous.
					core::vector3di atpos = lmap->getAtlasPos(mesh->solididx, i);
					if (atpos.Z != currAtlas) {
						currAtlas = atpos.Z;
						lt = getVideoDriver()->addTexture(("lmAtlas" + to_string(atpos.Z)).c_str(), lmap->getAtlas(atpos.Z));
					}
				}
				bool isTransparent = false;
				if (istga)
				{
					if (alpha == 255) {
						isTransparent = true;
					}
					else
						os::Printer::log(format("Texture of surface[{}] is tga and alpha={}", i, alpha).c_str());
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
					material = new CMaterial((to_string(mesh->solididx) + "-" + to_string(i)).c_str(), "TextureColor2LayerAlpha.xml");
				}
				

				material->setTexture(0, t);

				if (lmap->hasLightmaps() && args && args->isLightmapEnable()) {
					material->setTexture(1, lt);
					material->setUniform("uLightMapEnable", 1.0f);
				}
				else {
					material->setUniform("uLightMapEnable", 0.0f);
				}
				material->setBackfaceCulling(true);
				material->setUniform("uSelected", 0.0f);
				material->setUniform4("uSelectedColor", SColor(0, 0, 0, 0));
				material->updateShaderParams();

			}

			IIndexBuffer* ibuff = MeshBuffer->getIndexBuffer();
			IVertexBuffer* vbuff = MeshBuffer->getVertexBuffer();
			vbuff->set_used(0);
			ibuff->set_used(0);
			mesh->calcSurfVertices(i, lmap->getMults(mesh->solididx, i), vbuff, args->getAlpha());
			mesh->calcSurfIndices(i, ibuff);
			RenderMesh->addMeshBuffer(MeshBuffer, (to_string(mesh->solididx) + "-" + to_string(i)).c_str(), material);
			BackupData backupdata = {core::array<S3DVertex2TCoords>(),core::array<u32>()};
			
			S3DVertex2TCoords * vertices= static_cast<video::S3DVertex2TCoords*>(vbuff->getVertices());
			for (int v = 0; v < vbuff->getVertexCount(); v++) {
				backupdata.vertices.push_back(vertices[v]);
			}
			// Backup indices
			for (int i = 0; i < ibuff->getIndexCount(); i++) {
				backupdata.indices.push_back(ibuff->getIndex(i));
			}
			vis_backup.push_back(backupdata);
			vert_backup.push_back(backupdata);
			MeshBuffer->drop();
		}
	RenderMesh->recalculateBoundingBox();
	RenderMesh->setHardwareMappingHint(EHM_STATIC);

}

void CScnMeshData::setLightmapVisible(bool vis) {
	if ((RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 0.0 && vis)||
		RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 1.0 && !vis) {
		for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
			if (!vis) RenderMesh->Materials[i]->setUniform("uLightMapEnable", 0.0f);
			else RenderMesh->Materials[i]->setUniform("uLightMapEnable", 1.0f);
			RenderMesh->Materials[i]->updateShaderParams();
		}
	}
}
std::pair<int, int> CScnMeshData::getSurfaceIndx(CScn* scn, core::triangle3df tri) {
	core::vector3df pa = tri.pointA;
	core::vector3df pb = tri.pointB;
	core::vector3df pc = tri.pointC;
	int selsurf = -1;
	CScnSolid* solid = scn->getSolid(solidindx);
	for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
		scnSurf_t* surfi = &solid->surfs[i];
		bool foundpa = 0;
		bool foundpb = 0;
		bool foundpc = 0;

		for (int k = 0; k < surfi->vertidxlen; k++) {
			//Gets vertex index
			core::vector3df* verti = &solid->verts[solid->vertidxs[surfi->vertidxstart + k]];
			//Gets vector index
			core::vector3df vectori(verti->X, verti->Y, verti->Z);
			if (!foundpa) {
				if (vectori.equals(pa, 1))
					foundpa = 1;

			}
			if (!foundpb) {
				if (vectori.equals(pb, 1))
					foundpb = 1;
			}

			if (!foundpc) {
				if (vectori.equals(pc, 1))
					foundpc = 1;
			}
			if (foundpa && foundpb && foundpc) //if found all vertices
			{
				selsurf = i;
				break;
			}
		}
		if (selsurf > -1)
			break;

	}
	return make_pair(solidindx, selsurf);
}
core::array<int> CScnMeshData::getSharedSurface(CScn* scn, core::array<int>sindices) {
	core::array<int> final;
	for (int i = 0; i < sindices.size(); i++) {
		core::array<int> shared=getSharedSurface(scn,sindices[i]);
		for (int j = 0; j < shared.size(); j++) {
			if (final.linear_search(shared[j]) == -1 && sindices.linear_search(shared[j]) == -1)
				final.push_back(shared[j]);
		}
	}
	return final;
}


core::array<vertProp_t> CScnMeshData::getSurfVertProps(CScn* scn, int si) {
	CScnSolid* solid = scn->getSolid(solidindx);
	scnSurf_t* surfi = &solid->surfs[si];

	core::array<u32>* c_uvidxs;
	core::array<u32>* c_surfs;
	core::array<vertProp_t> vprops;

	//run through vertices, to see how many are shared
	bool bShared;

	for (u32 j = 0; j < surfi->vertidxlen; j++)
	{
		//start new index
		vertProp_t vp;
		
		vp.bShared = false; //assume vertex is not shared
		vp.surf_vertidx = j;    //redundant
		vp.vertidxidx = surfi->vertidxstart + j; //also redundant;

		vp.sharesWith.set_used(0);

		u32 uvidx = solid->uvidxs[surfi->vertidxstart + j]; //uvidx of this vertex


		c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

		for (u32 k = 0; k < c_uvidxs->size(); k++)              //run through all of these uvidxs
		{
			c_surfs = &solid->uvidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx

			//i don't think it ever happened where the same uvidx is called by more than one surface (ie, c_surfs->size() is always 1)
			//although that would make much more sense, mr swat3 dev.
			for (u32 m = 0; m < c_surfs->size(); m++)
			{
				u32 cs = (*c_surfs)[m]; //caller surface index
				if (cs != si)
				{
					vp.bShared = true; //this means this vertex is shared
					if (vp.sharesWith.binary_search(cs) == -1) //if not alread added, add
						vp.sharesWith.push_back(cs);
				}

			} //m
		} //k

		//CONTINUAR: boxes deviam ser desenhadas em todas as superficies! E rectas a unir os pontos se são diferentes no espaço
		  //         implementar, em main, retexturing de redsurfs, afecta bluesurfs


		vprops.push_back(vp);
	} //j

	return vprops;
}

core::array<int> CScnMeshData::getSharedSurface(CScn* scn, int si) {
	f32 dx = 1.0, dy = 1.0, dz = 1.0;

	CScnSolid* solid = scn->getSolid(solidindx);
	scnSurf_t* surfi = &solid->surfs[si];

	core::array<u32>* c_uvidxs;
	core::array<u32>* c_surfs;
	core::array<int> shared(16); //shared surfaces of si (not including si)


	//run through vertices, to see how many are shared
	bool bShared;

	for (u32 j = 0; j < surfi->vertidxlen; j++)
	{
		bShared = false; //assume vertex is not shared

		u32 uvidx = solid->uvidxs[surfi->vertidxstart + j]; //uvidx of this vertex

		c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

		for (u32 k = 0; k < c_uvidxs->size(); k++)              //run through all of these uvidxs
		{
			c_surfs = &solid->uvidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx

			//i don't think it ever happened where the same uvidx is called by more than one surface (ie, c_surfs->size() is always 1)
			//although that would make much more sense, mr swat3 dev.
			for (u32 m = 0; m < c_surfs->size(); m++)
			{
				u32 cs = (*c_surfs)[m]; //caller surface index
				if (cs != si && shared.linear_search(cs) == -1)
				{
					bShared = true; //this means this vertex is shared
					shared.push_back(cs); //add surface to list of shared surfaces
				}

			} //m
		} //k

	} //j

	return shared;
}

indexed_vertices CScnMeshData::getVertices(CScn* scn,core::array<int> selsurf, core::array<int> sharedsurf){
	CScnSolid* solid = scn->getSolid(solidindx);

	core::array<indexedVec3df_t> selverts;
	core::array<indexedVec3df_t> sharedverts;
	//Red surfaces and shared
	for (u32 i = 0; i < selsurf.size(); i++) {
		u32 r = selsurf[i];
		core::array<vertProp_t> vprops = getSurfVertProps(scn, r);

		for (u32 v = 0; v < vprops.size(); v++) {
			
			vertProp_t vp = vprops[v];
			core::vector3df vert = solid->verts[solid->vertidxs[vp.vertidxidx]];
			indexedVec3df_t indexedVert ={ vert,vp.vertidxidx,solidindx,r,vp.surf_vertidx };
			

			for (u32 s = 0; s < vp.sharesWith.size(); s++) {
				u32 swi= vp.sharesWith[s];
				s32 bi = sharedsurf.binary_search(swi);
				if (bi > -1) {
					//os::Printer::log("Red Surfs[{}]: vertex [{}]: shared with blue surf {}\n", r, vp.surf_vertidx, blueSurfs[bi]);
					//os::Printer::log("- Shared Surfs[{}]: vertex [{}]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.X, vert.Y, vert.Z);
					sharedverts.push_back(indexedVert);
					break;
				}
				else {
					// os::Printer::log("Red Surfs[{}]: vertex [{}]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.X, vert.Y, vert.Z);
					selverts.push_back(indexedVert);
				}
			}
			if (vp.sharesWith.size() == 0) {
				// os::Printer::log("Red Surfs[{}]: vertex [{}]: (x) %.0f (y) %.0f (z) %.0f\n", r, vp.surf_vertidx, vert.X, vert.Y, vert.Z);
				selverts.push_back(indexedVert);
			}
		}

	}
	return make_pair(selverts, sharedverts);
}
void CScnMeshData::select(int si, bool bShared) {
	RenderMesh->Materials[si]->setUniform("uSelected", 1.0f);
	if(bShared)RenderMesh->Materials[si]->setUniform4("uSelectedColor", SColor(150,100,100,255));
	else RenderMesh->Materials[si]->setUniform4("uSelectedColor", SColor(150, 255, 100, 100));
	RenderMesh->Materials[si]->updateShaderParams();
}
void CScnMeshData::deselect(int si) {
	RenderMesh->Materials[si]->setUniform("uSelected", 0.0f);
	RenderMesh->Materials[si]->setUniform4("uSelectedColor", SColor(0, 0, 0, 0));
	RenderMesh->Materials[si]->updateShaderParams();
}
void CScnMeshData::deselectAll() {
	for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
		RenderMesh->Materials[i]->setUniform("uSelected", 0.0f);
		RenderMesh->Materials[i]->setUniform4("uSelectedColor", SColor(0, 0, 0, 0));
		RenderMesh->Materials[i]->updateShaderParams();
	}
}
void CScnMeshData::hide(int si) {
	RenderMesh->getMeshBuffer(si)->getVertexBuffer()->set_used(0);
	RenderMesh->getMeshBuffer(si)->getIndexBuffer()->set_used(0);
	hiddensurfs.push_back(si);
}
void CScnMeshData::show() {
	for (int i = 0; i < hiddensurfs.size(); i++) {
		int si = hiddensurfs[i];
		RenderMesh->getMeshBuffer(si)->getVertexBuffer()->set_used(vis_backup[si].vertices.size());
		for (int v = 0; v < vis_backup[si].vertices.size(); v++) {
			RenderMesh->getMeshBuffer(si)->getVertexBuffer()->setVertex(v, &vis_backup[si].vertices[v]);
		}
		RenderMesh->getMeshBuffer(si)->getIndexBuffer()->set_used(vis_backup[si].indices.size());
		for (int j = 0; j < vis_backup[si].indices.size(); j++) {
			RenderMesh->getMeshBuffer(si)->getIndexBuffer()->setIndex(j, vis_backup[si].indices[j]);
		}
	}
}
void CScnMeshData::setTexture(CScn* scn, const char* path, int si) {
	std::string fullPath = str_split(path, ".")[0];
	core::array<std::string> split = str_split(fullPath.c_str(), "\\");
	const char* name = split[split.size() - 1].c_str();
	if (split.size() > 1 && strlen(name) <= 30) {
		scnSurf_t* surf = &scn->getSolid(solidindx)->surfs[si];
		strncpy(surf->texture, name,32);
	}
	ITexture* texture = convert_image(path);
	RenderMesh->Materials[si]->setTexture(0, texture);
}
indexedVec3df_t CScnMeshData::updateVert(CScn* scn, indexedVec3df_t vert, core::vector3df add) {
	CScnSolid* solid = scn->getSolid(solidindx);
	vert.pos += add;
	int si = vert.surfindx;
	solid->verts[solid->vertidxs[vert.vertindx]] = vert.pos;

	IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
	video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
	RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);
	vertices[vert.surf_vertindx].Pos = vert.pos;
	vis_backup[si].vertices[vert.surf_vertindx] = vertices[vert.surf_vertindx];
	return vert;
}
indexedVec3df_t CScnMeshData::resetVert(CScn* scn, indexedVec3df_t vert) {
	int si = vert.surfindx;
	video::S3DVertex2TCoords vertex = vert_backup[si].vertices[vert.surf_vertindx];
	vert.pos = vertex.Pos;
	return updateVert(scn, vert, core::vector3df(0, 0, 0));
}