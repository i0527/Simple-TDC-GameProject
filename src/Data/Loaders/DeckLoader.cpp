#include "Data/Loaders/DeckLoader.h"

#include "Core/TraceCompat.h"
#include "Data/DefinitionRegistry.h"

namespace {
constexpr const char *kFieldId = "id";
constexpr const char *kFieldSlots = "slots";
constexpr const char *kFieldSlotId = "slotId";
constexpr const char *kFieldCost = "cost";
constexpr const char *kFieldHealth = "health";
constexpr const char *kFieldAttackDamage = "attackDamage";
constexpr const char *kFieldAttackRange = "attackRange";
constexpr const char *kFieldAttackCooldown = "attackCooldown";
constexpr const char *kFieldSpawnCooldown = "spawnCooldown";
constexpr const char *kFieldKnockback = "knockback";
constexpr const char *kFieldAttackType = "attackType";
constexpr const char *kFieldHitCount = "hitCount";
} // namespace

namespace New::Data::Loaders {

bool DeckLoader::ParseFromJson(const nlohmann::json &json) {
  defs_.clear();

  if (!json.is_array()) {
    TRACELOG(LOG_ERROR, "DeckLoader: root must be an array");
    return false;
  }

  for (const auto &elem : json) {
    if (!elem.is_object()) {
      TRACELOG(LOG_WARNING, "DeckLoader: skipped non-object element");
      continue;
    }
    DeckDef def{};
    if (!elem.contains(kFieldId) || !elem[kFieldId].is_string()) {
      TRACELOG(LOG_ERROR, "DeckLoader: 'id' is required and must be string");
      return false;
    }
    def.id = elem[kFieldId].get<std::string>();

    if (elem.contains(kFieldSlots) && elem[kFieldSlots].is_array()) {
      for (const auto &slot : elem[kFieldSlots]) {
        if (!slot.is_object()) {
          TRACELOG(LOG_WARNING, "DeckLoader: skipped non-object slot");
          continue;
        }
        DeckSlotDef s{};
        if (slot.contains(kFieldSlotId) && slot[kFieldSlotId].is_string()) {
          s.id = slot[kFieldSlotId].get<std::string>();
        }
        if (slot.contains(kFieldCost) && slot[kFieldCost].is_number_integer()) {
          s.cost = slot[kFieldCost].get<int>();
        }
        if (slot.contains(kFieldHealth) && slot[kFieldHealth].is_number()) {
          s.health = slot[kFieldHealth].get<float>();
        }
        if (slot.contains(kFieldAttackDamage) &&
            slot[kFieldAttackDamage].is_number()) {
          s.attackDamage = slot[kFieldAttackDamage].get<float>();
        }
        if (slot.contains(kFieldAttackRange) &&
            slot[kFieldAttackRange].is_number()) {
          s.attackRange = slot[kFieldAttackRange].get<float>();
        }
        if (slot.contains(kFieldAttackCooldown) &&
            slot[kFieldAttackCooldown].is_number()) {
          s.attackCooldown = slot[kFieldAttackCooldown].get<float>();
        }
        if (slot.contains(kFieldSpawnCooldown) &&
            slot[kFieldSpawnCooldown].is_number()) {
          s.spawnCooldown = slot[kFieldSpawnCooldown].get<float>();
        }
        if (slot.contains(kFieldKnockback) &&
            slot[kFieldKnockback].is_number()) {
          s.knockback = slot[kFieldKnockback].get<float>();
        }
        if (slot.contains(kFieldAttackType) &&
            slot[kFieldAttackType].is_string()) {
          s.attackType = slot[kFieldAttackType].get<std::string>();
        }
        if (slot.contains(kFieldHitCount) &&
            slot[kFieldHitCount].is_number_integer()) {
          s.hitCount = std::max(1, slot[kFieldHitCount].get<int>());
        }
        def.slots.push_back(s);
      }
    }

    defs_.push_back(std::move(def));
  }

  if (defs_.empty()) {
    TRACELOG(LOG_WARNING, "DeckLoader: no valid decks parsed");
    return false;
  }

  return true;
}

bool DeckLoader::RegisterTo(DefinitionRegistry &registry) {
  bool ok = true;
  int inserted = 0;
  for (const auto &def : defs_) {
    if (!registry.RegisterDeck(def)) {
      ok = false;
    } else {
      ++inserted;
    }
  }

  if (inserted == 0) {
    defs_.clear();
    if (GenerateFallback()) {
      for (const auto &def : defs_) {
        if (registry.RegisterDeck(def)) {
          ++inserted;
        }
      }
    }
    ok = ok && inserted > 0;
  }

  return ok;
}

bool DeckLoader::GenerateFallback() {
  defs_.clear();
  DeckDef def{};
  def.id = "fallback_deck";
  def.slots = {
      DeckSlotDef{"slot1", 30, 180.0f, 20.0f, 260.0f, 0.9f, 0.8f, 24.0f,
                  "single", 1},
      DeckSlotDef{"slot2", 40, 220.0f, 24.0f, 260.0f, 1.0f, 1.0f, 26.0f,
                  "single", 1},
      DeckSlotDef{"slot3", 50, 180.0f, 30.0f, 280.0f, 1.1f, 1.2f, 28.0f,
                  "multi", 2},
      DeckSlotDef{"slot4", 60, 260.0f, 22.0f, 240.0f, 0.8f, 1.4f, 22.0f,
                  "single", 1},
      DeckSlotDef{"slot5", 80, 320.0f, 28.0f, 300.0f, 1.3f, 1.6f, 30.0f,
                  "pierce", 3},
  };
  defs_.push_back(def);
  TRACELOG(LOG_INFO, "DeckLoader: generated fallback deck");
  return true;
}

} // namespace New::Data::Loaders
