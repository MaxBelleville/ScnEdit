#pragma once
#include "Header/Base/CScn.h"
#include "Header/Base/util.h"
#include <set>

class CScnEntityData : public CRenderMeshData
{
protected:
	int m_indx;
	bool m_firstLoad=false;
public:
	IMeshBuffer *MeshBuffer;
	std::string m_origin = "";

public:
	CScnEntityData();

	virtual ~CScnEntityData();

	void initMesh(CScnEnt* ent);

	inline void select() {
		RenderMesh->Materials[0]->changeShader("TextureColor.xml");
	};
	inline void deselect() {
		if (!str_equals("TextureColor.xml", RenderMesh->Materials[0]->getShaderPath()))
			RenderMesh->Materials[0]->changeShader("TextureColorAlpha.xml");
	}

	inline int getEntityIndx() {
		return m_indx;
	}

};