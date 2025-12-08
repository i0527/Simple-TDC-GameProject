#include "new/Domain/TD/LineTD/Managers/WaveManager.h"

#include <algorithm>
#include <limits>

namespace New::Domain::TD::LineTD::Managers {

void WaveManager::SetWave(const New::Data::WaveDef *wave) {
  wave_ = wave;
  activeSpawns_.clear();
  started_ = false;
  finished_ = false;

  if (!wave_) {
    return;
  }

  for (const auto &entry : wave_->entries) {
    ActiveSpawn s{};
    s.request.enemyId = entry.enemyId;
    s.request.laneIndex = entry.laneIndex;
    s.remaining = std::max(1, entry.count);
    s.timer = entry.startDelaySeconds;
    s.interval = entry.intervalSeconds;
    activeSpawns_.push_back(s);
  }
}

void WaveManager::Update(
    float deltaTime, const std::function<void(const SpawnRequest &)> &spawn) {
  if (!wave_ || activeSpawns_.empty()) {
    if (started_ && activeSpawns_.empty()) {
      finished_ = true;
    }
    return;
  }

  started_ = true;
  for (auto &spawnEntry : activeSpawns_) {
    spawnEntry.timer -= deltaTime;
  }

  std::vector<ActiveSpawn> next;
  next.reserve(activeSpawns_.size());

  for (auto &spawnEntry : activeSpawns_) {
    while (spawnEntry.timer <= 0.0f && spawnEntry.remaining > 0) {
      spawn(spawnEntry.request);
      --spawnEntry.remaining;
      spawnEntry.timer += spawnEntry.interval > 0.0f
                              ? spawnEntry.interval
                              : std::numeric_limits<float>::max();
      if (spawnEntry.remaining == 0) {
        break;
      }
    }

    if (spawnEntry.remaining > 0) {
      next.push_back(spawnEntry);
    }
  }

  activeSpawns_.swap(next);
  if (started_ && activeSpawns_.empty()) {
    finished_ = true;
  }
}

} // namespace New::Domain::TD::LineTD::Managers
