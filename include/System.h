#pragma once

#include <entt/entt.hpp>

namespace Core {
    // すべてのゲームシステムの抽象インターフェース
    class ISystem {
    public:
        virtual ~ISystem() = default;
        
        // このシステムの入力を処理
        virtual void ProcessInput(entt::registry& registry) {}
        
        // システムロジックを更新
        virtual void Update(entt::registry& registry, float deltaTime) {}
        
        // システムエンティティをレンダリング
        virtual void Render(entt::registry& registry) {}
    };
}
