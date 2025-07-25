#pragma once

#include "Base/util.h"



class CScnArguments {
	public:
		
		inline CScnArguments(const std::vector<std::string>& argv) {
			m_Deskres.Width = GetSystemMetrics(SM_CXSCREEN);
			m_Deskres.Height = GetSystemMetrics(SM_CYSCREEN);
			//Assigns BaseDirectory
			m_BaseDirectory = new wchar_t[256];
			//Assigns Current Directory to the 256 bit Base
			int ret = GetCurrentDirectory(256, m_BaseDirectory);
			//Error message
			if (!ret || ret > 256)
				error(true, "GetCurrentDirectory failed. {}", ret);
			for (u32 i = 0; i < argv.size(); i++) {
				if (argv[i] == " " || argv[i] == "") {}
				else if (argv[i] == "-brief" ||
					argv[i] == "-br") {
				}
				else if (argv[i] == "-portal" ||
					argv[i] == "-p") {
				}
				else if (argv[i] == "-directx" ||
					argv[i] == "-dx") {
					m_DrivType = EDT_DIRECT3D11;
				}
				else if (argv[i] == "-resize") {
					bResizeable = true;
				}
				else if (argv[i] == "-res") {
					const char* tmpX = argv[++i].c_str();
					const char* tmpY = argv[++i].c_str();
					if (is_number(tmpX) && is_number(tmpY)) {
						m_Res = core::dimension2du(atoi(tmpX), atoi(tmpY));
						bResDefault = false;
					}
					else {
						printf("Resolution width and or height is not a number...\n");
						system("pause");
						exit(1337);
					}
				}
				else if (argv[i] == "-fullscreen" ||
					argv[i] == "-f") {
					bFullscreen = true;
				}
				else if (argv[i] == "-all") {
					bVertInfo = true;
					bLoadInbuiltDebugger = true;
					bDebug = false;
					m_Alpha = 125;
				}
				else if (argv[i] == "-map") {

					m_SCNFile = io::path(argv[++i].c_str());
					if (!m_SCNFile.empty()) {
						//if it doesn't have .scn add .scn to the fname
						if (!core::hasFileExtension(m_SCNFile, "scn"))
							m_SCNFile += ".scn";
					}
				}
				else if (argv[i] == "-vd" || argv[i] == "-viewdistance") {
					const char* tmp = argv[++i].c_str();
					if (is_number(tmp)) {
						char* end = nullptr;
						m_Viewdist = strtof(tmp, &end);
						if (m_Viewdist <= 0)m_Viewdist = 20;
					}
					else {
						getApplication()->showDebugConsole();
						errorMessage += "View distance isnt a number";
					}
				}
				else if (argv[i] == "-forcealpha" ||
					argv[i] == "-fa") {
					m_Alpha = 125;
				}
				else if (argv[i] == "-invertmouse" ||
					argv[i] == "-im") {
					bInvertMouse = true;
				}
				else if (argv[i] == "-entity" ||
					argv[i] == "-e") {
					
				}
				else if (argv[i] == "-vi" ||
					argv[i] == "-vertexinfo") {
					bVertInfo = true;
				}
				else if (argv[i] == "-movevertex" ||
					argv[i] == "-mv") {
				}
				else if (argv[i] == "-es" ||
					argv[i] == "-extrasolids") {
					
				}
				else if (argv[i] == "-bb" ||
					argv[i] == "-boundingbox") {

				}
				else if(argv[i] == "-decals") {
					bDecal = true;
				}
				else if (argv[i] == "-flip") {
					bFlipUV = true;
				}
				else if (argv[i] == "-l" ||
					argv[i] == "-lightmap") {
				
				}
				else if (argv[i] == "-nodebug" || argv[i] == "-nd") {
					bDebug = false;
				}
				else if (argv[i] == "-internal") {
					bLoadInbuiltDebugger = true;
				}
				else if (argv[i] == "-borderless") {
					bBorderless = true;
				}
				else {
					getApplication()->showDebugConsole();
					errorMessage += (std::format("\nUnknown option {}. \nAvailable options are:\n", argv[i].c_str()).c_str());
					errorMessage += ("   -res <width> <height>            Set screen resolution.\n"
						"   -f, -fullscreen                  Run in full screen mode.\n"
						"   -borderless                      Run in borderless mode.(Windows only)\n"
						"   -resize                          Allow the window to be resizeable\n"
						"   -nd, -nodebug                    Disabled debug console window.\n"
						"   -internal                        Gui-based console debug output, useful for fullscreen.\n"
						"   -im, -invertmouse                Inverts mouse. Because, you know, we are people too!\n\n"
						"   -map <mapname>                   Load <mapname> on start.\n"
						"   -dx, -directx                    Force Direct3d9 mode. Default is OpenGL.\n"
						"   -vd,-viewdistance <dist>         Set view distance. Default is 20\n"
						"   -fov <fov>                       Set camera fov. Default is 60\n"
						"   -fa, -forcealpha                 Forces showing transparent materials.\n"
						"   -flip							 Flip the texture UV's (WIP)\n"
						"   -decals							 Visually display a rough idea of what decals will look like(WIP)\n"
						"   -vi -vertexinfo                  When Vertex is selected, via CTRL or SHIFT detail boxes change."
						"   -all							 Helper that will enable mutiple flags(same as -fa -vi -nd -i)\n"
						);
				}
			}
		}

		inline ~CScnArguments() {};

		inline E_DRIVER_TYPE getDriverType() { return m_DrivType;  }
		inline core::dimension2du getRes() { return m_Res; }
		inline core::dimension2du getDesktopRes() { return m_Deskres; }
		inline core::dimension2du getCurrentRes() { return bFullscreen ? m_Deskres : m_Res; }
		inline f32 getViewDist() { return m_Viewdist; }
		inline f32 getFov() { return m_Fov; }
		inline io::path  getSCNPath() { return m_SCNFile; }
		inline void setSCNPath(io::path path) { m_SCNFile = path; }
		inline u16  getAlpha() { return m_Alpha; }
		inline wchar_t* getBaseDirectory() { return m_BaseDirectory; } //Not sure if even needed
		inline bool isDefault() { return bResDefault; }
		inline bool isMouseInvert() { return bInvertMouse; }
		inline bool isDecalEnabled() { return bDecal; }
		inline bool isFlippedUV() { return bFlipUV; }
		inline bool isDebugEnabled() { return bDebug; }
		inline bool isInternalDebug() { return bLoadInbuiltDebugger; }
		inline bool isBorderless() { return bBorderless; }
		inline bool isVertInfo() { return bVertInfo; }
		inline bool isResizeable() { return bResizeable; }
		inline bool isFullscreen() { return bFullscreen; }
		inline bool getScnLoaded() { return bLoaded; }
		inline void setScnLoaded(bool state) { bLoaded = state; }
		inline bool hasError() { return !errorMessage.empty(); }
		inline const char* getErrorText() { return errorMessage.c_str(); }
	private:
		bool bResDefault = true;
		bool bInvertMouse = false;
		bool bVertInfo = false;
		bool bLoadInbuiltDebugger = false;
		bool bLoaded = false;
		bool bDebug = true;
		bool bBorderless = false;
		bool bFullscreen = false;
		bool bFlipUV = false;
		bool bResizeable = false;
		bool bDecal = false;
		std::string errorMessage = "";
		E_DRIVER_TYPE m_DrivType = EDT_OPENGL;
		core::dimension2du m_Res = core::dimension2du(800,600);
		core::dimension2du m_Deskres = m_Res;
		f32 m_Viewdist = 20.0f;
		f32 m_Fov = 60.0f;
		io::path m_SCNFile = "";
		u16 m_Alpha = 0;
		wchar_t* m_BaseDirectory;
};
