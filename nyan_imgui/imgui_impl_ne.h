#ifndef IMGUI_IMPL_NE_H
#define IMGUI_IMPL_NE_H

#include <stdbool.h>
#include <wchar.h>

extern bool ImGui_ImplNyan_Init(const wchar_t *font_name = L"Karla-Regular.ttf", float font_size = 16, bool support_hidpi = true);
extern void ImGui_ImplNyan_NewFrame(void);
extern void ImGui_ImplNyan_RenderFrame(void);
extern void ImGui_ImplNyan_Shutdown(void);

#endif
