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

void CScnMeshData::initMesh(CScnSolid* mesh,CScnLightmap* lmap, CScnArguments* args)
{
	if (MeshBuffer != NULL)
		MeshBuffer->drop();

	if (RenderMesh != NULL)
		RenderMesh->drop();

	RenderMesh = new CMesh();
	IVideoDriver* driver = getVideoDriver();


	solidindx = mesh->solididx;
	std::unordered_map<std::string, std::string> findMap = {};
	
		video::ITexture* t = 0;
		video::ITexture* lt{};
		core::array<u32> is;
		s32 currAtlas = -1;
		u16 alpha;

		for (u32 i = 0; i < mesh->n_surfs; i++)
		{
			CScnLocalizedFace* local = &mesh->local_faces[i];

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
					alpha == 255 ? isTransparent = false : isTransparent = true;
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

			if (alpha != 255 && args->getAlpha() > 0) alpha = args->getAlpha(); //override with l

			local->calcVertices(alpha,mesh->surfs[i].hasVertexColors, vbuff);
			local->calcIndices(ibuff);

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

surfaceBox_t CScnMeshData::collectSurfFromTri(CScnSolid* solid, core::triangle3df tri) {
	core::vector3df point[3] = { tri.pointA, tri.pointB, tri.pointC };

	int selsurf = -1;

	for (int i = 0; i < solid->n_surfs; i++) {
		scnSurf_t* surfi = &solid->surfs[i];
		bool found[3] = { false,false,false };

		for (int k = 0; k < surfi->faceidxlen; k++) {
			//Gets vertex index
			core::vector3df* verti = &solid->local_faces[i].verts[k].pos;
			//Gets vector index
			core::vector3df vectori(verti->X, verti->Y, verti->Z);
			for (int p = 0; p < 3; p++) {
				if (!found[p]) {
					if (vectori.equals(point[p], 0.1))
						found[p] = true;
				}
			}
			if (found[0] && found[1] && found[2]) //if found all vertices
				return surfaceBox_t(solidindx, i);
		}
	}
	return surfaceBox_t(solidindx, -1);
}
core::array<surfaceBox_t> CScnMeshData::select(CScnSolid* solid, core::triangle3df tri, bool bAdd) {
	surfaceBox_t sel = collectSurfFromTri(solid, tri);
	if (sel.si == -1) return surfsels; //if no surf found return current selection

	bool alreadySelected = false;
	for (int i = 0; i < surfsels.size(); i++) {
		if (surfsels[i].si == sel.si) {
			alreadySelected = true;
			break;
		}
	}

	if (bAdd) {
		// --- MULTI-SELECT / TOGGLE MODE ---
		if (alreadySelected) 
			// Scenario 2 (Multi): Toggle off
			deselect(solid,sel.si);
		
		else 
			// Scenario 1: Add to existing
			surfsels.push_back(sel);
	}
	else {
		// --- SINGLE-SELECT MODE ---
		deselect(solid, -1);
		if (alreadySelected) {
			if (surfsels.size() != 1)
				// Scenario 4: Clear others, keep this one
				surfsels.push_back(sel);
		}
		else 
			// Scenario 3: Replace current selection with new one
			surfsels.push_back(sel);
		
	}

	selectMat(solid);


	return surfsels;
}

void CScnMeshData::deselect(CScnSolid* solid, int si) {
	if (si == -1) {
		for (int i = 0; i < surfsels.size(); i++) 
			deselectMat(solid, surfsels[i].si);
		surfsels.clear();
	}
	else {
		for (int i = 0; i < surfsels.size(); i++) {
			if (surfsels[i].si == si) {
				surfsels.erase(i);
				deselectMat(solid, surfsels[i].si);
				return;
			}
		}
	}
	
}



void CScnMeshData::selectMat(CScnSolid* solid) {
	for (int i = 0; i < surfsels.size(); i++) {
		// regular surfs
		int si = surfsels[i].si;
		CMaterial* mat = RenderMesh->Materials[si];

		for (int j = 0; j < solid->local_faces[si].shared.size(); j++) {
			//shared surfs
			int sharedsi = solid->local_faces[si].shared[j];

			CMaterial* sharedmat = RenderMesh->Materials[sharedsi];
			sharedmat->setUniform("uSelected", 1.0f);
			sharedmat->setUniform4("uSelectedColor", SColor(50, 100, 100, 255));
			sharedmat->updateShaderParams();
		}

	}
	for (int i = 0; i < surfsels.size(); i++) {
		int si = surfsels[i].si;
		CMaterial* mat = RenderMesh->Materials[si];

		mat->setUniform("uSelected", 1.0f);
		mat->setUniform4("uSelectedColor", SColor(50, 255, 100, 100));
		mat->updateShaderParams();
	}

}
void CScnMeshData::deselectMat(CScnSolid* solid, int si) {
	CMaterial* mat = RenderMesh->Materials[si];
	mat->setUniform("uSelected", 0.0f);
	mat->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
	mat->updateShaderParams();

	for (int j = 0; j < solid->local_faces[si].shared.size(); j++) {
		//shared surfs
		int sharedsi = solid->local_faces[si].shared[j];

		CMaterial* sharedmat = RenderMesh->Materials[sharedsi];
		sharedmat->setUniform("uSelected", 0.0f);
		sharedmat->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
		sharedmat->updateShaderParams();
	}
}


void CScnMeshData::setLightmapVisible(bool vis) {
	//Depending on the current lightmap state and vis update the lightmap enable uniform in the shader.
	if ((RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 0.0 && vis) ||
		RenderMesh->Materials[0]->getUniform("uLightMapEnable")->FloatValue[0] == 1.0 && !vis) {
		for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
			vis ? RenderMesh->Materials[i]->setUniform("uLightMapEnable", 1.0f) : RenderMesh->Materials[i]->setUniform("uLightMapEnable", 0.0f);
			RenderMesh->Materials[i]->updateShaderParams();
		}
	}
}

void CScnMeshData::deselectAll() {
	for (int i = 0; i < RenderMesh->getMeshBufferCount(); i++) {
		CMaterial* mat = RenderMesh->Materials[i];
		mat->setUniform("uSelected", 0.0f);
		mat->setUniform4("uSelectedColor", SColor(255, 0, 0, 0));
		mat->updateShaderParams();
	}
	surfsels.clear();
}

//Remove the surface in a mesh.
void CScnMeshData::hide(CScnSolid* solid,bool bShared) {
	for (int i = 0; i < surfsels.size(); i++) {
		// regular surfs
		int si = surfsels[i].si;
		RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);
		RenderMesh->getMeshBuffer(si)->getVertexBuffer()->set_used(0);
		RenderMesh->getMeshBuffer(si)->getIndexBuffer()->set_used(0);
		hiddensurfs.push_back(si);

		if (bShared) {
			for (int j = 0; j < solid->local_faces[si].shared.size(); j++) {
				int sharedsi = solid->local_faces[si].shared[j];
				if (sharedsi == si) continue; //skip if same surf

				RenderMesh->getMeshBuffer(sharedsi)->setHardwareMappingHint(EHM_NEVER);
				RenderMesh->getMeshBuffer(sharedsi)->getVertexBuffer()->set_used(0);
				RenderMesh->getMeshBuffer(sharedsi)->getIndexBuffer()->set_used(0);
				hiddensurfs.push_back(sharedsi);
			}
		}
	}
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
void CScnMeshData::setTexture(CScnSolid* solid, const char* path) {
	for (int i = 0; i < surfsels.size(); i++) {
		int si = surfsels[i].si;
		//Do some finally proccessing of the string to get the name of the texture
		std::string fullPath = str_split(path, ".")[0];
		core::array<std::string> split = str_split(fullPath.c_str(), "\\");
		const char* name = split[split.size() - 1].c_str();

		//Copy the texture name into the surf data.
		if (split.size() > 1 && strlen(name) <= 30) {
			scnSurf_t* surf = &solid->surfs[si];
			strncpy(surf->texture, name, 32);
		}
		//Conver the texture path into a real texture in the renderer.
		ITexture* texture = convert_image(path);
		RenderMesh->Materials[si]->setTexture(0, texture);
	}
}

void CScnMeshData::updatePlane(CScnSolid* solid, int si) {
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

void CScnMeshData::updateVert(CScnSolid* solid, vertBox_t vertsel, core::vector3df add) {

	IVertexBuffer* vb = RenderMesh->getMeshBuffer(vertsel.si)->getVertexBuffer();
	video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
	RenderMesh->getMeshBuffer(vertsel.si)->setHardwareMappingHint(EHM_NEVER);

	localizedVertex_t* vert = &solid->local_faces[vertsel.si].verts[vertsel.localidx];

	vert->pos += add;

	vertices[vertsel.localidx].Pos = vert->pos;


	updatePlane(solid, vertsel.si);

}

void CScnMeshData::resetVert(CScnSolid* solid, vertBox_t vertsel) {

	localizedVertex_t* vert = &solid->local_faces[vertsel.si].verts[vertsel.localidx];

	u32 vertIdx = vert->vertidx;

	if (vert->pos != solid->verts[vertIdx]) {
		IVertexBuffer* vb = RenderMesh->getMeshBuffer(vertsel.si)->getVertexBuffer();
		video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
		RenderMesh->getMeshBuffer(vertsel.si)->setHardwareMappingHint(EHM_NEVER);

		vert->pos = solid->verts[vertIdx];

		vertices[vertsel.localidx].Pos = vert->pos;

		updatePlane(solid, vertsel.si);
	}
	
}

void CScnMeshData::updateUV(CScnSolid* solid, UVMode mode, core::vector2df uvAdd) {
	for (u32 i = 0; i < surfsels.size(); i++) {
		int si = surfsels[i].si;
		if (mode == UVMode::FlipH) {
			solid->local_faces[si].flipX = !solid->local_faces[si].flipX;
		}

		if (mode == UVMode::FlipV) {
			solid->local_faces[si].flipY = !solid->local_faces[si].flipY;
		}

		scnSurf_t* surfi = &solid->surfs[si];
		IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
		video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
		RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);

		scnProjectionBasis_t* proj = &solid->projection[si];

		// 2. Find UV Bounds
		core::vector2df minUV(FLT_MAX), maxUV(-FLT_MAX);
		for (int f = 0; f < surfi->faceidxlen; f++) {
			core::vector2df uv = solid->local_faces[si].verts[f].uv;

			minUV.X = min(minUV.X, uv.X); minUV.Y = min(minUV.Y, uv.Y);
			maxUV.X = max(maxUV.X, uv.X); maxUV.Y = max(maxUV.Y, uv.Y);

		}
		//This is a very hacky solution to UV as we don't want to adjust file size right now.
		//Ulitmately what should happen is that if it's not uvidx that exists when we update uvs
		//It would add a new entry to uvidx and other arrays.
		for (int f = 0; f < surfi->faceidxlen; f++) {
			localizedVertex_t* vert = &solid->local_faces[si].verts[f];

			//if (mode == UVMode::Move) {
			//	solid->local_faces[si].addToUV(f, uvAdd);
			//	
			//}
			//else if (mode == UVMode::Resize) {
			//	solid->local_faces[si].verts[f].uv *= (core::vector2df(1.0f) - uvAdd);
			//}
			//else 
			if (mode == UVMode::FlipH) { // FLIP H
				vert->uv.X = (maxUV.X + minUV.X) - vert->uv.X;
			}
			else if (mode == UVMode::FlipV) { // FLIP V
				vert->uv.Y = (maxUV.Y + minUV.Y) - vert->uv.Y;
			}

			vertices[f].TCoords = vert->uv;
			vertices[f].TCoords2 = vert->uv;
			//do some sort of manipulation to the verts.

			if (solid->local_faces[si].hlmap) {
				f32* mults = solid->local_faces[si].hlmap->uv_mults;
				vertices[f].TCoords2.X = vertices[f].TCoords2.X * mults[0] + mults[2];
				vertices[f].TCoords2.Y = vertices[f].TCoords2.Y * mults[1] + mults[3];
			}

		}

		//if (mode == UVMode::Move) { // MOVE
		//	// Formula: OriginShift = -(UVShift * Dim) * Axis
		//	proj->origin -= (uvAdd.X * (f32)surfi->width) * proj->u_axis;
		//	proj->origin -= (uvAdd.Y * (f32)surfi->height) * proj->v_axis;
		//}
		//else if (mode == UVMode::Resize) { // RESIZE
		//	float scaleFactorX = (1.0f - uvAdd.X);
		//	float scaleFactorY = (1.0f - uvAdd.Y);

		//	// To stretch UVs (Resize > 1), the Axis must get shorter
		//	if (scaleFactorX != 0.0f) proj->u_axis /= scaleFactorX;
		//	if (scaleFactorY != 0.0f) proj->v_axis /= scaleFactorY;

		//}
		//else 
			if (mode == UVMode::FlipH) { // FLIP H
			// Formula: Move origin to the 'other side' of the selection
			// O_new = O_old + (SumOfUVs * Width) * Axis
			proj->origin += ((minUV.X + maxUV.X) * (f32)surfi->width) * proj->u_axis;
			proj->u_axis = -proj->u_axis;
		}
		else if (mode == UVMode::Move) { // FLIP V
			proj->origin += ((minUV.Y + maxUV.Y) * (f32)surfi->height) * proj->v_axis;
			proj->v_axis = -proj->v_axis;
		}
	}

}

void CScnMeshData::resetUV(CScnSolid* solid) {
	for (int i = 0; i < surfsels.size(); i++) {
		int si = surfsels[i].si;

		IVertexBuffer* vb = RenderMesh->getMeshBuffer(si)->getVertexBuffer();
		video::S3DVertex2TCoords* vertices = static_cast<video::S3DVertex2TCoords*>(vb->getVertices());
		RenderMesh->getMeshBuffer(si)->setHardwareMappingHint(EHM_NEVER);

		for (u32 f = 0; f < solid->local_faces[si].verts.size(); f++) {
			solid->local_faces[si].flipX = false;
			solid->local_faces[si].flipY = false;
			localizedVertex_t* vert = &solid->local_faces[si].verts[f];

			u32 uvi = vert->uvidx;

			if (solid->uvpos[uvi] == vert->uv)
				return;

			solid->local_faces[si].verts[f].uv = solid->uvpos[uvi];

			vertices[f].TCoords = vert->uv;
			vertices[f].TCoords2 = vert->uv;
			//do some sort of manipulation to the verts.

			if (solid->local_faces[si].hlmap) {
				f32* mults = solid->local_faces[si].hlmap->uv_mults;
				vertices[f].TCoords2.X = vertices[f].TCoords2.X * mults[0] + mults[2];
				vertices[f].TCoords2.Y = vertices[f].TCoords2.Y * mults[1] + mults[3];
			}
		}

		scnProjectionBasis_t* paramFrame = &solid->projection[si];
		paramFrame->origin = proj_backup[si].origin;
		paramFrame->u_axis = proj_backup[si].u_axis;
		paramFrame->v_axis = proj_backup[si].v_axis;
	}
}
