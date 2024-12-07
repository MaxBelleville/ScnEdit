#include "pch.h"
#if false
#include <irrlicht.h>
#include <iostream>
#include <fstream>
#include <Windows.h> 
#include <winuser.h> 
#include <windowsx.h>
#include <dwmapi.h>
#include <math.h>
#include <stdlib.h>
#include "scnedit.h"
#include "scn.h"
#include "util.h"
#include "scnmesh.h"
#include "EventReceiver.h"
#include "main.h"
#include "selector.h"
#include "CSEditGUI.h"
#include "resource.h"
#include "export.h"
#include <filesystem>


using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

using namespace std;
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#pragma comment (lib, "Dwmapi")
#endif

//Goes to buildGui
void buildGui(IrrlichtDevice* device);
//Declares variables
//Device
IrrlichtDevice* device = 0;
//unknown
CScn* scn = 0;
MyEventReceiver* receiver;
//Scene nodes
core::array<IMeshSceneNode*> nodes;
//Mesh
core::array<SMesh*> scnMeshes;
//selector
scene::ITriangleSelector* selector = 0;
//camera
ICameraSceneNode* camera = 0;
ISceneNodeAnimatorCameraFPS* ca;
ISceneNode* selectNode;
bool selected = false;
//red surface materials
SMaterial redmat;
//Variabl for reseting uv map
bool resetUV = false;
//CDynamicMeshBuffer * buffer=0;
//previous materials
E_MATERIAL_TYPE prevmt;
//selected surfaces
u32 selsurf;
//area of unsigned 32 bit integers that holdall selected surfaces
core::array < u32 > allselsurfs;
//selected solid
int selsolid = 0;
int selent = -1;
//a 32 bit float thats for resizing of the uv grid
f32 uvgridmult = 0.01f;
//output file stream
ofstream* output = 0;
//base directory
wchar_t* BaseDirectory;
//resizes uv map on texture
bool uvresize = false;
//displays less information in console
bool bBrief = false;
bool bData = false;
bool bInvertMouse = false;
bool bPortal = false;
bool bCellBVH = false;
bool bEntity = false;
bool bLoadAll = false;
bool bLoadLightmaps = false;
bool bLoadInbuiltDebugger = false;
bool bMoveVertex = false;
bool bBorderless = false;

bool switchToShared = false;
bool mapLoad = false;
bool finishLoading = false;
bool pageDown = false;
bool pageIncrease = false;
video::E_DRIVER_TYPE drivType = EDT_OPENGL;

io::path scnfilename = "";
u32 sharedIndx = 0;
u32 selIndx = 0;
core::array<IMeshSceneNode*> sharedSpheres;
core::array<IMeshSceneNode*> selSpheres;
core::array <core::array<IMeshSceneNode*>> boundingBox;
int counter = 0;
int superCounter = 0;
//Unsigned int thats 16 bit for the globalized alpha
u16 GLOBALALPHA = 0;
f32 viewdist = 20.0f;
//selections
CSelector* sels;
//gui
CSEditGUI* segui;
//Start program



int main(int argc, char** argv) {
	//unsigned ints thats 32 bit realated to resolution and n of bits
	u32 resx = 800, resy = 600, nbits = 16;
	bool resDefault = true;
	//gets driver (default open gl)
	
	bool bFullscreen = false;
	dimension2d < u32 > deskres;

	//WASD mapping for camera
	SKeyMap keyMap[4];
	keyMap[0].Action = EKA_MOVE_FORWARD;
	keyMap[0].KeyCode = KEY_KEY_W;
	keyMap[1].Action = EKA_MOVE_BACKWARD;
	keyMap[1].KeyCode = KEY_KEY_S;
	keyMap[2].Action = EKA_STRAFE_LEFT;
	keyMap[2].KeyCode = KEY_KEY_A;
	keyMap[3].Action = EKA_STRAFE_RIGHT;
	keyMap[3].KeyCode = KEY_KEY_D;

	//  read command line options
	for (u32 i = 1; i < argc; i++) {
		if (str_equals(argv[i], "-brief") ||
			str_equals(argv[i], "-br")) {
			bBrief = true;
		}
		else if (str_equals(argv[i], "-portal") ||
			str_equals(argv[i], "-p")) {
			bPortal = true;
		}
		else if (str_equals(argv[i], "-directx") ||
			str_equals(argv[i], "-dx")) {
			drivType = EDT_DIRECT3D9;
		}
		else if (str_equals(argv[i], "-irrlicht") ||
			str_equals(argv[i], "-irr")) {
			drivType = EDT_BURNINGSVIDEO;
		}
		else if (str_equals(argv[i], "-res")) {
			char* tmpX = argv[++i];
			char* tmpY = argv[++i];
			if (is_number(tmpX) && is_number(tmpY)) {
				resx = atoi(tmpX);
				resy = atoi(tmpY);
				resDefault = false;
			}
			else {
				segui->addLog(SAY("Resolution width and or height is not a number...\n"));
				system("pause");
				exit(1337);
			}
		}
		else if (str_equals(argv[i], "-fullscreen") ||
			str_equals(argv[i], "-f")) {
			IrrlichtDevice* nulldevice = createDevice(video::EDT_NULL);
			deskres = nulldevice->getVideoModeList()->getDesktopResolution();
	 
			bFullscreen = true;
		}
		else if (str_equals(argv[i], "-data") ||
			str_equals(argv[i], "-d")) {
			bData = true;
		}
		else if (str_equals(argv[i], "-map")) {

			scnfilename = path(argv[++i]);
		}
		else if (str_equals(argv[i], "-vd") || str_equals(argv[i], "-viewdistance")) {
			char* tmp = argv[++i];
			if (is_number(tmp)) {
				char* end = nullptr;
				viewdist=strtof(tmp, &end);
				if (viewdist <= 0)viewdist = 20;
			}
			else {
				segui->addLog(SAY("View distance isnt a number\n"));
				system("pause");
				exit(1337);
				
			}
		}
		else if (str_equals(argv[i], "-forcealpha") ||
			str_equals(argv[i], "-fa")) {
			GLOBALALPHA = 125;
		}
		else if (str_equals(argv[i], "-invertmouse") ||
			str_equals(argv[i], "-im")) {
			bInvertMouse = true;
		}
		else if (str_equals(argv[i], "-entity") ||
			str_equals(argv[i], "-e")) {
			bEntity = true;
		}
		else if (str_equals(argv[i], "-movevertex") ||
			str_equals(argv[i], "-mv")) {
			bMoveVertex = true;
		}
		else if (str_equals(argv[i], "-as") ||
			str_equals(argv[i], "-allsolids")) {
			bLoadAll = true;
		}
		else if (str_equals(argv[i], "-bb") ||
			str_equals(argv[i], "-boundingbox")) {
			bCellBVH = true;
		}

		else if (str_equals(argv[i], "-l") ||
			str_equals(argv[i], "-lightmap")) {
			bLoadLightmaps = true;
		}
		else if (str_equals(argv[i], "-debug")) {
			bLoadInbuiltDebugger = true;
		}
		else if (str_equals(argv[i], "-borderless")) {
			bBorderless = true;
		}
		else {
			SAY("Unknown option %s. \nAvailable options are:\n\n", argv[i]);
			SAY("   -br, -brief                      Brief console info on textures and coordinates.\n"
				"   -res <width> <height>            Set screen resolution.\n"
				"   -f, -fullscreen                  Run in full screen mode.\n"
				"   -borderless                      Run in borderless mode.(Windows only)\n"
				"   -debug                           Gui-based console debug output, useful for fullscreen.\n"
				"   -d, -data                        Displays data that is shown in editor after loading in the editor.\n"
				"   -im, -invertmouse                Inverts mouse. Because, you know, we are people too!\n\n"
				"   -map <mapname>                   Load <mapname> on start.\n"
				"   -irr, -irrlicht                  Force Irrlicht renderer mode. Default is OpenGL.\n"
				"   -dx, -directx                    Force Direct3d9 mode. Default is OpenGL.\n"
				"   -viewdistance <dist>, -vd <dist> Set view distance. Default is 20\n"
				"   -fa, -forcealpha                 Forces showing transparent materials.\n"
				"   -p, -portal                      Displays portals\n"
				"   -e, -entity                      Displays entites\n"
				"   -a, -allsolids                   Load all solids instead of just the base geometry\n"
				"   -mv, -movevertex                 Allows vertices to be moveable after surface selection\n"
				"   -bb, -boundingbox                Displays a gray box around all of the cell collison bounding boxes\n"
				"   -l, -lightmap                    Visually shows lightmaps(Warning: slower loading times)\n"
				"\n");
			system("pause");
			exit(1337);
		}
	}
	//Assigns BaseDirectory
	BaseDirectory = new wchar_t[256];
	//Assigns Current Directory to the 256 bit Base
	int ret = GetCurrentDirectory(256, BaseDirectory);
	//Error message
	if (!ret || ret > 256)
		error(true, "GetCurrentDirectory failed. %d", ret);
	//Creates device window
	if (bFullscreen == false&& bBorderless == false) {
		device =
			createDevice(drivType, dimension2d < u32 >(resx, resy), 32,
				false, false, true, 0);
	}
	else if (bFullscreen == true) {
		device =
			createDevice(drivType, deskres, 32,
				true, false, true, 0);
	}
	else if (bBorderless == true) {
		device =
			createDevice(drivType, dimension2d < u32 >(resx, resy), 32,
				false, false, true, 0);
	}

	


	//read data.res
	device->getGUIEnvironment()->getFileSystem() -> addFileArchive("sedata.res", true, true, EFAT_ZIP);
	//device->setResizable(true);
	device->setWindowCaption(L"SWAT3 SCN editor");
	device->getCursorControl()->setVisible(false);


	//Update icon
	#ifndef _IRR_WINDOWS_
	#else
	HWND irrlichtWindow = FindWindow(NULL, L"SWAT3 SCN editor");
	HINSTANCE hInstance = GetModuleHandle(0);
	DWORD style = WS_VISIBLE | WS_POPUP;

	SetClassLongPtr(irrlichtWindow, GCLP_HICON, (LONG_PTR)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_SHARED));
	if (bBorderless == true && bFullscreen == false) {
	 
		BOOL composition_enabled = FALSE;
		SetWindowLong(irrlichtWindow, GWL_STYLE, static_cast<LONG>(style));
		bool success = DwmIsCompositionEnabled(&composition_enabled) == S_OK;
		static const MARGINS shadow_state{ 0, 0, 0, 0 };
		DWM_WINDOW_CORNER_PREFERENCE no_round_corners{ DWMWCP_DONOTROUND };
		if (composition_enabled && success) {
			DwmExtendFrameIntoClientArea(irrlichtWindow, &shadow_state);
			DwmSetWindowAttribute(irrlichtWindow, DWMWA_WINDOW_CORNER_PREFERENCE, &no_round_corners, sizeof(no_round_corners));
		}

		SetWindowPos(irrlichtWindow, nullptr, 0, 0,resx, resy, SWP_FRAMECHANGED | SWP_NOMOVE  );
		ShowWindow(irrlichtWindow, SW_SHOW); //display window
	}
	if (bBorderless == false && bFullscreen==false) {
		RECT rect;
		GetWindowRect(irrlichtWindow, &rect);
		
		if ((int)resx >= (int)rect.right-18 && (int)resy >= (int)rect.bottom-96) {
			SetWindowPos(irrlichtWindow, nullptr, -10, 0, resx, resy, SWP_FRAMECHANGED | SWP_NOSIZE);
		}
		else  if ((int)resx >= (int)rect.right-18) {
			SetWindowPos(irrlichtWindow, nullptr, -10, rect.top, resx, resy, SWP_FRAMECHANGED | SWP_NOSIZE);
		}
		else if ((int)resy >= (int)rect.bottom-96) {
			SetWindowPos(irrlichtWindow, nullptr, rect.left, 0, resx, resy, SWP_FRAMECHANGED | SWP_NOSIZE);
		}
		//ShowWindow(irrlichtWindow, SW_SHOW); //display window
	}

	#endif
	//Sets up video driver
	IVideoDriver* driver = device->getVideoDriver();
	//driver->setTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED);
	//Sets up scene manager
	ISceneManager* smgr = device->getSceneManager();
	smgr->setAmbientLight(video::SColor(255, 255, 255, 255));
	//Sets up event reciver
	receiver = new MyEventReceiver(device);

	device->setEventReceiver(receiver);
	//IGUIEnvironment* guienv = device->getGUIEnvironment();
	//Sets up the gui
	segui = new CSEditGUI(device, uvresize, uvgridmult, bBorderless,bFullscreen);
	segui->enableDebugger(bLoadInbuiltDebugger);
	if (bData == true) {
		segui->infoBox(bData, 0, 0, 0, 0);
	}

	//Fake irlicht intro 
	IOSOperator* os = device->getOSOperator();
	core::stringw msg = L"Irrlicht Engine version ";
	msg += device->getVersion();
	msg += "\n";
	msg += os->getOperatingSystemVersion().c_str();
	msg += "\n";
	segui->addLog(msg);
	msg = L"Using Render: ";
	msg += driver->getName();
	msg += "\n";
	if(drivType ==EDT_OPENGL|| drivType == EDT_DIRECT3D9)segui->addLog(msg);
	if (drivType == EDT_DIRECT3D9) {
		msg = "Currently available Video Memory (kB) ";
		u32 avail=0;
		os->getSystemMemory(NULL,&avail);
		msg += avail;
		msg += "\n";
		segui->addLog(msg);
	}
	msg = L"Resizing window (";
	if (!bFullscreen)msg += resx;
	else msg += deskres.Width;
	msg += " ";
	if (!bFullscreen)msg += resy;
	else msg += deskres.Height;
	msg += ")\n";
	segui->addLog(msg);


	if (drivType == EDT_DIRECT3D9)segui->addLog("Resetting D3D9 device.\n");

	//buildGui(device);
	//Sets up the camera
	//  camera = device->getSceneManager()->addCameraSceneNodeMaya();
	camera = device->getSceneManager()->addCameraSceneNodeFPS(0, 100, 0.5, -1, keyMap, 4, false, 0.f, bInvertMouse);

	camera->setFarValue(viewdist * 100);
	//Starting Postion of camera IMPORTANT: change when you find the entities
	camera->setPosition(core::vector3df(0, 4, 0));
	core::list<ISceneNodeAnimator*>::ConstIterator al = camera->getAnimators().begin();
	ca = (irr::scene::ISceneNodeAnimatorCameraFPS*)*al;
	camera->setInputReceiverEnabled(true);
	device->getCursorControl()->setVisible(false);



	//debug
	// commented out the next line so only triggered with O key -SJ
	//export2obj();
	//exit(42);

	int lastFPS = -1;
	//Sets postion to 0,0,0
	vector3df lastpos(.0, .0, .0), pos;
	
	IMeshSceneNode* box = smgr->addCubeSceneNode();
	box->setMaterialFlag(EMF_LIGHTING, false);
	box->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
	//box->setMaterialFlag(EMF_ZBUFFER, false);
	device->getSceneManager()->getMeshManipulator()->setVertexColors(box->getMesh(), SColor(255, 0, 0, 0));
	while (device->run()&&driver)
		if (device->isWindowActive()|| finishLoading) {
			if (!scnfilename.empty() && !mapLoad) {
				segui->addLoading();
			}
			finishLoading = false;
			driver->beginScene(true, true, SColor(255, 100, 101, 140));
			smgr->drawAll();
			segui->getIrrGUIEnv()->drawAll();
			driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

		  

			i_point3f verts = { 0,0,0 };
			if (scn) {
				if (sels) {
					verts = getVertexIndex();
				}
			}
			if (!(verts.X == 0 && verts.Y == 0 && verts.Z == 0)) {
				box->setPosition(vector3df(verts.X - 0.2f, verts.Y - 0.2f, verts.Z - 0.2f));
				box->setScale(vector3df(0.4f));
			}
			else if (selent != -1) {
				box->setPosition(selectNode->getPosition() - vector3df(0.0375f, 0.0375f, 0.0375f));
				box->setScale(vector3df(0.075f));
			}
			else {
				box->setPosition(vector3df(0));
				box->setScale(vector3df(0));
			}

				//Begins scene and starts drawing scene and gui


				/*if (sel)
				{
					driver->draw3DTriangle(tri, video::SColor(0,255,0,0));
				}*/
 
	  
			//if (scn) {
			//    for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
			//        //Creates all boxes for the scene
			//        if (sels) {
			//            if (sels->boxes.size() > 0)
			//                for (u32 i = 0; i < sels->getBoxes(s).size(); i++)
			//                    driver->draw3DBox(sels->getBoxes(s)[i], SColor(200, 0, 0, 255)); //For some reason although 
			//        }
			//    }
			//}
			if (bCellBVH) {
				if (scn) {
					for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
						CScnSolid* solid = scn->getSolid(s);
						for (u32 j = 0; j < solid->n_cells; j++) {
							for (u32 l = 0; l < solid->rawcells[j].leafnode.size(); l++) {//TODO Abstract
								scnCellData_t leaf = *solid->rawcells[j].leafnode[l];
								f32 X = leaf.bb_verts[0].X;
								f32 Y = leaf.bb_verts[0].Y;
								f32 Z = leaf.bb_verts[0].Z;
								f32 X2 = leaf.bb_verts[1].X;
								f32 Y2 = leaf.bb_verts[1].Y;
								f32 Z2 = leaf.bb_verts[1].Z;
							
								if (fabs(X - X2) / 10 > 1000 || fabs(Y - Y2) / 10 > 1000 || fabs(Z - Z2) / 10 > 10000) {
									X2 = 0;
									Y2 = 0;
									Z2 = 0;
									X = 0;
									Y = 0;
									Z = 0;
								}
							   boundingBox[j][l]->setPosition(vector3df((X + X2) / 2, (Y + Y2) / 2, (Z + Z2) / 2));
							   boundingBox[j][l]->setScale(vector3df(fabs(X - X2) / 10, fabs(Y - Y2) / 10, fabs(Z - Z2) / 10));
							}
						}
					}
				}
			}
			driver->endScene();
			if (pageDown) {
				if (counter > lastFPS/7) {
					segui->changePage(pageIncrease);
					if (superCounter < lastFPS * 6)counter = 0;
					else counter = lastFPS / 9;
				}
				counter++;
				if(superCounter< lastFPS *6)superCounter++;
			}
			//Gets fps
			int fps = driver->getFPS();
			if (lastFPS != fps) {
				//Display fps and update last fps
				core::stringw str = L"SWAT3 SCN editor FPS: ";
				str += fps;
				device->setWindowCaption(str.c_str());
				if (bFullscreen||bBorderless) segui->setFPS(fps);
				lastFPS = fps;
				//Might be overkill to run every 'tick'
			   
				segui->updateLog(false);
	
			}
			//Load scn file
			if (!scnfilename.empty() && !mapLoad) {
				//if it doesn't have .scn add .scn to the fname
				if (!hasFileExtension(scnfilename, "scn"))
					scnfilename += ".scn";
				//Transfers path to the function that will deal with scn and bak files
				loadScnFile(scnfilename);
				segui->updateLog(true);
				mapLoad = true;
				segui->removeLoading();
			}
		}
	//Clears device information
	device->drop();

	return 0;
}
//gets solids and meshes
void selectCurrent(bool bAppend) {
	ISceneManager* smgr = device->getSceneManager();
	if (device->getCursorControl()->isVisible()) return;//Disables so you don't accidentally select stuff when accessing menus.
	if (!scn) return;
	//Gets solid
	for (u32 i = 0; i < scn->getSolidSize(bLoadAll); i++) {
		(scn->getSolid(i))->firstVal = true;
	}

	if (!nodes.size() >0 || !scnMeshes.size() > 0) {
		return;
	}
	for (int i = 0; i < scnMeshes.size(); i++) {
		if (!scnMeshes[i]) return;
	}
		
	//Gets scene collison manager
	core::line3d < f32 > line;
	ISceneCollisionManager* scmg = device->getSceneManager()->getSceneCollisionManager();

	//Gets cursor movement
	//line = scmg->getRayFromScreenCoordinates(device->getCursorControl()->getPosition());
	line.start = camera->getPosition();
	line.end = line.start +  (camera->getTarget() - line.start).normalize() * 1000.0f;
	//Loads up some variables
	core::vector3df intersection;

	core::triangle3df tri;
	//Figures out what your selecting
	selectNode = NULL;
	selent = -1;
	selectNode = scmg->getSceneNodeAndCollisionPointFromRay(line, intersection, tri,0);

	if (bData == true) {
		segui->infoBox(bData, 0, 0, 0, 0);
	}

	if (selectNode != NULL) {

		for (u32 i = 0; i < scn->getTotalEnts(); i++) {
			CScnEnt* ent = scn->getEnt(i);
			const char* originStr = ent->getField("origin");
			const char* className = ent->getField("classname");
			if (originStr) {
				core::array < std::string > arr = str_split(originStr, " ");
				int orgX = round(std::stof(arr[0]));
				int orgY = round(std::stof(arr[1]));
				int orgZ=  round(std::stof(arr[2]));
				int nodeX = round32(selectNode->getPosition().X);
				int nodeY = round32(selectNode->getPosition().Y);
				int nodeZ = round32(selectNode->getPosition().Z);
				if (nodeX>=orgX-1&&nodeX<=orgX+1 && 
					nodeY >= orgY - 1 && nodeY <= orgY + 1 &&
					nodeZ >= orgZ - 1 && nodeZ <= orgZ + 1) {
				
					segui->entity_sel(className);
					sels->clear();
					selent = i;
					sayProperties();
					return;
				}
			}
		}
	}
	//Vectors for unused triagles
	vector3df pa = tri.pointA;
	vector3df pb = tri.pointB;
	vector3df pc = tri.pointC;
	bool isSel = false;
	//gets surface and vertices
	for (u32 i = 0; i < scn->getSolidSize(bLoadAll); i++) {
		u32 selsurf = -1;
		CScnSolid* solid = scn->getSolid(i);
		for (u32 j = 0; j < solid->n_surfs; j++) {
			scnSurf_t* surfi = &solid->surfs[j];

			bool foundpa = 0;
			bool foundpb = 0;
			bool foundpc = 0;
			for (int k = 0; k < surfi->vertidxlen; k++) {
				//Gets vertex index
				core::vector3df* verti = &solid->verts[solid->vertidxs[surfi->vertidxstart + k]];
				//Gets vector index
				vector3df vectori(verti->X, verti->Y, verti->Z);
			  
				if (!foundpa) {
					if (vectori.equals(pa))
						foundpa = 1;

				}
				if (!foundpb) {
					if (vectori.equals(pb))
						foundpb = 1;
				}
				if (!foundpc) {
					if (vectori.equals(pc))
						foundpc = 1;
				}
				if (foundpa && foundpb && foundpc) //if found all vertices
				{
					selsurf = j;
					break;
				}
			}
			if (selsurf > -1)
				break;
		}
		if (sels) {
			//selects surface
			sels->selectSurf(i,selsurf, bAppend);
			
			//Activates selected boxes function
			//Updates selected surfaces on gui
			bool currSel= segui->surfs_update(sels,i);
			if (currSel) isSel = true;
		}
	}
	selIndx = 0;
	sharedIndx = 0;
	switchToShared = false;
	if (sels->getVerts().size() == 0) {
		switchToShared = true;
	}
	for (u32 i = 0; i < sharedSpheres.size(); i++) {

		sharedSpheres[i]->remove();

	}
	for (u32 i = 0; i < selSpheres.size(); i++) {

		selSpheres[i]->remove();

	}
	sharedSpheres.set_used(0);
	selSpheres.set_used(0);
	if (!isSel) segui->surf_notsel();
	else {
	  


		for (u32 i = 0; i < sels->getSharedVerts().size(); i++) {
			IMeshSceneNode* sphere = smgr->addSphereSceneNode();
			sphere->setMaterialFlag(EMF_LIGHTING, false);
		   sphere->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
			device->getSceneManager()->getMeshManipulator()->setVertexColors(sphere->getMesh(), SColor(200, 0, 255, 0));
			float dist = 0.15f;
			i_point3f vert = sels->getSharedVert(i);
			sphere->setPosition(vector3df(vert.X - dist, vert.Y - dist, vert.Z - dist));
			sphere->setScale(vector3df(dist * 2));

			sharedSpheres.push_back(sphere);
		   
		   
	 
		}
		for (u32 i = 0; i < sels->getVerts().size(); i++) {
			if (smgr) {
				IMeshSceneNode* sphere = smgr->addSphereSceneNode();
				sphere->setMaterialFlag(EMF_LIGHTING, false);
				sphere->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
				device->getSceneManager()->getMeshManipulator()->setVertexColors(sphere->getMesh(), SColor(200, 255, 0, 255));
				float dist = 0.2f;
				i_point3f vert = sels->getVert(i);
				sphere->setPosition(vector3df(vert.X - dist, vert.Y - dist, vert.Z - dist));
				sphere->setScale(vector3df(dist * 2));

				selSpheres.push_back(sphere);
			}
		}
		sayProperties();
	}

	if (selsurf == -1) {
		segui->addLog(WARN("Can't find surface!\n"));
	   // return;
	}
}
//Closing scn file(clear meshes and deletes other variables)
bool closeCurrentScnFile() {
	camera->setInputReceiverEnabled(false);
	//If it is not null
	if (scn) {
		//clears
		delete scn;
		scn = 0;
	}

	//Proccess above is repeated for the rest of them
	if (scnMeshes.size()>0) {
	 
		for (u32 i = 0; i < scnMeshes.size(); i++) {
			scnMeshes[i]->drop();
		}
		scnMeshes.clear();
	}

	if (nodes.size() > 0) {

		for (u32 i = 0; i < nodes.size(); i++) {
			nodes[i]->remove();
		}
		nodes.clear();
	}

	selent = -1;

	//Deselection
	selected = false;

	if (output) {
		output->close();
		delete output;
		output = 0;
	   
	}
	segui->showLightmapButton(false);
	segui->showStripLightmapButton(false);

	for (u32 i = 0; i < sharedSpheres.size(); i++) {

		sharedSpheres[i]->remove();

	}
	for (u32 i = 0; i < selSpheres.size(); i++) {

		selSpheres[i]->remove();

	}
	sharedSpheres.set_used(0);
	selSpheres.set_used(0);

	//TODO: clear texture list (using unique texture names)

	//clear selected surface
	if (sels) {
		delete sels;
		sels = 0;
	}
	
   
	
	return true;

}
void indirectLoad(path filename) {

	if (bData == true) {
		segui->infoBox(bData, 0, 0, 0, 0);
	}
	scnfilename = filename;
	mapLoad = false;

}

//File system for .bak and .scn
bool loadScnFile(path fname) {
	if (nodes.size() > 0)closeCurrentScnFile();

	//Gets postion of files last characters
	s32 pos = fname.findLastChar("\\/", 2); //search for \ or /
	if (pos > -1) {
		//Changes working directory based on the postion
		device->getFileSystem()->changeWorkingDirectoryTo((fname.subString(0, pos - 1)));
		//fname=fname.subString(pos+1,fname.size()-pos);
		//clears path
		deletePathFromFilename(fname);
	}
	path bakname;
	//removes any extentions related to scn and bak files
	cutFilenameExtension(bakname, fname);
	//Adds .bak
	bakname += ".bak";
	ifstream scnFile(fname.c_str(), ios::in | ios::binary);
	//Checks if you can't copy scn file to bak
	//printf("%i\n", scnFile.is_open());
	if (scnFile.is_open()) {
		if (!CopyFile_(fname.c_str(), bakname.c_str()))
			error(true, "can't copy from %s to %s", fname.c_str(), bakname.c_str());
	}
	//Gets data from file
	//Checks if it can't open
	//printf("%i\n", scnFile.is_open());
	if (!scnFile.is_open()) {
		error(true, "Can't open %s, and or it doesn't exist", fname.c_str());
	}
	else {
		scn = new CScn(&scnFile); //sets scn to file
		segui->addLog(scn->getLog());
	}
	//Closes file
	scnFile.close();
	//Allows to set data to file
	output = new ofstream(fname.c_str(), ios::in | ios::binary | ios::ate);

	//Gets meshes
	scnMeshes = scnCreateMesh(device, scn, bLoadAll, bLoadLightmaps);
	segui->addLog(getScnMeshLog());
	//Gets nodes
	for (int i = 0; i < scnMeshes.size(); i++) {
		IMeshSceneNode* node = device->getSceneManager()->addMeshSceneNode(scnMeshes[i]);
		//If if node is not Null
		if (node) {
			node->setReadOnlyMaterials(true); //this allows us to edit materials by accessing the meshbuffers
			node->setMaterialFlag(EMF_LIGHTING, false); //removes light making every thing the same light level
			node->setAutomaticCulling(EAC_FRUSTUM_BOX);
		   // node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
			node->setVisible(true);
		}

		//If if node is not Null
		if (node) {
			//Gets selector and puts it into node
			selector = device->getSceneManager()->createTriangleSelector(scnMeshes[i], node);
			node->setTriangleSelector(selector);
			//clears selector
			selector->drop();

		}
		nodes.push_back(node);
	//Sets selector for meshes
	}
	
	IVideoDriver* driver = device->getVideoDriver();
	driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT);
	//Sets up scene manager
	ISceneManager* smgr = device->getSceneManager();
	smgr->setAmbientLight(video::SColor(255, 100, 100, 100));
	device->setInputReceivingSceneManager(smgr);
	//Checks if window is active

	ITexture* texture = driver->getTexture("portal.png");
	SMaterial mat;
	mat.Lighting = false;
	driver->setMaterial(mat);
	if (bPortal||bCellBVH) {
		for (u32 i = 0; i < scn->getSolidSize(bLoadAll); i++) {
			IMeshSceneNode* box;
			CScnSolid* solid = scn->getSolid(i);
			for (u32 j = 0; j < solid->n_cells; j++) {
				if (bCellBVH) {
					core::array<IMeshSceneNode*> tmpBox;
					for (u32 l = 0; l < solid->rawcells[j].leafnode.size(); l++) {//TODO Abstract
						box = smgr->addCubeSceneNode();
						box->setMaterialFlag(EMF_LIGHTING, false);
						box->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
						device->getSceneManager()->getMeshManipulator()->setVertexColors(box->getMesh(), SColor(100, 50, 50, 50));
						tmpBox.push_back(box);
					}
					boundingBox.push_back(tmpBox);
				}
				if (bPortal) {
					for (s32 k = 0; k < solid->rawcells[j].n_portals; k++) {
						f32 X = solid->rawcells[j].portals[k].bb_verts[0].X;
						f32 Y = solid->rawcells[j].portals[k].bb_verts[0].Y;
						f32 Z = solid->rawcells[j].portals[k].bb_verts[0].Z;
						f32 X2 = solid->rawcells[j].portals[k].bb_verts[1].X;
						f32 Y2 = solid->rawcells[j].portals[k].bb_verts[1].Y;
						f32 Z2 = solid->rawcells[j].portals[k].bb_verts[1].Z;
						box = smgr->addCubeSceneNode();
						box->setMaterialFlag(EMF_LIGHTING, false);
						box->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
						box->setMaterialTexture(0, texture);
						if (fabs(X - X2) / 10 > 1000 || fabs(Y - Y2) / 10 > 1000 || fabs(Z - Z2) / 10 > 10000) {
							X2 = 0;
							Y2 = 0;
							Z2 = 0;
							X = 0;
							Y = 0;
							Z = 0;
						}
						box->setPosition(vector3df((X + X2) / 2, (Y + Y2) / 2, (Z + Z2) / 2));
						box->setScale(vector3df(fabs(X - X2) / 10, fabs(Y - Y2) / 10, fabs(Z - Z2) / 10));
						nodes.push_back(box);
					}
				}
			}

		}
	}
	if (bEntity) {
		IMeshSceneNode* box;
		SMaterial mat;
		ITexture* otexture = driver->getTexture("entity.png");
		ITexture* texture = otexture;
		for (u32 i = 0; i < scn->getTotalEnts(); i++) {

			CScnEnt* ent = scn->getEnt(i);
			const char* originStr = ent->getField("origin");
			const char* className = ent->getField("classname");
			float scaleX = 0.5f;
			float scaleY = 0.5f;
			float scaleZ = 0.5f;
			char path[128];
			char prevPath[128];
			if (originStr && className && !str_equiv(className, "Door")) {
				strcpy_s(path, className);
				strcat_s(path, +".png");
				if (strcmp(path, prevPath) != 0) {//Bit of optomization so not reloading textures
					if (device->getFileSystem()->existFile(path))
						texture = driver->getTexture(path);
					else texture = otexture;
					strcpy_s(prevPath, path);
				}
				if (str_equiv(className, "waypointnode")) {
					scaleX = 1;
					scaleY = 1;
					scaleZ = 1;
				}
				else if (str_equiv(className, "light_spot"))
				{
					scaleX = 0.4;
					scaleY = 1;
					scaleZ = 0.4;
				}
				else if (str_equiv(className, "light_ambient")) {
					scaleX = 1;
					scaleY = 1;
					scaleZ = 1;
				}
				core::array < std::string > arr = str_split(originStr, " ");
				int x = round(std::stof(arr[0]) - (scaleX / 2));
				int y = round(std::stof(arr[1]) - (scaleY / 2));
				int z = round(std::stof(arr[2]) - (scaleZ / 2));
				box = smgr->addCubeSceneNode();
				box->setMaterialFlag(EMF_LIGHTING, false);
				box->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
				box->setMaterialTexture(0, texture);
				
				box->setPosition(vector3df(x, y, z));
				box->setScale(vector3df(scaleX, scaleY, scaleZ));
				selector = device->getSceneManager()->createTriangleSelectorFromBoundingBox(box);
				box->setTriangleSelector(selector);
				//clears selector
				selector->drop();
				nodes.push_back(box);
			}

		}
	}

	sels = new CSelector(scn, scnMeshes, scn->getSolidSize(bLoadAll), device);
	//Sets camera input to true
	if (!device->isWindowActive()) {
		camera->setInputReceiverEnabled(false);
		device->getCursorControl()->setVisible(true);
	}
	else {
		camera->setInputReceiverEnabled(true);
		device->getCursorControl()->setVisible(false);
		finishLoading = true;
	}
	segui->showLightmapButton(bLoadLightmaps&&scn->getLightmap().hasLightmaps());
	segui->showStripLightmapButton(bLoadLightmaps&&scn->getLightmap().hasLightmaps());


	return 0;
}
//Rexturing
bool scnRetexture(path fname) {
	//gets file name
	stringc shortname = fname;

	char* ptex;
	//deletes path
	deletePathFromFilename(shortname);
	//cuts extension
	cutFilenameExtension(shortname, shortname);
	shortname = shortname.subString(0, min_(shortname.size(), 31u));
	//by using min we make sure we don't have tex names bigger than 32 chars (including zero terminated)

	//segui->addLog(SAY("Setting new texture to %s.\n", shortname.c_str()));
	//gets texture from file
	ITexture* t = device->getVideoDriver()->getTexture(fname);
	//gets red surfaces

	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
		core::array <u32>& rs = sels->getRedSurfs(s);
		//Checks if surface that is red was selected
		if (rs.size() > 0) {
			segui->addLog(SAY("Setting new texture to %s.\n", shortname.c_str()));
			CScnSolid* solid = scn->getSolid(s);
			//Checks for each selected surface
			for (u32 i = 0; i < rs.size(); i++) {
			//    //Puts each selected surface into surface index
			   u32 si = rs[i];
			//    //Checks if surface index is postive and smaller then the number of solid surfaces
				if (si >= 0 && si < solid->n_surfs) {
			//        //sets p texture to the solid surface index texture
				   ptex = solid->surfs[si].texture;
			//        //Copys the file name
				   memcpy(ptex, shortname.c_str(), strlen(shortname.c_str()));

				   //pad string with zeros
				   for (int j = shortname.size(); j < 32; j++)
					   ptex[j] = '\0';

			//        //show new texture in viewer
					((CDynamicMeshBuffer*)scnMeshes[s]->getMeshBuffer(si))->getMaterial().setTexture(0, t);
					//CDynamicMeshBuffer * buffer = (CDynamicMeshBuffer*)scnMesh->getMeshBuffer(si);
					//buffer->Material.setTexture(0,t);
				}
				else {
				   //If selected solid and surface index is invalid
					segui->addLog(WARN("Invalid surface index: Solid(%d).surf(%d)\n", selsolid, si));
					return false;
				}
			}
		}
	}
	return true;
}

//new version uses getSurfUVidxs so we only iterate once for each vertex
bool scnRetexture_UV(f32 uu, f32 vv)
//retexture after changing texture position (width,height,U,V)
{
	if (scn) {
		CScnLightmap lmap = scn->getLightmap();
		for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {

			//ATTENTION: we only want to update each vertex once!
			core::array < u32 >& rs = sels->getRedSurfs(s); //selected surfs
			core::array < u32 >& bs = sels->getBlueSurfs(s); //shared surfs
			uu *= uvgridmult;
			vv *= uvgridmult;
			//segui->addLog(SAY("Shifting UV by %f %f.\n",uu,vv));
			// If you didn't select a surface
			if (rs.size() > 0) {
				arrayu uvidxs = getSurfUVIdxs(s, rs); //returns all uvidxs of red surfaces, non repeating

				u32 uvi;
				CScnSolid* solid = scn->getSolid(s);

				while (uvidxs.iterate(uvi)) {
					//gets uv position of solids
					f32 U = solid->uvpos[uvi].X;
					f32 V = solid->uvpos[uvi].Y;
					f32 OU = solid->ouvpos[uvi].X;
					f32 OV = solid->ouvpos[uvi].Y;
					if (!uvresize) { //shift
						U += uu;
						V += vv;
					}
					else { //resize
						U *= (1.0f - uu);
						V *= (1.0f - vv);
					}
					if (resetUV == true) {
						segui->addLog(SAY("Orignal x:%f, Orignal y: %f, x: %f and y:%f for surface texture\n", OU, OV, U, V));
						solid->uvpos[uvi].X = OU;
						solid->uvpos[uvi].Y = OV;
					}
					else {
						solid->uvpos[uvi].X = U;
						solid->uvpos[uvi].Y = V;
					}
				}
			}

			//update what is displayed on screen, all red surfs
			if (resetUV == true) {
				resetUV = false;
			}
			updateBufferUVfromScnSurf(s, rs);

			//update what is displayed on screen, all blue surfs too
			updateBufferUVfromScnSurf(s, bs);
		}
	}
	return true;
}
void updateBufferUVfromScnSurf(u32 meshIndex,core::array < u32 > si) {
  //  CScnLightmap lmap = scn->getLightmap();
	for (u32 i = 0; i < si.size(); i++)
		updateBufferUVfromScnSurf(meshIndex, si[i]);
}

void updateBufferUVfromScnSurf(u32 meshIndex, u32 si) {
		CScnSolid* solid = scn->getSolid(meshIndex);
		scnSurf_t* surfi = &solid->surfs[si];
		CDynamicMeshBuffer* buffer = (CDynamicMeshBuffer*)scnMeshes[meshIndex]->getMeshBuffer(si);
		CScnLightmap lmap = scn->getLightmap();
		f32* mults = lmap.getMults(meshIndex, si);


		for (u32 i = 0; i < surfi->vertidxlen; i++) {
			core::vector2df uvi = solid->uvpos[solid->uvidxs[surfi->vertidxstart + i]];
			S3DVertex2TCoords * verts=(S3DVertex2TCoords*) buffer->getVertices();
			verts[i].TCoords.X = uvi.X;
			verts[i].TCoords.Y = uvi.Y;
			if (mults) {
				verts[i].TCoords2.X = (uvi.X * mults[0] + mults[2]);
				verts[i].TCoords2.Y = -(uvi.Y * mults[1] + mults[3]);
			}
			else {
				verts[i].TCoords2.X = (uvi.X);
				verts[i].TCoords2.Y = -(uvi.Y);
			}
		   
		}

}

//Save system
bool scnSaveFile() {
	//Gets solids
	
	//Record texture names
	//if file is open
	if (output&&output->is_open()) {
		for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
			CScnSolid* solid = scn->getSolid(s);
			//save textures to file
			for (u32 i = 0; i < solid->n_surfs; i++) {
				output->seekp(solid->surfsad[i]);
				write_generic(&solid->surfs[i],output, 72);
			 
			}

			//save planes
			output->seekp(solid->planessad);
			u32 len = (solid->n_planes) * sizeof(scnPlane_t);
			output->write((char*)(solid->planes), len);

			//save vertices
			output->seekp(solid->vertssad);
			len = (solid->n_verts) * sizeof(core::vector3df);
			output->write((char*)(solid->verts), len);


			//save uvpos
			output->seekp(solid->uvposad);
			len = (solid->n_uvpos) * sizeof(core::vector2df);
			output->write((char*)(solid->uvpos), len);

			for (u32 c = 0; c < solid->n_cells; c++) {
				for (u32 l = 0; l < solid->rawcells[c].leafnode.size(); l++){
					scnCellData_t* celldata = solid->rawcells[c].leafnode[l];
					output->seekp(celldata->bbsad);
					u32 len = 2 * sizeof(core::vector3df);
					output->write((char*)celldata->bb_verts, len);
				}
			}
		}
		for (u32 e = 0; e < scn->getTotalEnts(); e++) {
			CScnEnt* ent = scn->getEnt(e);
			for (u32 f = 0; f < ent->n_fields; f++)
			{
				CScnEnt::field* fi = &ent->fields[f];
				output->seekp(ent->entsad[f]);
				write_generic(fi->key, output, ent->keylengths[f]);
				write_generic(fi->value, output, ent->vallengths[f]);
			}
		}
		if (receiver->isStripped()) {
			size_t pos = scn->getLightmap().getOffset();
			output->close();
			filesystem::resize_file(scnfilename.c_str(), pos);
		}
		 

		segui->addLog(SAY("Changes saved to file.\n"));
		if (bData == true) {
			segui->infoBox(bData, 3, 0, 0, 0);
		}
		return true;
	}
	else
		//Displays error if file isn't open
		error(false, "Output file stream is not open! Make sure it's not read only.");
	return 0;
}
void uvgrid_increase() {
	//mutiplys uv grid by 2
	uvgridmult *= 2.0;
	//checks if grid is more then 1 then sets grid to 1
	if (uvgridmult > 1.0) uvgridmult = 1.0;
	segui->addLog(SAY("Setting UV grid to %4.3f\n", uvgridmult));
	//sets a variable the gui looks at
	segui->uvgrid_setMult(uvgridmult);
}

void uvgrid_decrease() {
	//just divides the uv grid by 2
	uvgridmult *= 0.5;
	segui->addLog(SAY("Setting UV grid to %4.3f\n", uvgridmult));
	segui->uvgrid_setMult(uvgridmult);
}

void toggleUVresize() {
	//Reverses uv resize
	uvresize = !uvresize;

	if (uvresize) segui->addLog(SAY("Now uv operation set to resize\n"));
	else segui->addLog(SAY("Now uv operation set to move\n"));
	//sets uv grid option in gui
	segui->uvgrid_setOp(uvresize);

}
wchar_t* getBaseDirectory() {
	return BaseDirectory;
}
//IMPORTANT figure out why he commented out this bit of code because this bit of code
//is supose to split up blue surfaces(shared surfaces) with red surfaces
/*void split()
//splits the uv vertices of the red surfaces from the blue surfaces, so they can move independently
//i'm sure there's much redundancy in this function, as it should be better implemented in CSelector,
//but as long as it works...
{
	core::array<u32>& rs = sel->getRedSurfs();
	core::array<u32>& bs = sel->getBlueSurfs();
	core::array<vertProp_t> verts;

	for (u32 i=0; i<rs.size(); i++)
	{
		vprops = sel->getSurfVertProps(i);
		for (u32 ii=0; ii<verts.size(); ii++)
		{
			if (verts[ii].bShared) //if vertex is shared
			{
				for (u32 k=0; k<verts[ii].sharesWith.size(); k++)
				{
					if (bs.binary_search(verts[ii].sharesWith[k])>-1) //if this vertex shares with a blue plane
					{
						makeNewUVVert(i,ii)
				}
			}
		}
	}

void makeNewUVVert(u32 si, u32 idxi)
{
}

}*/
arrayu getSurfUVIdxs(u32 meshIndx,u32 si) {
	//creates uv id
	arrayu uvidxs;
	//gets selected solid
	CScnSolid* solid = scn->getSolid(meshIndx);
	//gets surface index
	scnSurf_t* surfi = &solid->surfs[si];
	//checks the vertexs from surface
	for (u32 i = 0; i < surfi->vertidxlen; i++)
		uvidxs.push_back(solid->uvidxs[surfi->vertidxstart + i]);
	return uvidxs;
}

arrayu getSurfUVIdxs(u32 meshIndex, arrayu ss) {
	//creates uv index and surface index
	u32 si, uvi;
	arrayu all;
	while (ss.iterate(si)) {
		arrayu uvis = getSurfUVIdxs(meshIndex,si);
		//cannot use while (getSurfUVIdxs(si).iterate(uvi)) or else it will keep calling getSurfUVIdxs(si) each time it iterates
		while (uvis.iterate(uvi))
			if (all.binary_search(uvi) == -1) //if not already there, add
				all.push_back(uvi);
	}
	return all;
}

arrayu getSurfUVIdxs(u32 meshIndex,core::array < u32 > ss) {
	return getSurfUVIdxs(meshIndex,arrayu(ss));
}

core::list < u32 > getSurfUVIdxs_list(u32 meshIndex,u32 si) { //creates uv indexes
	core::list < u32 > uvidxs;
	//sets selected solid
	CScnSolid* solid = scn->getSolid(meshIndex);
	//sets surface
	scnSurf_t* surfi = &solid->surfs[si];
	//Checks vertexs from surface index
	for (u32 i = 0; i < surfi->vertidxlen; i++)
		uvidxs.push_back(solid->uvidxs[surfi->vertidxstart + i]);

	return uvidxs;
}

//void putBoxesInSharedVerts(u32 s) {
//    arrayu rs;
//    arrayu bs;
//    rs.push_back(sels->getRedSurfs(s));
//    bs.push_back(sels->getBlueSurfs(s));
//
//    u32 r, b = 0, swi;
//    vertProp_t vp;
//    if (sels->boxes.size()>0 && &sels->getBoxes(s)) {
//            sels->getBoxes(s).set_used(0);
//    }
//
//    while (rs.iterate(r)) {
//        myarray < vertProp_t > rvprops;
//        rvprops.push_back(sels->getSurfVertProps(s,r));
//        while (rvprops.iterate(vp)) {
//            arrayu sharesWith;
//            sharesWith.push_back(vp.sharesWith);
//            while (sharesWith.iterate(swi)) {
//                s32 bi = bs.binary_search(swi);
//                if (bi > -1) {
//                    segui->addLog(SAY("Surfs[%d]: vertex [%d]: shared with blue surf %d\n", r, vp.surf_vertidx, bs[bi]));
//
//                    CScnSolid* solid = scn->getSolid(s);
//                    core::vector3df vert = solid->verts[solid->vertidxs[vp.vertidxidx]];
//                    f32 dx, dy, dz = dx = dy = 1.0;
//                    sels->getBoxes(s).push_back(aabbox3df(vert.X - dx, vert.Y - dy, vert.Z - dz, vert.X + dx, vert.Y + dy, vert.Z + dz));
//                }
//            }
//        }
//    }
//
//}
i_point3f getVertexIndex() {
	if (sels) {
		if (sels->getSharedVerts().size()>0&& switchToShared) {
			return sels->getSharedVert(sharedIndx);
		}
		else if(sels->getVerts().size() > 0) {
			return sels->getVert(selIndx);
		}
	}
	return { 0,0,0 };
}

void sayProperties()
//writes properties of last select surface
{
	/*u32 uvi;
	  segui->addLog(SAY("Red surfs uvidxs: "));
	  arrayu verts = getSurfUVIdxs(sel->getRedSurfs());
	  while(verts.iterate(uvi))
		  segui->addLog(SAY("%d ", uvi));
	  segui->addLog(SAY("\n"));*/
	  //returns the size of the red surface array if it's 0
	std::string firstHalf = "";
	std::string secondHalf = "";
	char part[150];
	segui->infoBox(bData, 0, 0, 0, 0);
	if (selent != -1) {
		segui->addLog(SAY("Entity id: %i\n", selent));
		firstHalf += "| Id: ";
		firstHalf += std::to_string(selent);
		firstHalf += " ";
		CScnEnt* ent = scn->getEnt(selent);
		for (CScnEnt::field* fi = ent->fields; fi != &ent->fields[ent->n_fields]; fi++) {
		
			if(strcmp(fi->key,"classname")==0) segui->addLog(SAY("+ %s %s\n", fi->key, fi->value));
			else segui->addLog(SAY("| %s %s\n", fi->key, fi->value));
			if (bData == true&&bBrief==false) {
				strcpy_s(part, "");
				strcat_s(part , "| ");
				strcat_s(part, fi->key);
				strcat_s(part, ": ");
				strcat_s(part, fi->value);
				strcat_s(part, " ");
				if (firstHalf.length() < 100)firstHalf += part;
				else secondHalf += part;
			}
		}
		if (firstHalf.length() > 0) firstHalf += "|";
		if (secondHalf.length() > 0) secondHalf += "|";
		if (bData == true && bBrief == false) segui->entity_details(firstHalf.c_str(), secondHalf.c_str());
	}

	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
	//gets the surface index of the last red surface
		if ((sels->getRedSurfs(s)).size()>0) {
			u32 si = (sels->getRedSurfs(s)).getLast();
			//gets surface index of a solid surface

			CScnSolid* solid = scn->getSolid(s);
			scnSurf_t* surfi = &solid->surfs[si];
			segui->addLog(SAY("+ surf[%d]:\n", si));

			segui->addLog(SAY("|      texture = %s\n", surfi->texture));
			segui->addLog(SAY("|        flags = %uh\n", surfi->flags));
			segui->addLog(SAY("|        alpha = %uh\n", surfi->alpha));
			segui->addLog(SAY("| lighmap size = %ux%u\n", surfi->lmsize_h, surfi->lmsize_v));
			if (bData == true && bBrief == true) {
				segui->infoBox(bData, 1, si, surfi, 0);
			}

			segui->addLog(SAY("| texture size = %ux%u\n", surfi->width, surfi->height));

			if (bBrief == false) {
				segui->addLog(SAY("|verts idx idx = from %d to %d\n", surfi->vertidxstart, surfi->vertidxstart + surfi->vertidxlen));
				segui->addLog(SAY("|  + plane idx = %u\n", surfi->planeidx));
				scnPlane_t* planei = &(solid->planes[surfi->planeidx]);
				segui->addLog(SAY("|  | normal = (%.1f, %.1f, %.1f), d=%.1f\n", planei->a, planei->b, planei->c, planei->d));
				segui->addLog(SAY("|      shading = %u\n", surfi->hasVertexColors));

				//print shadind info
				/*if (surfi->more)
				{
					u8 * ps = surfi->shading;
					for (u32 i=0; i<surfi->vertidxlen; i++)
					{
						segui->addLog(SAY("               | %u %u %u %u\n",ps[0],ps[1],ps[2],ps[3]));
						ps+=4;
					}
				}*/
				segui->addLog(SAY("|    (unknown) = %.1f %.1f \n", surfi->unk[0], surfi->unk[1]));
				segui->addLog(SAY("|    (unknown 4 s16) = %hd %hd %hd %hd\n", surfi->stuff2[2], surfi->stuff2[4], surfi->stuff2[6], surfi->stuff2[8]));
				segui->addLog(SAY("|    (unknown 2 s32) = %d %d \n", surfi->stuff2[2], surfi->stuff2[6]));
				if (bData == true) {
					segui->infoBox(bData, 2, si, surfi, planei, getVertexIndex());
				}
				
				//SAY("|        istga = %u\n",scnMesh->getMeshbuffer(si)->get
			}
		}
	}
}
//using foreach, not working properly, (can't use getSurfUVIdxs inside foreach
/*void saySurfProps()
{
	segui->addLog(SAY("Red surfs uvidxs: "));
	arrayu verts = getSurfUVIdxs(sel->getRedSurfs());

	for_each(u32, uvi, getSurfUVIdxs(sel->getRedSurfs()))
		segui->addLog(SAY("%d ", *uvi));

//for(arrayu::iterator uvi=verts.startIteration(); !uvi.hasEnded(); uvi++)
		//segui->addLog(SAY("%d ", *uvi));

	segui->addLog(SAY("\n"));

}*/

/*void whereami()
{
	if (!camera) return;

	vector3df pos= camera->getPosition();
	should use instead of getAbsolutePosition, in case scene is not in origin

if (bBrief == false)
	{
	segui->addLog(SAY("Where Am I? %.3f %.3f %.3f\n",pos.X,pos.Y,pos.Z));
	}
	else
	{
	segui->addLog(SAY("Where Am I? (x y z) %.0f %.0f %.0f\n",pos.X,pos.Y,pos.Z));
	}

}*/
//sets last postion to 0, 0, 0
vector3df lastpos(.0, .0, .0);
//Basicly just checks when you move
void whereami() {
	if (!camera || !segui) return;
	//gets camera postion
	vector3df pos = camera->getPosition();
	//vector3df pos= camera->getAbsolutePositionPosition();
	//If postion isn't 0,0,0
	if (pos != lastpos) {
		segui->addLog(SAY("Camera position: (x) %.0f (y) %.0f (z) %.0f\n", pos.X, pos.Y, pos.Z));
		lastpos = pos;
	}
	
	i_point3f vert = getVertexIndex();
	if(!(vert.X==0&&vert.Y==0&&vert.Z==0)){
		segui->addLog(SAY("Surface position: (x) %.0f (y) %.0f (z) %.0f\n", vert.X, vert.Y, vert.Z));
	}
   
   // for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
	if (scn) {
		CScnSolid* solid = scn->getSolid(0);
		int cellindx = solid->getCellAtPos(pos);
		if (cellindx > -1) {
			scnRawCell_t cell = solid->rawcells[cellindx];
			segui->addLog(SAY("Cell: %s\n", cell.name));
			segui->pos_update(pos.X, pos.Y, pos.Z, cell);
		}
		else {
			segui->pos_update(pos.X, pos.Y, pos.Z);
		}
	}
	else {
		segui->pos_update(pos.X, pos.Y, pos.Z);
	}
}
void export2obj() {
	if (scn) {
		//exports object, map and 3ds(what ever that is)
		scnExportObj(scn, "exported");
		scnExport3ds(scn->getAllSolids(), scn->getSolidSize(bLoadAll), "exported");
		scnExportMap(scn, "exported");
		//exit(42);
	}
}
//TODO: Improve this mess.
void changeAlpha() {
	if (scn) {
		for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
			//returns the size of the red surface array if it's 0
			//gets the surface index of the last red surface
			if (sels->getRedSurfs(s).size() > 0) {
				u32 si = sels->getRedSurfs(s).getLast();
				//gets surface index of a solid surface

				CScnSolid* solid = scn->getSolid(s);
				scnSurf_t* surfi = &solid->surfs[si];

				u16 newAlpha = surfi->alpha;

				if (bData == false) {
					segui->addLog(SAY("Please enter a number from 0 to 255: "));
					cin >> newAlpha;
					if (newAlpha < 0 || newAlpha > 255) {
						segui->addLog(SAY("\nWell, it seems that you put an improper value"));
						segui->addLog(SAY("\nSo I made sure that the alpha didn't change"));
						newAlpha = surfi->alpha;
					}
					surfi->alpha = newAlpha;
					segui->displayValue("Alpha", newAlpha);
				}
			}
		}
		if (bData == true) {
			createAlpha();
		}
	}
}
void updateAlpha(std::string val, bool data) {
	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
		CScnSolid* solid = scn->getSolid(s);
		if (sels->getRedSurfs(s).size() > 0) {
			u32 si = sels->getRedSurfs(s).getLast();
			scnSurf_t* surfi = &solid->surfs[si];
			int intVal = atoi(val.c_str());
			if (val != "blank") {
				segui->displayUpdate(intVal, 255);
			}
			else {
				segui->askForInput(255);
			}
			if (data == true) {
				if (intVal > 255) {
					intVal = 255;
				}
				surfi->alpha = intVal;

				segui->displayValue("Alpha", intVal);
			}
		}
	}
}
void updateIndex(bool increase) {
	if (sels) {
		if (sels->getVerts().size() > 0 && !switchToShared) {
			if (selIndx < sels->getVerts().size()-1 && increase) {
				selIndx++;
			}
			else if (increase) {
				if (sels->getSharedVerts().size() > 0) {
					switchToShared = true;
			   
				}
				 selIndx = 0;
				 sharedIndx = 0;
			}
			else if(selIndx > 0&&!increase) {
				selIndx--;
			}
			else if (!increase) {
				if (sels->getSharedVerts().size() > 0) {
					switchToShared = true;
					sharedIndx = sels->getSharedVerts().size() - 1;
				}
				selIndx = sels->getVerts().size() - 1;
			}
		  
		}
		else if (sels->getSharedVerts().size() > 0 && switchToShared) {
			if (sharedIndx < sels->getSharedVerts().size()-1 && increase) {
				sharedIndx++;
			 
			}
			else if (increase) {
				if (sels->getVerts().size() > 0) {
					switchToShared = false;
				}
				selIndx = 0;
				sharedIndx = 0;
			}
			else if (sharedIndx > 0 && !increase) {
				sharedIndx--;
			}
			else if (!increase) {
				if (sels->getVerts().size() > 0) {
					switchToShared = false;
					selIndx = sels->getVerts().size() - 1;
				}
			  
				sharedIndx = sels->getSharedVerts().size() - 1;
			}       
		}
		i_point3f vert = getVertexIndex();
		segui->addLog(SAY("surf %u  \n",vert.surf_vertindx));
		segui->update_index(vert);
	}
   

}


void changeShading() {
	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
		//returns the size of the red surface array if it's 0
		//gets the surface index of the last red surface
		if (sels->getRedSurfs(s).size() > 0) {
			u32 si = sels->getRedSurfs(s).getLast();
			//gets surface index of a solid surface

			CScnSolid* solid = scn->getSolid(s);
			scnSurf_t* surfi = &solid->surfs[si];

			u16 newShading = surfi->hasVertexColors;

			if (bData == false) {
				segui->addLog(SAY("Please enter a number from 0 to 1: "));
				cin >> newShading;
				if (newShading < 0 || newShading > 1) {
					segui->addLog(SAY("\nWell, it seems that you put an improper value"));
					segui->addLog(SAY("\nSo I made sure that the alpha didn't change"));
					newShading = surfi->hasVertexColors;
				}
				surfi->alpha = newShading;
				segui->displayValue("Shading", newShading);
			}
		}
	}
	if (bData == true) {
		createShading();
	}
}
void updateShading(std::string val, bool data) {
	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
		CScnSolid* solid = scn->getSolid(s);
		if (sels->getRedSurfs(s).size() > 0) {
			u32 si = sels->getRedSurfs(s).getLast();
			scnSurf_t* surfi = &solid->surfs[si];
			int intVal = atoi(val.c_str());
			if (val != "blank") {
				segui->displayUpdate(intVal, 1);
			}
			else {
				segui->askForInput(1);
			}
			if (data == true) {
				if (intVal > 1) {
					intVal = 1;
				}
				surfi->hasVertexColors = intVal;

				segui->displayValue("Shading", intVal);
			}
		}
	}
}

void changeFlag() {
	if (scn) {
		for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
			//returns the size of the red surface array if it's 0
			//gets the surface index of the last red surface
			if (sels->getRedSurfs(s).size() > 0) {
				u32 si = sels->getRedSurfs(s).getLast();
				//gets surface index of a solid surface

				CScnSolid* solid = scn->getSolid(s);
				scnSurf_t* surfi = &solid->surfs[si];

				u16 newFlags = surfi->flags;

				if (bData == false) {
					segui->addLog(SAY("Please enter a number from 0 to 999: "));
					cin >> newFlags;
					if (newFlags < 0 || newFlags > 999) {
						segui->addLog(SAY("\nWell, it seems that you put an improper value"));
						segui->addLog(SAY("\nSo I made sure that the flag didn't change"));
						newFlags = surfi->flags;
					}
					surfi->flags = newFlags;
					segui->displayValue("Flag", newFlags);
				}
			}
		}
		if (bData == true) {
			createFlag();
		}
	}
}
void updateFlag(std::string val, bool data) {
	for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
		CScnSolid* solid = scn->getSolid(s);
		if (sels->getRedSurfs(s).size() > 0) {
			u32 si = sels->getRedSurfs(s).getLast();
			scnSurf_t* surfi = &solid->surfs[si];
			int intVal = atoi(val.c_str());
			if (val != "blank") {
				segui->displayUpdate(intVal, 9999);
			}
			else {
				segui->askForInput(9999);
			}
			if (data == true) {
				if (intVal > 9999) {
					intVal = 9999;
				}
				surfi->flags = intVal;

				segui->displayValue("Flag", intVal);
			}
		}
	}
}

void SetOriginalUV() {
	resetUV = true;
	segui->addLog(SAY("The uv texture on this plane has been reset\n"));
	segui->displayUV();
	scnRetexture_UV(0, 0);
}
void updateMovementSpeed(f32 increment) {
	
	f32 currVal=ca->getMoveSpeed()+increment;
	if (currVal > 0.01) {
		ca->setMoveSpeed(currVal);
	}
	segui->movementSpeed(ca->getMoveSpeed()*2);
}
void moveEntity(s32 x, s32 y, s32 z) {
	if (selent != -1) {
		CScnEnt* ent = scn->getEnt(selent);
		
		s32 xEnt = round32(selectNode->getPosition().X)+x;
		s32 yEnt = round32(selectNode->getPosition().Y)+y;
		s32 zEnt = round32(selectNode->getPosition().Z)+z;
		char outOrg[128];
		sprintf_s(outOrg, "%i %i %i", xEnt, yEnt, zEnt);
		ent->setField("origin", outOrg);
		selectNode->setPosition(vector3df(xEnt, yEnt, zEnt));
		segui->update_origin(outOrg);
	}
}
void moveVertex(s32 x, s32 y, s32 z) {
	
	
	if (bMoveVertex) {
		if (sels) {
			core::array<i_point3f> other_verts;
			for (u16 i = 0; i < sels->getSharedVerts().size(); i++) {
				if (!switchToShared || i != sharedIndx)other_verts.push_back(sels->getSharedVert(i));
			}
			for (u16 i = 0; i < sels->getVerts().size(); i++) {
				if(switchToShared||i!= selIndx)other_verts.push_back(sels->getVert(i));
			}
			i_point3f vert = { 0,0,0 };
			if (sels->getSharedVerts().size() > 0 && switchToShared) {
				float dist = 0.15f;
				vert = sels->getSharedVert(sharedIndx);
				vert.X += x;
				vert.Y += y;
				vert.Z += z;
				sels->setSharedVert(sharedIndx, vert);
				sharedSpheres[sharedIndx]->setPosition(vector3df(vert.X - dist, vert.Y - dist, vert.Z - dist));
			}
			else if (sels->getVerts().size() > 0) {
				float dist = 0.2f;
				vert = sels->getVert(selIndx);
				vert.X += x;
				vert.Y += y;
				vert.Z += z;
				sels->setVert(selIndx, vert);
				selSpheres[selIndx]->setPosition(vector3df(vert.X - dist, vert.Y - dist, vert.Z - dist));
			}
			if (!(vert.X == 0 && vert.Y == 0 && vert.Z == 0)) {
				CScnSolid* solid= scn->getSolid(vert.meshindx);
				scnCellData_t* celldata;
				for (u32 j = 0; j < solid->n_cells; j++) {
					celldata=solid->getBBFromSurf(vert.surfindx, j);
					if (celldata) {
						core::vector3df v1 = celldata->bb_verts[0];
						core::vector3df v2 = celldata->bb_verts[1];//Update celldata
						//I am lazy so here's some poorly done code.
						if(x>0 && v2.X < vert.X){
							celldata->bb_verts[1].X++;
						}
						if (y > 0 && v2.Y < vert.Y) {
							celldata->bb_verts[1].Y++;
						}
						if (z > 0 && v2.Z < vert.Z) {  
							celldata->bb_verts[1].Z++;
						}
						if (x < 0&&v1.X > vert.X) {
							celldata->bb_verts[0].X--;
						}
						if (y < 0 && v1.Y > vert.Y) {
							celldata->bb_verts[0].Y--;
						}
						if (z < 0 && v1.Z > vert.Z) {
							celldata->bb_verts[0].Z--;
						}
						bool larger = false;
						for (u16 i = 0; i < other_verts.size(); i++) {
							if (x < 0 && v2.X-1.5 > other_verts[i].X) {
								larger = true;
							}
							if (y < 0 && v2.Y-1.5 > other_verts[i].Y) {
								larger = true;
							}
							if (z < 0 && v2.Z-1.5 > other_verts[i].Z) {
								larger = true;
							}
							if (x > 0 && v1.X+1.5 < other_verts[i].X) {
								larger = true;
							}
							if (y > 0 && v1.Y+1.5 < other_verts[i].Y) {
								larger = true;
							}
							if (z > 0 && v1.Z+1.5 < other_verts[i].Z) {
								larger = true;
							  
							}
							if (larger) break;
						}
						if (x < 0 && larger) {
							celldata->bb_verts[1].X--;
						}
						if (y < 0 && larger) {
							celldata->bb_verts[1].Y--;
						}
						if (z < 0 && larger) {
							celldata->bb_verts[1].Z--;
						}
						if (x > 0 && larger) {
							celldata->bb_verts[0].X++;
						}
						if (y > 0 && larger) {
							celldata->bb_verts[0].Y++;
						}
						if (z > 0 && larger) {
							celldata->bb_verts[0].Z--;
						}
					}
				}

				solid->verts[solid->vertidxs[vert.vertindx]] = { vert.X,vert.Y,vert.Z };

				updateMeshAndNormal(vert);
			}
			//segui->update_index(vert);
		}
	}

}void updateMeshAndNormal(i_point3f vert)
{   
	u32 x = vert.surf_vertindx;
	scene::SMesh* mesh = (scene::SMesh * )nodes[vert.meshindx]->getMesh();
	//First update the mesh
	if (mesh->getMeshBufferCount() >= vert.surfindx) {
		CDynamicMeshBuffer* buffer = (CDynamicMeshBuffer*)mesh->getMeshBuffer(vert.surfindx);
		S3DVertex2TCoords* verts = (S3DVertex2TCoords*)buffer->getVertices();
		verts[vert.surf_vertindx].Pos = vector3df(vert.X, vert.Y, vert.Z);
	
		for (u32 i = 0; i < mesh->getMeshBufferCount(); i++) {
			if (mesh->getMeshBufferCount() >= vert.surfindx&& i != vert.surfindx) {
				CDynamicMeshBuffer* extrabuffer = (CDynamicMeshBuffer*)mesh->getMeshBuffer(i);
				S3DVertex2TCoords* everts = (S3DVertex2TCoords*)extrabuffer->getVertices();
				for (u32 j = 0; j < extrabuffer->getVertexCount(); j++) {
					vector3df tmp = everts[j].Pos;
					if (fabs(vert.X - tmp.X) <=1 && fabs(vert.Y - tmp.Y) <= 1 && fabs(vert.Z - tmp.Z) <=1) {
						everts[j].Pos = vector3df(vert.X, vert.Y, vert.Z);
					}
				}
			}
		}
		
		//Refresh the selected to follow new geometry
		selector = device->getSceneManager()->createTriangleSelector(mesh, nodes[vert.meshindx]);
		nodes[vert.meshindx]->setTriangleSelector(selector);
		//clears selector
		selector->drop();

		//Calculate normal using mesh buffer.
		u32 weights=0;
		vector3df sum= vector3df(0,0,0);
		vector3df sumMidpoint = vector3df(0, 0, 0);
		for (u32 i = 0; i<buffer->getIndexCount(); i+=3) {
			S3DVertex2TCoords* verts = (S3DVertex2TCoords*)buffer->getVertices();
			u16* indices = buffer->getIndices();
			vector3df vecP1 = verts[indices[i]].Pos;
			vector3df vecP2 = verts[indices[i+1]].Pos;
			vector3df vecP3 = verts[indices[i + 2]].Pos;
			
			vector3df midpoint = (vecP1 + vecP2 + vecP3) / 3;

			vector3df vecA = vecP2 - vecP1;
			vector3df vecB = vecP3 - vecP1;
			vector3df dir= vecA.crossProduct(vecB);
			f32 area = (0.5 * dir.getLength());

			vector3df normal= dir.normalize();
			sum += normal*area;
			sumMidpoint += midpoint * area;
			weights += area;
		}
		vector3df avg = sum / weights;
		vector3df avgMidpoint = sumMidpoint / weights;
		f32 d = -((avg.X * avgMidpoint.X) + (avg.Y * avgMidpoint.Y) + (avg.Z * avgMidpoint.Z));
		CScnSolid* solid = scn->getSolid(vert.meshindx);
		scnSurf_t* surfi = &solid->surfs[vert.surfindx];
		scnPlane_t* planei = &(solid->planes[surfi->planeidx]);
		planei->a = avg.X;
		planei->b = avg.Y;
		planei->c = avg.Z;
		planei->d = d;
		segui->infoBox(bData, 2, 0, surfi, planei, getVertexIndex());
	}

}
void toggleLightmap() {
	if (scn) {
		for (u32 s = 0; s < scn->getSolidSize(bLoadAll); s++) {
			CScnSolid* solid = scn->getSolid(s);
			scene::SMesh* mesh = (scene::SMesh*)nodes[s]->getMesh();
			if (mesh->getMeshBufferCount() >= solid->n_surfs) {
				for (u32 i = 0; i < solid->n_surfs; i++) {
					CDynamicMeshBuffer* buffer = (CDynamicMeshBuffer*)mesh->getMeshBuffer(i);
					E_MATERIAL_TYPE mt = getBaseEMT(s, i);
					ITexture* lt =nullptr;
					if (!receiver->isHidden()) lt = getBaseLightmap(s, i);
					if (receiver->isHidden() && mt == EMT_LIGHTMAP) mt = EMT_SOLID;
					buffer->getMaterial().MaterialType = mt;
					buffer->getMaterial().setTexture(1, lt);
				}
			}
			if (sels->getRedSurfs(s).size() > 0) {
				sels->paint(s, sels->getRedSurfs(s), CSelector::E_PAINT_TYPE::RED);
			}
			if (sels->getBlueSurfs(s).size() > 0) {
				sels->paint(s, sels->getBlueSurfs(s), CSelector::E_PAINT_TYPE::BLUE);
			}
		}
	}
}
bool canChangeLightmap() {
	if (scn) {
		return scn->getLightmap().hasLightmaps();
	}
	return false;
}

void lookAtVertex() {
	i_point3f vert = getVertexIndex();
	if (!(vert.X == 0 && vert.Y == 0 && vert.Z==0)) {
		camera->bindTargetAndRotation(true);
		camera->setTarget(vector3df(vert.X,vert.Y,vert.Z));
		camera->bindTargetAndRotation(false);
	}
}
void addLogFromReciever(core::stringw str) {
	if (segui) {
		segui->addLog(str);
	}
}
void changePage(bool state, bool increase) {
	pageIncrease = increase;
	pageDown = state;
	counter = 100000;
	superCounter = 0;

}
void openDebug() {
	if (segui) {
		segui->openDebug();
	}

}
//La fin
#endif