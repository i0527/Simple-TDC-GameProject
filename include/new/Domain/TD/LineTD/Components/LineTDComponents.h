#pragma once

#include <string>

namespace New::Domain::TD::LineTD::Components {

struct Lane {
  int laneIndex = 0;      // レーン番号
  float yPosition = 0.0f; // レーンのY座標
  float startX = 0.0f;    // 進行開始位置
  float endX = 1920.0f;   // 進行終了位置
};

struct LaneMovement {
  int laneIndex = 0;
  float speed = 160.0f;
  float currentX = 0.0f;
  bool movingRight = true;
};

struct LanePlacement {
  int laneIndex = 0;
  float xPosition = 0.0f;
};

enum class TeamSide { Player, Enemy };

struct Team {
  TeamSide side = TeamSide::Player;
};

struct CombatStats {
  float attackDamage = 10.0f;
  float attackRange = 200.0f;
  float attackCooldown = 1.0f;
  float attackTimer = 0.0f;
};

} // namespace New::Domain::TD::LineTD::Components
