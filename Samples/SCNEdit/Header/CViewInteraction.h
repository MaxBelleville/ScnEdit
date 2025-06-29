#pragma once

#include "SkylichtEngine.h"
#include "Managers/CView.h"
#include "CScnArguments.h"
#include "Header/Managers/CViewManager.h"
#include "Collision/CCollisionManager.h"

class CViewInteraction :
	public CView
{
protected:
	CScnArguments* m_arguments;

	static inline bool m_rebuildRequired = false;
	static inline core::array<CGameObject*> m_hideSolids;
	static inline core::array<CGameObject*> m_hidePortal;
	static inline core::array<CGameObject*> m_hideEntity;
public:
	CViewInteraction(CScnArguments* args);

	virtual ~CViewInteraction();

	virtual void onInit();

	virtual void onDestroy();


	virtual void onUpdate();

	virtual void onRender();

	virtual void onPostRender();


protected:
	static void updateVisbility(CGameObject* obj, bool state, bool bbCollision);
	static void checkVisbility();
	static void deselectAll(CGameObject* current);
	static void updateEntityPos(core::vector3df);
	static void resetSolid();
	static void getClosestVertex(core::vector3df);
	static void getSelectedVertex();
};