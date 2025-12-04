#pragma once

#include <entt/entt.hpp>

namespace Core {
    // ゲームシステムの共通インターフェース
    class ISystem {
    public:
        virtual ~ISystem() = default;
        
        // 入力処理
        virtual void ProcessInput(entt::registry& registry) {}
        
        // 更新処理
        virtual void Update(entt::registry& registry, float deltaTime) {}
        
        // 描画処理
        virtual void Render(entt::registry& registry) {}
    };
}

