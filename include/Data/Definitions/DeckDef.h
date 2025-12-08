#pragma once

#include <string>
#include <vector>

namespace New::Data {

struct DeckSlotDef {
  std::string id;
  int cost = 0;
  float health = 0.0f;
  float attackDamage = 0.0f;
  float attackRange = 0.0f;
  float attackCooldown = 1.0f;
  float spawnCooldown = 0.0f;
  float knockback = 20.0f; // 押し返し強度（正: 右向き前進基準）
  std::string attackType = "single"; // single / multi / pierce
  int hitCount = 1;                  // multi/pierce時の最大ヒット数
};

struct DeckDef {
  std::string id;
  std::vector<DeckSlotDef> slots;
};

} // namespace New::Data
