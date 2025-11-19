#pragma once

#include "System.h"
#include <entt/entt.hpp>

namespace Systems {
    // 移動システム - 速度に基づいて位置を更新
    class MovementSystem : public Core::ISystem {
    public:
        void Update(entt::registry& registry, float deltaTime) override;
    };

    // レンダリングシステム - すべてのレンダリング可能なエンティティを描画
    class RenderSystem : public Core::ISystem {
    public:
        void Render(entt::registry& registry) override;
    };

    // 入力システム - プレイヤーの入力を処理
    class InputSystem : public Core::ISystem {
    public:
        void ProcessInput(entt::registry& registry) override;
    };
}
