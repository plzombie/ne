// Based on https://github.com/ocornut/imgui/blob/v1.49/examples/opengl_example/imgui_impl_glfw.cpp
/*
The MIT License (MIT)

Copyright (c) 2014-2015 Omar Cornut and ImGui contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "../nyan/nyan_publicapi.h"

#include "../nyan_imgui/imgui_impl_ne.h"

#include "../forks/imgui/imgui.h"

NYAN_MAIN
{
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	NYAN_INIT

	if(!nvAttachRender((wchar_t *)(L"ngl"))) return 0;
	if(!naAttachLib((wchar_t *)(L"nullal"))) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetScreen(1280, 720, 32, NV_MODE_WINDOWED, 0);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;

	if(!ImGui_ImplNyan_Init()) return 0;

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		ImGui_ImplNyan_NewFrame();

		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			if(ImGui::Button("Test Window")) show_test_window ^= 1;
			if(ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		if(show_another_window)
		{
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello");
			ImGui::End();
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		if(show_test_window) {
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
			ImGui::ShowDemoWindow(&show_test_window);
		}

		nvSetStatusf(NV_STATUS_WINBCRED, clear_color.x);
		nvSetStatusf(NV_STATUS_WINBCGREEN, clear_color.y);
		nvSetStatusf(NV_STATUS_WINBCBLUE, clear_color.z);

		ImGui_ImplNyan_RenderFrame();

		nUpdate();
	}

	ImGui_ImplNyan_Shutdown();

	nClose();

	NYAN_CLOSE

	return 0;
}
