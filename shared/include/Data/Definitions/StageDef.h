#pragma once

#include <string>
#include <vector>

namespace Shared::Data {

/// @brief ステージ定義
struct StageDef {
  std::string id;
  std::string name;
  std::string description;
  int difficulty = 1;
  std::string domain;          // 例: "World1"
  int subdomain = 0;           // 例: 1, 2, ...
  int base_arrival_damage = 1; // 敵到達時に城へ与えるダメージ

  struct Environment {
    std::string background_image;
    std::string bgm_id;
    float scroll_speed = 0.0f;
  } environment;

  struct CastleHP {
    int player_castle_hp = 10000;
    int enemy_castle_hp = 10000;
  } castle_hp;

  std::vector<std::string> wave_ids;

  struct Rewards {
    int gold = 0;
    int exp = 0;
    std::vector<std::string> items;
  } rewards;

  std::vector<std::string> tags;
};

} // namespace Shared::Data
