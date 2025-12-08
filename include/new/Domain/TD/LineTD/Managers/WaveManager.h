#pragma once

#include <functional>
#include <queue>
#include <string>
#include <vector>

#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/WaveDef.h"

namespace New::Domain::TD::LineTD::Managers {

struct SpawnRequest {
  std::string enemyId;
  int laneIndex = 0;
};

class WaveManager {
public:
  WaveManager() = default;

  void SetStage(const New::Data::StageDef *stage) { stage_ = stage; }
  void SetWave(const New::Data::WaveDef *wave);

  void Update(float deltaTime,
              const std::function<void(const SpawnRequest &)> &spawnCallback);

  bool IsFinished() const { return finished_; }

private:
  struct ActiveSpawn {
    SpawnRequest request;
    int remaining = 1;
    float timer = 0.0f;
    float interval = 0.0f;
  };

  const New::Data::StageDef *stage_ = nullptr;
  const New::Data::WaveDef *wave_ = nullptr;
  bool started_ = false;
  bool finished_ = false;
  std::vector<ActiveSpawn> activeSpawns_;
};

} // namespace New::Domain::TD::LineTD::Managers
