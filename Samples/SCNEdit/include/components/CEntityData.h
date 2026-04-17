#pragma once
#include "include/core/CScn.h"

class CEntityData : public CRenderMeshData
{
protected:
	int m_indx = -1;
	bool m_firstLoad = false;
	bool selected = false;

public:
	IMeshBuffer* MeshBuffer;
	std::string m_origin = "";

public:
	CEntityData();

	virtual ~CEntityData();

	void initMesh(CEnt* ent);

	inline bool toggleSelect() {
		return selected ? deselect() : select();
	}

	inline bool select() {
		RenderMesh->Materials[0]->changeShader("TextureColor.xml");
		return (selected = true);
	};
	inline bool deselect() {
		if (!str_equals("TextureColor.xml", RenderMesh->Materials[0]->getShaderPath())) {
			RenderMesh->Materials[0]->changeShader("TextureColorAlpha.xml");
			return (selected = false);
		}
		return selected;
	}

	inline int getEntityIndx() {
		return m_indx;
	}

	inline bool getSelected() {
		return selected;
	}
};