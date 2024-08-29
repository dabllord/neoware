#pragma once

#define k2_drawline_offset 0x8FE5370
#define k2_drawtext_offset 0x8FE5600
#define k2_textsize_offset 0x8FE62E0
#define get_actors_offset 0x8A5218C
#define get_bone_pos_offset 0x51ED534
#define line_of_sight_to_offset 0x896C034
#define get_current_weapon_offset 0x4D196F4
#define get_parachute_velocity_offset 0x4EA3B5C
#define get_muzzle_transform_index 0x153
#define on_press_fire_offset 0x52BC380
#define on_release_fire_offset 0x52BC420

#define get_bone_idx_offset 0x8D2277C

#define android_application_offset 0xc443a08
#define engine_offset 0xCB34E08
#define get_names_offset 0x6F0BCB4
#define guobject_array_offset 0xC8E1270

#define enable_frame_pacing_offset 0xCB5E0A4
#define swappy_delay_offset 0xADD4A20

#include <kitty_memory/KittyMemoryMgr.hpp>
#include <android/log.h>
#include <ida_defs.h>

extern KittyMemoryMgr _memory_mgr;
extern ElfScanner ue4_elf, swappy_elf;

#define LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "neoware", __VA_ARGS__))