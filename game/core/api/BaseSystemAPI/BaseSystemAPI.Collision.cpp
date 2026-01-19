#include "../CollisionSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

namespace game {
namespace core {

CollisionSystemAPI::CollisionSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

bool CollisionSystemAPI::CheckCollisionPointRec(Vector2 point, Rectangle rec) {
  return ::CheckCollisionPointRec(point, rec);
}

bool CollisionSystemAPI::CheckCollisionRecs(Rectangle rec1, Rectangle rec2) {
  return ::CheckCollisionRecs(rec1, rec2);
}

bool CollisionSystemAPI::CheckCollisionCircles(Vector2 center1, float radius1,
                                               Vector2 center2, float radius2) {
  return ::CheckCollisionCircles(center1, radius1, center2, radius2);
}

bool CollisionSystemAPI::CheckCollisionCircleRec(Vector2 center, float radius,
                                                 Rectangle rec) {
  return ::CheckCollisionCircleRec(center, radius, rec);
}

Rectangle CollisionSystemAPI::GetCollisionRec(Rectangle rec1,
                                              Rectangle rec2) {
  return ::GetCollisionRec(rec1, rec2);
}

} // namespace core
} // namespace game
