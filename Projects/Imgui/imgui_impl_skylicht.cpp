/*
!@
MIT License

Copyright (c) 2019 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "pch.h"
#include "imgui_impl_skylicht.h"
#include "Material/Shader/CShaderManager.h"

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#endif

using namespace Skylicht;
using namespace irr;

ITexture *g_fontTexture = NULL;
int g_vertexColorShader = -1;
int g_textureColorShader = -1;

IMeshBuffer *g_meshBuffer = NULL;

bool ImGui_Impl_Skylicht_Init()
{
	ImGuiIO& io = ImGui::GetIO();
	printf("Setup imgui io\n");
	io.BackendPlatformName = "imgui_impl_skylicht";

	unsigned char* pixels = nullptr;
	int width = 0, height = 0;
	io.Fonts->AddFontDefault();
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	IVideoDriver *driver = getVideoDriver();
	core::dimension2du size((u32)width, (u32)height);

	IImage *img = driver->createImageFromData(video::ECF_A8R8G8B8, size, pixels, true, false);

	g_fontTexture = driver->addTexture("imgui_font", img);

	io.Fonts->SetTexID((ImTextureID)(intptr_t)g_fontTexture);
	img->drop();

	g_meshBuffer = new CMeshBuffer<S3DVertex>(driver->getVertexDescriptor(EVT_STANDARD), video::EIT_32BIT);
	g_meshBuffer->setHardwareMappingHint(EHM_STREAM);

	return true;
}

void ImGui_Impl_Skylicht_Shutdown()
{
	if (g_fontTexture != NULL)
	{
		getVideoDriver()->removeTexture(g_fontTexture);
		g_fontTexture = NULL;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->SetTexID(NULL);
	}

	if (g_meshBuffer != NULL)
	{
		g_meshBuffer->drop();
		g_meshBuffer = NULL;
	}
}

void ImGui_Impl_Skylicht_NewFrame()
{
	// Setup time step
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = Skylicht::getTimeStep() / 1000.0f;
	io.ClearInputKeys();
	io.ClearInputCharacters();

	// Start the frame
	ImGui::NewFrame();
}

void ImGui_Impl_Skylicht_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
{
	if (g_textureColorShader == -1)
	{
		CShaderManager *shaderManager = CShaderManager::getInstance();
		g_vertexColorShader = shaderManager->getShaderIDByName("VertexColorAlpha");
		g_textureColorShader = shaderManager->getShaderIDByName("TextureColorAlpha");
	}

	// set viewport
	IVideoDriver *driver = getVideoDriver();
	driver->setViewPort(core::recti(0, 0, fb_width, fb_height));

	core::matrix4 orthoMatrix;
	orthoMatrix.buildProjectionMatrixOrthoLH(
		draw_data->DisplaySize.x,
		-draw_data->DisplaySize.y,
		-1.0f,
		1.0f);
	orthoMatrix.setTranslation(core::vector3df(-1, 1, 0));

	driver->setTransform(video::ETS_PROJECTION, orthoMatrix);
	driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
}

void ImGui_Impl_Skylicht_RenderDrawData(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;

	IVideoDriver *driver = getVideoDriver();
	driver->enableScissor(true);

	ImGui_Impl_Skylicht_SetupRenderState(draw_data, fb_width, fb_height);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

		SVertexBuffer *vtx = (SVertexBuffer*)g_meshBuffer->getVertexBuffer(0);
		int vtxSize = cmd_list->VtxBuffer.Size;
		vtx->set_used(vtxSize);

		for (int i = 0, n = vtxSize; i < n; i++)
		{
			S3DVertex &v = vtx->getVertex(i);
			const ImDrawVert* imguiVertex = vtx_buffer + i;

			v.Pos.X = imguiVertex->pos.x;
			v.Pos.Y = imguiVertex->pos.y;
			v.Pos.Z = 0;
			v.TCoords.X = imguiVertex->uv.x;
			v.TCoords.Y = imguiVertex->uv.y;
			v.Color.set(imguiVertex->col);
		}

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					ImGui_Impl_Skylicht_SetupRenderState(draw_data, fb_width, fb_height);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
					// Apply scissor/clipping rectangle
					core::recti r((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w);
					driver->setScissor(r);

					int n = pcmd->ElemCount;
					IIndexBuffer *idxBuffer = g_meshBuffer->getIndexBuffer();
					idxBuffer->set_used(n);

					u32* idx = (u32*)idxBuffer->getIndices();
					for (int i = 0; i < n; i++)
						idx[i] = (u32)idx_buffer[i];

					g_meshBuffer->setPrimitiveType(scene::EPT_TRIANGLES);
					g_meshBuffer->setDirty();

					SMaterial material;
					material.ZBuffer = video::ECFN_ALWAYS;
					material.ZWriteEnable = false;

					if (pcmd->GetTexID() == NULL)
						material.MaterialType = g_vertexColorShader;
					else
					{
						material.MaterialType = g_textureColorShader;
						material.setTexture(0, (ITexture*)pcmd->GetTexID());
					}

					driver->setMaterial(material);
					driver->drawMeshBuffer(g_meshBuffer);
				}
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	driver->enableScissor(false);
}

void ImGui_Impl_Skylicht_ResizeFunc(int w, int h)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)w, (float)h);
}

void ImGui_Impl_Skylicht_MouseMoveFunc(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
}

void ImGui_Impl_Skylicht_MouseButtonFunc(int button, int state, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	if (state == 1)
		io.MouseDown[button] = true;
	else
		io.MouseDown[button] = false;
}

void ImGui_Impl_Skylicht_MouseWheelFunc(int dir, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	io.MouseWheel += dir;
}

void ImGui_Impl_Skylicht_CharFunc(unsigned int c)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(c);
}


 

void ImGui_Impl_Skylicht_KeyPressedFunc(int key, bool ctrl, bool shift, bool alt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(translateKey(key), true); // Notify ImGui about the key press
	io.AddKeyEvent(ImGuiMod_Ctrl, ctrl); // Update modifier state
	io.AddKeyEvent(ImGuiMod_Shift, shift);
	io.AddKeyEvent(ImGuiMod_Alt, alt);

}

void ImGui_Impl_Skylicht_KeyReleasedFunc(int key, bool ctrl, bool shift, bool alt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(translateKey(key), false); // Notify ImGui about the key release
	io.AddKeyEvent(ImGuiMod_Ctrl, ctrl); // Update modifier state
	io.AddKeyEvent(ImGuiMod_Shift, shift);
	io.AddKeyEvent(ImGuiMod_Alt, alt);
}

ImGuiKey translateKey(int code)
{
	switch (code)
	{
	case KEY_TAB: return ImGuiKey_Tab;
	case KEY_LEFT: return ImGuiKey_LeftArrow;
	case KEY_RIGHT: return ImGuiKey_RightArrow;
	case KEY_UP: return ImGuiKey_UpArrow;
	case KEY_DOWN: return ImGuiKey_DownArrow;
	case KEY_PRIOR: return ImGuiKey_PageUp;
	case KEY_NEXT: return ImGuiKey_PageDown;
	case KEY_HOME: return ImGuiKey_Home;
	case KEY_END: return ImGuiKey_End;
	case KEY_INSERT: return ImGuiKey_Insert;
	case KEY_DELETE: return ImGuiKey_Delete;
	case KEY_BACK: return ImGuiKey_Backspace;
	case KEY_SPACE: return ImGuiKey_Space;
	case KEY_RETURN: return ImGuiKey_Enter;
	case KEY_ESCAPE: return ImGuiKey_Escape;

	case KEY_SHIFT:
	case KEY_LSHIFT: return ImGuiKey_LeftShift;
	case KEY_RSHIFT: return ImGuiKey_RightShift;

	case KEY_CONTROL:
	case KEY_LCONTROL: return ImGuiKey_LeftCtrl;
	case KEY_RCONTROL: return ImGuiKey_RightCtrl;

	case KEY_MENU:
	case KEY_LMENU: return ImGuiKey_LeftAlt;
	case KEY_RMENU: return ImGuiKey_RightAlt;

	case KEY_KEY_0: return ImGuiKey_0;
	case KEY_KEY_1: return ImGuiKey_1;
	case KEY_KEY_2: return ImGuiKey_2;
	case KEY_KEY_3: return ImGuiKey_3;
	case KEY_KEY_4: return ImGuiKey_4;
	case KEY_KEY_5: return ImGuiKey_5;
	case KEY_KEY_6: return ImGuiKey_6;
	case KEY_KEY_7: return ImGuiKey_7;
	case KEY_KEY_8: return ImGuiKey_8;
	case KEY_KEY_9: return ImGuiKey_9;

	case KEY_KEY_A: return ImGuiKey_A;
	case KEY_KEY_B: return ImGuiKey_B;
	case KEY_KEY_C: return ImGuiKey_C;
	case KEY_KEY_D: return ImGuiKey_D;
	case KEY_KEY_E: return ImGuiKey_E;
	case KEY_KEY_F: return ImGuiKey_F;
	case KEY_KEY_G: return ImGuiKey_G;
	case KEY_KEY_H: return ImGuiKey_H;
	case KEY_KEY_I: return ImGuiKey_I;
	case KEY_KEY_J: return ImGuiKey_J;
	case KEY_KEY_K: return ImGuiKey_K;
	case KEY_KEY_L: return ImGuiKey_L;
	case KEY_KEY_M: return ImGuiKey_M;
	case KEY_KEY_N: return ImGuiKey_N;
	case KEY_KEY_O: return ImGuiKey_O;
	case KEY_KEY_P: return ImGuiKey_P;
	case KEY_KEY_Q: return ImGuiKey_Q;
	case KEY_KEY_R: return ImGuiKey_R;
	case KEY_KEY_S: return ImGuiKey_S;
	case KEY_KEY_T: return ImGuiKey_T;
	case KEY_KEY_U: return ImGuiKey_U;
	case KEY_KEY_V: return ImGuiKey_V;
	case KEY_KEY_W: return ImGuiKey_W;
	case KEY_KEY_X: return ImGuiKey_X;
	case KEY_KEY_Y: return ImGuiKey_Y;
	case KEY_KEY_Z: return ImGuiKey_Z;

	case KEY_F1: return ImGuiKey_F1;
	case KEY_F2: return ImGuiKey_F2;
	case KEY_F3: return ImGuiKey_F3;
	case KEY_F4: return ImGuiKey_F4;
	case KEY_F5: return ImGuiKey_F5;
	case KEY_F6: return ImGuiKey_F6;
	case KEY_F7: return ImGuiKey_F7;
	case KEY_F8: return ImGuiKey_F8;
	case KEY_F9: return ImGuiKey_F9;
	case KEY_F10: return ImGuiKey_F10;
	case KEY_F11: return ImGuiKey_F11;
	case KEY_F12: return ImGuiKey_F12;

	case KEY_CAPITAL: return ImGuiKey_CapsLock;
	case KEY_SCROLL: return ImGuiKey_ScrollLock;
	case KEY_NUMLOCK: return ImGuiKey_NumLock;

	case KEY_SNAPSHOT: return ImGuiKey_PrintScreen;
	case KEY_PAUSE: return ImGuiKey_Pause;

	case KEY_NUMPAD0: return ImGuiKey_Keypad0;
	case KEY_NUMPAD1: return ImGuiKey_Keypad1;
	case KEY_NUMPAD2: return ImGuiKey_Keypad2;
	case KEY_NUMPAD3: return ImGuiKey_Keypad3;
	case KEY_NUMPAD4: return ImGuiKey_Keypad4;
	case KEY_NUMPAD5: return ImGuiKey_Keypad5;
	case KEY_NUMPAD6: return ImGuiKey_Keypad6;
	case KEY_NUMPAD7: return ImGuiKey_Keypad7;
	case KEY_NUMPAD8: return ImGuiKey_Keypad8;
	case KEY_NUMPAD9: return ImGuiKey_Keypad9;

	case KEY_DECIMAL: return ImGuiKey_KeypadDecimal;
	case KEY_DIVIDE: return ImGuiKey_KeypadDivide;
	case KEY_MULTIPLY: return ImGuiKey_KeypadMultiply;
	case KEY_SUBTRACT: return ImGuiKey_KeypadSubtract;
	case KEY_ADD: return ImGuiKey_KeypadAdd;

	case KEY_PLUS: return ImGuiKey_Equal;
	case KEY_MINUS: return ImGuiKey_Minus;
	case KEY_COMMA: return ImGuiKey_Comma;
	case KEY_PERIOD: return ImGuiKey_Period;
	case KEY_OEM_1: return ImGuiKey_Semicolon;
	case KEY_OEM_2: return ImGuiKey_Slash;
	case KEY_OEM_3: return ImGuiKey_GraveAccent;
	case KEY_OEM_4: return ImGuiKey_LeftBracket;
	case KEY_OEM_5: return ImGuiKey_Backslash;
	case KEY_OEM_6: return ImGuiKey_RightBracket;
	case KEY_OEM_7: return ImGuiKey_Apostrophe;

	case KEY_LBUTTON: return ImGuiKey_MouseLeft;
	case KEY_RBUTTON: return ImGuiKey_MouseRight;
	case KEY_MBUTTON: return ImGuiKey_MouseMiddle;
	case KEY_XBUTTON1: return ImGuiKey_MouseX1;
	case KEY_XBUTTON2: return ImGuiKey_MouseX2;

	default:
		return ImGuiKey_None;
	}
}