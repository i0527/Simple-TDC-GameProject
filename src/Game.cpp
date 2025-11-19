#include "Game.h"
#include "Components.h"
#include "Systems.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Game::Game() 
    : isRunning_(true), 
      screenWidth_(800), 
      screenHeight_(600), 
      windowTitle_("Simple TDC Game") {
    
    LoadConfig();
    InitWindow(screenWidth_, screenHeight_, windowTitle_.c_str());
    SetTargetFPS(60);
    InitializeSystems();
}

Game::~Game() {
    CloseWindow();
}

void Game::LoadConfig() {
    // Try to load configuration from JSON file
    try {
        std::ifstream configFile("assets/config.json");
        if (configFile.is_open()) {
            json config = json::parse(configFile);
            
            // Load window settings with default fallbacks
            if (config.contains("window")) {
                screenWidth_ = config["window"].value("width", 800);
                screenHeight_ = config["window"].value("height", 600);
                windowTitle_ = config["window"].value("title", "Simple TDC Game");
            }
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        // Continue with default values
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // Continue with default values
    }
}

void Game::InitializeSystems() {
    // Register systems with the SystemManager
    systemManager_.AddSystem(std::make_unique<Systems::InputSystem>());
    systemManager_.AddSystem(std::make_unique<Systems::MovementSystem>());
    systemManager_.AddSystem(std::make_unique<Systems::RenderSystem>());
    
    // Create a sample player entity
    auto player = registry_.create();
    registry_.emplace<Components::Position>(player, screenWidth_ / 2.0f, screenHeight_ / 2.0f);
    registry_.emplace<Components::Velocity>(player, 0.0f, 0.0f);
    registry_.emplace<Components::Renderable>(player, BLUE, 20.0f);
    registry_.emplace<Components::Player>(player);
}

void Game::Run() {
    while (!WindowShouldClose() && isRunning_) {
        float deltaTime = GetFrameTime();
        
        ProcessInput();
        Update(deltaTime);
        Render();
    }
}

void Game::ProcessInput() {
    systemManager_.ProcessInput(registry_);
}

void Game::Update(float deltaTime) {
    systemManager_.Update(registry_, deltaTime);
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    systemManager_.Render(registry_);
    
    DrawText("Simple TDC Game - Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);
    DrawFPS(10, 40);
    
    EndDrawing();
}
