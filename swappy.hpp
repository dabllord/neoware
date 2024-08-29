#include <android_native_app_glue.h>
#include <basic_math.hpp>
#include <chrono>
#include <egl/egl.h>
#include <gui.hpp>
#include <memory_mgr.hpp>
#include <thread>
#include <unistd.h>

namespace swappy {
void enable_frame_pacing();
void swappy_delay(uint64_t miliseconds);
void initialize();
} // namespace swappy
