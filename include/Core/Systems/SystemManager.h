#pragma once

#include "Core/Systems/ISystem.h"

#include <vector>
#include <memory>

namespace Core {
    // ゲームシステムのライフサイクルを一元管理
    class SystemManager {
    public:
        SystemManager() = default;
        ~SystemManager() = default;
        
        // システムを登録
        void AddSystem(std::unique_ptr<ISystem> system);
        
        // 登録済みシステムの入力処理
        void ProcessInput(entt::registry& registry);
        
        // 登録済みシステムの更新処理
        void Update(entt::registry& registry, float deltaTime);
        
        // 登録済みシステムの描画処理
        void Render(entt::registry& registry);
        
    private:
        std::vector<std::unique_ptr<ISystem>> systems_;
    };
}

