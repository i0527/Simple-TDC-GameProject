#include "Game/Systems/MovementSystem.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float DEFAULT_MIN_X = 0.0f;
constexpr float DEFAULT_MIN_Y = 0.0f;
constexpr float DEFAULT_MAX_X = 1920.0f;
constexpr float DEFAULT_MAX_Y = 1080.0f;
constexpr float STOP_DISTANCE = 50.0f;
constexpr float LANE_TOLERANCE = 30.0f;
} // namespace

namespace Game::Systems {

void MovementSystem::Update(entt::registry &registry, float delta_time) {
  UpdatePositions(registry, delta_time);
  HandleBoundaries(registry);
}

void MovementSystem::UpdatePositions(entt::registry &registry,
                                     float delta_time) {
  auto view = registry.view<Components::Transform, Components::Velocity,
                            Components::Stats, Components::Team>();

  for (auto entity : view) {
    auto &transform = view.get<Components::Transform>(entity);
    const auto &velocity = view.get<Components::Velocity>(entity);
    const auto &stats = view.get<Components::Stats>(entity);
    const auto &team = view.get<Components::Team>(entity);

    bool blocked =
        HasFrontObstacle(registry, entity, transform, stats, team, velocity);

    if (auto *anim = registry.try_get<Components::Animation>(entity)) {
      if (blocked) {
        anim->state = Components::Animation::State::Attack;
      } else if (std::abs(velocity.x) > 0.01f || std::abs(velocity.y) > 0.01f) {
        anim->state = Components::Animation::State::Walk;
      } else {
        anim->state = Components::Animation::State::Idle;
      }
    }

    if (blocked) {
      continue; // 前方に敵がいるので足を止める
    }

    transform.x += velocity.x * stats.move_speed * delta_time;
    transform.y += velocity.y * stats.move_speed * delta_time;
  }
}

void MovementSystem::HandleBoundaries(entt::registry &registry) const {
  auto view = registry.view<Components::Transform>();

  for (auto entity : view) {
    auto &transform = view.get<Components::Transform>(entity);
    transform.x = std::clamp(transform.x, DEFAULT_MIN_X, DEFAULT_MAX_X);
    transform.y = std::clamp(transform.y, DEFAULT_MIN_Y, DEFAULT_MAX_Y);
  }
}

bool MovementSystem::HasFrontObstacle(
    entt::registry &registry, entt::entity entity,
    const Components::Transform &self_transform,
    const Components::Stats &self_stats,
    const Components::Team &self_team,
    const Components::Velocity &self_vel) const {
  if (std::abs(self_vel.x) < 0.01f) {
    return false;
  }

  const bool moving_left = self_vel.x < 0.0f;

  auto view = registry.view<Components::Transform, Components::Team>();
  for (auto other : view) {
    if (other == entity)
      continue;

    const auto &team = view.get<Components::Team>(other);
    if (team.type == self_team.type)
      continue; // 同陣営は無視

    const auto &tr = view.get<Components::Transform>(other);

    if (std::abs(tr.y - self_transform.y) > LANE_TOLERANCE)
      continue;

    float dx = tr.x - self_transform.x;
    if (moving_left && dx > 0)
      continue;
    if (!moving_left && dx < 0)
      continue;

    const float desired_stop =
        std::max(20.0f, static_cast<float>(self_stats.range) - 10.0f);
    if (std::abs(dx) <= desired_stop) {
      return true;
    }
  }

  return false;
}

} // namespace Game::Systems
