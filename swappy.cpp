#include "swappy.hpp"

bool bimgui_initialized = false;
int screen_x, screen_y;

void swappy::enable_frame_pacing() {
  *reinterpret_cast<uint32_t *>(ue4_elf.base() + enable_frame_pacing_offset) =
      1;
}

void swappy::swappy_delay(uint64_t miliseconds) {
  return reinterpret_cast<void (*)(uint64_t)>(ue4_elf.base() +
                                              swappy_delay_offset)(miliseconds);
}

EGLBoolean swappy_swap(EGLDisplay display, EGLSurface surface) {
  eglQuerySurface(display, surface, EGL_WIDTH, &screen_x);
  eglQuerySurface(display, surface, EGL_HEIGHT, &screen_y);

  if (!bimgui_initialized) {
    settings::load();
    gui::initialize();
    bimgui_initialized = true;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplAndroid_NewFrame(screen_x, screen_y);
  ImGui::NewFrame();

  gui::render_gui();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  swappy::swappy_delay(99999999999);
  return eglSwapBuffers(display, surface);
}

int32_t (*onInputEvent)(struct android_app *app, AInputEvent *inputEvent);
int32_t honInputEvent(struct android_app *app, AInputEvent *inputEvent) {
  if (bimgui_initialized) {
    ImGui_ImplAndroid_HandleInputEvent(
        inputEvent,
        {(float)ANativeWindow_getWidth(app->window) / (float)screen_x,
         (float)ANativeWindow_getHeight(app->window) / (float)screen_y});
  }

  return onInputEvent(app, inputEvent);
}

void swappy::initialize() {
  swappy::enable_frame_pacing();
  sleep(1);
  android_app *application = *reinterpret_cast<android_app **>(
      ue4_elf.base() + android_application_offset);
  onInputEvent = decltype(onInputEvent)(application->onInputEvent);
  application->onInputEvent = honInputEvent;

  auto swappy_instance =
      reinterpret_cast<uintptr_t *>(swappy_elf.base() + 0xF93F0);
  while (!*swappy_instance) {
    std::this_thread::yield();
  }

  auto egl = reinterpret_cast<uintptr_t *>(*swappy_instance + 48);
  while (!*egl) {
    std::this_thread::yield();
  }

  auto ppfn = reinterpret_cast<decltype(&eglSwapBuffers) *>(*egl + 16);
  while (!*ppfn) {
    std::this_thread::yield();
  }

  *ppfn = swappy_swap;

  return;
}