#pragma once

namespace New::Game::Components {

struct UnitPreset {
  int cost = 0;
  float health = 0.0f;
  float attackDamage = 0.0f;
  float attackRange = 0.0f;
  float attackCooldown = 1.0f;
  float spawnCooldown = 0.0f;
  float knockback = 12.0f;
  std::string attackType = "single"; // single/multi/pierce
  int hitCount = 1;
};

// ユニットがどのスロットから召喚されたかを記録
struct SlotId {
  int slotIndex = -1; // 0-9のスロットインデックス
};

} // namespace New::Game::Components
