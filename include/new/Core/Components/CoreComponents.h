#pragma once

#include <algorithm>
#include <string>

#include "Core/Components/Transform.h"
#include "raylib.h"

namespace New::Core::Components {

// 基本スプライト情報（テクスチャは未対応のため色矩形描画を想定）
struct Sprite {
  float width = 64.0f;
  float height = 64.0f;
  Color tint = WHITE;
};

// シンプルな体力コンポーネント
struct Health {
  int current = 1;
  int max = 1;

  bool IsDead() const { return current <= 0; }

  void ApplyDamage(int amount) {
    if (amount <= 0) {
      return;
    }
    current = std::max(0, current - amount);
  }
};

} // namespace New::Core::Components
