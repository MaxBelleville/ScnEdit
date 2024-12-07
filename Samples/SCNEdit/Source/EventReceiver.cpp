#include "pch.h"

#include <irrlicht.h>
#include "Header/EventReceiver.h"
#include "Header/main.h"
#if false
using namespace std;
using namespace irr;
using namespace gui;
using namespace video;
using namespace core;

bool canselectsurf=true;
bool setAlpha=false;
bool setFlag = false;
bool setShading = false;
bool isDialogOpen=false;
bool switchClick=false;
bool bCtrlIsPressed=false;
bool willStripLightmap = false;
bool willHideLightmap = false;
bool prevLightmap = false;
bool uvmove=true;
std::string val = "";
std::string valFull = "";
s32 id;


//constructor
MyEventReceiver::MyEventReceiver(IrrlichtDevice * dev)
{
   device=dev;
   env = device->getGUIEnvironment();
   cnt=0;
}

bool MyEventReceiver::OnEvent(const SEvent& event)
//should return true if event was processed
{
   switch (event.EventType)
   {
	   //Input for editor
	   case EET_LOG_TEXT_EVENT:return ERhandleLogs(event.LogEvent);
	   case EET_GUI_EVENT: return ERhandleGUI(event.GUIEvent);
	   case EET_MOUSE_INPUT_EVENT: return ERhandleMouse(event.MouseInput);
	   case EET_KEY_INPUT_EVENT: return ERhandleKey(event.KeyInput);
	   default: return false;
   }
}

bool MyEventReceiver::ERhandleLogs(const SEvent::SLogEvent& LogEvent) {
	//IGUIElement* ldebug = env->getRootGUIElement()->getElementFromId(104);
	switch (LogEvent.Level) {
		case ELL_DEBUG:
			return true;
			break;
		case ELL_ERROR:
			error("%s\n", LogEvent.Text);
			return true;
			break;
		case ELL_INFORMATION:
			return true;
			break;
		case ELL_WARNING:
			//core::stringw log = 
			addLogFromReciever(WARN("%s\n", LogEvent.Text));
			return true;
			break;
		default:
			return true;
	}
}

bool MyEventReceiver::ERhandleGUI(const SEvent::SGUIEvent& GUIEvent)
{

   id = GUIEvent.Caller->getID();
   IGUIElement* dialog = GUIEvent.Caller;
   io::path filename;
   IGUIElement* lmapGui = env->getRootGUIElement()->getElementFromId(104);
   IGUIElement* stripGui = env->getRootGUIElement()->getElementFromId(105);

   switch(GUIEvent.EventType)
   {    
	  case EGET_BUTTON_CLICKED:
		 if (!isDialogOpen)
		 {
			switch (id)
			{

			   case (199): //quit
				  device->closeDevice();
				  break;
			   case (101): //open
				  env->addFileOpenDialog(L"Please choose a file.",true,0,200);
				  camera->setInputReceiverEnabled(false);
				  canselectsurf=false;
				  isDialogOpen=true;
				  break;
			   case (102): //close
				   willStripLightmap = false;
				   willHideLightmap = false;
				   stripGui->setText(L"Strip Lightmaps");
				   lmapGui->setText(L"Hide Lightmaps");
				  closeCurrentScnFile();
				  break;
			   case (103): //save
				  scnSaveFile();
				  break;
			   case (104): //toggle lightmap
				   if (canChangeLightmap()) {
					   willHideLightmap = !willHideLightmap;
					   prevLightmap = !prevLightmap;
				   }
					if (willHideLightmap) GUIEvent.Caller->setText(L"Show Lightmaps");
					else GUIEvent.Caller->setText(L"Hide Lightmaps");
					toggleLightmap();
				   break;
			   case (105): //strip lightmap
				   if (canChangeLightmap() && !willStripLightmap) {
						env->addMessageBox(L"Strip lightmaps?", L"When enabled all lightmaps will be stripped when you save, are you sure?", true, EMBF_OK | EMBF_CANCEL);
				   }
				   else if(canChangeLightmap()) {
					   willStripLightmap=false;
					   if (prevLightmap != willHideLightmap) {
						   willHideLightmap = prevLightmap;
						   toggleLightmap();
					   }
					   if (lmapGui != 0)   lmapGui->setVisible(true);   
					   GUIEvent.Caller->setText(L"Strip Lightmaps");
				   }
				  
				   break;
			   case (106): //open debug
				   openDebug();
				   break;
			   default:
				  return false;
			}
		 }
		 break;
	  case EGET_FILE_SELECTED:
		 filename = ((IGUIFileOpenDialog*)dialog)->getFileName();

		 switch(id)
		 {
			case(200):  //select scn file
			   //cast dialog as type IGUIFileOpenDialog and get filename
			   indirectLoad(filename);
			   isDialogOpen=false;
			   break;

			case (201): //select texture
				scnRetexture(filename);
				isDialogOpen=false;
				break;

			default:
			   return false;
		 }
		 canselectsurf=true;
		 break;

	  case EGET_MESSAGEBOX_OK:
		  if (lmapGui != 0) { lmapGui->setVisible(false); }
		  willHideLightmap = true;
		  willStripLightmap = true;
		  toggleLightmap();
		  if (stripGui != 0) { lmapGui->setVisible(false); }stripGui->setText(L"Unstrip Lightmaps");
		  break;
	  case EGET_FILE_CHOOSE_DIALOG_CANCELLED:
		 isDialogOpen=false;
		 canselectsurf=true;
		 break;

	  default:
		 return false;
   }
   return true;
}

bool MyEventReceiver::ERhandleMouse(const SEvent::SMouseInput& MouseInput)
{
	

   switch (MouseInput.Event)
   {
	  case EMIE_RMOUSE_PRESSED_DOWN: //rmouse
		 //toggle free-mouse
		 device->getCursorControl()->setVisible(!device->getCursorControl()->isVisible());
		 camera->setInputReceiverEnabled(!camera->isInputReceiverEnabled());
		 
		 env->setFocus(env->getRootGUIElement());
		 break;
	  case EMIE_LMOUSE_LEFT_UP:
		  if (id == 107|| id ==108)changePage(false);
		  break;
	  case EMIE_LMOUSE_PRESSED_DOWN: //lmouse
		 if (canselectsurf && !isDialogOpen)
			 selectCurrent(MouseInput.Control); //if control is pressed, append surface, else, don't

		if (id == 107) {
			changePage(true, false);
		}
		if (id == 108) {
			changePage(true, true);
		}
		 break;

   }
   return false; //always return false because we need the mouse for other events
}

bool MyEventReceiver::ERhandleKey(const SEvent::SKeyInput& KeyInput)
{
   //check to see if control is pressed down
   //from v0.3
   /*if (KeyInput.Ctrl == KEY_CONTROL)
   {
	   if (KeyInput.PressedDown) bCtrlIsPressed=true;
	   else bCtrlIsPressed = false;
	   return false; //return false because we use ctrl for other things
   }*/

	if (id == 404) {
		if (KeyInput.Key == KEY_KEY_C && KeyInput.Control) return false;
		if (KeyInput.Key == KEY_KEY_A && KeyInput.Control) return false;
		return true;
	}
	
   //only check key just pressed down events.
   if (!KeyInput.PressedDown) return false;
   if (KeyInput.Key >= KEY_KEY_0 && KeyInput.Key <= KEY_KEY_9) {
	   val = valFull;
	   valFull += std::to_string(KeyInput.Key - 48);
	   if (setAlpha == true) updateAlpha(valFull, false);
		if (setShading == true) updateShading(valFull, false);
		if (setFlag == true) updateFlag(valFull, false);
   }
   switch (KeyInput.Key)
   {
	case KEY_MINUS:
		if (KeyInput.Shift) {
			updateMovementSpeed(-0.01f);
		}
		else {
			updateIndex(false);
		}
		break;
	case KEY_PLUS:
		if (KeyInput.Shift) {
			updateMovementSpeed(0.01f);
		}
		else {
			updateIndex(true);
		}

		break;
	  case KEY_KEY_T:
		 env->addFileOpenDialog(L"Select new texture.",true,0,201);
		 camera->setInputReceiverEnabled(false);
		 device->getCursorControl()->setVisible(true);
		 canselectsurf=false;
		 isDialogOpen=true;
		 break;

	  case KEY_UP:
		  if (KeyInput.Control) {
			  if (uvmove)
			  {
				  scnRetexture_UV(0, 1);
			  }
			  else
			  {
				  scnRetexture_UV(0, -1);
			  }
		  }
		  else if (KeyInput.Shift) {

			  //UV Move if statments are for resizing else statments are for moving
			  //SAY("move %d\n",uvmove);
			  moveEntity(0, 0, 1);
			  moveVertex(0, 0, 1);
		  }
		  else {
			  moveEntity(0, 1, 0);
			  moveVertex(0, 1, 0);
		  }
		 break;
	  case KEY_DOWN:
		  if (KeyInput.Control) {
			  if (uvmove)
			  {
				  scnRetexture_UV(0, -1);
			  }
			  else
			  {
				  scnRetexture_UV(0, 1);
			  }
		  }
		  else if (KeyInput.Shift) {
			  moveEntity(0, 0, -1);
			  moveVertex(0, 0, -1);
		  }
		  else {
			  moveEntity(0, -1, 0);
			  moveVertex(0, -1, 0);
		  }

		 break;
	  case KEY_LEFT:
		  if (KeyInput.Control) {

			  if (uvmove)
			  {
				  scnRetexture_UV(1, 0);
			  }
			  else
			  {
				  scnRetexture_UV(-1, 0);
			  }
		  }
		  else {
			  moveEntity(1, 0, 0);
			  moveVertex(1, 0, 0);
		  }
		 break;
	  case KEY_RIGHT:
		  if (KeyInput.Control) {

			  if (uvmove)
			  {
				  scnRetexture_UV(-1, 0);
			  }
			  else
			  {
				  scnRetexture_UV(1, 0);
			  }
		  }
		  else {
			  moveEntity(-1, 0, 0);
			  moveVertex(-1, 0, 0);
		  }
		 break;

	  case KEY_KEY_E:
		 uvgrid_increase();
		 break;
	  case KEY_KEY_Q:
		 uvgrid_decrease();
		 break;
	  case KEY_KEY_G:
		  lookAtVertex();
		  break;
	  case KEY_KEY_R:
		 toggleUVresize();
		 break;
	  case KEY_KEY_O:
		 export2obj();
		 break;
	case KEY_KEY_C:
		 SetOriginalUV();
		 break;
	  case KEY_KEY_F:
		 changeFlag();
		 break;
	  case KEY_KEY_P:
		  changeAlpha();
		  break;
	  case KEY_KEY_X:
		 whereami();
		 break;
	  case KEY_BACK:
		valFull = val;
		val = "";
		if (setAlpha == true) updateAlpha(valFull,false);
		if (setShading == true) updateShading(valFull, false);
		if (setFlag == true) updateFlag(valFull, false);
		break;
	case KEY_RETURN:
		if (setAlpha == true) updateAlpha(valFull, true);
		if (setShading == true) updateShading(valFull, true);
		if (setFlag == true) updateFlag(valFull, true);
		setAlpha = false;
		setFlag = false;
		setShading = false;
		 break;
	  default:
		 return false;
   }
   return true;
}
void createAlpha() {
valFull = "";
val= "";
setAlpha = true;
updateAlpha("blank",false);
}

void createFlag() {
	valFull = "";
	val = "";
	setFlag = true;
	updateFlag("blank", false);
}

void createShading() {
	valFull = "";
	val = "";
	setShading = true;
	updateShading("blank", false);
}


bool MyEventReceiver::isStripped()
{
	return willStripLightmap;
}
bool MyEventReceiver::isHidden()
{
	return willHideLightmap;
}
#endif