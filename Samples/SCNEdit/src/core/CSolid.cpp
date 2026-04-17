#include "pch.h"
#include "include/core/CSolid.h"
#include <set>
#include <format>

using namespace std;
using namespace irr;
using namespace video;

//default constructor
CSolid::CSolid()
{
	offset = 0;
	length = 0;
	//setting pointers to zero
	rawcells = 0;
	uvidxs = 0;
	vertidxs = 0;
	uvpos = 0;
	verts = 0;
	planes = 0;
	tree = 0;
	surfs = 0;
}

//Deconstructor for solids
CSolid::~CSolid()
{
	textures.clear();
	//pointers must be set to zero initially otherwise errors
	if (!rawcells.empty())
	{
		for (u32 i = 0; i < n_cells; i++)
		{
			//Deletes cells node id
			if (rawcells[i]->nodesidxs)
				delete[] rawcells[i]->nodesidxs;

			//Deletes cell portal vertices and portals
			if (!rawcells[i]->portals.empty())
			{
				for (s32 j = 0; j < rawcells[i]->n_portals; j++) {
					if (rawcells[i]->portals[j]->verts)
						delete[] rawcells[i]->portals[j]->verts;
				}

				rawcells[i]->portals.clear();
			}
		}
		//Deletes cells
		rawcells.clear();
	}
	//Deletes uv id
	if (uvidxs)
		delete[] uvidxs;
	//Deletes vertex id
	if (vertidxs)
		delete[] vertidxs;
	//Deletes uvpos
	if (uvpos)
		delete[] uvpos;
	//Deletes vertices
	if (verts)
		delete[] verts;

	planes.clear();
	//Stop destroying trees...
	//Deletes trees
	if (tree)
		delete tree;

	projection.clear();
	if (!surfs.empty())
	{
		//Deletes surface shading then surfaces
		for (u32 i = 0; i < n_surfs; i++) {
			if (surfs[i]->hasVertexColors == 1)
				delete[] surfs[i]->shading;
		}

		surfs.clear();
	}
	//Deletes uv postion caller
	if (uvpos_caller)
		delete[] uvpos_caller;
	if (vertpos_caller)
		delete[] vertpos_caller;
	//Deletes uv id caller
	if (faceidxs_caller)
		delete[] faceidxs_caller;
}
int CSolid::calcUniqueTexturesNames()
{
	int ret = 0;
	bool found = false;
	//loop for amount of surfaces
	for (u32 i = 0; i < n_surfs; i++)
	{
		//sets texture name to surface texture
		string texname(surfs[i]->texture);
		found = false;
		//loop for texture sizes
		for (int j = 0; i < textures.size(); i++)
		{
			//checks if textures is equal to texture names
			if (textures[j] == texname)
			{
				found = true;
				break;
			}
		}
		//if not found
		if (!found)
		{
			//create textures with texture name
			textures.push_back(texname);
			ret++;
		}
	}
	return ret;
}

void CSolid::buildBackTree()
{
	os::Printer::log("\tBuilding back tree...");
	uvpos_caller = new core::array<u32>[n_faceidx];
	vertpos_caller = new core::array<u32>[n_faceidx];

	for (u32 i = 0; i < n_faceidx; i++) {
		uvpos_caller[uvidxs[i]].push_back(i);
		vertpos_caller[vertidxs[i]].push_back(i);
	}

	faceidxs_caller = new core::array<u32>[n_faceidx];
	for (u32 i = 0; i < n_surfs; i++) {
		for (u32 j = 0; j < surfs[i]->faceidxlen; j++)
			faceidxs_caller[surfs[i]->faceidxstart + j].push_back(i);
	}
	os::Printer::log("\t\tdone.");
}

cellData_t* CSolid::getBBFromSurf(u16 surfindx, cellData_t* celldata) {
	cellData_t* founddata;
	for (u32 s = 0; s < celldata->n_surfs; s++) {
		if (celldata->surfsidxs[s] == surfindx)
			return celldata;
	}
	for (u32 c = 0; c < celldata->n_children; c++) {
		founddata = getBBFromSurf(surfindx, celldata->children[c]);
		if (founddata)
			return founddata;
	}
	return nullptr;
}

void CSolid::extractSurfaces() {
	os::Printer::log("\tExtracting surfaces into internal...");
	for (u32 i = 0; i < n_surfs; i++) {
		CLocalizedFace face;
		face.si = i;
		// don't call set_used here; reserve capacity instead
		face.verts.set_used(0);
		face.verts.reallocate(surfs[i]->faceidxlen); // if reallocate is available, otherwise rely on push_back

		for (u32 j = 0; j < surfs[i]->faceidxlen; j++) {
			u32 faceidx = surfs[i]->faceidxstart + j;

			// bounds checks

			u32 vindex = vertidxs[faceidx];
			u32 uindex = uvidxs[faceidx];

			localVert_t vert;
			vert.localidx = j;
			vert.pos = verts[vindex];
			vert.uv = uvpos[uindex];
			vert.vertidx = vindex;
			vert.uvidx = uindex;
			vert.faceidx = faceidx;
			vert.parent_si = i;
			vert.hasShading = false;

			if (surfs[i]->hasVertexColors && surfs[i]->shading) {
				size_t shadingSize = 4 * (size_t)surfs[i]->faceidxlen;
				size_t off = (size_t)j * 4;
				if (off + 4 <= shadingSize) {
					vert.hasShading = true;
					memcpy(vert.color, &surfs[i]->shading[off], 4);
				}
				else {
					os::Printer::log("color failed");
				}
			}

			face.verts.push_back(vert);
		}
		local_faces.push_back(face);
	}

	for (u32 i = 0; i < local_faces.size(); i++) {
		for (u32 j = 0; j < local_faces[i].verts.size(); j++) {
			// Get a pointer to the current localized vertex
			localVert_t* vA = &local_faces[i].verts[j];

			// 1. Get all global face indices sharing this UV/Pos point
			core::array<u32>* peer_face_indices = &uvpos_caller[vA->uvidx];

			for (u32 k = 0; k < peer_face_indices->size(); k++) {
				u32 peer_fidx = (*peer_face_indices)[k]; // Use 'k', not 'i'!

				// Skip if this is the same global index we are currently processing
				if (peer_fidx == vA->faceidx) continue;

				// 2. Find which surface(s) own this peer face index
				core::array<u32>* peer_surfs = &faceidxs_caller[peer_fidx];

				for (u32 s = 0; s < peer_surfs->size(); s++) {
					u32 other_si = (*peer_surfs)[s]; // Use 's', not 'j'!

					if (other_si != local_faces[i].si) {
						// 3. Search the other surface for the peer_fidx
						// (NOT vA->faceidx, which only exists on the current surface)
						int other_local_idx = local_faces[other_si].findVertFromGlobal(peer_fidx);

						if (other_local_idx != -1) {
							// Link the pointers
							localVert_t* vB = &local_faces[other_si].verts[other_local_idx];
							vA->shared.push_back(vB);
						}

						// Track neighbor surface ID at the face level
						if (local_faces[i].shared.linear_search(other_si) == -1)
							local_faces[i].shared.push_back(other_si);
					}
				}
			}
		}
	}
	os::Printer::log("\t\tdone.");
}

void CSolid::rebuildSurfaces() {
	core::array < core::vector2df> tmp_uvpos;

	os::Printer::log(std::format("{}", n_uvpos).c_str());

	for (u32 i = 0; i < local_faces.size(); i++) {
		for (u32 j = 0; j < local_faces[i].verts.size(); j++) {
			localVert_t* vA = &local_faces[i].verts[j];

			s32 found_uv_idx = -1;

			verts[vA->vertidx] = vA->pos;

			for (u32 k = 0; k < tmp_uvpos.size(); k++) {

				if (tmp_uvpos[k].equals(vA->uv)) {
					found_uv_idx = k;
					break;
				}
			}

			if (found_uv_idx == -1) {
				found_uv_idx = tmp_uvpos.size();

				tmp_uvpos.push_back(vA->uv);
			}

			uvidxs[vA->faceidx] = found_uv_idx;
		}
	}

	// 5. CRITICAL FIX: Update the actual class members safely
	n_uvpos = tmp_uvpos.size();

	// If uvpos is a raw pointer (vector2df*), we must allocate new memory
	delete[] uvpos; // Clean up old data first!
	uvpos = new core::vector2df[n_uvpos];
	for (u32 i = 0; i < n_uvpos; i++) {
		uvpos[i] = tmp_uvpos[i];
	}

	os::Printer::log(std::format("Rebuilt UVs. New count: {}", n_uvpos).c_str());
}