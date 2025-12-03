#pragma once

#include "SceneManager.h"
#include <raylib.h>
#include <vector>
#include <string>

namespace Scenes {
    /**
     * @brief ホ�Eムシーン
     * 3つのゲームを選択できるメニューシーン
     */
    class HomeScene : public Core::IScene {
    public:
        HomeScene();
        ~HomeScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        struct MenuOption {
            std::string label;
            std::string sceneName;
            Rectangle buttonRect;
        };
        
        std::vector<MenuOption> menuOptions_;
        int selectedIndex_;
        float buttonAnimationTime_;
        
        void UpdateInput();
        void RenderMenu();
        bool IsPointInRectangle(Vector2 point, Rectangle rect);
    };
}

