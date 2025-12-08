#include "Data/Loaders/WaveLoader.h"
#include "Core/TraceCompat.h"
#include <algorithm>


#include "Data/DefinitionRegistry.h"

namespace {
constexpr const char *kFieldId = "id";
constexpr const char *kFieldEntries = "entries";
constexpr const char *kFieldEnemyId = "enemyId";
constexpr const char *kFieldDelay = "delay";
constexpr const char *kFieldLane = "lane";
constexpr const char *kFieldCount = "count";
constexpr const char *kFieldInterval = "interval";
} // namespace

namespace New::Data::Loaders {

bool WaveLoader::ParseFromJson(const nlohmann::json &json) {
  defs_.clear();

  if (!json.is_array()) {
    TRACELOG(LOG_ERROR, "WaveLoader: root must be an array");
    return false;
  }

  for (const auto &elem : json) {
    if (!elem.is_object()) {
      TRACELOG(LOG_WARNING, "WaveLoader: skipped non-object element");
      continue;
    }
    WaveDef def{};
    if (!elem.contains(kFieldId) || !elem[kFieldId].is_string()) {
      TRACELOG(LOG_ERROR, "WaveLoader: 'id' is required and must be string");
      return false;
    }
    def.id = elem[kFieldId].get<std::string>();

    if (elem.contains(kFieldEntries) && elem[kFieldEntries].is_array()) {
      int added = 0;
      for (const auto &entry : elem[kFieldEntries]) {
        if (!entry.is_object()) {
          TRACELOG(LOG_WARNING, "WaveLoader: skipped non-object entry");
          continue;
        }
        WaveEntry we{};
        if (entry.contains(kFieldEnemyId) && entry[kFieldEnemyId].is_string()) {
          we.enemyId = entry[kFieldEnemyId].get<std::string>();
        } else {
          TRACELOG(LOG_WARNING,
                   "WaveLoader: entry missing 'enemyId', skipped in wave %s",
                   def.id.c_str());
          continue;
        }
        if (entry.contains(kFieldLane) &&
            entry[kFieldLane].is_number_integer()) {
          we.laneIndex = entry[kFieldLane].get<int>();
        }
        if (entry.contains(kFieldDelay) && entry[kFieldDelay].is_number()) {
          we.startDelaySeconds = entry[kFieldDelay].get<float>();
        }
        if (entry.contains(kFieldCount) &&
            entry[kFieldCount].is_number_integer()) {
          we.count = std::max(1, entry[kFieldCount].get<int>());
        }
        if (entry.contains(kFieldInterval) &&
            entry[kFieldInterval].is_number()) {
          we.intervalSeconds = entry[kFieldInterval].get<float>();
        }
        def.entries.push_back(std::move(we));
        ++added;
      }

      if (added == 0) {
        TRACELOG(LOG_WARNING,
                 "WaveLoader: no valid entries in wave %s (skipped all)",
                 def.id.c_str());
      }
    } else {
      TRACELOG(LOG_WARNING, "WaveLoader: wave %s has no entries array",
               def.id.c_str());
    }

    if (!def.entries.empty()) {
      defs_.push_back(std::move(def));
    }
  }

  if (defs_.empty()) {
    TRACELOG(LOG_WARNING,
             "WaveLoader: no valid waves parsed (empty or skipped)");
    return false; // triggers fallback in LoadFromFile
  }

  return true;
}

bool WaveLoader::RegisterTo(DefinitionRegistry &registry) {
  bool ok = true;
  int insertedCount = 0;
  for (const auto &def : defs_) {
    const bool inserted = registry.RegisterWave(def);
    if (!inserted) {
      TRACELOG(LOG_WARNING, "WaveLoader: duplicate or invalid id: %s",
               def.id.c_str());
      ok = false;
    } else {
      ++insertedCount;
    }
  }

  if (insertedCount == 0) {
    TRACELOG(LOG_WARNING,
             "WaveLoader: no waves registered, generating fallback");
    defs_.clear();
    if (GenerateFallback()) {
      for (const auto &def : defs_) {
        if (registry.RegisterWave(def)) {
          ++insertedCount;
        }
      }
    }
    ok = ok && insertedCount > 0;
  }

  return ok;
}

bool WaveLoader::GenerateFallback() {
  defs_.clear();
  WaveDef def{};
  def.id = "fallback_wave";
  WaveEntry entry{};
  entry.enemyId = "fallback_entity";
  entry.startDelaySeconds = 0.0f;
  def.entries.push_back(entry);
  defs_.push_back(def);
  TRACELOG(LOG_INFO, "WaveLoader: generated procedural fallback wave");
  return true;
}

} // namespace New::Data::Loaders
