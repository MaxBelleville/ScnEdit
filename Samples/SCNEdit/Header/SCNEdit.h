#pragma once

#include "IApplicationEventReceiver.h"
#include <SkylichtEngine.h>
#include "Header/CScnArguments.h"
#include "Header/Base/CScn.h"
#include "Header/Components/CScnMeshComponent.h"
#include "Header/Components/CScnPortalComponent.h"
#include "Header/Components/CScnCellBBComponent.h"
#include "Header/Components/CScnEntityComponent.h"
#include "Header/Context/CContext.h"
#include "Header/ViewManager/CViewManager.h"

#include "Header/CViewInit.h"

class SCNEdit : public IApplicationEventReceiver
{
private:
	CScnArguments* arguments = nullptr;
	inline static std::ofstream* output=0;
	inline static CScn* scn;
protected:

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