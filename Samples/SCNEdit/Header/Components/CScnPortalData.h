#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnPortalData : public CRenderMeshData
{

protected:
	portalBox_t portaldata = portalBox_t(-1, -1);
	bool selected = false;

public:
	IMeshBuffer *MeshBuffer;
	
public:
	CScnPortalData();

	virtual ~CScnPortalData();

	void initMesh(CScnSolid* solid, u32 cellindx, s32 portalindx);

	inline bool toggleSelect() {
		return selected ? deselect() : select();
	}

	inline bool select() {
		RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColor.xml");
		return (selected = true);
	}

	inline bool deselect() {
		if (!str_equals("BuiltIn/Shader/Basic/VertexColorAlpha.xml", RenderMesh->Materials[0]->getShaderPath())) {
			RenderMesh->Materials[0]->changeShader("BuiltIn/Shader/Basic/VertexColorAlpha.xml");
			return (selected = false);
		}
		return selected;
	}

	inline portalBox_t getPortalData() {
		return portaldata;
	}

	inline bool getSelected() {
		return selected;
	}
};