#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Shared::Data {

/// @brief エンティティ（キャラクター・敵）定義
struct EntityDef {
    // 定義ファイルのパス（相対参照解決用）
    std::string source_path;
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

    struct Combat {
        float attack_point = 0.0f; // 0.0 to 1.0
        int attack_frame = -1;

        struct Hitbox {
            float width = 32.0f;
            float height = 32.0f;
            float offset_x = 0.0f;
            float offset_y = 0.0f;
        } hitbox;
    } combat;

    std::string draw_type;  // "parts_animation" / "sprite"

    struct Display {
        struct AnimClip {
            std::string atlas;  // スプライト画像（アニメーションごとに個別指定可）
            std::string json;   // Aseprite JSON
            bool loop = true;
            bool mirror_h = false;
            bool mirror_v = false;
        };

        // 固定4アニメーション用の新形式（idle/walk/attack/death 想定）
        std::unordered_map<std::string, AnimClip> animations;

        std::string sprite_sheet;
        std::string idle_animation;
        std::string walk_animation;
        std::string attack_animation;
        std::string death_animation;
        std::string atlas_texture; // Aseprite画像パス
        std::unordered_map<std::string, std::string> sprite_actions; // action -> aseprite json path
        std::string icon; // UI用アイコンパス
        bool mirror_h = false; // 水平ミラー既定
        bool mirror_v = false; // 垂直ミラー既定
        // アクション別ミラー既定
        std::unordered_map<std::string, bool> action_mirror_h;
        std::unordered_map<std::string, bool> action_mirror_v;
        // 開発用分割スプライト管理
        std::string dev_animation_config_path; // 開発用アニメーション設定JSONのパス
        bool use_dev_mode = false; // 開発モード使用フラグ
    } display;

    std::vector<std::string> skill_ids;
    std::vector<std::string> ability_ids;

    std::vector<std::string> tags;
};

} // namespace Shared::Data
