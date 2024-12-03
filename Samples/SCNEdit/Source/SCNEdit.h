#pragma once

#include "IApplicationEventReceiver.h"
#include <SkylichtEngine.h>
#include "CScnArguments.h"
#include "CScn.h"
#include "CScnMeshComponent.h"

class SCNEdit : public IApplicationEventReceiver
{
private:
	CScnArguments* arguments = nullptr;
	inline static std::ofstream* output=0;
	inline static CScn* scn;
protected:
	CDeferredRP* m_rendering;
	CCamera* m_camera;
	CScene* m_scene;
	CCamera* m_guiCamera;
	bool m_bakeSHLighting;

	std::vector<CGameObject*> m_meshes;
public:
	SCNEdit(CScnArguments*);
	virtual ~SCNEdit();

	virtual void onUpdate();

	virtual void onRender();

	virtual void onPostRender();

	virtual void onResume();

	virtual void onPause();

	virtual bool onBack();

	virtual void onResize(int w, int h);

	virtual void onInitApp();

	virtual void onQuitApp();

	static bool loadScnFile(io::path fname);

	static CScn* getSCN() {
		return scn;
	}

	static std::ofstream* getOutput() {
		return output;
	}
	
};