#pragma once

#include "../../config/RenderTypes.hpp"
#include "../entities/Character.hpp"

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief 戦闘コンポーネント
struct Combat {
    entities::AttackType attack_type = entities::AttackType::Single;
    Vector2 attack_size = {0.0f, 0.0f};  // (攻撃有効範囲の距離, キャラ-攻撃有効範囲距離)
    entities::EffectType effect_type = entities::EffectType::Normal;
    float attack_span = 1.0f;  // 攻撃スパン（秒）
    float last_attack_time = 0.0f;  // 最後の攻撃時刻
    bool is_attacking = false;
    float attack_start_time = 0.0f;
    float attack_hit_time = 0.0f;
    float attack_duration = 0.0f;
    bool attack_hit_fired = false;

    Combat() = default;
    Combat(entities::AttackType type, Vector2 size, entities::EffectType effect, float span,
           float hitTime = 0.0f, float duration = 0.0f)
        : attack_type(type),
          attack_size(size),
          effect_type(effect),
          attack_span(span),
          attack_hit_time(hitTime),
          attack_duration(duration) {}

    bool CanAttack(float current_time) const {
        return (current_time - last_attack_time) >= attack_span;
    }
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
