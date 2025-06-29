#pragma once

#include "IApplicationEventReceiver.h"
#include <SkylichtEngine.h>
#include "CScnArguments.h"
#include "Base/CScn.h"
#include "Components/CScnMeshComponent.h"
#include "Components/CScnPortalComponent.h"
#include "Components/CScnCellBBComponent.h"
#include "Components/CScnEntityComponent.h"
#include "Primitive/CCube.h"
#include "Primitive/CSphere.h"
#include "Base/export.h"




class SCNEdit : public IApplicationEventReceiver
{
private:
	CScnArguments* m_arguments = nullptr;
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

	inline static void closeScnFile() {
		if (scn) {
			delete scn;
			scn = 0;
		}
	}

	static bool loadScnFile(io::path fname);

	static void proccessQuit();

	static bool saveSCN(io::path path, bool bExtra);
	static inline void exportSCN(bool bExtra) {
		scnExportObj(scn, "exported");
		scnExport3ds(scn->getAllSolids(), scn->getSolidSize(bExtra), "exported");
		scnExportMap(scn, "exported");
	}
	inline static CScn* getSCN() {
		return scn;
	}

	inline static std::ofstream* getOutput() {
		return output;
	}

};