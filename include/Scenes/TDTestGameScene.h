#pragma once

#include "SceneManager.h"
#include "GameNew.h"
#include "TD/UI/GameUI.h"
#include <raylib.h>
#include <memory>

namespace Scenes {
    /**
     * @brief TDテストゲームシーン
     * GameNewをISceneとしてラップ
     */
    class TDTestGameScene : public Core::IScene {
    public:
        TDTestGameScene();
        ~TDTestGameScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        std::unique_ptr<GameNew> game_;
        std::unique_ptr<TD::UI::GameUI> gameUI_;
        bool isInitialized_;
    };
}

