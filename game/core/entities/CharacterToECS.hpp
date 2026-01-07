#pragma once

#include "Character.hpp"
#include "EntityCreationData.hpp"
#include "../api/GameModuleAPI.hpp"
#include "../ecs/defineComponents.hpp"
#include "../../utils/Log.h"

namespace game {
namespace core {
namespace entities {
namespace ecs_converter {

/// @brief CharacterデータからECSエンティティを生成
/// 
/// Characterの情報をECSコンポーネントに変換してエンティティを作成します。
/// 
/// @param api GameModuleAPIへの参照
/// @param character キャラクターデータ
/// @param creation_data 生成時のパラメータ
/// @return 作成されたエンティティID
inline entt::entity CreateEntityFromCharacter(
    GameModuleAPI& api,
    const Character& character,
    const EntityCreationData& creation_data
) {
    // エンティティ作成
    entt::entity entity = api.Create();
    
    // Positionコンポーネント
    api.Add<ecs::components::Position>(entity, creation_data.position.x, creation_data.position.y);
    
    // Healthコンポーネント
    int max_hp = character.GetTotalHP();
    api.Add<ecs::components::Health>(entity, max_hp, max_hp);
    
    // Statsコンポーネント（攻撃力・防御力）
    api.Add<ecs::components::Stats>(entity, character.GetTotalAttack(), character.defense);
    
    // Movementコンポーネント
    api.Add<ecs::components::Movement>(entity, character.move_speed);
    
    // Combatコンポーネント
    api.Add<ecs::components::Combat>(entity, 
        character.attack_type,
        character.attack_size,
        character.effect_type,
        character.attack_span
    );
    
    // Spriteコンポーネント（移動スプライトを使用）
    api.Add<ecs::components::Sprite>(entity, 
        character.move_sprite.sheet_path,
        character.move_sprite.frame_width,
        character.move_sprite.frame_height
    );
    
    // Animationコンポーネント（移動アニメーション）
    api.Add<ecs::components::Animation>(entity,
        character.move_sprite.frame_count,
        character.move_sprite.frame_duration,
        ecs::components::AnimationType::Move,
        true  // ループ
    );
    
    // CharacterIdコンポーネント（マスターデータ参照用）
    api.Add<ecs::components::CharacterId>(entity, character.id);
    
    LOG_INFO("Created entity from character: {} at ({}, {})", 
        character.id, 
        creation_data.position.x, 
        creation_data.position.y);
    
    return entity;
}

} // namespace ecs_converter
} // namespace entities
} // namespace core
} // namespace game
