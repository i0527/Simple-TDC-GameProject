#pragma once

#include <string>
#include <vector>

namespace New::Data {

struct WaveEntry {
  std::string enemyId;
  int laneIndex = 0;
  int count = 1;
  float startDelaySeconds = 0.0f;
  float intervalSeconds = 0.0f;
};

struct WaveDef {
  std::string id;
  std::vector<WaveEntry> entries;
};

} // namespace New::Data
