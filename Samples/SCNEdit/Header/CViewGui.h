#pragma once
#include "SkylichtEngine.h"
#include "Managers/CView.h" 
#include "Header/Managers/CViewManager.h"
#include "UserInterface/CUIContainer.h"
#include "UserInterface/CUIListView.h"
#include "UserInterface/CUIGridView.h"
#include "UserInterface/CUIButton.h"
#include "UserInterface/CUITextBox.h"
#include "CScnArguments.h"
#include <imguial_msgbox.h>
#include <FileDialog/ImGuiFileDialog.h>

struct InfoText {
	std::string section1;
	std::string section2;
	std::string section3;
};


class CViewGui : public CView
{
protected:
	CScnArguments* m_arguments;

	static inline std::string m_tooltip = ""; //Exclusive to ui so be private
	static inline CCanvas* m_canvas = NULL;
	static inline UI::CUIContainer* m_uiContainer;
	static inline UI::CUITextBox* m_textSection1;
	static inline UI::CUITextBox* m_textSection2;
	static inline UI::CUITextBox* m_textSection3a;
	static inline UI::CUITextBox* m_textSection3b;
	static inline UI::CUITextBox* m_textSection3c;
	static inline bool isSection1Set = false;
	static inline bool isSection2Set = false;
	static inline bool savePressed = false;
	static inline ImGuiAl::MsgBox m_msgbox;

	static inline UI::CUIButton* m_quitButton;
	static inline UI::CUIButton* m_saveButton;
	static inline UI::CUIButton* m_openButton;
	static inline UI::CUIButton* m_closeButton;
	static inline UI::CUIButton* m_exportButton;

	u32 m_missingItems = 0;
public:
	CViewGui(CScnArguments* args);

	virtual ~CViewGui();

	virtual void onInit();

	virtual void onData();

	virtual void onDestroy();

	virtual void onUpdate();

	virtual void onRender();

	virtual void onPostRender();

protected:
	void setupEvents();

	void openFile();
	void openTextures();
	void scrapeLight();
	void saveFile();
	void helpDialog();
	void openLogger();
	void closeFile();
	void quit();
	void onGUI();
	void drawTooltip();
	static void updateSections(bool isViInfo = false);
	static void printSections(bool isViInfo = false);
	static InfoText getSections(bool isViInfo = false);
	//The 'nuclear option' is for mass editing all details of a object
	static InfoText getSolidInfo(bool isViInfo = false);
	static InfoText getPortalInfo();
	static InfoText getEntityInfo();
	static void resetEditText(bool isViInfo=false);
	static UI::CUITextBox* addTextbox(const char* textPath);
	static UI::CUIButton* addButton(const char* btnPath);
	void addTooltip(UI::CUIButton*, std::string, std::string);
};
