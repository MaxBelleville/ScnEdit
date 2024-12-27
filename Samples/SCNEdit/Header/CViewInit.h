#pragma once
#include "SkylichtEngine.h"
#include "Decal/CDecals.h"
#include "RenderPipeline/CDeferredSimpleRP.h"
#include "SkyBox/CSkyBox.h"
#include "SCNEdit.h"
#include "Managers/CView.h"
#include "Header/Managers/CViewManager.h"

class CViewInit : public CView
{
public:
	enum EInitState
	{
		Start,
		ReadScn,
		InitScene,
		BuildScnComponents,
		BuildLightmap,
		Error,
		Finished
	};

protected:

	EInitState m_initState;
	CScnArguments* m_arguments;
	CGameObject* m_guiObject;
	CCamera* m_guiCamera;
	CCamera* m_camera;
	CGameObject* m_skyObject;
	CSkyBox* m_skyBox;
	CGlyphFont* m_font;
	CGUIText* m_textInfo;
	u32 m_start;
	core::array<CDecals*> m_decals;
protected:
	io::path getBuiltInPath(const char* name);


public:
	CViewInit(CScnArguments* args);

	virtual ~CViewInit();

	virtual void onInit();

	virtual void onDestroy();

	virtual void onUpdate();

	virtual void onRender();

	void buildScnComponents();


protected:

	void initScene();
};
