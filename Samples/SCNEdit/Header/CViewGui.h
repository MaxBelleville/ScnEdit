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
	CCanvas* m_canvas;
	static inline UI::CUIContainer* m_uiContainer;
	static inline UI::CUITextBox* m_textSection1;
	static inline UI::CUITextBox* m_textSection2;
	static inline UI::CUITextBox* m_textSection3a;
	static inline UI::CUITextBox* m_textSection3b;
	static inline UI::CUITextBox* m_textSection3c;
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

	void drawTooltip();
	void openFile();
	void openTextures();
	void saveFile();
	void exportFile();
	void closeFile();
	void quit();
	void onGUI();
	static void updateSections();
	static void resetSections();
	static void updateVertex();
	UI::CUITextBox* addTextbox(const char* textPath);
	UI::CUIButton* addButton(const char* btnPath);
	void addTooltip(UI::CUIButton*, std::string, std::string);
};
