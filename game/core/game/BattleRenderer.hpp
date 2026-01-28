#pragma once

// 標準ライブラリ
#include <string>

// プロジェクト�E
#include "../api/BaseSystemAPI.hpp"
#include "../api/ECSystemAPI.hpp"
#include "../ecs/defineComponents.hpp"

namespace game {
namespace core {
namespace game {

/// @brief ECS上�E Sprite/Animation/Position/Team を最小限描画するレンダラ
class BattleRenderer {
public:
    BattleRenderer(BaseSystemAPI* systemAPI, ECSystemAPI* ecsAPI);
    ~BattleRenderer() = default;

    void SetEcsAPI(ECSystemAPI* ecsAPI) { ecsAPI_ = ecsAPI; }
    void UpdateAnimations(ECSystemAPI* ecsAPI, float deltaTime);
    void RenderEntities(ECSystemAPI* ecsAPI);

private:
    BaseSystemAPI* systemAPI_;
    ECSystemAPI* ecsAPI_;

    void RenderEntity(const ecs::components::Position& pos,
                      const ecs::components::Sprite& sprite,
                      const ecs::components::Animation* anim,
                      const ecs::components::Team* team);

    /// ???????????? or ??????????1??????????????
    static Rectangle MakeSourceRect(const ecs::components::Sprite& sprite,
                                    const ecs::components::Animation* anim,
                                    int sheetWidth,
                                    int sheetHeight,
                                    bool flipHorizontally);
};

} // namespace game
} // namespace core
} // namespace game

