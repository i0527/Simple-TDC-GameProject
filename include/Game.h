#pragma once

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
    void ProcessInput();
    void InitializeSystems();
    void LoadConfig();

    entt::registry registry_;
    bool isRunning_;
    int screenWidth_;
    int screenHeight_;
    std::string windowTitle_;
};
