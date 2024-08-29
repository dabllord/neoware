#include "post_render.hpp"

linear_color knocked_color = linear_color(200, 10, 10, 255),
             alive_color = linear_color(55, 255, 95, 255),
             bot_color = linear_color(166, 255, 41, 255),
             player_color = linear_color(55, 255, 95, 255),
             bot_nvisible_color = linear_color(252, 76, 79, 255),
             player_nvisible_color = linear_color(200, 10, 10, 255);

void (*oPostRender)(uintptr_t caller, uintptr_t canvas);
void hPostRender(uintptr_t caller, uintptr_t canvas) {
  uintptr_t local_player, local_controller;

  auto k2_drawline = [](uintptr_t canvas, vec2 pos1, vec2 pos2, float think,
                        linear_color color) -> void {
    return reinterpret_cast<void (*)(uintptr_t, vec2, vec2, float,
                                     linear_color)>(
        ue4_elf.base() + k2_drawline_offset)(canvas, pos1, pos2, think, color);
  };

  auto k2_drawtext = [](uintptr_t canvas, uintptr_t font, fstring text,
                        vec2 pos, linear_color color, float kerning,
                        linear_color shadow_color, vec2 shadow_offset,
                        bool bcenterx, bool bcentery, bool boutline,
                        linear_color outline_color) -> void {
    return reinterpret_cast<void (*)(uintptr_t, uintptr_t, fstring &, vec2,
                                     linear_color, float, linear_color, vec2,
                                     bool, bool, bool, linear_color)>(
        ue4_elf.base() + k2_drawtext_offset)(
        canvas, font, text, pos, color, kerning, shadow_color, shadow_offset,
        bcenterx, bcentery, boutline, outline_color);
  };

  auto k2_textsize = [](uintptr_t canvas, uintptr_t font, fstring text,
                        vec2 scale) -> vec2 {
    return reinterpret_cast<vec2 (*)(uintptr_t, uintptr_t, fstring &, vec2)>(
        ue4_elf.base() + k2_textsize_offset)(canvas, font, text, scale);
  };

  auto world2screen = [](uintptr_t canvas, vec3 world_pos,
                         vec2 &out_pos) -> bool {
    mat4 vp = *reinterpret_cast<mat4 *>(canvas + 0x270);
    vec2 screen_center(*reinterpret_cast<float *>(canvas + 0x30) / 2,
                       *reinterpret_cast<float *>(canvas + 0x34) / 2);

    const float clip_x = world_pos.x * vp._11 + world_pos.y * vp._21 +
                         world_pos.z * vp._31 + vp._41;
    const float clip_y = world_pos.x * vp._12 + world_pos.y * vp._22 +
                         world_pos.z * vp._32 + vp._42;
    const float clip_w = world_pos.x * vp._14 + world_pos.y * vp._24 +
                         world_pos.z * vp._34 + vp._44;
    if (clip_w < 0.0001f)
      return false;

    out_pos.x = screen_center.x + screen_center.x * clip_x / clip_w;
    out_pos.y = screen_center.y - screen_center.y * clip_y / clip_w;
    return true;
  };

  auto get_font = []() -> uintptr_t {
    uintptr_t engine =
        *reinterpret_cast<uintptr_t *>(ue4_elf.base() + engine_offset);
    if (engine) {
      uintptr_t small_font = *reinterpret_cast<uintptr_t *>(engine + 0x90);
      *reinterpret_cast<int *>(small_font + 0x134) = 11;
      return small_font;
    }
  };

  auto get_actors = [](uintptr_t world_context_object,
                       uintptr_t actor_class) -> tarray<uintptr_t> {
    tarray<uintptr_t> result;
    reinterpret_cast<void (*)(uintptr_t, uintptr_t, tarray<uintptr_t> &)>(
        ue4_elf.base() + get_actors_offset)(world_context_object, actor_class,
                                            result);
    return result;
  };

  auto get_bone_pos = [](uintptr_t player, fname bone_name,
                         vec3 offset = {}) -> vec3 {
    return reinterpret_cast<vec3 (*)(uintptr_t, fname, vec3)>(
        ue4_elf.base() + get_bone_pos_offset)(player, bone_name, offset);
  };

  auto line_of_sight_to = [](uintptr_t controller, uintptr_t other,
                             vec3 view_point, bool balt_checks) -> bool {
    return reinterpret_cast<bool (*)(uintptr_t, uintptr_t, vec3, bool)>(
        ue4_elf.base() + line_of_sight_to_offset)(controller, other, view_point,
                                                  balt_checks);
  };

  auto get_current_weapon = [](uintptr_t player) -> uintptr_t {
    return reinterpret_cast<uintptr_t (*)(uintptr_t)>(
        ue4_elf.base() + get_current_weapon_offset)(player);
  };

  auto decrypt_local_player = [](uintptr_t game_instance) -> uintptr_t {
    return *reinterpret_cast<uintptr_t *>(game_instance + 264) ^
           **reinterpret_cast<uintptr_t **>(game_instance + 56);
  };

  auto calculate_vector_angles_radar = [](const vec3 &forward, vec3 &angles) {
    if (forward.x == 0.f && forward.y == 0.f) {
      angles.x = forward.z > 0.f ? -90.f : 90.f;
      angles.y = 0.f;
    } else {
      angles.x = RAD_TO_DEG(
          atan2(-forward.z, sqrt(forward.x * forward.x + forward.y * forward.y +
                                 forward.z * forward.z)));
      angles.y = RAD_TO_DEG(atan2(forward.y, forward.x));
    }
    angles.z = 0.f;
  };

  auto rotate_triangle = [](std::array<vec3, 3> &points, float rotation) {
    const auto center = (points[0] + points[1] + points[2]) / 3;
    for (auto &point : points) {
      point = point - center;
      const auto temp_x = point.x;
      const auto temp_y = point.y;
      const auto theta = DEG_TO_RAD(rotation);
      const auto cos_theta = cosf(theta);
      const auto sin_theta = sinf(theta);
      point.x = temp_x * cos_theta - temp_y * sin_theta;
      point.y = temp_x * sin_theta + temp_y * cos_theta;
      point = point + center;
    }
  };

  auto world_to_radar = [](float yaw, vec3 origin, vec3 local_origin,
                           float pos_x, float pos_y, vec3 size,
                           bool &outbuff) -> vec3 {
    bool flag = false;
    double yaw_rad = static_cast<double>(yaw) * 0.017453292519943295;
    float cos_yaw = static_cast<float>(std::cos(yaw_rad));
    float sin_yaw = static_cast<float>(std::sin(yaw_rad));

    float delta_x = origin.x - local_origin.x;
    float delta_y = origin.y - local_origin.y;

    vec3 radar_pos;
    radar_pos.x = (delta_y * cos_yaw - delta_x * sin_yaw) / 150.f;
    radar_pos.y = (delta_x * cos_yaw + delta_y * sin_yaw) / 150.f;

    vec3 transformed_pos;
    transformed_pos.x = radar_pos.x + pos_x + size.x / 2.f;
    transformed_pos.y = -radar_pos.y + pos_y + size.y / 2.f;

    if (transformed_pos.x > pos_x + size.x) {
      transformed_pos.x = pos_x + size.x;
    } else if (transformed_pos.x < pos_x) {
      transformed_pos.x = pos_x;
    }

    if (transformed_pos.y > pos_y + size.y) {
      transformed_pos.y = pos_y + size.y;
    } else if (transformed_pos.y < pos_y) {
      transformed_pos.y = pos_y;
    }

    if (transformed_pos.y == pos_y || transformed_pos.x == pos_x) {
      flag = true;
    }

    outbuff = flag;
    return transformed_pos;
  };

  static std::vector<std::string> right_arm{
      "neck_01", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r", "item_r"};

  static std::vector<std::string> left_arm{
      "neck_01", "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l", "item_l"};

  static std::vector<std::string> spine{"Head",     "neck_01",  "spine_03",
                                        "spine_02", "spine_01", "pelvis"};

  static std::vector<std::string> lower_right{"pelvis", "thigh_r", "calf_r",
                                              "foot_r"};

  static std::vector<std::string> lower_left{"pelvis", "thigh_l", "calf_l",
                                             "foot_l"};

  static std::vector<std::vector<std::string>> skeleton{
      right_arm, left_arm, spine, lower_right, lower_left};

  uintptr_t world = *reinterpret_cast<uintptr_t *>(caller + 0x78);
  if (world) {
    uintptr_t game_instance = *reinterpret_cast<uintptr_t *>(world + 0x220);
    if (game_instance) {
      uintptr_t ulocal_player = decrypt_local_player(game_instance);
      if (ulocal_player) {
        local_controller = *reinterpret_cast<uintptr_t *>(ulocal_player + 0x30);
        if (setting.memory.wide_view) {
          *reinterpret_cast<uint8_t *>(
              ulocal_player +
              guobject_array->find_offset(
                  "ByteProperty "
                  "Engine.LocalPlayer.AspectRatioAxisConstraint")) = 3;
        } else {
          *reinterpret_cast<uint8_t *>(
              ulocal_player +
              guobject_array->find_offset(
                  "ByteProperty "
                  "Engine.LocalPlayer.AspectRatioAxisConstraint")) = 2;
        }
      }
    }

    if (local_controller) {
      local_player = *reinterpret_cast<uintptr_t *>(
          local_controller + guobject_array->find_offset(
                                 "ObjectProperty "
                                 "Engine.PlayerController.AcknowledgedPawn"));
      if (local_player &&
          is_a(local_controller,
               guobject_array->find_object(
                   "Class ShadowTrackerExtra.STExtraPlayerController"))) {
        shoot_inner::initialize(local_player);
        if (setting.memory.speed_knock) {
          *reinterpret_cast<float *>(
              local_player + guobject_array->find_offset(
                                 "FloatProperty "
                                 "ShadowTrackerExtra.STExtraBaseCharacter."
                                 "EnergySpeedScale")) = 999.f;
        }

        uintptr_t stplayer_camera_manager = *reinterpret_cast<uintptr_t *>(
            local_controller +
            guobject_array->find_offset(
                "ObjectProperty "
                "Engine.PlayerController.PlayerCameraManager"));
        if (stplayer_camera_manager) {
          if (setting.memory.no_shake) {
            *reinterpret_cast<bool *>(
                stplayer_camera_manager +
                guobject_array->find_offset(
                    "BoolProperty "
                    "ShadowTrackerExtra.STPlayerCameraManager."
                    "bApplyCameraShake")) = false;
          } else {
            *reinterpret_cast<bool *>(
                stplayer_camera_manager +
                guobject_array->find_offset(
                    "BoolProperty "
                    "ShadowTrackerExtra.STPlayerCameraManager."
                    "bApplyCameraShake")) = true;
          }
        }

        uintptr_t movement_comp = *reinterpret_cast<uintptr_t *>(
            local_player +
            guobject_array->find_offset(
                "ObjectProperty Engine.Character.CharacterMovement"));
        if (movement_comp) {
          if (setting.memory.strafe) {
            *reinterpret_cast<float *>(
                movement_comp +
                guobject_array->find_offset(
                    "FloatProperty "
                    "Engine.CharacterMovementComponent.AirControl")) = 999.f;
            *reinterpret_cast<float *>(
                movement_comp + guobject_array->find_offset(
                                    "FloatProperty "
                                    "Engine.CharacterMovementComponent."
                                    "AirControlBoostMultiplier")) = 999.f;
            *reinterpret_cast<float *>(
                movement_comp + guobject_array->find_offset(
                                    "FloatProperty "
                                    "Engine.CharacterMovementComponent."
                                    "AirControlBoostVelocityThreshold")) =
                999.f;
          }

          if (setting.memory.high_jump) {
            *reinterpret_cast<float *>(
                movement_comp +
                guobject_array->find_offset(
                    "FloatProperty "
                    "Engine.CharacterMovementComponent.JumpZVelocity")) =
                setting.memory.high_jump_scale;
          }

          if (setting.memory.gravity) {
            *reinterpret_cast<float *>(
                movement_comp +
                guobject_array->find_offset(
                    "FloatProperty "
                    "Engine.CharacterMovementComponent.GravityScale")) =
                setting.memory.gravity_scale;
          } else {
            *reinterpret_cast<float *>(
                movement_comp +
                guobject_array->find_offset(
                    "FloatProperty "
                    "Engine.CharacterMovementComponent.GravityScale")) = 1.f;
          }
        }

        if (setting.memory.fast_parachute) {
          uintptr_t parachute_data = *reinterpret_cast<uintptr_t *>(
              local_controller +
              guobject_array->find_offset(
                  "StructProperty "
                  "ShadowTrackerExtra.STExtraPlayerController."
                  "ParachuteData"));
          if (parachute_data) {
            *reinterpret_cast<float *>(parachute_data) = 500.f;
          }
        }

        if (setting.memory.hit_effect) {
          uintptr_t my_hud = *reinterpret_cast<uintptr_t *>(
              local_controller +
              guobject_array->find_offset(
                  "ObjectProperty Engine.PlayerController.MyHUD"));
          if (my_hud) {
            uintptr_t hit_perform_ptr = *reinterpret_cast<uintptr_t *>(
                my_hud + guobject_array->find_offset(
                             "StructProperty "
                             "ShadowTrackerExtra.SurviveHUD.HitPerform"));
            if (hit_perform_ptr) {
              *reinterpret_cast<float *>(hit_perform_ptr + 16) =
                  setting.memory.hit_effect_scale;
              *reinterpret_cast<float *>(hit_perform_ptr + 80) =
                  setting.memory.hit_effect_scale;
              *reinterpret_cast<float *>(hit_perform_ptr + 144) =
                  setting.memory.hit_effect_scale;
              *reinterpret_cast<float *>(hit_perform_ptr + 208) =
                  setting.memory.hit_effect_scale;
            }
          }
        }

        int local_team_id = *reinterpret_cast<int *>(
            local_player + guobject_array->find_offset(
                               "IntProperty Gameplay.UAECharacter.TeamID"));
        float local_rotation = *reinterpret_cast<float *>(
            *reinterpret_cast<uintptr_t *>(
                local_controller +
                guobject_array->find_offset("ObjectProperty "
                                            "Engine.PlayerController."
                                            "PlayerCameraManager")) +
            guobject_array->find_offset(
                "StructProperty "
                "Engine.PlayerCameraManager.CameraCache") +
            0x10 + 0x18 + 0x4);
        tarray<uintptr_t> players = get_actors(
            world, guobject_array->find_object(
                       "Class ShadowTrackerExtra.STExtraPlayerCharacter"));
        for (int i = 0; i < players.get_count(); i++) {
          uintptr_t player = players[i];
          if (player) {
            bool bhidden = *reinterpret_cast<bool *>(
                     player + guobject_array->find_offset(
                                  "BoolProperty Engine.Actor.bHidden")),
                 bensure = *reinterpret_cast<bool *>(
                     player +
                     guobject_array->find_offset(
                         "BoolProperty Gameplay.UAECharacter.bEnsure"));
            float _near_death_breath = *reinterpret_cast<float *>(
                player + guobject_array->find_offset(
                             "FloatProperty "
                             "ShadowTrackerExtra.STExtraBaseCharacter."
                             "NearDeathBreath"));
            int team_id = *reinterpret_cast<int *>(
                player + guobject_array->find_offset(
                             "IntProperty Gameplay.UAECharacter.TeamID"));
            if (bhidden && _near_death_breath == 0.f)
              continue;
            if (local_team_id == team_id)
              continue;

            if (setting.visuals.alert) {
              float screenx = *reinterpret_cast<float *>(canvas + 0x30),
                    screeny = *reinterpret_cast<float *>(canvas + 0x34);
              vec3 local_pos, enemy_pos;
              bool bbbbbb;
              uintptr_t local_vehicle = *reinterpret_cast<uintptr_t *>(
                            local_player +
                            guobject_array->find_offset(
                                "ObjectProperty "
                                "ShadowTrackerExtra.STExtraCharacter."
                                "CurrentVehicle")),
                        enemy_vehicle = *reinterpret_cast<uintptr_t *>(
                            player + guobject_array->find_offset(
                                         "ObjectProperty "
                                         "ShadowTrackerExtra.STExtraCharacter."
                                         "CurrentVehicle"));

              local_pos =
                  local_vehicle
                      ? *reinterpret_cast<vec3 *>(
                            *reinterpret_cast<uintptr_t *>(
                                local_vehicle +
                                guobject_array->find_offset("ObjectProperty "
                                                            "Engine.Actor."
                                                            "RootComponent")) +
                            guobject_array->find_offset(
                                "StructProperty "
                                "Engine.SceneComponent.RelativeLocation"))
                      : *reinterpret_cast<vec3 *>(
                            *reinterpret_cast<uintptr_t *>(
                                local_player +
                                guobject_array->find_offset("ObjectProperty "
                                                            "Engine.Actor."
                                                            "RootComponent")) +
                            guobject_array->find_offset(
                                "StructProperty "
                                "Engine.SceneComponent.RelativeLocation"));
              enemy_pos =
                  enemy_vehicle
                      ? *reinterpret_cast<vec3 *>(
                            *reinterpret_cast<uintptr_t *>(
                                enemy_vehicle +
                                guobject_array->find_offset("ObjectProperty "
                                                            "Engine.Actor."
                                                            "RootComponent")) +
                            guobject_array->find_offset(
                                "StructProperty "
                                "Engine.SceneComponent.RelativeLocation"))
                      : *reinterpret_cast<vec3 *>(
                            *reinterpret_cast<uintptr_t *>(
                                player +
                                guobject_array->find_offset("ObjectProperty "
                                                            "Engine.Actor."
                                                            "RootComponent")) +
                            guobject_array->find_offset(
                                "StructProperty "
                                "Engine.SceneComponent.RelativeLocation"));

              vec3 radar_position =
                  world_to_radar(local_rotation, enemy_pos, local_pos, 0.f, 0.f,
                                 vec3(screenx, screeny, 0), bbbbbb);

              vec3 angle;
              vec3 forward = vec3(
                  static_cast<float>(screenx / 2) - radar_position.x,
                  static_cast<float>(screeny / 2) - radar_position.y, 0.0f);

              calculate_vector_angles_radar(forward, angle);
              float angle_yaw_rad = DEG_TO_RAD(angle.y + 180.f);
              float new_point_x =
                  (screenx / 2) + (55 / 2 * 8 * cosf(angle_yaw_rad));
              float new_point_y =
                  (screeny / 2) + (55 / 2 * 8 * sinf(angle_yaw_rad));

              std::array<vec3, 3> triangle_points{
                  vec3(new_point_x - ((40 / 4) + 2) / 2,
                       new_point_y - ((40 / 4) + 2) / 2, 0.f),
                  vec3(new_point_x + ((40 / 4) + 2) / 4, new_point_y, 0.f),
                  vec3(new_point_x - ((40 / 4) + 2) / 2,
                       new_point_y + ((40 / 4) + 2) / 2, 0.f)};

              rotate_triangle(triangle_points, angle.y + 180.f);

              linear_color alert_color =
                  line_of_sight_to(
                      local_controller,
                      *reinterpret_cast<uintptr_t *>(
                          local_controller +
                          guobject_array->find_offset("ObjectProperty "
                                                      "Engine.PlayerController."
                                                      "PlayerCameraManager")),
                      get_bone_pos(player, fname("Head"), vec3(10, 0, 0)),
                      false)
                      ? (bensure ? bot_color : player_color)
                      : (bensure ? bot_nvisible_color : player_nvisible_color);

              k2_drawline(canvas, {triangle_points[0].x, triangle_points[0].y},
                          {triangle_points[1].x, triangle_points[1].y}, 3.f,
                          alert_color);
              k2_drawline(canvas, {triangle_points[1].x, triangle_points[1].y},
                          {triangle_points[2].x, triangle_points[2].y}, 3.f,
                          alert_color);
            }

            if (setting.visuals.skeleton) {
              for (auto &bone_structure : skeleton) {
                std::string last_bone;
                for (std::string &current_bone : bone_structure) {
                  if (!last_bone.empty()) {
                    vec3 bone_from =
                             get_bone_pos(player, fname(last_bone.c_str())),
                         bone_to =
                             get_bone_pos(player, fname(current_bone.c_str()));
                    linear_color bone_color =
                        line_of_sight_to(local_controller,
                                         *reinterpret_cast<uintptr_t *>(
                                             local_controller +
                                             guobject_array->find_offset(
                                                 "ObjectProperty "
                                                 "Engine.PlayerController."
                                                 "PlayerCameraManager")),
                                         bone_from, false)
                            ? (!bensure ? player_color : bot_color)
                            : (!bensure ? player_nvisible_color
                                        : bot_nvisible_color);
                    vec2 bone_from2d, bone_to2d;
                    if (world2screen(canvas, bone_from, bone_from2d) &&
                        world2screen(canvas, bone_to, bone_to2d)) {
                      k2_drawline(canvas, bone_from2d, bone_to2d, 1.f,
                                  bone_color);
                    }
                  }

                  last_bone = current_bone;
                }
              }
            }

            vec2 line_pos, root_pos, head_pos;

            if (world2screen(
                    canvas, get_bone_pos(player, fname("Head"), vec3(60, 0, 0)),
                    line_pos) &&
                world2screen(
                    canvas, get_bone_pos(player, fname("Head"), vec3(32, 0, 0)),
                    head_pos) &&
                world2screen(canvas,
                             get_bone_pos(player, fname("Root"), vec3()),
                             root_pos)) {
              if (setting.visuals.line)
                k2_drawline(
                    canvas,
                    vec2(*reinterpret_cast<float *>(canvas + 0x30) / 2, 55),
                    line_pos, 1.f,
                    line_of_sight_to(
                        local_controller,
                        *reinterpret_cast<uintptr_t *>(
                            local_controller + guobject_array->find_offset(
                                                   "ObjectProperty "
                                                   "Engine.PlayerController."
                                                   "PlayerCameraManager")),
                        get_bone_pos(player, fname("Head"), vec3(10, 0, 0)),
                        false)
                        ? (bensure ? bot_color : player_color)
                        : (bensure ? bot_nvisible_color
                                   : player_nvisible_color));

              if (setting.visuals.health) {
                float health = *reinterpret_cast<float *>(
                          player + guobject_array->find_offset(
                                       "FloatProperty "
                                       "ShadowTrackerExtra."
                                       "STExtraCharacter.Health")),
                      near_death_breath = *reinterpret_cast<float *>(
                          player + guobject_array->find_offset(
                                       "FloatProperty "
                                       "ShadowTrackerExtra."
                                       "STExtraBaseCharacter.NearDeathBreath"));

                int current_health =
                    (int)std::max(0, std::min((int)health, 70));
                linear_color health_col = linear_color(55, 255, 95, 255);

                if (health == 0.f) {
                  current_health =
                      (int)std::max(0, std::min((int)near_death_breath, 70));
                  health_col = linear_color(200, 10, 10, 255);
                }

                k2_drawline(canvas, vec2(head_pos.x - 35, head_pos.y),
                            vec2(head_pos.x - 35 + current_health, head_pos.y),
                            6.f, health_col);

                k2_drawline(canvas, vec2(head_pos.x - 38, head_pos.y - 4),
                            vec2(head_pos.x + 38, head_pos.y - 4), 2.f,
                            linear_color(50, 50, 50, 255));
                k2_drawline(canvas, vec2(head_pos.x - 38, head_pos.y - 4),
                            vec2(head_pos.x - 38, head_pos.y + 4), 2.f,
                            linear_color(50, 50, 50, 255));
                k2_drawline(canvas, vec2(head_pos.x + 38, head_pos.y - 4),
                            vec2(head_pos.x + 38, head_pos.y + 4), 2.f,
                            linear_color(50, 50, 50, 255));
                k2_drawline(canvas, vec2(head_pos.x - 38, head_pos.y + 4),
                            vec2(head_pos.x + 38, head_pos.y + 4), 2.f,
                            linear_color(50, 50, 50, 255));
              }

              if (setting.visuals.nickname) {
                fstring player_name = *reinterpret_cast<fstring *>(
                    player +
                    guobject_array->find_offset(
                        "StrProperty Gameplay.UAECharacter.PlayerName"));
                vec2 player_name_size = k2_textsize(
                    canvas, get_font(), player_name, vec2(1.f, 1.f));
                k2_drawtext(canvas, get_font(), player_name,
                            vec2(head_pos.x - player_name_size.x / 2,
                                 head_pos.y - 4 - player_name_size.y),
                            linear_color(255, 255, 255, 255), 0.f,
                            linear_color(), vec2(), false, false, false,
                            linear_color());
              }
            }
          }
        }
      }
    }
  }

  return oPostRender(caller, canvas);
}

void post_render::initialize() {
  while (true) {
    uintptr_t engine =
        *reinterpret_cast<uintptr_t *>(ue4_elf.base() + engine_offset);
    if (engine) {
      uintptr_t viewport = *reinterpret_cast<uintptr_t *>(engine + 0x810);
      if (viewport) {
        int PostRender_index = 131;
        void **vtable = *reinterpret_cast<void ***>(viewport);

        auto f_mprotect = [](uintptr_t addr, size_t len,
                             int32_t prot) -> int32_t {
          static_assert(PAGE_SIZE == 4096);
          constexpr size_t page_size = static_cast<size_t>(PAGE_SIZE);
          void *start = reinterpret_cast<void *>(addr & -page_size);
          uintptr_t end = (addr + len + page_size - 1) & -page_size;
          return mprotect(start, end - reinterpret_cast<uintptr_t>(start),
                          prot);
        };

        if (vtable[PostRender_index] != hPostRender) {
          oPostRender = decltype(oPostRender)(vtable[PostRender_index]);
          f_mprotect(reinterpret_cast<uintptr_t>(&vtable[PostRender_index]),
                     sizeof(uintptr_t), PROT_READ | PROT_WRITE);
          vtable[PostRender_index] = reinterpret_cast<void *>(hPostRender);
        } else {
          return;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  return;
}