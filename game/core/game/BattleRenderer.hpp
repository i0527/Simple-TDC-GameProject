#pragma once

// 標準ライブラリ
#include <string>

// 外部ライブラリ
#include <entt/entt.hpp>

// プロジェクト内
#include "../api/BaseSystemAPI.hpp"
#include "../ecs/defineComponents.hpp"

namespace game {
namespace core {
namespace game {

/// @brief ECS上の Sprite/Animation/Position/Team を最小限描画するレンダラ
class BattleRenderer {
public:
    explicit BattleRenderer(BaseSystemAPI* systemAPI);
    ~BattleRenderer() = default;

    void UpdateAnimations(entt::registry& registry, float deltaTime);
    void RenderEntities(entt::registry& registry);

private:
    BaseSystemAPI* systemAPI_;

    void RenderEntity(const ecs::components::Position& pos,
                      const ecs::components::Sprite& sprite,
                      const ecs::components::Animation* anim,
                      const ecs::components::Team* team);

    static Rectangle MakeSourceRect(const ecs::components::Sprite& sprite,
                                    const ecs::components::Animation* anim,
                                    bool flipHorizontally);
};

} // namespace game
} // namespace core
} // namespace game

