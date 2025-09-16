#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshData.h"
#include <set>


CScnMeshData::CScnMeshData() :
	MeshBuffer(NULL){}

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

		for (u32 i = 0; i < mesh->n_surfs; i++)
		{
			MeshBuffer = new CMeshBuffer<S3DVertex2TCoords>(driver->getVertexDescriptor(EVT_2TCOORDS), EIT_16BIT);
			alpha = mesh->surfs[i].alpha;
			bool istga = false;
			t = 0;

			//TODO: instead of using the base directory to load the textures consider loading
			//all general textures first then we don't need to load them here
	
			CMaterial* material = new CMaterial((to_string(mesh->solididx) + "-" + to_string(i)).c_str(), 
				"TextureColor2Layer.xml");

			if (try_load_texture(t, cantFind, "./textures/%s.bmp", nullptr, mesh->surfs[i].texture,istga)||
				try_load_texture(t, cantFind, "%S/general/%s.bmp", args->getBaseDirectory(), mesh->surfs[i].texture, istga)
				||try_load_texture(t, cantFind, "./textures/%s.tga", nullptr, mesh->surfs[i].texture, istga)||
				try_load_texture(t, cantFind, "%S/general/%s.tga", args->getBaseDirectory(), mesh->surfs[i].texture,istga)) 
			{ 
				//Load in lightmap atlas pos (ie where in the lightmap texture the solid surface currently is.)
				//And add that portion of the lightmap to be used later in a material.
				if (lmap->hasLightmaps() && args) {
					
					core::vector3di atpos = lmap->getAtlasPos(mesh->solididx, i);
					if (atpos.Z != currAtlas) {
						currAtlas = atpos.Z;
						lt = getVideoDriver()->addTexture(("lmAtlas" + to_string(atpos.Z)).c_str(), 
							lmap->getAtlas(atpos.Z));
					}
				}
				//If tga assume material is transparent.
				bool isTransparent = false;
				if (istga)
				{
					if (alpha == 255) 
						isTransparent = true;
					else
						os::Printer::log(format("Texture of surface[{}] is tga and alpha={}", i, alpha).c_str());
				}
				else
				{
					//If BMP only assume material is transparent when alpha isn't solid.
					if (alpha == 255) 
						isTransparent = false;
					else
						isTransparent = true;
				}
				//If it's transparent use the texture color 2 layer + alpha instead of just the regular texture color 2 layer. 
				if (isTransparent) 
					material = new CMaterial((to_string(mesh->solididx) + "-" + to_string(i)).c_str(), 
						"TextureColor2LayerAlpha.xml");
				

				material->setTexture(0, t);

				if (lmap->hasLightmaps() && args) {
					material->setTexture(1, lt);
					material->setUniform("uLightMapEnable", 1.0f);
				}
				else 
					material->setUniform("uLightMapEnable", 0.0f);

				material->setBackfaceCulling(true);
				
				material->setUniform("uSelected", 0.0f);
				material->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
				material->updateShaderParams();

			}


			IIndexBuffer* ibuff = MeshBuffer->getIndexBuffer();
			IVertexBuffer* vbuff = MeshBuffer->getVertexBuffer();
			vbuff->set_used(0); ibuff->set_used(0);

			mesh->calcSurfVertices(i, lmap->getMults(mesh->solididx, i), vbuff, args->getAlpha());
			mesh->calcSurfIndices(i, ibuff);
			MeshBuffer->recalculateBoundingBox();
		
			RenderMesh->addMeshBuffer(MeshBuffer, (to_string(mesh->solididx) + "-" + to_string(i)).c_str(), material);


			//Once mesh has been proccessed we can now load in the backup data used in reseting states.
			BackupData backupdata = {core::array<S3DVertex2TCoords>(),core::array<u32>()};
			
			S3DVertex2TCoords * vertices= static_cast<video::S3DVertex2TCoords*>(vbuff->getVertices());

			for (int v = 0; v < vbuff->getVertexCount(); v++) 
				backupdata.vertices.push_back(vertices[v]);

			// Backup indices
			for (int idx = 0; idx < ibuff->getIndexCount(); idx++) 
				backupdata.indices.push_back(ibuff->getIndex(idx));
			vis_backup.push_back(backupdata);
			vert_backup.push_back(backupdata);
			param_backup.push_back(mesh->paramFrames[i]);
			MeshBuffer->drop();

		}
	RenderMesh->recalculateBoundingBox();
	RenderMesh->setHardwareMappingHint(EHM_STATIC);
	
}

bool CScnMeshData::try_load_texture(video::ITexture*& t, std::set<std::string>& findSet, 
	const char* format, const wchar_t* dir, const char* tex, bool& istga) {
	if (t != 0)
		return true;

	char tmp[128] = "";

	if(dir != nullptr)
		sprintf_s(tmp, format, dir, tex);
	else 
		sprintf_s(tmp, format, tex);
	io::path file = tmp;

	if (!findSet.contains(tmp)) {
		t = convert_image(tmp);
		if (t == 0) 
			findSet.insert(tmp);
		istga = hasFileExtension(file, "tga");
		//May set to true in the case were can't convert tga but that shouldn't matter due to 
		// the fact we only care when it does work.
	}
	
	return t!=0;
}

void CScnMeshData::setLightmapVisible(bool vis) {
	//Depending on the current lightmap state and vis update the lightmap enable uniform in the shader.
	if ((RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 0.0 && vis)||
		RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 1.0 && !vis) {
		for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
			if (!vis) 
				RenderMesh->Materials[i]->setUniform("uLightMapEnable", 0.0f);
			else 
				RenderMesh->Materials[i]->setUniform("uLightMapEnable", 1.0f);

			RenderMesh->Materials[i]->updateShaderParams();
		}
	}
}
solidSelect_t CScnMeshData::getSurfaceIndx(CScn* scn, core::triangle3df tri) {
	core::vector3df point[3] = { tri.pointA, tri.pointB, tri.pointC };

	int selsurf = -1;
	CScnSolid* solid = scn->getSolid(solidindx);
	for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
		scnSurf_t* surfi = &solid->surfs[i];
		bool found[3] = { false,false,false };

		for (int k = 0; k < surfi->faceidxlen; k++) {
			//Gets vertex index
			core::vector3df* verti = &solid->verts[solid->vertidxs[surfi->faceidxstart + k]];
			//Gets vector index
			core::vector3df vectori(verti->X, verti->Y, verti->Z);
			for (int p = 0; p < 3; p++) {
				if (!found[p]) {
					if (vectori.equals(point[p], 1))
						found[p] = true;
				}
			}
			if (found[0] && found[1] && found[2]) //if found all vertices
				return solidSelect_t(solidindx, i);
		}
	}
	return solidSelect_t(solidindx, -1);
}

core::array<int> CScnMeshData::getUVSharedSurface(CScn* scn, core::array<int>sindices) {
	//Loop through all surfaces indices, get the shared surface of each indivual surface
	// Then compare to check if shared surface isn't included in the surface indice and isn't already included in final.
	core::array<int> final;
	for (int i = 0; i < sindices.size(); i++) {
		core::array<int> shared= getUVSharedSurface(scn,sindices[i]);
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

	for (u32 j = 0; j < surfi->faceidxlen; j++)
	{
		//start new index
		vertProp_t vp;
		
		vp.bShared = false; //assume vertex is not shared
		vp.surf_vertidx = j;    //redundant but useful for easy access later
		vp.faceidx = surfi->faceidxstart + j; //also redundant;
		vp.sharesWith.set_used(0);

		u32 uvidx = solid->uvidxs[surfi->faceidxstart + j]; //uvidx of this vertex

		c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

		for (u32 k = 0; k < c_uvidxs->size(); k++)          //run through all of these uvidxs
		{
			c_surfs = &solid->uvvertidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx

			//i don't think it ever happened where the same uvidx is called by more than one surface 
			// (ie, c_surfs->size() is always 1)
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

			} 
		}

		//CONTINUE: boxes should be drawn on all surfaces! And straight lines joining the dots if 
		// they are different in space 
		// implementing, in main, retexturing of selected surf, affects blue surfaces.

		vprops.push_back(vp);
	} //j

	return vprops;
}
//Same as getSurfVertProps but only for figuring out the shared index. 
core::array<int> CScnMeshData::getUVSharedSurface(CScn* scn, int si) {
	CScnSolid* solid = scn->getSolid(solidindx);
	scnSurf_t* surfi = &solid->surfs[si];

	core::array<u32>* c_uvidxs;
	core::array<u32>* c_surfs;
	core::array<int> shared(16); //shared surfaces of si (not including si)


	//run through vertices, to see how many are shared
	for (u32 j = 0; j < surfi->faceidxlen; j++)
	{
		u32 uvidx = solid->uvidxs[surfi->faceidxstart + j]; //uvidx of this vertex

		c_uvidxs = &solid->uvpos_caller[uvidx];             //array with the uvidxs that point to this uvpos vertex

		for (u32 k = 0; k < c_uvidxs->size(); k++)          //run through all of these uvidxs
		{
			c_surfs = &solid->uvvertidxs_caller[(*c_uvidxs)[k]]; //array with the indexes of surfs that have this uvidx
			for (u32 m = 0; m < c_surfs->size(); m++)
			{
				u32 cs = (*c_surfs)[m]; //caller surface index
				if (cs != si && shared.linear_search(cs) == -1)
				{
					shared.push_back(cs); //add surface to list of shared surfaces
				}

			} //m
		} //k

	} //j

	return shared;
}

core::array<int> CScnMeshData::getVertSharedSurface(CScn* scn, int si, int vertidx) {
	CScnSolid* solid = scn->getSolid(solidindx);
	scnSurf_t* surfi = &solid->surfs[si];

	core::array<u32>* c_vertidxs;
	core::array<u32>* c_surfs;
	core::array<int> shared(16); //shared surfaces of si (not including si)


	//run through vertices, to see how many are shared

	c_vertidxs = &solid->vertpos_caller[vertidx];             //array with the vertidx that point to this uvpos vertex

	for (u32 k = 0; k < c_vertidxs->size(); k++)          //run through all of these uvidxs
	{
		c_surfs = &solid->uvvertidxs_caller[(*c_vertidxs)[k]]; //array with the indexes of surfs that have this uvidx
		for (u32 m = 0; m < c_surfs->size(); m++)
		{
			u32 cs = (*c_surfs)[m]; //caller surface index
			if (cs != si && shared.linear_search(cs) == -1)
			{
				shared.push_back(cs); //add surface to list of shared surfaces
			}

		} //m
	} //k


	return shared;
}

indexedVec3df_t CScnMeshData::getVertexFromPos(CScn* scn, u32 si, u32 faceidx) {
	//Get the indexed Vec3/vert information based on the surface and vertex index. 
	CScnSolid* solid = scn->getSolid(solidindx);
	scnSurf_t* surfi = &solid->surfs[si];
	for (u32 j = 0; j < surfi->faceidxlen; j++)
	{
		u32 sharedvertidx =surfi->faceidxstart + j;
		core::vector3df vert = solid->verts[solid->vertidxs[sharedvertidx]];

		if (solid->vertidxs[sharedvertidx] == solid->vertidxs[faceidx])
			return { vert,sharedvertidx,solidindx,si,j };
		
	}
	return {};
}

indexed_vertices CScnMeshData::getVertices(CScn* scn,core::array<int> selsurf, core::array<int> sharedsurf){
	CScnSolid* solid = scn->getSolid(solidindx);

	core::array<indexedVec3df_t> selverts;
	core::array<indexedVec3df_t> sharedverts;
	//Go through red/Selected surfaces and get the vert properties. 
	for (u32 i = 0; i < selsurf.size(); i++) {
		u32 r = selsurf[i];
		core::array<vertProp_t> vprops = getSurfVertProps(scn, r);

		//Loop through all of the verts getting the indexed version of the vert and split it depending on 
		// if it's shared or not.
		for (u32 v = 0; v < vprops.size(); v++) {
			
			vertProp_t vp = vprops[v];
			core::vector3df vert = solid->verts[solid->vertidxs[vp.faceidx]];
			indexedVec3df_t indexedVert ={ vert,vp.faceidx,solidindx,r,vp.surf_vertidx,vp.bShared, vp.sharesWith };
			
			for (u32 s = 0; s < vp.sharesWith.size(); s++) {
				u32 swi= vp.sharesWith[s];
				s32 bi = sharedsurf.binary_search(swi);
				if (bi > -1) {
					sharedverts.push_back(indexedVert);
					break;
				}
				else 
					selverts.push_back(indexedVert);
			}
			if (vp.sharesWith.size() == 0) 
				selverts.push_back(indexedVert);
		}

	}
	return make_pair(selverts, sharedverts);
}


core::array<u32> CScnMeshData::getSurfUVIdxs(CScn* scn, core::array<int> selsurfs) {

	CScnSolid* solid = scn->getSolid(solidindx);
	core::array<u32> all;
	//loops through all of the selected surface and gets the uv indices of each vertex in the surface.
	for (int i = 0; i < selsurfs.size(); i++) {
		int si = selsurfs[i];
		scnSurf_t* surfi = &solid->surfs[si];

		for (u32 f = 0; f < surfi->faceidxlen; f++) {
			u32 uvi = solid->uvidxs[surfi->faceidxstart + f];
			if (all.binary_search(uvi) == -1) //if not already there, add
				all.push_back(uvi);
		}
	}
	return all;
}
void CScnMeshData::select(int si, bool bShared) {
	CMaterial* mat = RenderMesh->Materials[si];
	mat->setUniform("uSelected", 1.0f);
	if(bShared)
		mat->setUniform4("uSelectedColor", SColor(50,100,100,255));
	else 
		mat->setUniform4("uSelectedColor", SColor(50, 255, 100, 100));
	mat->updateShaderParams();
}
void CScnMeshData::deselect(int si) {
	CMaterial* mat = RenderMesh->Materials[si];
	mat->setUniform("uSelected", 0.0f);
	mat->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
	mat->updateShaderParams();
}
void CScnMeshData::deselectAll() {
	for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
		CMaterial* mat = RenderMesh->Materials[i];
		mat->setUniform("uSelected", 0.0f);
		mat->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
		mat->updateShaderParams();
	}
}
//Remove the surface in a mesh.
void CScnMeshData::hide(int si) {
	RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);
	RenderMesh->getMeshBuffer(si)->getVertexBuffer()->set_used(0);
	RenderMesh->getMeshBuffer(si)->getIndexBuffer()->set_used(0);
	hiddensurfs.push_back(si);
}
//Readds the surface in a mesh using backup
void CScnMeshData::show() {
	for (int i = 0; i < hiddensurfs.size(); i++) {
		int si = hiddensurfs[i];
		IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
		IIndexBuffer* ib = RenderMesh->getMeshBuffer(si)->getIndexBuffer();
		vb->set_used(vis_backup[si].vertices.size());
		ib->set_used(vis_backup[si].indices.size());

		for (int v = 0; v < vis_backup[si].vertices.size(); v++) 
			vb->setVertex(v, &vis_backup[si].vertices[v]);

		for (int j = 0; j < vis_backup[si].indices.size(); j++) 
			ib->setIndex(j, vis_backup[si].indices[j]);
	}
}
void CScnMeshData::setTexture(CScn* scn, const char* path, int si) {
	//Do some finally proccessing of the string to get the name of the texture
	std::string fullPath = str_split(path, ".")[0];
	core::array<std::string> split = str_split(fullPath.c_str(), "\\");
	const char* name = split[split.size() - 1].c_str();

	//Copy the texture name into the surf data.
	if (split.size() > 1 && strlen(name) <= 30) {
		scnSurf_t* surf = &scn->getSolid(solidindx)->surfs[si];
		strncpy(surf->texture, name,32);
	}
	//Conver the texture path into a real texture in the renderer.
	ITexture* texture = convert_image(path);
	RenderMesh->Materials[si]->setTexture(0, texture);
}

void CScnMeshData::updatePlane(CScn* scn, int si) {
	CScnSolid* solid = scn->getSolid(solidindx);
	IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
	IIndexBuffer* ib = RenderMesh->getMeshBuffer(si)->getIndexBuffer();
	video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
	u16* indices = static_cast<u16*>(ib->getIndices());
	
	u32 weights = 0;
	core::vector3df sum = core::vector3df(0);
	core::vector3df  sumMidpoint = core::vector3df(0);

	for (u32 i = 0; i < ib->getIndexCount(); i += 3) {
		core::vector3df point[3] = { core::vector3df(0),  core::vector3df(0), core::vector3df(0) };
		for (int p = 0; p < 3; p++) 
			point[p] = vertices[indices[i + p]].Pos;

		core::vector3df midpoint = (point[0] + point[1]+ point[2]) / 3;

		//Dir is the cross product of the difference between the top left and top right and top left and bottom left.
		core::vector3df dir = (point[1] - point[0]).crossProduct(point[2] - point[0]);
		f32 area = (0.5 * dir.getLength());

		//sum is the collection of normalized dir and midpoints mutiplied by the area weight.
		core::vector3df normal = dir.normalize();
		sum += normal * area;
		sumMidpoint += midpoint * area;
		weights += area;
	}
	//Weighted average of sum diff and sum midpoint using area as a weight.
	//Then gets the d value and updates the plane.
	core::vector3df avg = sum / weights, avgMidpoint = sumMidpoint / weights;
	f32 d = -((avg.X * avgMidpoint.X) + (avg.Y * avgMidpoint.Y) + (avg.Z * avgMidpoint.Z));
	scnSurf_t* surfi = &solid->surfs[si];
	scnPlane_t* planei = &(solid->planes[surfi->planeidx]);
	planei->a = avg.X;
	planei->b = avg.Y;
	planei->c = avg.Z;
	planei->d = d;
}

void CScnMeshData::updateVert(CScn* scn, indexedVec3df_t& vert, core::vector3df add) {
	//Update indexed verts, solid verts, and mesh verts.
	CScnSolid* solid = scn->getSolid(solidindx);
	
	vert.pos += add;
	solid->verts[solid->vertidxs[vert.faceidx]] = vert.pos;

	u32 vertidx = solid->vertidxs[vert.faceidx]; //vertidx of this vertex

	core::array<int> shared = getVertSharedSurface(scn,vert.surfidx, vertidx);

	updateMeshVert(vert.surfidx, vert.surf_vertidx, vert.pos);
	updatePlane(scn, vert.surfidx);

	for (int j = 0; j < shared.size(); j++) {
		int si = shared[j];
		scnSurf_t* surf = &solid->surfs[si];
		u32 surf_vertidx = -1;
		for (u32 j = 0; j < surf->faceidxlen; j++) {
			if (solid->vertidxs[surf->faceidxstart + j] == vertidx) {
				surf_vertidx = j;
				break;
			}
		}
		updateMeshVert(si, surf_vertidx, vert.pos);
		updatePlane(scn, si);
	}

}

void CScnMeshData::resetVert(CScn* scn, indexedVec3df_t& vert) {
	int si = vert.surfidx;
	video::S3DVertex2TCoords vertex = vert_backup[si].vertices[vert.surf_vertidx];
	if (vert.pos != vertex.Pos) {
		vert.pos = vertex.Pos;
		updateVert(scn, vert, core::vector3df(0));
	}
	
}
void CScnMeshData::updateMeshVert(int si, int surf_vertidx, core::vector3df pos) {
	IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
	video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
	RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);

	vertices[surf_vertidx].Pos = pos;
	vis_backup[si].vertices[surf_vertidx] = vertices[surf_vertidx];
}

void CScnMeshData::updateMeshUV(CScn* scn, core::array <int> surf) {
	CScnSolid* solid = scn->getSolid(solidindx);
	CScnLightmap* lmap = scn->getLightmap();

	for (u32 i = 0; i < surf.size(); i++) {
		int si = surf[i];
		f32* mults = lmap->getMults(solidindx, si);

		scnSurf_t* surfi = &solid->surfs[si];
		IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
		video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
		RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);

		for (u32 f = 0; f < surfi->faceidxlen; f++) {
			core::vector2df uvs = solid->uvpos[solid->uvidxs[surfi->faceidxstart + f]];
			vertices[f].TCoords = uvs;
			video::S3DVertex2TCoords back = vert_backup[si].vertices[f];
			vertices[f].TCoords2 = back.TCoords;
			//do some sort of manipulation to the verts.
			if (mults) {
				vertices[f].TCoords2.X = vertices[f].TCoords2.X * mults[0] + mults[2];
				vertices[f].TCoords2.Y = vertices[f].TCoords2.Y * mults[1] + mults[3];
			}
			
		}
	}
}
void CScnMeshData::updateUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf,
	int uvmode, core::vector2df uvShift) {
	CScnSolid* solid = scn->getSolid(solidindx);
//	core::array<u32> uvidxs = getSurfUVIdxs(scn, selsurf);
	CScnLightmap* lmap = scn->getLightmap();


	for (u32 i = 0; i < selsurf.size(); i++) {
	 int si = selsurf[i];
	 scnSurf_t* surfi = &solid->surfs[si];
	 f32* mults = lmap->getMults(solidindx, si);
	 scnSurfParamFrame_t* paramFrame = &solid->paramFrames[si];

	// Get the unknown offsets (very likely atlas texel offsets)
	core::vector2df minUV(FLT_MAX);
	core::vector2df maxUV(-FLT_MAX);

	for (int f = 0; f < surfi->faceidxlen; f++) {
		u32 uvi = solid->uvidxs[surfi->faceidxstart + f];
		core::vector2df uv = solid->uvpos[uvi];

		minUV = core::vector2df(min(minUV.X, uv.X), min(minUV.Y, uv.Y));
		maxUV = core::vector2df(max(maxUV.X, uv.X), max(maxUV.Y, uv.Y));

	}
	// Get the unknown offsets (very likely atlas texel offsets)
	core::vector3df minVert(FLT_MAX);
	core::vector3df maxVert(-FLT_MAX);
	for (int f = 0; f < surfi->faceidxlen; f++) {
		u32 verti = solid->vertidxs[surfi->faceidxstart + f];
		core::vector3df vert = solid->verts[verti];

		minVert = core::vector3df(min(minVert.X, vert.X), min(minVert.Y, vert.Y), min(minVert.Z, vert.Z));
		maxVert = core::vector3df(max(maxVert.X, vert.X), max(maxVert.Y, vert.Y), max(maxVert.Z, vert.Z));

	}

	os::Printer::log(format("MinUV {} {}", minUV.X, minUV.Y).c_str());
	os::Printer::log(format("MaxUV {} {}", maxUV.X, maxUV.Y).c_str());
	os::Printer::log(format("MinVert {} {} {}", minVert.X, minVert.Y , minVert.Z).c_str());
	os::Printer::log(format("MaxVert {} {} {}", maxVert.X, maxVert.Y, maxVert.Z).c_str());

	os::Printer::log(format("Unknown vec2 offset {} {}", surfi->lmoff.X, surfi->lmoff.Y).c_str());

	float s_shift = surfi->lmoff.X / 16;
	float t_shift = surfi->lmoff.Y / 16;
	// half-extent center in luxels
	float u_center = s_shift + (float)surfi->lmsize_h * 0.5f; // e.g. 47 + 11/2 = 52.5
	float v_center = t_shift + (float)surfi->lmsize_v * 0.5f; // e.g.  3 +  7/2 = 6.5
	//Change uvs for solid
	for (int f = 0; f < surfi->faceidxlen; f++) {
		u32 uvi = solid->uvidxs[surfi->faceidxstart + f];
		if (uvmode == 0) //MOVE
			solid->uvpos[uvi] += uvShift;
		else if (uvmode == 1) {//RESIZE
			solid->uvpos[uvi].X *= (1.0f - uvShift.X);
			solid->uvpos[uvi].Y *= (1.0f - uvShift.Y);
		}
		//else if (uvmode == 2) //FLIP H
		//	solid->uvpos[uvi].X = (minUV.X+ maxUV.X) - solid->uvpos[uvi].X;
		//else  //FLIP V 
		//	solid->uvpos[uvi].Y = (minUV.Y + maxUV.Y) - solid->uvpos[uvi].Y;
	}

		if (uvmode == 0) {
			paramFrame->origin -= uvShift.X * paramFrame->u_axis;
			paramFrame->origin -= uvShift.Y * paramFrame->v_axis;
			if (mults) {
				mults[2] += uvShift.X * mults[0];
				mults[3] += uvShift.Y * mults[1];
			}


		}
		else if (uvmode == 1) {
			paramFrame->u_axis *= (1.0f - uvShift.X);
			paramFrame->v_axis *= (1.0f - uvShift.Y);
			if (mults) {
				mults[0] -= uvShift.X / 4.0f;
				mults[1] -= uvShift.Y / 4.0f;
			}
		}
	/*	else if (uvmode == 2) {
			paramFrame->u_axis *= -1;
			paramFrame->origin += (maxUV.X- minUV.X) *paramFrame->u_axis;
		}

		else {
			paramFrame->v_axis *= -1;
			paramFrame->origin += (maxUV.Y - minUV.Y) * paramFrame->v_axis;
		}*/
	}

	updateMeshUV(scn, selsurf);
	updateMeshUV(scn, sharedsurf);
}
void CScnMeshData::resetUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf) {
	CScnSolid* solid = scn->getSolid(solidindx);
	CScnLightmap* lmap = scn->getLightmap();

	for (int i = 0; i < selsurf.size(); i++) {
		int si = selsurf[i];
		scnSurf_t* surfi = &solid->surfs[si];
		
		for (u32 f = 0; f < surfi->faceidxlen; f++) {
			video::S3DVertex2TCoords vertex = vert_backup[si].vertices[f];
			u32 uvi = solid->uvidxs[surfi->faceidxstart + f];
			
			if (solid->uvpos[uvi] == vertex.TCoords) 
				return;

			solid->uvpos[uvi] = vertex.TCoords;
		}

		lmap->resetMults(solidindx, si);
		//Probably better way to handle this but what ever.
		scnSurfParamFrame_t* paramFrame = &solid->paramFrames[si];
		paramFrame->origin = param_backup[si].origin;
		paramFrame->u_axis = param_backup[si].u_axis;
		paramFrame->v_axis = param_backup[si].v_axis;
	

	}
	updateMeshUV(scn, selsurf);
	updateMeshUV(scn, sharedsurf);
}
