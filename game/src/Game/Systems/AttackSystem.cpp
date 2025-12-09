#include "Game/Systems/AttackSystem.h"

#include <cmath>
#include <limits>

namespace Game::Systems {

void AttackSystem::Update(entt::registry &registry, float delta_time) {
  auto view = registry.view<Components::Transform, Components::Stats,
                            Components::Team, Components::AttackCooldown>();

  for (auto entity : view) {
    auto &cooldown = view.get<Components::AttackCooldown>(entity);
    const auto &stats = view.get<Components::Stats>(entity);
    const auto &transform = view.get<Components::Transform>(entity);

    // クールダウン更新
    if (cooldown.remaining > 0.0f) {
      cooldown.remaining = std::max(0.0f, cooldown.remaining - delta_time);
      continue;
    }

    // 攻撃対象探索
    entt::entity target = FindTarget(registry, entity, transform,
                                     static_cast<float>(stats.range));
    if (target == entt::null) {
      continue;
    }

    // ダメージ適用
    ApplyDamage(registry, entity, target);

    // クールダウン再設定（攻撃速度=1秒あたり攻撃回数）
    const float attack_rate = std::max(0.001f, stats.attack_speed);
    cooldown.remaining = 1.0f / attack_rate;
  }
}

entt::entity
AttackSystem::FindTarget(entt::registry &registry, entt::entity attacker,
                         const Components::Transform &attacker_transform,
                         float range) const {
  auto view = registry.view<Components::Transform, Components::Team>();
  const auto attacker_team = view.get<Components::Team>(attacker).type;
  const float range_sq = range * range;
  // TODO: 優先順位切替（最前線/最弱/最強）を実装する場合はここで分岐
  float preferred_direction = 0.0f;
  if (auto *vel = registry.try_get<Components::Velocity>(attacker)) {
    preferred_direction = vel->x;
  }

  entt::entity best = entt::null;
  float best_dist_sq = std::numeric_limits<float>::max();

  for (auto entity : view) {
    if (entity == attacker) {
      continue;
    }
    const auto &team = view.get<Components::Team>(entity);
    if (team.type == attacker_team) {
      continue; // 同じ陣営はスキップ
    }

    const auto &transform = view.get<Components::Transform>(entity);
    const float dx = transform.x - attacker_transform.x;
    const float dy = transform.y - attacker_transform.y;
    const float dist_sq = dx * dx + dy * dy;

    if (preferred_direction < -0.1f && dx > 0) {
      continue;
    }
    if (preferred_direction > 0.1f && dx < 0) {
      continue;
    }

    if (dist_sq <= range_sq && dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      best = entity;
    }
  }

  return best;
}

void AttackSystem::ApplyDamage(entt::registry &registry, entt::entity attacker,
                               entt::entity target) const {
  auto &target_stats = registry.get<Components::Stats>(target);
  const auto &attacker_stats = registry.get<Components::Stats>(attacker);

  target_stats.current_hp -= attacker_stats.attack;
  if (auto *anim = registry.try_get<Components::Animation>(attacker)) {
    anim->state = Components::Animation::State::Attack;
    anim->current_frame = 0;
    anim->frame_timer = 0.0f;
  }
  if (auto *hit = registry.try_get<Components::HitEffect>(target)) {
    hit->timer = 0.2f;
  } else {
    registry.emplace<Components::HitEffect>(target, 0.2f);
  }

  // ダメージポップ
  int dmg = attacker_stats.attack;
  if (auto *popup = registry.try_get<Components::DamagePopup>(target)) {
    popup->value = dmg;
    popup->timer = 0.0f;
    popup->duration = 0.8f;
    popup->offset = {0.0f, -20.0f};
  } else {
    Components::DamagePopup new_popup{};
    new_popup.value = dmg;
    new_popup.timer = 0.0f;
    new_popup.duration = 0.8f;
    new_popup.offset = {0.0f, -20.0f};
    registry.emplace<Components::DamagePopup>(target, new_popup);
  }

  if (target_stats.current_hp <= 0) {
    target_stats.current_hp = 0;
    if (!registry.any_of<Components::Dead>(target)) {
      registry.emplace<Components::Dead>(target);
    }
    if (auto *anim_target = registry.try_get<Components::Animation>(target)) {
      anim_target->state = Components::Animation::State::Death;
      anim_target->current_frame = 0;
      anim_target->frame_timer = 0.0f;
      anim_target->playing = true;
    }
  }
}

} // namespace Game::Systems
