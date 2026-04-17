#pragma once

#include "SkylichtEngine.h"
#include "managers/CView.h"
#include "io/CArguments.h"
#include "managers/CViewManager.h"
#include "Collision/CCollisionManager.h"
#include "include/core/scntypes.h"

class CViewInteraction :
	public CView
{
protected:
	CArguments* m_arguments;

	static inline bool m_rebuildRequired = false;
	static inline core::array<CGameObject*> m_hideSolids;
	static inline core::array<CGameObject*> m_hidePortal;
	static inline core::array<CGameObject*> m_hideEntity;
public:
	CViewInteraction(CArguments* args);

	virtual ~CViewInteraction();

	virtual void onInit();

	virtual void onDestroy();

	virtual void onUpdate();

	virtual void onRender();

	virtual void onPostRender();

protected:
	static void moveVertNBounds(bool reset);
	static void updateObjectVisbility(CGameObject* obj, bool state, bool bbCollision);
	static void updateVisbility();
	static void deselectAll(CGameObject* current);
	static void updateEntityPos(core::vector3df);
	static void resetSolid();
	static void updateSelectedVertCube(core::vector3df, bool isSelected);
	static void updateSurfVertCubes();
	static void collectSurfs(core::array<surfaceBox_t> surfsels);
};