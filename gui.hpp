#pragma once

#include <GLES3/gl3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_android.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <map>
#include <settings.hpp>

namespace gui {
bool render_tab(const char *label, const char *icon, bool selected);
bool render_subtab(const char *label, const ImVec2 &size_arg,
                   const bool selected);
void render_tabs();
void render_subtabs();
void initialize();
void render_gui();
} // namespace gui