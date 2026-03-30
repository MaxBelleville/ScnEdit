#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnPortalData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;
	portalSelect_t portaldata = portalSelect_t(-1,-1);
public:
	CScnPortalData();

	virtual ~CScnPortalData();

	void initMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);

	inline void select() {

		RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColor.xml");

	}

	inline void deselect() {

		if (!str_equals("BuiltIn/Shader/Basic/VertexColorAlpha.xml", RenderMesh->Materials[0]->getShaderPath())) {
			RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColorAlpha.xml");
		}

	}


};