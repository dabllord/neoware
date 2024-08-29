#pragma once

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define target_type_multipoint 0
#define target_type_bone2bone 1

struct _settings {
  struct {
    bool enable;
    bool ignore_knocked;
    bool ignore_bots;
    bool auto_fire;
    bool instant;
    bool rapid;
    bool norecoil;
    bool multipoints;
    float accuracy = 5.f;
    int target_type;
  } aimbot;

  struct {
    bool line;
    bool nickname;
    bool health;
    bool skeleton;
    bool alert;
  } visuals;

  struct {
    bool wide_view;
    bool speed_knock;
    bool no_shake;
    bool fast_parachute;
    bool strafe;
    bool hit_effect;
    float hit_effect_scale;
    bool high_jump;
    float high_jump_scale;
    bool gravity;
    float gravity_scale;
  } memory;
};

extern _settings setting;

namespace settings {
void load();
void save();
} // namespace settings