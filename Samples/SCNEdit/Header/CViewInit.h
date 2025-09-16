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
		Unload,
		Start,
		ReadScn,
		InitScene,
		BuildScnComponents,
		BuildLightmap,
		Error,
		Finished
	};

protected:

	EInitState m_initState = CViewInit::Start;

	CScnArguments* m_arguments;
	u32 m_start;

	CGameObject* m_guiObject = NULL;
	CCamera* m_guiCamera = NULL;
	CCamera* m_camera = NULL;
	CGameObject* m_skyObject = NULL;
	CSkyBox* m_skyBox = NULL;
	CGlyphFont* m_font = NULL;
	CGUIText* m_textInfo = NULL;
	core::array<CDecals*> m_decals = NULL;

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
	template<class T>
	void initShapeCollection(SColor, CContainerObject* parent, const char*);

protected:

	void initScene();
};
