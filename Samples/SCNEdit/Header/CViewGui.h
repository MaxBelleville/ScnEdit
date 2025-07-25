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
#include "msgbox/imguial_msgbox.h"

class CViewGui : public CView
{
protected:
	CScnArguments* m_arguments;

	static inline ImGuiAl::MsgBox m_msgbox;   //Shared.
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
	void saveFile();
	void helpDialog();
	void exportFile();
	void closeFile();
	void quit();
	void onGUI();
	void drawTooltip();
	static void updateSections();
	//The 'nuclear option' is for mass editing all details of a object
	static void updateSolidInfo(); 
	static void updatePortalInfo();
	static void updateEntityInfo();
	static void updateUVInfo();
	static void resetEditText();
	static UI::CUITextBox* addTextbox(const char* textPath);
	static UI::CUIButton* addButton(const char* btnPath);
	void addTooltip(UI::CUIButton*, std::string, std::string);
};
