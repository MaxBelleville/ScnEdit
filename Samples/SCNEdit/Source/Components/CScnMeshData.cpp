#include "pch.h"
#include "SkylichtEngine.h"
#include "Header/Components/CScnMeshData.h"

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
	std::unordered_map<std::string, std::string> findMap = {};
	
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

			if (try_load_texture(t, findMap, args->getBaseDirectory(), mesh->surfs[i].texture, istga))
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
			proj_backup.push_back(mesh->projection[i]);
			MeshBuffer->drop();

		}
	RenderMesh->recalculateBoundingBox();
	RenderMesh->setHardwareMappingHint(EHM_STATIC);
	
}
bool CScnMeshData::try_load_texture(video::ITexture*& t, std::unordered_map<std::string, std::string>& findSet,
	const wchar_t* dir, const char* tex, bool& istga) {
	if (t != nullptr) return true;

	if (findSet.contains(tex)) {
		if (!findSet[tex].empty()) {
			t = convert_image(findSet[tex].c_str());
			istga = (findSet[tex].find(".tga") != std::string::npos ||
				findSet[tex].find(".TGA") != std::string::npos);
			return true;
		}
		else {
			return false;
		}
	}

	// 1. Prepare potential search locations
	std::wstring ws(dir);
	std::string baseDirStr(ws.begin(), ws.end());
	std::vector<std::string> roots = { "./textures/", baseDirStr + "/general/" };

	// Potential extensions to check
	std::vector<std::string> exts = { ".tga", ".bmp", ".TGA", ".BMP" };

	// 2. Iterate through base paths and search recursively
	for (const std::string& root : roots) {
		core::array<std::string> matches = search_dir_recursive(root, tex);

		for (u32 j = 0; j < matches.size(); ++j) {
			std::string fullPath = matches[j];

			// 4. Attempt to load/convert
			t = convert_image(fullPath.c_str());

			if (t != nullptr) {
				// Success! Set the TGA flag based on the actual file found
				istga = (fullPath.find(".tga") != std::string::npos ||
					fullPath.find(".TGA") != std::string::npos);
				findSet[tex] = fullPath;
				return true;
			}
		}
	}
	os::Printer::log(format("Missing texture: {}", tex).c_str(), ELL_WARNING);
	findSet[tex] = ""; 
	return false;
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
					if (vectori.equals(point[p], 0.1))
						found[p] = true;
				}
			}
			if (found[0] && found[1] && found[2]) //if found all vertices
				return solidSelect_t(solidindx, i);
		}
	}
	return solidSelect_t(solidindx, -1);
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

void CScnMeshData::updateUVSurf(CScnSolid* solid, int si, int uvmode, core::vector2df uvShift) {
	scnSurf_t* surfi = &solid->surfs[si];
	scnProjectionBasis_t* paramFrame = &solid->projection[si];

	// 1. Get the translation scales (UV to Texel)
	// If your game applies a global texture scale, you may need to multiply these.
	// For now, assuming direct division by width/height based on the struct.
	float scaleU = (float)surfi->width;
	float scaleV = (float)surfi->height;

	// 2. Find UV Bounds
	core::vector2df minUV(FLT_MAX), maxUV(-FLT_MAX);
	for (int f = 0; f < surfi->faceidxlen; f++) {
		u32 uvi = solid->uvidxs[surfi->faceidxstart + f];
		core::vector2df uv = solid->uvpos[uvi];
		minUV.X = min(minUV.X, uv.X); minUV.Y = min(minUV.Y, uv.Y);
		maxUV.X = max(maxUV.X, uv.X); maxUV.Y = max(maxUV.Y, uv.Y);
	}

	// 2. Apply Transformations
	for (int f = 0; f < surfi->faceidxlen; f++) {
		u32 uvi = solid->uvidxs[surfi->faceidxstart + f];

		if (uvmode == 0) { // MOVE
			solid->uvpos[uvi] += uvShift;
		}
		else if (uvmode == 1) {//RESIZE
			solid->uvpos[uvi].X *= (1.0f - uvShift.X);
			solid->uvpos[uvi].Y *= (1.0f - uvShift.Y);
		}
		else if (uvmode == 2) { // FLIP H
			solid->uvpos[uvi].X = (maxUV.X + minUV.X) - solid->uvpos[uvi].X;
		}
		else if (uvmode == 3) { // FLIP V
			solid->uvpos[uvi].Y = (maxUV.Y + minUV.Y) - solid->uvpos[uvi].Y;
		}

	}

	if (uvmode == 0) { // MOVE
		// Formula: OriginShift = -(UVShift * Dim) * Axis
		paramFrame->origin -= (uvShift.X * (f32)surfi->width) * paramFrame->u_axis;
		paramFrame->origin -= (uvShift.Y * (f32)surfi->height) * paramFrame->v_axis;
	}
	else if (uvmode == 1) { // RESIZE
		float scaleFactorX = (1.0f - uvShift.X);
		float scaleFactorY = (1.0f - uvShift.Y);

		// To stretch UVs (Resize > 1), the Axis must get shorter
		if (scaleFactorX != 0.0f) paramFrame->u_axis /= scaleFactorX;
		if (scaleFactorY != 0.0f) paramFrame->v_axis /= scaleFactorY;

	}
	else if (uvmode == 2) { // FLIP H
		// Formula: Move origin to the 'other side' of the selection
		// O_new = O_old + (SumOfUVs * Width) * Axis
		paramFrame->origin += ((minUV.X + maxUV.X) * scaleU) * paramFrame->u_axis;
		paramFrame->u_axis = -paramFrame->u_axis;
	}
	else if (uvmode == 3) { // FLIP V
		paramFrame->origin += ((minUV.Y + maxUV.Y) * scaleV) * paramFrame->v_axis;
		paramFrame->v_axis = -paramFrame->v_axis;
	}

}

void CScnMeshData::resetUVSurf(CScnSolid* solid, int si) {
	scnSurf_t* surfi = &solid->surfs[si];

	for (u32 f = 0; f < surfi->faceidxlen; f++) {
		video::S3DVertex2TCoords vertex = vert_backup[si].vertices[f];
		u32 uvi = solid->uvidxs[surfi->faceidxstart + f];

		if (solid->uvpos[uvi] == vertex.TCoords)
			return;

		solid->uvpos[uvi] = vertex.TCoords;
	}

	//Probably better way to handle this but what ever.
	scnProjectionBasis_t* paramFrame = &solid->projection[si];
	paramFrame->origin = proj_backup[si].origin;
	paramFrame->u_axis = proj_backup[si].u_axis;
	paramFrame->v_axis = proj_backup[si].v_axis;
}

void CScnMeshData::updateUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf, int uvmode, core::vector2df uvShift) {
	CScnSolid* solid = scn->getSolid(solidindx);
	CScnLightmap* lmap = scn->getLightmap();

	for (u32 i = 0; i < selsurf.size(); i++) {
		int si = selsurf[i];
		updateUVSurf(solid, si, uvmode, uvShift);
	}


	updateMeshUV(scn, selsurf);
	updateMeshUV(scn, sharedsurf);
}
void CScnMeshData::resetUV(CScn* scn, core::array<int> selsurf, core::array<int> sharedsurf) {
	CScnSolid* solid = scn->getSolid(solidindx);
	CScnLightmap* lmap = scn->getLightmap();

	for (int i = 0; i < selsurf.size(); i++) {
		int si = selsurf[i];

		resetUVSurf(solid, si);
		lmap->resetMults(solidindx, si);
	}


	updateMeshUV(scn, selsurf);
	updateMeshUV(scn, sharedsurf);
}
