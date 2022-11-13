#include <wchar.h>
#include <stdint.h>

#include "imgui_impl_ne.h"

#include "../nyan/nyan_publicapi.h"

#include "../forks/imgui/imgui.h"

static nv_2dvertex_type *imgui_implnyan_varray;
static unsigned int imgui_implnyan_varray_size;
static unsigned int imgui_implnyan_fonttexid;

static void ImGui_ImplNyan_RenderDrawLists(ImDrawData *draw_data);
//static const char *ImGui_ImplNyan_GetClipboardText(void);
//static void ImGui_ImplNyan_SetClipboardText(const char *text);
#include <stdio.h>
bool ImGui_ImplNyan_Init(const wchar_t *font_name, float font_size, bool support_hidpi)
{
	ImGuiContext *imgui_context = ImGui::CreateContext();
	
	ImGui::SetCurrentContext(imgui_context);

	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	io.KeyMap[ImGuiKey_Tab] = NK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = NK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = NK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = NK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = NK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = NK_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = NK_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = NK_HOME;
	io.KeyMap[ImGuiKey_End] = NK_END;
	io.KeyMap[ImGuiKey_Insert] = NK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = NK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = NK_BACK;
	io.KeyMap[ImGuiKey_Space] = NK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = NK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = NK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = NK_A;
	io.KeyMap[ImGuiKey_C] = NK_C;
	io.KeyMap[ImGuiKey_V] = NK_V;
	io.KeyMap[ImGuiKey_X] = NK_X;
	io.KeyMap[ImGuiKey_Y] = NK_Y;
	io.KeyMap[ImGuiKey_Z] = NK_Z;

	unsigned int f;

	f = nFileOpen(font_name);
	if(f) {
		int file_len;
		void *file_data;

		file_len = (int)nFileLength(f);
		file_data = ImGui::MemAlloc(file_len);
		if(file_data) {
			if(nFileRead(f, file_data, file_len) == file_len) {
				ImFontConfig font_cfg = ImFontConfig();

				if(support_hidpi) {
					font_cfg.OversampleH = 6;
					font_cfg.OversampleV = 2;
				} else {
					font_cfg.OversampleH = 3;
					font_cfg.OversampleV = 1;
				}

				font_cfg.FontDataOwnedByAtlas = true;

				io.Fonts->AddFontFromMemoryTTF(file_data, file_len, font_size, &font_cfg);
			} else
				ImGui::MemFree(file_data);
		}

		nFileClose(f);
	}

	int width, height;
	unsigned char *pixelbuffer;

	io.Fonts->GetTexDataAsAlpha8(&pixelbuffer, &width, &height);

	nv_texture_type nv_tex;

	nv_tex.nglrowalignment = 1;
	nv_tex.nglcolorformat = NGL_COLORFORMAT_L8A8;
	nv_tex.sizex = width;
	nv_tex.sizey = height;
	nv_tex.buffer = (unsigned char *)nAllocMemory(nv_tex.sizex*nv_tex.sizey*2);

	if(nv_tex.buffer) {
		unsigned int texid;

		for(int i = 0; i < width*height; i++) {
			nv_tex.buffer[2*i] = 255;
			nv_tex.buffer[2*i+1] = pixelbuffer[i];
		}

		texid = nvCreateTextureFromMemory(&nv_tex, NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);

		nvLoadTexture(texid);

		nFreeMemory(nv_tex.buffer);

		io.Fonts->TexID = (void *)(intptr_t)texid;
	} else {
		io.Fonts->TexID = (void *)(intptr_t)0;
	}

	return true;
}

void ImGui_ImplNyan_NewFrame(void)
{
	unsigned int winx, winy;
	int mx, my, mbl, mbr, mbm;
	ImGuiIO &io = ImGui::GetIO();
	const wchar_t *inputstr;

	winx = nvGetStatusi(NV_STATUS_WINX);
	winy = nvGetStatusi(NV_STATUS_WINY);

	mx = nvGetStatusi(NV_STATUS_WINMX);
	my = nvGetStatusi(NV_STATUS_WINMY);
	mbl = nvGetStatusi(NV_STATUS_WINMBL);
	mbr = nvGetStatusi(NV_STATUS_WINMBR);
	mbm = nvGetStatusi(NV_STATUS_WINMBM);
	inputstr = nvGetStatusw(NV_STATUS_WINTEXTINPUTBUF);

	io.DisplaySize = ImVec2((float)winx, (float)winy);
	io.DeltaTime = (float)nGetspf();
	io.MousePos = ImVec2((float)mx, (float)my);
	io.MouseDown[0] = ((mbl==NV_KEYSTATUS_PRESSED) || ((mbl==NV_KEYSTATUS_RELEASED) && (!io.MouseDown[0])))?true:false;
	io.MouseDown[1] = ((mbr==NV_KEYSTATUS_PRESSED) || ((mbr==NV_KEYSTATUS_RELEASED) && (!io.MouseDown[1])))?true:false;
	io.MouseDown[2] = ((mbm==NV_KEYSTATUS_PRESSED) || ((mbm==NV_KEYSTATUS_RELEASED) && (!io.MouseDown[2])))?true:false;
	io.MouseWheel = (float)(nvGetStatusi(NV_STATUS_WINMWHEEL));

	for(int i = 0; i < 256; i++)
		io.KeysDown[i] = (nvGetKey(i) == 1)?true:false;

	io.KeyCtrl = io.KeysDown[NK_CONTROL];
	io.KeyShift = io.KeysDown[NK_SHIFT];
	io.KeyAlt = io.KeysDown[NK_ALT];
	io.KeySuper = io.KeysDown[NK_LWIN] || io.KeysDown[NK_RWIN];

	while(*inputstr) {
		wchar_t wc;

		wc = *inputstr;

#ifdef N_WCHAR32
		io.AddInputCharacter(wc);
#else
		io.AddInputCharacterUTF16(wc);
#endif
			

		inputstr++;
	}

	ImGui::NewFrame();
}

void ImGui_ImplNyan_Shutdown(void)
{
	if(imgui_implnyan_fonttexid) {
		nvDestroyTexture(imgui_implnyan_fonttexid);
		imgui_implnyan_fonttexid = 0;
	}

	if(imgui_implnyan_varray_size) {
		imgui_implnyan_varray_size = 0;
		free(imgui_implnyan_varray);
	}
	imgui_implnyan_varray = 0;

	ImGui::DestroyContext(); // Удалится текущий контекст
}

static void ImGui_ImplNyan_RenderDrawLists(ImDrawData *draw_data)
{
	int old_clippingregion, old_clippingregion_sx, old_clippingregion_sy, old_clippingregion_ex, old_clippingregion_ey;

	old_clippingregion = nvGetStatusi(NV_STATUS_WINCLIPPINGREGION);
	old_clippingregion_sx = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX);
	old_clippingregion_sy = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY);
	old_clippingregion_ex = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX);
	old_clippingregion_ey = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY);

	nvSetStatusi(NV_STATUS_WINCLIPPINGREGION, true);

	nvBegin2d();

	for(int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;
		unsigned int current_varray_size = cmd_list->IdxBuffer.Size;
		nv_2dvertex_type *varray;

		if(imgui_implnyan_varray_size < current_varray_size) {
			nv_2dvertex_type *_varray;

			_varray = (nv_2dvertex_type *)realloc(imgui_implnyan_varray, current_varray_size*sizeof(nv_2dvertex_type));
			if(!_varray)
				break;

			imgui_implnyan_varray = _varray;
			imgui_implnyan_varray_size = current_varray_size;
		}

		for(unsigned int i = 0; i < current_varray_size; i++) {
			unsigned int idx = idx_buffer[i];

			imgui_implnyan_varray[i].x = vtx_buffer[idx].pos.x;
			imgui_implnyan_varray[i].y = vtx_buffer[idx].pos.y;
			imgui_implnyan_varray[i].z = 0;
			imgui_implnyan_varray[i].tx = vtx_buffer[idx].uv.x;
			imgui_implnyan_varray[i].ty = vtx_buffer[idx].uv.y;
			imgui_implnyan_varray[i].colorRGBA = vtx_buffer[idx].col;
		}

		varray = imgui_implnyan_varray;

		for(int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

			if(pcmd->UserCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			else {
				nvSetClippingRegion((int)(pcmd->ClipRect.x), (int)(pcmd->ClipRect.y), (int)(pcmd->ClipRect.z), (int)(pcmd->ClipRect.w));
				nvDraw2d(NV_DRAWTRIANGLE, pcmd->ElemCount/3, (unsigned int)(intptr_t)(pcmd->GetTexID()), varray+pcmd->IdxOffset);
			}
		}
	}

	nvSetStatusi(NV_STATUS_WINCLIPPINGREGION, old_clippingregion);
	if(old_clippingregion) {
		nvSetClippingRegion(old_clippingregion_sx, old_clippingregion_sy, old_clippingregion_ex, old_clippingregion_ey);
	}

	nvEnd2d();
}

void ImGui_ImplNyan_RenderFrame(void)
{
	ImGui::Render();

	ImDrawData *draw_data = ImGui::GetDrawData();

	ImGui_ImplNyan_RenderDrawLists(draw_data);
}

/*static const char *ImGui_ImplNyan_GetClipboardText(void)
{

}

static void ImGui_ImplNyan_SetClipboardText(const char *text)
{

}*/
