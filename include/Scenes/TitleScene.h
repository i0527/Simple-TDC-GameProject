#pragma once

#include "SceneManager.h"
#include <raylib.h>

namespace Scenes {
    /**
     * @brief タイトルシーン
     * ゲームタイトルを表示し、任意のキー入力または一定時間でホームシーンへ遷移
     */
    class TitleScene : public Core::IScene {
    public:
        TitleScene();
        ~TitleScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        float elapsedTime_;
        float fadeAlpha_;
        bool shouldTransition_;
    };
}

