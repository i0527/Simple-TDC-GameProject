#pragma once

#include <string>
#include <vector>

namespace New::Data {

struct LaneDef {
  int index = 0;
  float y = 0.0f;
  float startX = 0.0f;
  float endX = 1920.0f;
};

struct StageDef {
  std::string id;
  std::vector<LaneDef> lanes;
  std::vector<std::string> waves;
  int playerLife = 10;
  int startingCost = 100;
  int baseArrivalDamage = 1;
  int killReward = 5;
  float minGap = 80.0f;        // 前線の最小間隔（敵味方間）
  int frontlineIterations = 3; // 押し合い補正の反復回数
};

} // namespace New::Data
