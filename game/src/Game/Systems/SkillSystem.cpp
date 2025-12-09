#include "Game/Systems/SkillSystem.h"

#include <algorithm>

namespace {
// 暫定のデフォルトクールダウン（全スキル共通の簡易実装）
constexpr float DEFAULT_SKILL_COOLDOWN = 5.0f;
} // namespace

namespace Game::Systems {

void SkillSystem::Update(entt::registry &registry, float delta_time) {
  UpdateCooldowns(registry, delta_time);
  TryCastSkills(registry);
}

void SkillSystem::UpdateCooldowns(entt::registry &registry,
                                  float delta_time) const {
  auto view = registry.view<Components::SkillCooldown>();
  for (auto entity : view) {
    auto &cooldown = view.get<Components::SkillCooldown>(entity);
    if (cooldown.remaining > 0.0f) {
      cooldown.remaining = std::max(0.0f, cooldown.remaining - delta_time);
    }
  }
}

void SkillSystem::TryCastSkills(entt::registry &registry) const {
  auto view =
      registry.view<Components::SkillHolder, Components::SkillCooldown>();

  for (auto entity : view) {
    auto &cooldown = view.get<Components::SkillCooldown>(entity);
    const auto &holder = view.get<Components::SkillHolder>(entity);

    if (holder.skill_ids.empty()) {
      continue;
    }

    if (cooldown.remaining > 0.0f) {
      continue; // クールダウン中
    }

    // TODO: スキル発動条件チェック（マナ/ステート等）
    // TODO: スキル効果適用（バフ/デバフ/範囲攻撃など）

    // 暫定: クールダウンのみ再設定（効果は後続で実装）
    cooldown.remaining = DEFAULT_SKILL_COOLDOWN;
  }
}

} // namespace Game::Systems
