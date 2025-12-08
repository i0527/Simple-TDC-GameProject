#pragma once

#include <string>
#include <vector>

namespace New::Data {

struct StatModifier {
  std::string stat; // e.g., attack, defense, speed
  int amount = 0;   // positive for buff, negative for debuff
};

struct AbilityDef {
  std::string id;
  std::string name;
  std::string description;
  int cost = 0;
  float cooldown = 0.0f;
  std::string type;    // e.g., damage/heal/buff/debuff
  std::string element; // e.g., fire/water/neutral
  int power = 0;       // magnitude of effect (damage/heal amount)
  std::string target;  // e.g., self/ally/enemy
  float critMultiplier = 1.0f;
  float duration = 0.0f; // for DOT/HoT or buff duration (seconds)
  std::vector<StatModifier> modifiers;
  bool isDot = false;
  bool isHot = false;
};

} // namespace New::Data
