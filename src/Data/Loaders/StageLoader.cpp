#include "Data/Loaders/StageLoader.h"

#include "Core/TraceCompat.h"
#include "Data/DefinitionRegistry.h"

namespace {
constexpr const char *kFieldId = "id";
constexpr const char *kFieldLanes = "lanes";
constexpr const char *kFieldIndex = "index";
constexpr const char *kFieldY = "y";
constexpr const char *kFieldStartX = "startX";
constexpr const char *kFieldEndX = "endX";
constexpr const char *kFieldWaves = "waves";
constexpr const char *kFieldPlayerLife = "playerLife";
constexpr const char *kFieldStartingCost = "startingCost";
constexpr const char *kFieldBaseArrivalDamage = "baseArrivalDamage";
constexpr const char *kFieldKillReward = "killReward";
constexpr const char *kFieldMinGap = "minGap";
constexpr const char *kFieldFrontlineIterations = "frontlineIterations";
} // namespace

namespace New::Data::Loaders {

bool StageLoader::ParseFromJson(const nlohmann::json &json) {
  defs_.clear();

  if (!json.is_array()) {
    TRACELOG(LOG_ERROR, "StageLoader: root must be an array");
    return false;
  }

  for (const auto &elem : json) {
    if (!elem.is_object()) {
      TRACELOG(LOG_WARNING, "StageLoader: skipped non-object element");
      continue;
    }
    StageDef def{};
    if (!elem.contains(kFieldId) || !elem[kFieldId].is_string()) {
      TRACELOG(LOG_ERROR, "StageLoader: 'id' is required and must be string");
      return false;
    }
    def.id = elem[kFieldId].get<std::string>();

    if (elem.contains(kFieldLanes) && elem[kFieldLanes].is_array()) {
      for (const auto &lane : elem[kFieldLanes]) {
        if (!lane.is_object()) {
          TRACELOG(LOG_WARNING, "StageLoader: skipped non-object lane");
          continue;
        }
        LaneDef l{};
        if (lane.contains(kFieldIndex) &&
            lane[kFieldIndex].is_number_integer()) {
          l.index = lane[kFieldIndex].get<int>();
        }
        if (lane.contains(kFieldY) && lane[kFieldY].is_number()) {
          l.y = lane[kFieldY].get<float>();
        }
        if (lane.contains(kFieldStartX) && lane[kFieldStartX].is_number()) {
          l.startX = lane[kFieldStartX].get<float>();
        }
        if (lane.contains(kFieldEndX) && lane[kFieldEndX].is_number()) {
          l.endX = lane[kFieldEndX].get<float>();
        }
        def.lanes.push_back(l);
      }
    }

    if (elem.contains(kFieldWaves) && elem[kFieldWaves].is_array()) {
      for (const auto &waveId : elem[kFieldWaves]) {
        if (waveId.is_string()) {
          def.waves.push_back(waveId.get<std::string>());
        }
      }
    }

    if (elem.contains(kFieldPlayerLife) &&
        elem[kFieldPlayerLife].is_number_integer()) {
      def.playerLife = elem[kFieldPlayerLife].get<int>();
    }
    if (elem.contains(kFieldStartingCost) &&
        elem[kFieldStartingCost].is_number_integer()) {
      def.startingCost = elem[kFieldStartingCost].get<int>();
    }
    if (elem.contains(kFieldBaseArrivalDamage) &&
        elem[kFieldBaseArrivalDamage].is_number_integer()) {
      def.baseArrivalDamage = elem[kFieldBaseArrivalDamage].get<int>();
    }
    if (elem.contains(kFieldKillReward) &&
        elem[kFieldKillReward].is_number_integer()) {
      def.killReward = elem[kFieldKillReward].get<int>();
    }
    if (elem.contains(kFieldMinGap) && elem[kFieldMinGap].is_number()) {
      def.minGap = elem[kFieldMinGap].get<float>();
    }
    if (elem.contains(kFieldFrontlineIterations) &&
        elem[kFieldFrontlineIterations].is_number_integer()) {
      def.frontlineIterations =
          std::max(1, elem[kFieldFrontlineIterations].get<int>());
    }

    if (def.lanes.empty()) {
      TRACELOG(LOG_WARNING, "StageLoader: stage %s has no lanes",
               def.id.c_str());
    }
    if (def.waves.empty()) {
      TRACELOG(LOG_WARNING, "StageLoader: stage %s has no waves",
               def.id.c_str());
    }

    defs_.push_back(std::move(def));
  }

  if (defs_.empty()) {
    TRACELOG(LOG_WARNING, "StageLoader: no valid stages parsed");
    return false;
  }

  return true;
}

bool StageLoader::RegisterTo(DefinitionRegistry &registry) {
  bool ok = true;
  int inserted = 0;
  for (const auto &def : defs_) {
    if (!registry.RegisterStage(def)) {
      ok = false;
    } else {
      ++inserted;
    }
  }

  if (inserted == 0) {
    defs_.clear();
    if (GenerateFallback()) {
      for (const auto &def : defs_) {
        if (registry.RegisterStage(def)) {
          ++inserted;
        }
      }
    }
    ok = ok && inserted > 0;
  }

  return ok;
}

bool StageLoader::GenerateFallback() {
  defs_.clear();
  StageDef def{};
  def.id = "fallback_stage";
  def.playerLife = 5;
  def.startingCost = 50;
  def.baseArrivalDamage = 1;
  def.killReward = 5;
  def.minGap = 80.0f;
  def.frontlineIterations = 3;
  def.lanes.push_back(LaneDef{0, 360.0f, 0.0f, 1920.0f});
  def.waves.push_back("fallback_wave");
  defs_.push_back(def);
  TRACELOG(LOG_INFO, "StageLoader: generated procedural fallback stage");
  return true;
}

} // namespace New::Data::Loaders
