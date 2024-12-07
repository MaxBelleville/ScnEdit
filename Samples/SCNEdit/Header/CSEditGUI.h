
#include "Header/main.h"
#if false
using namespace irr;
using namespace core;

extern bool uvmove;
//IMPORTANT need to add the u and v to gui so I can display it or just add texture x and texture y
class CSEditGUI
{
	private:
		IGUIEnvironment* env;
		IGUIElement* root;
		bool uvresize;
		f32 uvm;
		u32 resx, resy, desky, deskx;
		bool isFullscreen=false;
		bool isBorderless = false;
		bool isDebuggerOpen = false;
		wchar_t surf[128];
		int textOffset = 0;
		core::stringw log = L"";
		core::stringw tmpLog = L"";
		int nOfLines = 100;
		int currLine = 0;
		core::stringw prevLog = L"";
		core::array<core::stringw> lines;
		IGUIStaticText* ge_uvgrid;
		IGUIStaticText* ge_surfs;
		IGUIStaticText* ge_pos;
		IGUIStaticText* ge_data;
		IGUIStaticText* ge_data2;
		IGUIStaticText* ge_black;
		IGUIStaticText* ge_crosshair;
		IGUIStaticText* ge_fps;
		IGUIStaticText* ge_crosshair2;

		IGUIStaticText* ge_debug_back;
		IGUIStaticText* ge_loading;
		IGUIEditBox* ge_debug_panel;
		IGUIStaticText* ge_pages;
		IGUIButton* ge_lightmap;
		IGUIButton* ge_strip;
		IGUIButton* ge_debug;
		IGUIButton* ge_page_left;
		IGUIButton* ge_page_right;
		bool isDebuggerEnabled = false;

		void buildGui()
		{
		   
			//Creates gui skin
			IGUISkin* skin = env->getSkin();
			for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
			{
				//sets color
				video::SColor col = skin->getColor((gui::EGUI_DEFAULT_COLOR)i);
				//sets alpha of color
				col.setAlpha(255);
				//Sets skin color
				skin->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
			}
		//Adds buttons that will be used in the event receiver
			env->addButton(rect<s32>(10,10,60,26), 0, 101, L"Open", L"Opens a file");
			env->addButton(rect<s32>(70,10,120,26), 0, 102, L"Close", L"Close current file");
			env->addButton(rect<s32>(130,10,180,26), 0, 103, L"Save", L"Saves the changes to the file");
			env->addButton(rect<s32>(190, 10, 240, 26), 0, 199, L"Quit", L"Exits Program");

			ge_lightmap = env->addButton(rect<s32>(resx-230, 10, resx-130, 26), 0, 104, L"Hide Lightmaps", L"Toggle lightmaps visiblity/loading");
			ge_lightmap->setVisible(false);
			ge_strip= env->addButton(rect<s32>(resx-120, 10, resx-10, 26), 0, 105, L"Strip Lightmaps", L"Toggle if the light maps are stripped when you save");
			ge_strip->setVisible(false);
			ge_debug = env->addButton(rect<s32>(10, 36, 100, 52), 0, 106, L"Show Logs", L"Toggle debug panel");

			
			nOfLines = (resy-80-70) / 14;
			//u32 x,y,lx,ly;

			//ge_uvgrid    = env->addStaticText(L"\tMove, Grid size: 0.01",rect<s32>(resx-200, resy-15, resx, resy),1,0,0,301,1);
			//ge_surfs  = env->addStaticText(L"", rect<s32>(0, resy-15, resx-200, resy),1,0,0,302,1);
			//ge_pos    = env->addStaticText(L"",rect<s32>(resx*b,resy-15,resx,resy),1,0,0,303,1);

			f32 a = 0.35f, b=0.55f;

			//sets displays surfaces selected, uv grid and postion boxes
			u32 offset = 0;
			if (!isFullscreen && !isBorderless && resy>=desky-48 && resx >= deskx) {
				offset = max(0, (resy-desky)+43);
			}
			else if (!isFullscreen && !isBorderless && resy >= desky - 100) {
				offset = max(0, (resy - desky) + 100);
			}
			if (isBorderless && resx >= deskx &&resy >= desky) {
				offset = 0;
			}
			else if (isBorderless && resy >= desky - 58) {
				offset = max(0, (resy - desky) + 63);
			}
			else if (isBorderless && resy >= desky - 100) {
				offset = max(0, (resy - desky) + 80);
			}

	   
			ge_page_left= env->addButton(rect<s32>(10, resy - 68 - offset, (resx / 16), resy - 54 - offset), 0, 107, L"<", L"Decrease page");
			ge_page_right=env->addButton(rect<s32>((resx / 3), resy - 68 - offset, (resx / 2.5), resy - 54 - offset), 0, 108, L">", L"Increase page");
			ge_page_left->setVisible(false);
			ge_page_right->setVisible(false);

			ge_fps = env->addStaticText(L"FPS -", rect<s32>(250, 9, 300, 25));
			ge_fps->setBackgroundColor(video::SColor(200, 200, 200, 200));
			ge_fps->setDrawBorder(true);
			ge_fps->setVisible(false);
			ge_crosshair = env->addStaticText(L"", rect<s32>(resx / 2 - 4, resy/2-1 , resx / 2 + 4, resy / 2 + 1));
			ge_crosshair->setBackgroundColor(video::SColor(200, 50, 50, 50));
			ge_crosshair2 = env->addStaticText(L"", rect<s32>(resx / 2 - 1, resy / 2 - 4, resx / 2 + 1, resy / 2 + 4));
		  

			ge_crosshair2->setBackgroundColor(video::SColor(200, 50, 50, 50));
			ge_surfs  = env->addStaticText(L"", rect<s32>(0, resy-15- offset, resx*a, resy- offset),1,0,0,302,1);
			ge_uvgrid = env->addStaticText(L"\tUV Move, Grid size: 0.01",rect<s32>(resx*a, resy-15- offset, resx*b, resy- offset),1,0,0,301,1);
			ge_pos    = env->addStaticText(L"",rect<s32>(resx*b,resy-15- offset,resx+1,resy- offset),1,0,0,303,1);
			ge_data   = env->addStaticText(L"",rect<s32>(0,resy-45- offset,resx,resy-30- offset));

			ge_loading = env->addStaticText(L"Loading...", rect<s32>(resx/2-100, resy/2-25, resx / 2+100, resy/2+25));
			ge_loading->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
			ge_loading->setDrawBorder(true);
			ge_loading->setBackgroundColor(video::SColor(255, 0, 0, 50));
			ge_loading->setOverrideColor(video::SColor(255, 255, 255, 255));
			ge_loading->setVisible(false);

			ge_data2   = env->addStaticText(L"",rect<s32>(0,resy-30- offset,resx,resy-15- offset));
			ge_debug_back = env->addStaticText(L"", rect<s32>(10, 62, resx / 2.5, resy - 80 - offset));
			ge_debug_back->setBackgroundColor(video::SColor(175, 255, 255, 255));
	   
			ge_debug_back->setVisible(false);
			ge_debug_panel = env->addEditBox(L"", rect<s32>(10, 62, resx/2.5, resy - 80 - offset),false,0,404);
			ge_debug_panel->setAutoScroll(true);
			ge_debug_panel->setMultiLine(true);
			ge_debug_panel->setTextAlignment(EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
			ge_debug_panel->setWordWrap(false);
			ge_debug_panel->setDrawBackground(false);
			ge_debug_panel->setDrawBorder(true);



			ge_debug_panel->setWordWrap(false);
			ge_debug_panel->setVisible(false);

			ge_pages = env->addStaticText(L"0 of 0", rect<s32>(10+(resx / 16), resy - 70 - offset, resx / 3 - 10, resy - 54 - offset));
			ge_pages->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
			ge_pages->setBackgroundColor(video::SColor(175, 200, 200, 200));
			ge_pages->setWordWrap(false);
			ge_pages->setDrawBorder(true);
			ge_pages->setVisible(false);

			ge_black   = env->addStaticText(L"",rect<s32>(0,resy-offset,resx, resy));
			ge_black->setBackgroundColor(video::SColor(255,255,255,255));
		  
			IGUIFont* font = env->getFont("fonts/font.xml"); //load font from swat3 :)
			skin->setFont(font);
			/*if (font)
			{
			   ge_uvgrid->setOverrideFont(font);
			   ge_redsurfs->setOverrideFont(font);
			   ge_bluesurfs->setOverrideFont(font);
			}*/
		  


		}
		//updates grid
		void uvgrid_update()
		{
			wchar_t text[128];
			wchar_t op[32];

			if (uvresize)
			{
				//Displays if its in the resize option
				swprintf_irr (op,L"UV Resize");
				uvmove=false;
			}
			else
			{
				//Displays if its in the move option
				swprintf_irr (op,L"UV Move");
				uvmove=true;
			}
			//Displays gride size
			swprintf_irr (text,L"\t%s. Grid size: %4.3f",op,uvm);

			root->getElementFromId(301)->setText(text);
		}

	public:
		CSEditGUI(IrrlichtDevice * device, bool r, f32 m,bool border, bool full)
		{
			//MOAR VARIABLES
			env = device->getGUIEnvironment();
			root = env->getRootGUIElement();
			uvresize = r;
			uvm = m;
			//set render size
			dimension2d<u32> res = device->getVideoDriver()->getCurrentRenderTargetSize();
			dimension2d<u32> deskres= device->getVideoModeList()->getDesktopResolution();
			isFullscreen = full;
			isBorderless = border;
			//sets resolution size
			resx = res.Width;
			resy = res.Height;
			desky = deskres.Height;
			deskx = deskres.Width;
			//sets Gui size
			buildGui();

		}
		//Creates gui
		IGUIEnvironment * getIrrGUIEnv()
		{
			return env;
		}
		//old version, displays selected surfaces indexes
		/*void surfs_update(CSelector * sel)
		{
			if (!sel) return;

			//update red, blue surfs text ***********************************
			wchar_t text1[256] = L"Selected Surfs";
			wchar_t text2[256] = L"Shared Surfs";
			array<u32>& rs = sel->getRedSurfs();
			array<u32>& bs = sel->getBlueSurfs();

			swprintf_irr (text1,L"%ls (%d):",text1,rs.size());
			swprintf_irr (text2,L"%ls (%d):",text2,bs.size());

			for (u32 i=0; i<rs.size(); i++)
				swprintf_irr (text1,L"%ls %d", text1, rs[i]);
			for (u32 i=0; i<bs.size(); i++)
				swprintf_irr (text2,L"%ls %d", text2, bs[i]);

			ge_redsurfs->setText(text1);
			ge_bluesurfs->setText(text2);
		}*/
		bool surfs_update(CSelector * sel,u32 meshIdx)
		{
			//If nothing is selected exit loop
			if (!sel) return false;
   
			//Displays red surfaces selected and blue surafaces selected
			 if(sel->getRedSurfs(meshIdx).size()!=0||sel->getBlueSurfs(meshIdx).size()!=0){
				 addLog(sel->getLog());
				 textOffset= swprintf_irr (surf,L"\tSurfaces: Selected (%d) Shared (%d)",sel->getRedSurfs(meshIdx).size(),sel->getBlueSurfs(meshIdx).size());

				ge_surfs->setText(surf);
				return true;
			 }
			 return false;
		}
		void surf_notsel() {
			wchar_t text[128];
			swprintf_irr (surf,L"");
			textOffset = 0;
		   swprintf_irr (text, L"\tNo Surface Selected");
			ge_surfs->setText(text);
		}

		void uvgrid_setMult(f32 m)
		{
			//sets uv move
			uvm = m;
			uvgrid_update();
		}
		void uvgrid_setOp(bool r)
		{
			//sets uv resize
			uvresize = r;
			uvgrid_update();
		}
		void update_index(i_point3f vert)
		{
			if (!(vert.X == 0 && vert.Y == 0 && vert.Z == 0)) {
			   swprintf_irr (surf + textOffset, 256, L" vertex = %.1f %.1f %.1f", vert.X, vert.Y, vert.Z);
				ge_surfs->setText(surf);
			}
		}

		void pos_update(f32 x, f32 y, f32 z, scnRawCell_t cell)
		{
			wchar_t text[256];
			//Displays postion after it has been updated
			//SAY("Where Am I? (x y z) %10.0f %10.0f %10.0f\n",x,y,z);
		  
		   swprintf_irr (text,L"Position: (x) %.0f (y) %.0f (z) %.0f, Cell: %S",x,y,z, cell.name);
			ge_pos->setText(text);
		}

		void pos_update(f32 x, f32 y, f32 z)
		{
			wchar_t text[256];
			//Displays postion after it has been updated
			//SAY("Where Am I? (x y z) %10.0f %10.0f %10.0f\n",x,y,z);
		   swprintf_irr (text, L"Position: (x) %.0f (y) %.0f (z) %.0f", x, y, z);
			ge_pos->setText(text);
		}
		void displayUV()
		{

		wchar_t text[256];

	   swprintf_irr (text,L"UV for current surface has been reset.");
		ge_pos->setText(text);
		}
		void infoBox(bool turnOn, u16 value, u32 si, scnSurf_t * sd, scnPlane_t * planei, i_point3f vert={0,0,0}) {
		if (turnOn == true) {
		wchar_t text[256];

		ge_data->setBackgroundColor(video::SColor(255,200,200,200));
		ge_data->setDrawBorder(true);
		ge_data2->setBackgroundColor(video::SColor(255,200,200,200));
		ge_data2->setDrawBorder(true);
		if(value==0) {
	   swprintf_irr (text,L"");
		ge_data->setText(text);
	   swprintf_irr (text,L"");
		ge_data2->setText(text);
		}
		if(value==1) {
	   swprintf_irr (text,L"| surf[%d]: | texture = %S | flags = %uh | alpha = %uh | lighmap size = %ux%u |  texture size = %ux%u |",si,sd->texture,sd->flags,sd->alpha,sd->lmsize_h,sd->lmsize_v,sd->height,sd->width);
		ge_data->setText(text);
	   swprintf_irr (text,L"");
		ge_data2->setText(text);
		}
		if(value==2) {
		swprintf_irr (text,L"| surf[%d]: | texture = %S | flags = %uh | alpha = %uh | lighmap size = %ux%u | texture size = %ux%u |",si,sd->texture,sd->flags,sd->alpha,sd->lmsize_h,sd->lmsize_v,sd->height,sd->width);
		ge_data->setText(text);
		
	   swprintf_irr (text,L"| verts idx idx = from %d to %d | plane idx = %u | normal = (%.1f, %.1f, %.1f), d=%.1f | shading = %u |", sd->vertidxstart, sd->vertidxstart + sd->vertidxlen, sd->planeidx, planei->a,planei->b,planei->c,planei->d,sd->hasVertexColors);
		ge_data2->setText(text);
  
		
		if (!(vert.X == 0 && vert.Y == 0 && vert.Z == 0)) {
		   swprintf_irr (surf + textOffset, 256, L" vertex = %.1f %.1f %.1f", vert.X, vert.Y, vert.Z);
			ge_surfs->setText(surf);
		}
	   // env->drawAll();
		}
		if(value==3) {
		   swprintf_irr (text,L"Changes saved to file.");
		ge_data->setText(text);
	   swprintf_irr (text,L"");
		ge_data2->setText(text);
		}
		}
		}
		void askForInput(u16 limit) {
		wchar_t text[128];
	   swprintf_irr (text,L"Please enter a number from 0 to %u:_",limit);
		ge_data->setText(text);
	   swprintf_irr (text,L"");
		ge_data2->setText(text);
		}

		void movementSpeed(f32 s) {
			wchar_t text[128];
		   swprintf_irr (text, L"Updated movement speed to %.4f", s);
			ge_data->setText(text);
		   swprintf_irr (text, L"");
			ge_data2->setText(text);
		}
		void displayValue(const char* str, u16 value) {
		wchar_t text[128];
	   swprintf_irr (text,L"%S for that surface is now: %u",str, value);
		ge_pos->setText(text);
	   swprintf_irr (text, L"");
		ge_data->setText(text);
		}
		void displayUpdate(u16 alpha,u16 limit) {
				wchar_t text[128];
	   swprintf_irr (text,L"Please enter a number from 0 to %u: %u",limit,alpha);
		ge_data->setText(text);
	   swprintf_irr (text,L"");
		ge_data2->setText(text);
		}

		void entity_sel(const char* entity_class) {
			wchar_t text[128];
		   swprintf_irr (text, L"\t Entity Selected: %S", entity_class);
			ge_surfs->setText(text);
		}
		void entity_details(const char* first, const char* second) {
			wchar_t text[256];
			swprintf_irr (text, L"\t %S", first);
			ge_data->setText(text);
			swprintf_irr (text, L"\t %S", second);
			ge_data2->setText(text);
		}
		void update_origin(const char* org) {
			wchar_t text[256];
		   swprintf_irr (text, L"\t Moved Origin To: %S", org);
			ge_data->setText(text);
		   swprintf_irr (text, L"");
			ge_data2->setText(text);
		}
		void addLoading() {
			ge_loading->setVisible(true);
		}
		void removeLoading() {
			ge_loading->setVisible(false);
		}
		void showLightmapButton(bool show) {
			ge_lightmap->setVisible(show);
		}
		void showStripLightmapButton(bool show) {
			ge_strip->setVisible(show);
		}
		void openDebug() {
			isDebuggerOpen = !isDebuggerOpen;
			ge_debug_panel->setVisible(isDebuggerOpen);
			ge_page_left->setVisible(isDebuggerOpen);
			ge_page_right->setVisible(isDebuggerOpen);
			ge_pages->setVisible(isDebuggerOpen);
			ge_debug_back->setVisible(isDebuggerOpen);
			if (isDebuggerOpen) ge_debug->setText(L"Hide Logs");
			else ge_debug->setText(L"Show Logs");
		}

		void enableDebugger(bool enable) {
			isDebuggerEnabled = enable;
			ge_debug->setVisible(enable);
		}
		void setFPS(int fps) {
			if (!ge_fps->isVisible()) ge_fps->setVisible(true);
			wchar_t text[128];
			swprintf_irr (text, L"FPS: %i", fps);
			ge_fps->setText(text);
		}
		void updateLog(bool init) {
			if (isDebuggerEnabled) {
				if (prevLog != log) {
					if (init) {
						currLine = (int)lines.size() - nOfLines;
					}
					setDebugText();
					prevLog = log;
				}
			}
		}

		void setDebugText() {
		  
			if ((int)lines.size() > 0) {
				core:stringw lineCollector = L"";
				for (int p = currLine; p < (int)lines.size() && p < currLine + nOfLines; p++) {
					lineCollector += lines[p] + L"\n";
				}
			   
				ge_debug_panel->setText(lineCollector.c_str());
			}
			int end = (int)lines.size() / nOfLines;
			if (end == 0) end++;
			int curr = currLine/ nOfLines;
			wchar_t text[128];
			swprintf_irr (text, L"%i of %i", curr,end);
			ge_pages->setText(text);
		}
		
		void addLog(core::stringw newStr) {
			if (isDebuggerEnabled) {
				log += newStr;
				tmpLog += newStr;
				if (tmpLog.find("\n") != -1) {
					core::array<core::stringw> tmpLines;
					int size = tmpLog.split(tmpLines, L"\n",1,false);
					tmpLog = L"";
					for (u32 i = 0; i < size; i++) {
						lines.push_back(tmpLines[i]);
					}
					if (currLine + nOfLines + (nOfLines - 1) > (int)lines.size() && (int)lines.size() - nOfLines>0) {
						currLine = (int)lines.size() - nOfLines;
					}
				}
			}
			
		   // ge_debug_panel->setText(log.c_str());
		}
		void setLog(core::stringw setStr) {
			if (isDebuggerEnabled) {
				log = setStr;
				nOfLines = setStr.split(lines, L"\n");
				currLine = (int)lines.size() - nOfLines;
			}
		}
		void clearLog(core::stringw setStr) {
			if (isDebuggerEnabled) {
				log = L"";
				tmpLog = L"";
				//ge_debug_panel->setText(log.c_str());
				lines.clear();
				currLine = 0;
			}
		}

		void changePage(bool increase) {
			if ((int)lines.size()>0) {
				if (increase && currLine + nOfLines< (int)lines.size()) {
					currLine += nOfLines;
				}
				else if (increase && currLine + nOfLines >= (int)lines.size() && (int)lines.size() - nOfLines > 0) {
					currLine = (int)lines.size()-nOfLines;
				}
				else if (!increase && currLine - nOfLines >= 0) {
					currLine -= nOfLines;
				}
				else if (!increase && currLine - nOfLines < 0){
					currLine = 0;
				}
				setDebugText();
			}
		}
};

#endif