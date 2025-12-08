#include "Data/Loaders/AbilityLoader.h"
#include "Core/TraceCompat.h"

#include "Data/DefinitionRegistry.h"

namespace {
constexpr const char *kFieldId = "id";
constexpr const char *kFieldName = "name";
constexpr const char *kFieldDescription = "description";
constexpr const char *kFieldCost = "cost";
constexpr const char *kFieldCooldown = "cooldown";
constexpr const char *kFieldType = "type";
constexpr const char *kFieldElement = "element";
constexpr const char *kFieldPower = "power";
constexpr const char *kFieldTarget = "target";
constexpr const char *kFieldCrit = "critMultiplier";
constexpr const char *kFieldDuration = "duration";
constexpr const char *kFieldModifiers = "modifiers";
constexpr const char *kFieldIsDot = "isDot";
constexpr const char *kFieldIsHot = "isHot";
constexpr const char *kFieldStat = "stat";
constexpr const char *kFieldAmount = "amount";
} // namespace

namespace New::Data::Loaders {

bool AbilityLoader::ParseFromJson(const nlohmann::json &json) {
  defs_.clear();

  if (!json.is_array()) {
    TRACELOG(LOG_ERROR, "AbilityLoader: root must be an array");
    return false;
  }

  for (const auto &elem : json) {
    if (!elem.is_object()) {
      TRACELOG(LOG_WARNING, "AbilityLoader: skipped non-object element");
      continue;
    }
    AbilityDef def{};
    if (!elem.contains(kFieldId) || !elem[kFieldId].is_string()) {
      TRACELOG(LOG_ERROR, "AbilityLoader: 'id' is required and must be string");
      return false;
    }
    def.id = elem[kFieldId].get<std::string>();

    if (elem.contains(kFieldName) && elem[kFieldName].is_string()) {
      def.name = elem[kFieldName].get<std::string>();
    }
    if (elem.contains(kFieldDescription) &&
        elem[kFieldDescription].is_string()) {
      def.description = elem[kFieldDescription].get<std::string>();
    }
    if (elem.contains(kFieldCost) && elem[kFieldCost].is_number_integer()) {
      def.cost = elem[kFieldCost].get<int>();
    }
    if (elem.contains(kFieldCooldown) &&
        elem[kFieldCooldown].is_number_float()) {
      def.cooldown = elem[kFieldCooldown].get<float>();
    }
    if (elem.contains(kFieldType) && elem[kFieldType].is_string()) {
      def.type = elem[kFieldType].get<std::string>();
    }
    if (elem.contains(kFieldElement) && elem[kFieldElement].is_string()) {
      def.element = elem[kFieldElement].get<std::string>();
    }
    if (elem.contains(kFieldPower) && (elem[kFieldPower].is_number_integer() ||
                                       elem[kFieldPower].is_number_float())) {
      def.power = static_cast<int>(elem[kFieldPower].get<double>());
    }
    if (elem.contains(kFieldTarget) && elem[kFieldTarget].is_string()) {
      def.target = elem[kFieldTarget].get<std::string>();
    }
    if (elem.contains(kFieldCrit) && elem[kFieldCrit].is_number()) {
      def.critMultiplier = static_cast<float>(elem[kFieldCrit].get<double>());
    }
    if (elem.contains(kFieldDuration) && elem[kFieldDuration].is_number()) {
      def.duration = static_cast<float>(elem[kFieldDuration].get<double>());
    }
    if (elem.contains(kFieldIsDot) && elem[kFieldIsDot].is_boolean()) {
      def.isDot = elem[kFieldIsDot].get<bool>();
    }
    if (elem.contains(kFieldIsHot) && elem[kFieldIsHot].is_boolean()) {
      def.isHot = elem[kFieldIsHot].get<bool>();
    }
    if (elem.contains(kFieldModifiers) && elem[kFieldModifiers].is_array()) {
      for (const auto &m : elem[kFieldModifiers]) {
        if (!m.is_object()) {
          TRACELOG(LOG_WARNING,
                   "AbilityLoader: skipped non-object modifier in %s",
                   def.id.c_str());
          continue;
        }
        StatModifier mod{};
        bool ok = true;
        if (m.contains(kFieldStat) && m[kFieldStat].is_string()) {
          mod.stat = m[kFieldStat].get<std::string>();
        } else {
          TRACELOG(LOG_WARNING, "AbilityLoader: modifier missing 'stat' in %s",
                   def.id.c_str());
          ok = false;
        }
        if (m.contains(kFieldAmount) && m[kFieldAmount].is_number_integer()) {
          mod.amount = m[kFieldAmount].get<int>();
        } else {
          TRACELOG(LOG_WARNING,
                   "AbilityLoader: modifier missing 'amount' in %s",
                   def.id.c_str());
          ok = false;
        }
        if (ok) {
          def.modifiers.push_back(mod);
        }
      }
    }

    defs_.push_back(std::move(def));
  }

  if (defs_.empty()) {
    TRACELOG(LOG_WARNING,
             "AbilityLoader: no valid abilities parsed (empty or skipped)");
    return false; // triggers fallback
  }

  return true;
}

bool AbilityLoader::RegisterTo(DefinitionRegistry &registry) {
  bool ok = true;
  int insertedCount = 0;
  for (const auto &def : defs_) {
    const bool inserted = registry.RegisterAbility(def);
    if (!inserted) {
      TRACELOG(LOG_WARNING, "AbilityLoader: duplicate or invalid id: %s",
               def.id.c_str());
      ok = false;
    } else {
      ++insertedCount;
    }
  }

  if (insertedCount == 0) {
    TRACELOG(LOG_WARNING,
             "AbilityLoader: no abilities registered, generating fallback");
    defs_.clear();
    if (GenerateFallback()) {
      for (const auto &def : defs_) {
        if (registry.RegisterAbility(def)) {
          ++insertedCount;
        }
      }
    }
    ok = ok && insertedCount > 0;
  }

  return ok;
}

bool AbilityLoader::GenerateFallback() {
  defs_.clear();
  AbilityDef def{};
  def.id = "fallback_ability";
  def.name = "Fallback Ability";
  def.description = "Generated fallback ability";
  def.cost = 0;
  def.cooldown = 0.0f;
  def.type = "generic";
  def.element = "neutral";
  def.power = 0;
  def.target = "enemy";
  def.critMultiplier = 1.0f;
  def.duration = 0.0f;
  def.isDot = false;
  def.isHot = false;
  def.modifiers = {};
  defs_.push_back(def);
  TRACELOG(LOG_INFO, "AbilityLoader: generated procedural fallback ability");
  return true;
}

} // namespace New::Data::Loaders
