#pragma once

#include "IApplicationEventReceiver.h"
#include <SkylichtEngine.h>
#include "io/CArguments.h"
#include "core/CScn.h"
#include "components/CScnMeshComponent.h"
#include "components/CPortalComponent.h"
#include "components/CCellBBComponent.h"
#include "components/CEntityComponent.h"
#include "Primitive/CCube.h"
#include "Primitive/CSphere.h"
#include "io/export.h"

class SCNEdit : public IApplicationEventReceiver
{
private:
	CArguments* m_arguments = nullptr;
	inline static io::path outputPath = "";
	inline static CScn* scn;
protected:

public:
	SCNEdit(CArguments*);
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

	static bool saveSCN();
	static inline void exportSCN() {
		scnExportObj(scn, "exported");
		scnExport3ds(scn, "exported");
		scnExportMap(scn, "exported");
	}

	inline static CScn* getSCN() {
		return scn;
	}

};