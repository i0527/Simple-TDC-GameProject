#pragma once

#include "SceneManager.h"
#include "Roguelike/RoguelikeGame.h"
#include <raylib.h>
#include <memory>

namespace Scenes {
    /**
     * @brief Nethack風ゲームシーン
     * RoguelikeGameをISceneとしてラチE�E
     */
    class NethackGameScene : public Core::IScene {
    public:
        NethackGameScene();
        ~NethackGameScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        std::unique_ptr<Roguelike::RoguelikeGame> game_;
        bool isInitialized_;
        std::string originalWindowTitle_;
        int originalScreenWidth_;
        int originalScreenHeight_;
    };
}

