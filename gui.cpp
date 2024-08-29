#include "gui.hpp"

#include <algorithm>
#include <fonts/icons.h>
#include <fonts/iconslogs.h>
#include <fonts/intermedium.h>
#include <fonts/lexenddeca.h>
#include <images/logo.h>
#include <imgui/stb_image.h>

ImFont *LexendDecaFont;
ImFont *IconFont;
ImFont *IconFontLogs;
ImFont *InterMedium;

GLuint logo_texture;

static int tab;
static int subtab;

GLuint load_texture(const unsigned char *byteArray, size_t byteArraySize) {
  int width, height, channels;
  unsigned char *pixels =
      stbi_load_from_memory(byteArray, static_cast<int>(byteArraySize), &width,
                            &height, &channels, 0);
  GLuint texture;
  glGenTextures(1, &texture);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  if (channels == 3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, pixels);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
  }

  stbi_image_free(pixels);

  return texture;
}

bool gui::render_tab(const char *label, const char *icon, bool selected) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext &g = *GImGui;
  const ImGuiStyle &style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

  ImVec2 pos = window->DC.CursorPos;
  ImVec2 size = {31, 31};

  const ImRect bb(pos, ImVec2(pos.x + label_size.x + 5, pos.y + 20));

  const ImRect bb2(ImVec2(pos.x, pos.y - 30),
                   ImVec2(pos.x + label_size.x + 5, pos.y + 20));

  ImGui::ItemSize(ImVec4(bb2.Min.x, bb2.Min.y, bb2.Max.x + 2.f, bb2.Max.y),
                  style.FramePadding.y);
  if (!ImGui::ItemAdd(bb2, id))
    return false;

  bool hovered, held;
  bool pressed = ImGui::ButtonBehavior(bb2, id, &hovered, &held, NULL);

  int current_tab;

  if (hovered || held)
    ImGui::SetMouseCursor(9);

  float deltatime = 1.5f * ImGui::GetIO().DeltaTime;
  static std::map<ImGuiID, float> hover_animation;
  auto it_hover = hover_animation.find(id);
  if (it_hover == hover_animation.end()) {
    hover_animation.insert({id, 0.f});
    it_hover = hover_animation.find(id);
  }

  it_hover->second =
      std::clamp(it_hover->second + (2.15f * ImGui::GetIO().DeltaTime *
                                     (hovered ? 1.f : -1.f)),
                 0.0f, 0.0f);
  it_hover->second *= ImGui::GetStyle().Alpha;

  static std::map<ImGuiID, float> filled_animation;
  auto it_filled = filled_animation.find(id);

  if (it_filled == filled_animation.end()) {
    filled_animation.insert({id, 0.f});
    it_filled = filled_animation.find(id);
  }

  it_filled->second =
      std::clamp(it_filled->second + (2.15f * ImGui::GetIO().DeltaTime *
                                      (selected ? 1.f : -1.5f)),
                 it_hover->second, 1.f);
  it_filled->second *= ImGui::GetStyle().Alpha;

  if (selected) {
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(bb.Min.x - 10, bb.Min.y - 33),
        ImVec2(bb.Min.x + 12 + label_size.x, bb.Min.y + 18),
        ImColor(48, 48, 48, int(255 * ImGui::GetStyle().Alpha)), 10);
    window->DrawList->AddText(
        ImVec2((bb.Min.x + bb.Max.x) / 2.f - (label_size.x / 2.f),
               (bb.Min.y + bb.Max.y) / 2.f - (label_size.y / 2.f) - 5),
        ImColor(230, 230, 230, int(255 * ImGui::GetStyle().Alpha)), label);

    ImGui::PushFont(IconFont);
    window->DrawList->AddText(
        ImVec2(bb.Min.x - 10 + (label_size.x / 2.f),
               (bb.Min.y + bb.Max.y) / 2.f - (label_size.y / 2.f) - 30),
        ImColor(230, 230, 230, int(255 * ImGui::GetStyle().Alpha)), icon);
    ImGui::PopFont();
  } else {
    window->DrawList->AddText(
        ImVec2((bb.Min.x + bb.Max.x) / 2.f - (label_size.x / 2.f),
               (bb.Min.y + bb.Max.y) / 2.f - (label_size.y / 2.f) - 5),
        ImColor(127, 127, 127, int(255 * ImGui::GetStyle().Alpha)), label);

    ImGui::PushFont(IconFont);
    window->DrawList->AddText(
        ImVec2(bb.Min.x - 10 + (label_size.x / 2.f),
               (bb.Min.y + bb.Max.y) / 2.f - (label_size.y / 2.f) - 30),
        ImColor(127, 127, 127, int(255 * ImGui::GetStyle().Alpha)), icon);
    ImGui::PopFont();
  }

  return pressed;
}

bool gui::render_subtab(const char *label, const ImVec2 &size_arg,
                        const bool selected) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  static float sizeplus = 0.f;

  ImGuiContext &g = *GImGui;
  const ImGuiStyle &style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

  ImVec2 pos = window->DC.CursorPos;

  ImVec2 size =
      ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f,
                          label_size.y + style.FramePadding.y * 2.0f);

  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ImGui::ItemSize(size, style.FramePadding.y);
  if (!ImGui::ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

  if (hovered || held)
    ImGui::SetMouseCursor(7);

  if (selected) {
    window->DrawList->AddRectFilled({bb.Min.x - 12, bb.Min.y - 3},
                                    {bb.Min.x - 10, bb.Max.y + 4},
                                    ImColor(193, 154, 164));
    window->DrawList->AddRectFilledMultiColor(
        {bb.Min.x - 10, bb.Min.y - 3}, {bb.Min.x + 131, bb.Max.y + 4},
        ImColor(46, 46, 46), ImColor(34, 34, 34), ImColor(34, 34, 34),
        ImColor(46, 46, 46));

    ImGui::PushFont(InterMedium);
    window->DrawList->AddText(
        ImVec2(bb.Min.x,
               bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2),
        ImColor(230, 230, 230), label);
    ImGui::PopFont();
  } else {
    ImGui::PushFont(InterMedium);
    window->DrawList->AddText(
        ImVec2(bb.Min.x,
               bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2),
        ImColor(140, 140, 140), label);
    ImGui::PopFont();
  }

  return pressed;
}

void aimbot_tab() {
  if (tab == 0) {
    ImGui::PushFont(InterMedium);

    ImGui::SetCursorPos(ImVec2(166, 78));
    ImGui::MenuChild("Bullet Tracking", ImVec2(340, 250));
    {
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();

      ImGui::Checkbox("Enable", &setting.aimbot.enable);
      ImGui::Checkbox("Ignore knocked", &setting.aimbot.ignore_knocked);
      ImGui::Checkbox("Ignore Bots", &setting.aimbot.ignore_bots);
      ImGui::Checkbox("Auto fire", &setting.aimbot.auto_fire);
      if (setting.aimbot.target_type == target_type_multipoint) {
        ImGui::SliderFloat("Accuracy", &setting.aimbot.accuracy, 5.f, 20.f);
      }
      const char *items[] = {"Multipoints", "Bone to bone"};
      ImGui::Combo("Target Type", &setting.aimbot.target_type, items, 2);
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(166 + 350, 78));
    ImGui::MenuChild("Shoot parametrs", ImVec2(340, 200));
    {
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();

      ImGui::Checkbox("No Recoil", &setting.aimbot.norecoil);
      ImGui::Checkbox("Instant Hit", &setting.aimbot.instant);
      ImGui::Checkbox("Rapid Fire", &setting.aimbot.rapid);
    }
    ImGui::EndChild();
    ImGui::PopFont();
  }
}

void visuals_tab() {
  if (tab == 1) {
    ImGui::PushFont(InterMedium);

    ImGui::SetCursorPos(ImVec2(166, 78));
    ImGui::MenuChild("Player", ImVec2(340, 200));
    {
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();

      ImGui::Checkbox("Line", &setting.visuals.line);
      ImGui::Checkbox("Nickname", &setting.visuals.nickname);
      ImGui::Checkbox("Skeleton", &setting.visuals.skeleton);
      ImGui::Checkbox("Health", &setting.visuals.health);
      ImGui::Checkbox("Alert", &setting.visuals.alert);
    }

    ImGui::EndChild();
    ImGui::PopFont();
  }
}

void misc_tabs() {
  if (tab == 2) {
    if (subtab == 0) {
      ImGui::PushFont(InterMedium);
      ImGui::SetCursorPos(ImVec2(166, 78));
      ImGui::MenuChild("Memory", ImVec2(340, 430));
      {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Checkbox("Wide View", &setting.memory.wide_view);
        ImGui::Checkbox("Speed Knocked", &setting.memory.speed_knock);
        ImGui::Checkbox("Disable Shake", &setting.memory.no_shake);
        ImGui::Checkbox("Fast Parachute", &setting.memory.fast_parachute);
        ImGui::Checkbox("High Jump", &setting.memory.high_jump);
        ImGui::Checkbox("Hit Effect", &setting.memory.hit_effect);
        ImGui::Checkbox("Strafe", &setting.memory.strafe);
        ImGui::Checkbox("Gravity", &setting.memory.gravity);
        ImGui::SliderFloat("Gravity Scale", &setting.memory.gravity_scale,
                           0.0000001f, 2.f);
        ImGui::SliderFloat("High Jump Scale", &setting.memory.high_jump_scale,
                           1.f, 9999.f);
        ImGui::SliderFloat("Hit Effect Scale", &setting.memory.hit_effect_scale,
                           1.f, 9999.f);
      }
      ImGui::EndChild();
      ImGui::PopFont();
    } else if (subtab == 1) {
      ImGui::PushFont(InterMedium);
      ImGui::SetCursorPos(ImVec2(166, 78));
      ImGui::MenuChild("Config", ImVec2(340, 200));
      {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::SetCursorPos({10, 50});
        if (ImGui::Button("Load Config", ImVec2(320, 50))) {
          settings::load();
        }

        ImGui::SetCursorPos({10, 50 + 60});
        if (ImGui::Button("Save Config", ImVec2(320, 50))) {
          settings::save();
        }
      }
      ImGui::EndChild();
      ImGui::PopFont();
    }
  }
}

void gui::render_tabs() {
  ImGui::PushFont(InterMedium);
  ImGui::SetCursorPosX(350.f);
  ImGui::SetCursorPosY(590);
  ImGui::BeginGroup();
  if (gui::render_tab("Aimbot", "a", tab == 0))
    tab = 0;
  ImGui::SameLine();
  ImGui::SetCursorPosX(350.f + 75);
  if (gui::render_tab("Visuals", "c", tab == 1))
    tab = 1;
  ImGui::SameLine();
  ImGui::SetCursorPosX(350.f + 75 + 75);
  if (gui::render_tab("Misc", "d", tab == 2))
    tab = 2;
  ImGui::SameLine();

  ImGui::EndGroup();
  ImGui::PopFont();

  switch (tab) {
  case 0:
    aimbot_tab();
  case 1:
    visuals_tab();
  case 2:
    misc_tabs();
    break;
  }
}

void gui::render_subtabs() {
  ImGui::PushFont(InterMedium);
  ImGui::SetCursorPos({21, 79});

  ImGui::BeginGroup();
  {
    if (tab == 0) {
      gui::render_subtab("Bullet tracking", ImVec2(100, 25), true);
    } else if (tab == 1) {
      gui::render_subtab("Players", ImVec2(100, 25), true);
    } else if (tab == 2) {
      if (gui::render_subtab("Fun", ImVec2(100, 25), subtab == 0))
        subtab = 0;

      ImGui::Spacing();
      ImGui::Spacing();
      if (gui::render_subtab("Configs", ImVec2(100, 25), subtab == 1))
        subtab = 1;
    }
  }

  ImGui::EndGroup();
  ImGui::PopFont();
}

void gui::render_gui() {
  static bool bopened = false;
  if (bopened) {
    ImGui::SetNextWindowSize(ImVec2(876, 623));
    if (ImGui::Begin(("#gui_main"), nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoScrollbar)) {
      ImVec2 s = ImVec2(ImGui::GetWindowSize().x -
                            ImGui::GetStyle().WindowPadding.x * 2,
                        ImGui::GetWindowSize().y -
                            ImGui::GetStyle().WindowPadding.y * 2),
             p = ImVec2(
                 ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x,
                 ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y);

      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y),
                                                ImVec2(p.x + 863, p.y + 610),
                                                ImColor(29, 29, 29), 10);
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y),
                                                ImVec2(p.x + 863, p.y + 55),
                                                ImColor(41, 41, 41), 10, 3);
      ImGui::GetWindowDrawList()->AddRectFilled(
          ImVec2(p.x, p.y + 540), ImVec2(p.x + 863, p.y + 70 + 540),
          ImColor(41, 41, 41), 10, 12);
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 56),
                                                ImVec2(p.x + 144, p.y + 539),
                                                ImColor(34, 34, 34));
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 144, p.y + 56),
                                                ImVec2(p.x + 145, p.y + 539),
                                                ImColor(41, 41, 41));
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 145, p.y + 56),
                                                ImVec2(p.x + 146, p.y + 539),
                                                ImColor(23, 23, 23));
      ImGui::GetWindowDrawList()->AddImage(
          (ImTextureID)logo_texture, ImVec2(p.x + 11, p.y + 8),
          ImVec2(p.x + 11 + 147, p.y + 8 + 38));
      ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x, p.y),
                                          ImVec2(p.x + 863, p.y + 610),
                                          ImColor(41, 41, 41), 10);
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 55),
                                                ImVec2(p.x + 863, p.y + 56),
                                                ImColor(193, 154, 164));
      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x, p.y + 539),
                                                ImVec2(p.x + 863, p.y + 540),
                                                ImColor(193, 154, 164));

      ImGui::SetCursorPos({876 - 60, 15});
      ImGui::PushFont(InterMedium);
      if (ImGui::Button("close", ImVec2(50, 40)))
        bopened = !bopened;

      ImGui::PopFont();

      gui::render_tabs();
      gui::render_subtabs();
    }

    ImGui::End();
  } else {
    ImGui::SetNextWindowSize({70, 70});
    if (ImGui::Begin("#openwindow", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoScrollbar)) {
      ImGui::GetWindowDrawList()->AddRectFilled(
          ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x,
                 ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y),
          ImVec2(
              ImGui::GetWindowPos().x + 70 - ImGui::GetStyle().WindowPadding.x,
              ImGui::GetWindowPos().y + 70 - ImGui::GetStyle().WindowPadding.y),
          ImColor(29, 29, 29), 4.f);
      ImGui::PushFont(InterMedium);
      ImGui::SetCursorPos({10, 10});
      if (ImGui::Button("open", ImVec2(50, 50))) {
        bopened = !bopened;
      }
      ImGui::PopFont();
    }
    ImGui::End();
  }
}

void gui::initialize() {
  ImGui::CreateContext();
  ImGuiStyle *style = &ImGui::GetStyle();
  ImGuiIO &io = ImGui::GetIO();
  ImGui::StyleColorsDark(style);

  style->FrameRounding = 6;
  style->ChildRounding = 10;
  style->PopupRounding = 5;

  logo_texture = load_texture(logo, sizeof(logo));

  IconFont = io.Fonts->AddFontFromMemoryTTF(icons, sizeof(icons), 22, NULL,
                                            io.Fonts->GetGlyphRangesCyrillic());
  IconFontLogs =
      io.Fonts->AddFontFromMemoryTTF(iconslogs, sizeof(iconslogs), 25, NULL,
                                     io.Fonts->GetGlyphRangesCyrillic());
  LexendDecaFont =
      io.Fonts->AddFontFromMemoryTTF(lexenddeca, sizeof(lexenddeca), 22, NULL,
                                     io.Fonts->GetGlyphRangesCyrillic());
  InterMedium =
      io.Fonts->AddFontFromMemoryTTF(intermedium, sizeof(intermedium), 17, NULL,
                                     io.Fonts->GetGlyphRangesCyrillic());

  ImGui_ImplAndroid_Init();
  ImGui_ImplOpenGL3_Init("#version 300 es");
}