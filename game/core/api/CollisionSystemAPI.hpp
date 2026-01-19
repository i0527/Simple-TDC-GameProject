#pragma once

// プロジェクト内
#include "../config/RenderTypes.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief 衝突判定API
class CollisionSystemAPI {
public:
  explicit CollisionSystemAPI(BaseSystemAPI* owner);
  ~CollisionSystemAPI() = default;

  bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
  bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
  bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2,
                             float radius2);
  bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);
  Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);

private:
  BaseSystemAPI* owner_;
};

} // namespace core
} // namespace game
