#include "shoot_inner.hpp"

void (*oShootBulletInner)(uintptr_t caller, vec3 start, vec3 rot, int id);
void hShootBulletInner(uintptr_t caller, vec3 start, vec3 rot, int id) {
  auto get_actors = [](uintptr_t world_context_object,
                       uintptr_t actor_class) -> tarray<uintptr_t> {
    tarray<uintptr_t> result;
    reinterpret_cast<void (*)(uintptr_t, uintptr_t, tarray<uintptr_t> &)>(
        ue4_elf.base() + get_actors_offset)(world_context_object, actor_class,
                                            result);
    return result;
  };

  auto get_bone_pos = [](uintptr_t player, fname bone_name,
                         vec3 offset) -> vec3 {
    return reinterpret_cast<vec3 (*)(uintptr_t, fname, vec3)>(
        ue4_elf.base() + get_bone_pos_offset)(player, bone_name, offset);
  };

  auto get_velocity = [](uintptr_t actor) -> vec3 {
    void **vtable = *reinterpret_cast<void ***>(actor);
    return reinterpret_cast<vec3 (*)(uintptr_t)>(vtable[105])(actor);
  };

  auto get_parachute_velocity = [](uintptr_t parachute_comp) -> vec3 {
    return reinterpret_cast<vec3 (*)(uintptr_t)>(ue4_elf.base() +
                                                 0x4EA3B5C)(parachute_comp);
  };

  auto get_local_controller = [](uintptr_t engine) -> uintptr_t {
    uintptr_t game_instance = *reinterpret_cast<uintptr_t *>(engine + 0xe08);
    if (game_instance) {
      uintptr_t ulocal_player =
          *reinterpret_cast<uintptr_t *>(game_instance + 264) ^
          **reinterpret_cast<uintptr_t **>(game_instance + 56);
      return *reinterpret_cast<uintptr_t *>(ulocal_player + 0x30);
    }
  };

  auto apply_instant = [](uintptr_t swec) {
    uintptr_t bullet_fire_speed_proxy =
        *reinterpret_cast<uintptr_t *>(swec + 1280);
    if (bullet_fire_speed_proxy) {
      struct enc_float {
        float value;
        uint8_t key;
      };

      uint8_t index =
          *reinterpret_cast<uint8_t *>(bullet_fire_speed_proxy + 48);
      enc_float *storage =
          reinterpret_cast<enc_float *>(bullet_fire_speed_proxy + 6 * index);

      storage->value = 650000.f;
      LOBYTE(storage->value) ^= storage->key;
      BYTE1(storage->value) ^= storage->key;
      BYTE2(storage->value) ^= storage->key;
      HIBYTE(storage->value) ^= storage->key;
    }
  };

  auto apply_rapid = [](uintptr_t swec) {
    *reinterpret_cast<uint8_t *>(
        swec + guobject_array->find_offset(
                   "ByteProperty "
                   "ShadowTrackerExtra.ShootWeaponEntity.ShootType")) = 4;
    *reinterpret_cast<float *>(
        swec + guobject_array->find_offset(
                   "FloatProperty "
                   "ShadowTrackerExtra.ShootWeaponEntity.ShootInterval")) = 0.f;
  };

  auto get_bullet_fire_speed = [](uintptr_t swec) -> float {
    uintptr_t bullet_fire_speed_proxy =
        *reinterpret_cast<uintptr_t *>(swec + 1280);
    if (bullet_fire_speed_proxy) {
      struct enc_float {
        float value;
        uint8_t key;
      };

      uint8_t index =
          *reinterpret_cast<uint8_t *>(bullet_fire_speed_proxy + 48);
      enc_float *storage =
          reinterpret_cast<enc_float *>(bullet_fire_speed_proxy + 6 * index);

      float bfs = storage->value;

      LOBYTE(bfs) ^= storage->key;
      BYTE1(bfs) ^= storage->key;
      BYTE2(bfs) ^= storage->key;
      HIBYTE(bfs) ^= storage->key;

      return bfs;
    }

    return 0.f;
  };

  auto apply_norecoil = [](uintptr_t swec) {
    uintptr_t accessories_vrecoil_factor_proxy =
        *reinterpret_cast<uintptr_t *>(swec + 2856);
    if (accessories_vrecoil_factor_proxy) {
      struct enc_float {
        float value;
        uint8_t key;
      };

      uint8_t index =
          *reinterpret_cast<uint8_t *>(accessories_vrecoil_factor_proxy + 48);
      enc_float *storage = reinterpret_cast<enc_float *>(
          accessories_vrecoil_factor_proxy + 6 * index);

      storage->value = 0.f;
      LOBYTE(storage->value) ^= storage->key;
      BYTE1(storage->value) ^= storage->key;
      BYTE2(storage->value) ^= storage->key;
      HIBYTE(storage->value) ^= storage->key;
    }

    uintptr_t accessories_hrecoil_factor_proxy =
        *reinterpret_cast<uintptr_t *>(swec + 2864);
    if (accessories_hrecoil_factor_proxy) {
      struct enc_float {
        float value;
        uint8_t key;
      };

      uint8_t index =
          *reinterpret_cast<uint8_t *>(accessories_hrecoil_factor_proxy + 48);
      enc_float *storage = reinterpret_cast<enc_float *>(
          accessories_hrecoil_factor_proxy + 6 * index);

      storage->value = 0.f;
      LOBYTE(storage->value) ^= storage->key;
      BYTE1(storage->value) ^= storage->key;
      BYTE2(storage->value) ^= storage->key;
      HIBYTE(storage->value) ^= storage->key;
    }

    uintptr_t accessories_recovery_factor_proxy =
        *reinterpret_cast<uintptr_t *>(swec + 2872);
    if (accessories_recovery_factor_proxy) {
      struct enc_float {
        float value;
        uint8_t key;
      };

      uint8_t index =
          *reinterpret_cast<uint8_t *>(accessories_recovery_factor_proxy + 48);
      enc_float *storage = reinterpret_cast<enc_float *>(
          accessories_recovery_factor_proxy + 6 * index);

      storage->value = 0.f;
      LOBYTE(storage->value) ^= storage->key;
      BYTE1(storage->value) ^= storage->key;
      BYTE2(storage->value) ^= storage->key;
      HIBYTE(storage->value) ^= storage->key;
    }
  };

  uintptr_t engine =
                *reinterpret_cast<uintptr_t *>(ue4_elf.base() + engine_offset),
            swec = *reinterpret_cast<uintptr_t *>(
                caller + guobject_array->find_offset(
                             "ObjectProperty "
                             "ShadowTrackerExtra.STExtraShootWeaponComponent."
                             "ShootWeaponEntityComponent"));
  if (setting.aimbot.instant)
    apply_instant(swec);
  if (setting.aimbot.rapid)
    apply_rapid(swec);
  if (setting.aimbot.norecoil)
    apply_norecoil(swec);
  if (setting.aimbot.enable) {
    if (engine) {
      uintptr_t local_controller = get_local_controller(engine), local_player;
      if (local_controller) {
        local_player = *reinterpret_cast<uintptr_t *>(
            local_controller + guobject_array->find_offset(
                                   "ObjectProperty "
                                   "Engine.PlayerController.AcknowledgedPawn"));
        if (local_player) {
          tarray<uintptr_t> players = get_actors(
              local_controller,
              guobject_array->find_object(
                  "Class ShadowTrackerExtra.STExtraPlayerCharacter"));
          vec3 aimpos;
          uintptr_t target =
              shoot_inner::get_target(players, local_player, aimpos);
          if (target) {
            vec3 velocity;
            float distance = aimpos.distance(start) / 100,
                  bfs = get_bullet_fire_speed(swec),
                  travel_time = distance / bfs;

            uintptr_t vehicle = *reinterpret_cast<uintptr_t *>(
                target + guobject_array->find_offset(
                             "ObjectProperty "
                             "ShadowTrackerExtra.STExtraCharacter."
                             "CurrentVehicle"));
            if (vehicle) {
              velocity = get_velocity(vehicle);
            } else {
              if (*reinterpret_cast<uint8_t *>(
                      target + guobject_array->find_offset(
                                   "EnumProperty "
                                   "ShadowTrackerExtra.STExtraPlayerCharacter."
                                   "ParachuteState")) != 0) {
                velocity =
                    get_parachute_velocity(*reinterpret_cast<uintptr_t *>(
                        target + guobject_array->find_offset(
                                     "ObjectProperty "
                                     "ShadowTrackerExtra.STExtraBaseCharacter."
                                     "ParachuteComponent")));
              } else {
                velocity = get_velocity(target);
              }
            }

            aimpos = aimpos + (velocity * travel_time);
            rot = vec3::calc_angle(start, aimpos);
          }
        }
      }
    }
  }

  return oShootBulletInner(caller, start, rot, id);
}

uintptr_t shoot_inner::get_target(tarray<uintptr_t> &players,
                                  uintptr_t local_player,
                                  vec3 &out_target_pos) {
  auto lts = [](uintptr_t world_context_obj, vec3 start, vec3 end) -> bool {
    struct fhit_result {
      unsigned char bblocking_hit : 1;
      unsigned char bstart_penetrating : 1;
      unsigned char unk_data_01[0x3];
      float time;
      float distance;
      struct vec3 location;
      struct vec3 impact_point;
      struct vec3 normal;
      struct vec3 impact_normal;
      struct vec3 trace_start;
      struct vec3 trace_end;
      float penetration_depth;
      int item;
      uintptr_t phys_material;
      uintptr_t actor;
      uintptr_t component;
      unsigned char unk_data_02[0x4];
      fname bone_name;
      int face_index;
      unsigned char unk_data_03[0x4];
      tarray<struct vec3> triangle_vertex_location_array;
    };

    fhit_result out_hit;
    tarray<uintptr_t> actors_to_ignore;
    return reinterpret_cast<bool (*)(
        uintptr_t, vec3, vec3, uint8_t, bool, tarray<uintptr_t> &, uint8_t,
        fhit_result &, bool, linear_color, linear_color, float)>(
        ue4_elf.base() + 0x8B2A3C4)(world_context_obj, start, end, 8, true,
                                    actors_to_ignore, 1, out_hit, true,
                                    linear_color(), linear_color(), 0.f);
  };

  auto get_bone_pos = [](uintptr_t player, fname bone_name,
                         vec3 offset) -> vec3 {
    return reinterpret_cast<vec3 (*)(uintptr_t, fname, vec3)>(
        ue4_elf.base() + get_bone_pos_offset)(player, bone_name, offset);
  };

  auto get_current_weapon = [](uintptr_t player) -> uintptr_t {
    return reinterpret_cast<uintptr_t (*)(uintptr_t)>(
        ue4_elf.base() + get_current_weapon_offset)(player);
  };

  auto get_muzzle_transform = [](uintptr_t weapon) -> transform {
    void **vtable = *reinterpret_cast<void ***>(weapon);
    return reinterpret_cast<transform (*)(uintptr_t)>(
        vtable[get_muzzle_transform_index])(weapon);
  };

  auto apply_magic = [](uintptr_t player) {
    uintptr_t mesh = *reinterpret_cast<uintptr_t *>(
        player +
        guobject_array->find_offset("ObjectProperty Engine.Character.Mesh"));
    if (mesh) {
      uintptr_t skeletal_mesh = *reinterpret_cast<uintptr_t *>(
          mesh + guobject_array->find_offset(
                     "ObjectProperty "
                     "Engine.SkinnedMeshComponent.SkeletalMesh"));
      if (skeletal_mesh) {
        uintptr_t passet = *reinterpret_cast<uintptr_t *>(
            skeletal_mesh +
            guobject_array->find_offset(
                "ObjectProperty Engine.SkeletalMesh.PhysicsAsset"));
        if (passet) {
          int32_t bone_index = reinterpret_cast<int32_t (*)(uintptr_t, fname)>(
              ue4_elf.base() + get_bone_idx_offset)(passet, fname("Head"));
          if (bone_index != -1) {
            uintptr_t setup = *reinterpret_cast<uintptr_t *>(
                *reinterpret_cast<uintptr_t *>(passet + 0x38) +
                8 * (uint32_t)bone_index);
            if (setup) {
              uintptr_t boxes =
                  *reinterpret_cast<uintptr_t *>(setup + 0x28 + 0x10);
              if (boxes) {
                *reinterpret_cast<float *>(boxes + 0x88) = 260.f;
                *reinterpret_cast<float *>(boxes + 0x8C) = 260.f;
                *reinterpret_cast<float *>(boxes + 0x90) = 260.f;
              }
            }
          }
        }
      }
    }
  };

  static std::vector<std::pair<int, std::string>> priority_struct = {
      {1, "Head"},       {2, "spine_02"},   {3, "pelvis"},
      {4, "clavicle_r"}, {4, "clavicle_l"}, {5, "upperarm_r"},
      {5, "upperarm_l"}, {6, "lowerarm_r"}, {6, "lowerarm_l"},
      {7, "hand_r"},     {7, "hand_l"},     {8, "thigh_r"},
      {8, "thigh_l"},    {9, "calf_r"},     {9, "calf_l"},
      {10, "foot_r"},    {10, "foot_l"}};

  uintptr_t result = 0;
  float max = std::numeric_limits<float>::infinity();
  int _bone_priority = 11;

  for (int i = 0; i < players.get_count(); i++) {
    uintptr_t player = players[i];
    if (player) {
      float health = *reinterpret_cast<float *>(
                player + guobject_array->find_offset(
                             "FloatProperty "
                             "ShadowTrackerExtra.STExtraCharacter.Health")),
            near_death_breath = *reinterpret_cast<float *>(
                player + guobject_array->find_offset(
                             "FloatProperty "
                             "ShadowTrackerExtra.STExtraBaseCharacter."
                             "NearDeathBreath"));
      bool bhidden = *reinterpret_cast<bool *>(
               player + guobject_array->find_offset(
                            "BoolProperty Engine.Actor.bHidden")),
           bensure = *reinterpret_cast<bool *>(
               player + guobject_array->find_offset(
                            "BoolProperty Gameplay.UAECharacter.bEnsure"));
      int team_id = *reinterpret_cast<int *>(
              player + guobject_array->find_offset(
                           "IntProperty Gameplay.UAECharacter.TeamID")),
          local_team_id = *reinterpret_cast<int *>(
              local_player + guobject_array->find_offset(
                                 "IntProperty Gameplay.UAECharacter.TeamID"));

      if (bhidden && near_death_breath == 0.f)
        continue;
      if (local_team_id == team_id)
        continue;
      if (setting.aimbot.ignore_knocked && health == 0.f)
        continue;
      if (setting.aimbot.ignore_bots && bensure)
        continue;

      vec3 muzzle_pos = get_muzzle_transform(get_current_weapon(local_player))
                            .translation,
           target_position;

      if (setting.aimbot.target_type == target_type_bone2bone) {
        for (std::pair<int, std::string> bone : priority_struct) {
          vec3 bone_pos =
              get_bone_pos(player, fname(bone.second.c_str()), vec3());
          if (lts(local_player, muzzle_pos, bone_pos))
            continue;

          if (_bone_priority > bone.first) {
            _bone_priority = bone.first;
            target_position = bone_pos;
          }
        }
      } else if (setting.aimbot.target_type == target_type_multipoint) {
        apply_magic(player);
        vec3 head_pos = get_bone_pos(player, fname("Head"), vec3(0, 0, 50));
        float _x, _y, _z;
        for (_x = 0.f; _x < 150.f; _x += setting.aimbot.accuracy)
          ;
        {
          for (_z = 0.f; _z < 150.f; _z += setting.aimbot.accuracy) {
            vec3 finally_pos = head_pos + vec3(_x, 0, _z);
            if (!lts(local_player, muzzle_pos, head_pos)) {
              target_position = head_pos;
              break;
            }

            if (lts(local_player, muzzle_pos, finally_pos))
              continue;

            target_position = finally_pos;
            break;
          }
        }
      }

      if (lts(local_player, muzzle_pos, target_position))
        continue;

      float distance = muzzle_pos.distance(target_position);

      if (distance < max) {
        out_target_pos = target_position;
        result = player;
        max = distance;
      }
    }
  }

  return result;
}

void shoot_inner::initialize(uintptr_t local_player) {
  auto get_current_weapon = [](uintptr_t player) -> uintptr_t {
    return reinterpret_cast<uintptr_t (*)(uintptr_t)>(
        ue4_elf.base() + get_current_weapon_offset)(player);
  };

  auto start_fire = [](uintptr_t controller) -> void {
    return reinterpret_cast<void (*)(uintptr_t)>(
        ue4_elf.base() + on_press_fire_offset)(controller);
  };

  auto release_fire = [](uintptr_t controller) -> void {
    return reinterpret_cast<void (*)(uintptr_t)>(
        ue4_elf.base() + on_release_fire_offset)(controller);
  };

  auto get_actors = [](uintptr_t world_context_object,
                       uintptr_t actor_class) -> tarray<uintptr_t> {
    tarray<uintptr_t> result;
    reinterpret_cast<void (*)(uintptr_t, uintptr_t, tarray<uintptr_t> &)>(
        ue4_elf.base() + get_actors_offset)(world_context_object, actor_class,
                                            result);
    return result;
  };

  auto lod_bypass = [](uintptr_t local_player) {
    uintptr_t part_hit_comp = *reinterpret_cast<uintptr_t *>(
        local_player +
        guobject_array->find_offset(
            "ObjectProperty "
            "ShadowTrackerExtra.STExtraCharacter.PartHitComponent"));
    if (part_hit_comp) {
      struct colision_lod {
        float distsq, angle;
      };

      tarray<colision_lod> config_collision_dist_sq_angles =
          *reinterpret_cast<tarray<colision_lod> *>(
              part_hit_comp +
              guobject_array->find_offset("ArrayProperty "
                                          "ShadowTrackerExtra.PartHitComponent."
                                          "ConfigCollisionDistSqAngles"));
      for (int i = 0; i < config_collision_dist_sq_angles.get_count(); i++) {
        config_collision_dist_sq_angles[i].angle = 180.0f;
        config_collision_dist_sq_angles[i].distsq = 180.0f;
      }

      *reinterpret_cast<tarray<colision_lod> *>(
          part_hit_comp +
          guobject_array->find_offset("ArrayProperty "
                                      "ShadowTrackerExtra.PartHitComponent."
                                      "ConfigCollisionDistSqAngles")) =
          config_collision_dist_sq_angles;
    }
  };

  lod_bypass(local_player);
  uintptr_t weapon = get_current_weapon(local_player);
  if (weapon) {
    uintptr_t swc = *reinterpret_cast<uintptr_t *>(
        weapon +
        guobject_array->find_offset("ObjectProperty "
                                    "ShadowTrackerExtra.STExtraShootWeapon."
                                    "ShootWeaponComponent"));
    if (swc) {
      int ShootBulletInner_index = 168;
      void **vtable = *reinterpret_cast<void ***>(swc);

      auto f_mprotect = [](uintptr_t addr, size_t len,
                           int32_t prot) -> int32_t {
        static_assert(PAGE_SIZE == 4096);
        constexpr size_t page_size = static_cast<size_t>(PAGE_SIZE);
        void *start = reinterpret_cast<void *>(addr & -page_size);
        uintptr_t end = (addr + len + page_size - 1) & -page_size;
        return mprotect(start, end - reinterpret_cast<uintptr_t>(start), prot);
      };

      if (vtable[ShootBulletInner_index] != hShootBulletInner) {
        oShootBulletInner =
            decltype(oShootBulletInner)(vtable[ShootBulletInner_index]);
        f_mprotect(reinterpret_cast<uintptr_t>(&vtable[ShootBulletInner_index]),
                   sizeof(uintptr_t), PROT_READ | PROT_WRITE);
        vtable[ShootBulletInner_index] =
            reinterpret_cast<void *>(hShootBulletInner);
      }

      static bool bcan_fire = false;
      tarray<uintptr_t> players = get_actors(
          local_player, guobject_array->find_object(
                            "Class ShadowTrackerExtra.STExtraPlayerCharacter"));
      vec3 target_pos;
      uintptr_t target =
                    shoot_inner::get_target(players, local_player, target_pos),
                local_controller = *reinterpret_cast<uintptr_t *>(
                    local_player +
                    guobject_array->find_offset(
                        "ObjectProperty "
                        "ShadowTrackerExtra.STExtraPlayerCharacter."
                        "STPlayerController"));
      int bullet_num_in_clip = *reinterpret_cast<int *>(
          weapon +
          guobject_array->find_offset("IntProperty "
                                      "ShadowTrackerExtra.STExtraShootWeapon."
                                      "CurBulletNumInClip"));
      if (target && target_pos != vec3() && setting.aimbot.auto_fire &&
          bullet_num_in_clip != 0) {
        bcan_fire = true;
        start_fire(local_controller);
      } else {
        if (bcan_fire) {
          release_fire(local_controller);
          bcan_fire = false;
        }
      }
    }
  }
}