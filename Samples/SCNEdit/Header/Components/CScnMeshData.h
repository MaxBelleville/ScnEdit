#pragma once
#include "Header/Base/CScn.h"
#include "Header/CScnArguments.h"
#include "Header/Base/util.h"
#include <set>
#include <future>
#include <thread>


class CScnMeshData : public CRenderMeshData
{
public:
	IMeshBuffer *MeshBuffer;

public:
	CScnMeshData();

	virtual ~CScnMeshData();

	void initMesh(CScn* scn, CScnArguments*);

	
};