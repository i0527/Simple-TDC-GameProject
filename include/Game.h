#pragma once

#include "SceneManager.h"
#include "ConfigManager.h"
#include "InputManager.h"
#include "UIManager.h"
#include <entt/entt.hpp>
#include <raylib.h>
#include <string>
#include <memory>

class Game {
public:
    Game();
    ~Game();

    void Run();
    
    // ===== シーン統合用のメソッド =====
    
    /**
     * @brief ゲームを更新（シーン統合用）
     * @param deltaTime フレーム時間
     */
    void Update(float deltaTime);
    
    /**
     * @brief ゲームを描画（シーン統合用）
     */
    void Render();

private:
    void InitializeScenes();
    void LoadConfig();

    entt::registry registry_;
    Core::SceneManager& sceneManager_;
    Core::ConfigManager& configManager_;
    Core::InputManager& inputManager_;
    UI::UIManager& uiManager_;
    bool isRunning_;
    int screenWidth_;
    int screenHeight_;
    std::string windowTitle_;
};
