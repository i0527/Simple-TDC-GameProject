#pragma once

#include <string>
#include <vector>

namespace Shared::Data {

/// @brief エンティティ（キャラクター・敵）定義
struct EntityDef {
    std::string id;
    std::string name;
    std::string description;
    int rarity = 1;
    std::string type;  // "main" / "sub"
    bool is_enemy = false;

    int cost = 0;
    float cooldown = 0.0f;

    struct Stats {
        int hp = 100;
        int attack = 10;
        float attack_speed = 1.0f;
        int range = 100;
        float move_speed = 50.0f;
        int knockback = 0;
    } stats;

    std::string draw_type;  // "parts_animation" / "sprite"

    struct Display {
        std::string sprite_sheet;
        std::string idle_animation;
        std::string walk_animation;
        std::string attack_animation;
        std::string death_animation;
    } display;

    std::vector<std::string> skill_ids;
    std::vector<std::string> ability_ids;

    std::vector<std::string> tags;
};

} // namespace Shared::Data
