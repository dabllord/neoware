#include <basic_math.hpp>
#include <basic_sdk.hpp>
#include <memory_mgr.hpp>
#include <settings.hpp>

namespace shoot_inner {
uintptr_t get_target(tarray<uintptr_t> &players, uintptr_t local_player,
                     vec3 &out_target_pos);
void initialize(uintptr_t local_player);
} // namespace shoot_inner