#pragma once

#include <string>
#include <vector>

namespace New::Domain::TD::LineTD::Components {

struct SpawnJob {
  std::string enemyId;
  int laneIndex = 0;
};

struct SpawnQueue {
  std::vector<SpawnJob> jobs;
};

} // namespace New::Domain::TD::LineTD::Components
