#include "../ECSystemAPI.hpp"

// 標準ライブラリ
#include <algorithm>

namespace game {
namespace core {

entt::entity ECSystemAPI::CreateEntityFromCharacter(
    const entities::Character& character,
    const entities::EntityCreationData& creationData) {
    // エンティティ作成
    entt::entity entity = Create();

    // Positionコンポーネント
    Add<ecs::components::Position>(entity, creationData.position.x, creationData.position.y);

    // Healthコンポーネント
    int maxHp = character.GetTotalHP();
    Add<ecs::components::Health>(entity, maxHp, maxHp);

    // Statsコンポーネント（攻撃力・防御力）
    Add<ecs::components::Stats>(entity, character.GetTotalAttack(), character.GetTotalDefense());

    // Movementコンポーネント
    Add<ecs::components::Movement>(entity, character.move_speed);

    // Combatコンポーネント
    const float attackDuration = std::max(
        0.01f,
        character.attack_sprite.frame_duration *
            static_cast<float>(std::max(1, character.attack_sprite.frame_count)));
    Add<ecs::components::Combat>(entity,
        character.attack_type,
        character.attack_size,
        character.effect_type,
        character.attack_span,
        character.attack_hit_time,
        attackDuration);

    // Spriteコンポーネント（移動スプライトを使用）
    Add<ecs::components::Sprite>(entity,
        character.move_sprite.sheet_path,
        character.move_sprite.frame_width,
        character.move_sprite.frame_height);

    // Animationコンポーネント（移動アニメーション）
    Add<ecs::components::Animation>(entity,
        character.move_sprite.frame_count,
        character.move_sprite.frame_duration,
        ecs::components::AnimationType::Move,
        true);

    // CharacterIdコンポーネント（マスターデータ参照用）
    Add<ecs::components::CharacterId>(entity, character.id);

    LOG_INFO("Created entity from character: {} at ({}, {})",
        character.id,
        creationData.position.x,
        creationData.position.y);

    return entity;
}

entt::entity ECSystemAPI::CreateBattleEntityFromCharacter(
    const entities::Character& character,
    const entities::EntityCreationData& creationData,
    ecs::components::Faction faction,
    const SpawnOverrides* overrides) {
    entt::entity entity = CreateEntityFromCharacter(character, creationData);
    if (entity == entt::null) {
        LOG_WARN("CreateBattleEntityFromCharacter: create failed");
        return entt::null;
    }

    const int baseMaxHp = character.GetTotalHP();
    const int baseAttack = character.GetTotalAttack();
    const int baseDefense = character.GetTotalDefense();

    if (Has<ecs::components::Health>(entity)) {
        auto& hp = Get<ecs::components::Health>(entity);
        const int maxHp = (overrides && overrides->maxHp) ? *overrides->maxHp : baseMaxHp;
        hp.max = maxHp;
        hp.current = maxHp;
    }

    if (Has<ecs::components::Stats>(entity)) {
        auto& stats = Get<ecs::components::Stats>(entity);
        stats.attack = (overrides && overrides->attack) ? *overrides->attack : baseAttack;
        stats.defense = (overrides && overrides->defense) ? *overrides->defense : baseDefense;
    }

    if (Has<ecs::components::Movement>(entity)) {
        auto& move = Get<ecs::components::Movement>(entity);
        move.speed = (overrides && overrides->moveSpeed) ? *overrides->moveSpeed : character.move_speed;
    }

    if (Has<ecs::components::Combat>(entity)) {
        auto& combat = Get<ecs::components::Combat>(entity);
        combat.last_attack_time = -9999.0f;
        combat.is_attacking = false;
        combat.attack_hit_fired = false;
        combat.attack_start_time = 0.0f;
        combat.attack_hit_time = character.attack_hit_time;
        combat.attack_duration = std::max(
            0.01f,
            character.attack_sprite.frame_duration *
                static_cast<float>(std::max(1, character.attack_sprite.frame_count)));
        combat.attack_size = (overrides && overrides->attackSize) ? *overrides->attackSize : character.attack_size;
        combat.attack_span = (overrides && overrides->attackSpan) ? *overrides->attackSpan : character.attack_span;
    }

    if (!Has<ecs::components::Team>(entity)) {
        Add<ecs::components::Team>(entity, faction);
    } else {
        auto& team = Get<ecs::components::Team>(entity);
        team.faction = faction;
    }

    return entity;
}

void ECSystemAPI::QueueDestroy(entt::entity entity) {
    if (!Valid(entity)) {
        return;
    }
    pendingDestroy_.push_back(entity);
}

void ECSystemAPI::FlushDestroyQueue() {
    if (pendingDestroy_.empty()) {
        return;
    }
    for (auto entity : pendingDestroy_) {
        if (Valid(entity)) {
            Destroy(entity);
        }
    }
    pendingDestroy_.clear();
}

size_t ECSystemAPI::DestroyDeadEntities() {
    size_t destroyed = 0;
    auto view = View<ecs::components::Health>();
    for (auto entity : view) {
        const auto& hp = view.get<ecs::components::Health>(entity);
        if (hp.current <= 0) {
            QueueDestroy(entity);
            ++destroyed;
        }
    }
    return destroyed;
}

void ECSystemAPI::ResetForScene() {
    pendingDestroy_.clear();
    Clear();
}

} // namespace core
} // namespace game
