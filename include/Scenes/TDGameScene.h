#pragma once

#include "SceneManager.h"
#include <raylib.h>

namespace Scenes {
    /**
     * @brief TDゲーム本体シーン
     * Gameクラスの既存シーン（sample、test）を使用
     * このシーンは既存のGameクラスのシーン管理システムを使用するため、
     * 特別な実装は不要
     */
    class TDGameScene : public Core::IScene {
    public:
        TDGameScene();
        ~TDGameScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        bool isInitialized_;
        std::string targetSceneName_;  // "sample" または "test"
    };
}

