#pragma once

#include "SceneManager.h"
#include "ConfigManager.h"
#include "InputManager.h"
#include <entt/entt.hpp>
#include <raylib.h>
#include <string>
#include <memory>

class Game {
public:
    Game();
    ~Game();

    void Run();

private:
    void Update(float deltaTime);
    void Render();
    void InitializeScenes();
    void LoadConfig();

    entt::registry registry_;
    Core::SceneManager& sceneManager_;
    Core::ConfigManager& configManager_;
    Core::InputManager& inputManager_;
    bool isRunning_;
    int screenWidth_;
    int screenHeight_;
    std::string windowTitle_;
};
