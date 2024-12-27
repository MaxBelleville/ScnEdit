#pragma once

#include "Base/util.h"



class CScnArguments {
	public:
		
		CScnArguments(const std::vector<std::string>& argv) {
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
					argv[i] == "-br") {}
				else if (argv[i] == "-portal" ||
					argv[i] == "-p") {
					bPortal = true;
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
					bEntity = true;
					bPortal = true;
					bCellBVH = true;
					bLoadExtra = true;
					bLoadLightmaps = true;
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
						printf("View distance isnt a number\n");
						system("pause");
						exit(1337);

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
					bEntity = true;
				}
				else if (argv[i] == "-movevertex" ||
					argv[i] == "-mv") {
					bMoveVertex = true;
				}
				else if (argv[i] == "-es" ||
					argv[i] == "-extrasolids") {
					bLoadExtra = true;
				}
				else if (argv[i] == "-bb" ||
					argv[i] == "-boundingbox") {
					bCellBVH = true;
				}
				else if(argv[i] == "-decals") {
					bDecal = true;
				}

				else if (argv[i] == "-l" ||
					argv[i] == "-lightmap") {
					bLoadLightmaps = true;
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
					printf("Unknown option %s. \nAvailable options are:\n\n", argv[i].c_str());
					printf("   -res <width> <height>            Set screen resolution.\n"
						"   -f, -fullscreen                  Run in full screen mode.\n"
						"   -borderless                      Run in borderless mode.(Windows only)\n"
						"   -resize                          Allow the window to be resizeable\n"
						"   -nd, -nodebug                    Disabled debug console window.\n"
						"   -internal                        Gui-based console debug output, useful for fullscreen.\n"
						"   -im, -invertmouse                Inverts mouse. Because, you know, we are people too!\n\n"
						"   -map <mapname>                   Load <mapname> on start.\n"
						"   -dx, -directx                    Force Direct3d9 mode. Default is OpenGL.\n"
						"   -vd,-viewdistance <dist>         Set view distance. Default is 20"
						"   -fov <fov>                       Set camera fov. Default is 60"
						"   -fa, -forcealpha                 Forces showing transparent materials.\n"
						"   -p, -portal                      Displays portals\n"
						"   -e, -entity                      Displays entites\n"
						"   -decals                      Visually display a rough idea of what decals will look like(WIP)\n"
						"   -all							 Will load every component with lightmaps(same as -l -es -e -p -bb)\n"
						"   -es, -extrasolids                Load extra solids(ex: doors) instead of just the base geometry\n"
						"   -mv, -movevertex                 Allows vertices to be moveable after surface selection\n"
						"   -bb, -boundingbox                Displays a gray box around all of the cell collison bounding boxes\n"
						"   -l, -lightmap                    Visually shows lightmaps(Warning: slower loading times)\n"
						"\n");
					system("pause");
					exit(1337);
				}
			}
		}

		~CScnArguments() {};

		E_DRIVER_TYPE getDriverType() { return m_DrivType;  }
		core::dimension2du getRes() { return m_Res; }
		core::dimension2du getDesktopRes() { return m_Deskres; }
		core::dimension2du getCurrentRes() { return bFullscreen ? m_Deskres : m_Res; }
		f32 getViewDist() { return m_Viewdist; }
		f32 getFov() { return m_Fov; }
		io::path  getSCNPath() { return m_SCNFile; }
		void setSCNPath(io::path path) { m_SCNFile = path; }
		u16  getAlpha() { return m_Alpha; }
		wchar_t* getBaseDirectory() { return m_BaseDirectory; } //Not sure if even needed
		bool isDefault() { return bResDefault; }
		bool isMouseInvert() { return bInvertMouse; }
		bool isDecalEnabled() { return bDecal; }
		bool isPortalEnabled() { return bPortal; }
		bool isBBEnabled() { return bCellBVH; }
		bool isEntityEnabled() { return bEntity; }
		bool isExtrasEnabled() { return bLoadExtra; }
		bool isLightmapEnable() { return bLoadLightmaps; }
		bool isDebugEnabled() { return bDebug; }
		bool isInternalDebug() { return bLoadInbuiltDebugger; }
		bool isVertMoveable() { return bMoveVertex; }
		bool isBorderless() { return bBorderless; }
		bool isResizeable() { return bResizeable; }
		bool isFullscreen() { return bFullscreen; }
		bool getScnLoaded() { return bLoaded; }
		void setScnLoaded(bool state) { bLoaded = state; }

	private:
		bool bResDefault = true;
		bool bInvertMouse = false;
		bool bPortal = false;
		bool bCellBVH = false;
		bool bEntity = false;
		bool bLoadExtra = false;
		bool bLoadLightmaps = false;
		bool bLoadInbuiltDebugger = false;
		bool bLoaded = false;
		bool bDebug = true;
		bool bMoveVertex = false;
		bool bBorderless = false;
		bool bFullscreen = false;
		bool bResizeable = false;
		bool bDecal = false;
		E_DRIVER_TYPE m_DrivType = EDT_OPENGL;
		core::dimension2du m_Res = core::dimension2du(800,600);
		core::dimension2du m_Deskres = m_Res;
		f32 m_Viewdist = 20.0f;
		f32 m_Fov = 60.0f;
		io::path m_SCNFile = "";
		u16 m_Alpha = 0;
		wchar_t* m_BaseDirectory;
};
