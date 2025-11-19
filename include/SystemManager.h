#pragma once

#include "System.h"
#include <vector>
#include <memory>

namespace Core {
    // ゲームシステムのライフサイクルと実行を管理
    class SystemManager {
    public:
        SystemManager() = default;
        ~SystemManager() = default;
        
        // マネージャーにシステムを追加
        void AddSystem(std::unique_ptr<ISystem> system);
        
        // すべてのシステムの入力を処理
        void ProcessInput(entt::registry& registry);
        
        // すべてのシステムを更新
        void Update(entt::registry& registry, float deltaTime);
        
        // すべてのシステムを描画
        void Render(entt::registry& registry);
        
    private:
        std::vector<std::unique_ptr<ISystem>> systems_;
    };
}
