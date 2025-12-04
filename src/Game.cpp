#include "Game.h"
#include "ComponentsNew.h"
// Note: SpriteAnimation, SpriteFrame, SpriteTexture are still in Components namespace (legacy)
#include "Components.h"
#include "Game/Systems/MovementSystem.h"
#include "Game/Systems/AnimationSystem.h"
#include "ResourceManager.h"
#include "Scenes/TitleScene.h"
#include "Scenes/HomeScene.h"
#include "Scenes/TDGameScene.h"
#include "Scenes/TDTestGameScene.h"
#include "Scenes/NethackGameScene.h"
#include <iostream>
#include <memory>

// Sample Scene class
class SampleScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "Sample Scene Initialized" << std::endl;
        
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();
        
        // Load all character JSON automatically
        imageMgr.LoadAllSpriteSheets("assets/json", "assets/atlas");
        
        // Get loaded sprite sheet list
        std::vector<std::string> allSprites = imageMgr.GetAllSpriteSheetNames();
        std::cout << "Loaded " << allSprites.size() << " sprite sheets:" << std::endl;
        for (const auto& name : allSprites) {
            std::cout << "  - " << name << std::endl;
        }
        
        // === cupslime (Arrow Key control) ===
        std::vector<std::string> cupslimeFrames = imageMgr.GetAllFrameNames("cupslime");
        if (!cupslimeFrames.empty()) {
            auto cupslime = registry.create();
            registry.emplace<Core::Components::Position>(cupslime, 300.0f, 300.0f);
            registry.emplace<Core::Components::Velocity>(cupslime, 0.0f, 0.0f);
            registry.emplace<GameComponents::PlayerControlled>(cupslime);
            registry.emplace<Core::Components::Scale>(cupslime, 1.75f, 1.75f);
            
            auto& anim = registry.emplace<Components::SpriteAnimation>(cupslime);
            anim.spriteName = "cupslime";
            anim.frames = cupslimeFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            auto firstFrameInfo = imageMgr.GetFrameInfo(cupslimeFrames[0]);
            registry.emplace<Components::SpriteFrame>(cupslime, 
                Components::SpriteFrame{cupslimeFrames[0], firstFrameInfo.rect});
            registry.emplace<Components::SpriteTexture>(cupslime, 
                Components::SpriteTexture{"cupslime"});
            
            std::cout << "cupslime loaded with " << cupslimeFrames.size() << " frames" << std::endl;
        }
        
        // === yodarehaki (WASD control) ===
        std::vector<std::string> yodarehakiFrames = imageMgr.GetAllFrameNames("yodarehaki");
        if (!yodarehakiFrames.empty()) {
            auto yodarehaki = registry.create();
            registry.emplace<Core::Components::Position>(yodarehaki, 600.0f, 300.0f);
            registry.emplace<Core::Components::Velocity>(yodarehaki, 0.0f, 0.0f);
            registry.emplace<GameComponents::WASDControlled>(yodarehaki);
            registry.emplace<Core::Components::Scale>(yodarehaki, 1.75f, 1.75f);
            
            auto& anim = registry.emplace<Components::SpriteAnimation>(yodarehaki);
            anim.spriteName = "yodarehaki";
            anim.frames = yodarehakiFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            auto firstFrameInfo = imageMgr.GetFrameInfo(yodarehakiFrames[0]);
            registry.emplace<Components::SpriteFrame>(yodarehaki, 
                Components::SpriteFrame{yodarehakiFrames[0], firstFrameInfo.rect});
            registry.emplace<Components::SpriteTexture>(yodarehaki, 
                Components::SpriteTexture{"yodarehaki"});
            
            std::cout << "yodarehaki loaded with " << yodarehakiFrames.size() << " frames" << std::endl;
        }
    }
    
    void Update(entt::registry& registry, float deltaTime) override {
        // Input processing
        Core::InputManager& inputManager = Core::InputManager::GetInstance();
        inputManager.Update();
        
        // Arrow Keys: Control Player (including cupslime)
        auto arrowView = registry.view<Core::Components::Position, Core::Components::Velocity, GameComponents::PlayerControlled>();
        for (auto entity : arrowView) {
            auto& vel = arrowView.get<Core::Components::Velocity>(entity);
            
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (inputManager.IsKeyDown(KEY_RIGHT)) vel.dx = 200.0f;
            if (inputManager.IsKeyDown(KEY_LEFT)) vel.dx = -200.0f;
            if (inputManager.IsKeyDown(KEY_DOWN)) vel.dy = 200.0f;
            if (inputManager.IsKeyDown(KEY_UP)) vel.dy = -200.0f;
        }
        
        // WASD: Control yodarehaki
        auto wasdView = registry.view<Core::Components::Position, Core::Components::Velocity, GameComponents::WASDControlled>();
        for (auto entity : wasdView) {
            auto& vel = wasdView.get<Core::Components::Velocity>(entity);
            
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (IsKeyDown(KEY_D)) vel.dx = 200.0f;   // Right
            if (IsKeyDown(KEY_A)) vel.dx = -200.0f;  // Left
            if (IsKeyDown(KEY_S)) vel.dy = 200.0f;   // Down
            if (IsKeyDown(KEY_W)) vel.dy = -200.0f;  // Up
        }
        
        // Movement update
        Systems::MovementSystem::Update(registry, deltaTime);
        
        // Animation update
        Systems::AnimationSystem::Update(registry, deltaTime);
    }
    
    void Render(entt::registry& registry) override {
        // Sprite rendering (cupslime + yodarehaki)
        Systems::SpriteRenderSystem::Render(registry);
        
        // Helper functions for text rendering
        UI::DrawText(u8"Arrow Keys: Move cupslime (1.75x scale, animated)", {10, 100}, 16, DARKGRAY);
        UI::DrawText(u8"WASD: Move yodarehaki (1.75x scale, animated)", {10, 120}, 16, DARKGRAY);
    }
    
    void Shutdown(entt::registry& registry) override {
        std::cout << "Sample Scene Shutdown" << std::endl;
    }
};

// Forward declaration for TestScene creation function
std::unique_ptr<Core::IScene> CreateTestScene();

Game::Game() 
    : sceneManager_(Core::SceneManager::GetInstance()),
      configManager_(Core::ConfigManager::GetInstance()),
      inputManager_(Core::InputManager::GetInstance()),
      uiManager_(UI::UIManager::GetInstance()),
      isRunning_(true), 
      screenWidth_(800), 
      screenHeight_(600), 
      windowTitle_("Simple TDC Game") {
    
    this->LoadConfig();
    InitWindow(screenWidth_, screenHeight_, windowTitle_.c_str());
    SetTargetFPS(60);
    
    // Initialize UI Manager (Japanese font support)
    // Font path assumes assets/fonts/NotoSansJP-Medium.ttf
    uiManager_.Initialize("assets/fonts/NotoSansJP-Medium.ttf", 18.0f);
    
    this->InitializeScenes();
    
    // On startup, transition to title scene (already set in InitializeScenes())
}

Game::~Game() {
    // Shutdown UI Manager
    uiManager_.Shutdown();
    CloseWindow();
}

void Game::LoadConfig() {
    try {
        configManager_.LoadConfig("assets/config.json");
        
        // Load window settings
        screenWidth_ = configManager_.GetInt("window.width", 800);
        screenHeight_ = configManager_.GetInt("window.height", 600);
        windowTitle_ = configManager_.GetString("window.title", "Simple TDC Game");
        
        std::cout << "Config loaded: " << screenWidth_ << "x" << screenHeight_ << std::endl;
    } catch (const Core::ConfigException& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        // Use default values
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // Use default values
    }
}

void Game::InitializeScenes() {
    // Register scenes
    // Title Scene
    sceneManager_.RegisterScene("title", std::make_unique<Scenes::TitleScene>());
    
    // Home Scene
    sceneManager_.RegisterScene("home", std::make_unique<Scenes::HomeScene>());
    
    // Game Scenes
    sceneManager_.RegisterScene("td_game", std::make_unique<Scenes::TDGameScene>());
    sceneManager_.RegisterScene("td_test", std::make_unique<Scenes::TDTestGameScene>());
    sceneManager_.RegisterScene("nethack", std::make_unique<Scenes::NethackGameScene>());
    
    // Legacy scenes (for backward compatibility)
    sceneManager_.RegisterScene("sample", std::make_unique<SampleScene>());
    sceneManager_.RegisterScene("test", CreateTestScene());
    
    // Set initial scene to title
    sceneManager_.ChangeScene("title");
    
    std::cout << "Scenes initialized" << std::endl;
}

void Game::Run() {
    while (!WindowShouldClose() && isRunning_) {
        float deltaTime = GetFrameTime();
        
        // Scene change processing
        sceneManager_.ProcessSceneChange(registry_);
        
        // Update current scene
        sceneManager_.UpdateCurrentScene(registry_, deltaTime);
        
        // Render
        Render();
        
        // ESC key to exit
        if (inputManager_.IsKeyPressed(KEY_ESCAPE)) {
            isRunning_ = false;
        }
    }
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // === 1. Game World / Scene Rendering ===
    sceneManager_.RenderCurrentScene(registry_);
    
    // === 2. Debug Info Display ===
    UI::DrawText(u8"Simple TDC Game - ESC to Exit", {10, 10}, 20, DARKGRAY);
    DrawFPS(10, 40);
    
    std::string sceneText = "Current Scene: " + sceneManager_.GetCurrentSceneName();
    UI::DrawText(sceneText.c_str(), {10, 70}, 16, DARKGRAY);
    
    // === 3. UIManager Rendering (raygui + ImGui) ===
    uiManager_.DrawSampleUI();
    
    // === 4. ImGui Rendering (single Begin/End for all windows) ===
    uiManager_.BeginImGui();
    uiManager_.DrawDebugWindow(registry_);
    uiManager_.EndImGui();
    
    EndDrawing();
}

void Game::Update(float deltaTime) {
    // Scene change processing
    sceneManager_.ProcessSceneChange(registry_);
    
    // Update current scene
    sceneManager_.UpdateCurrentScene(registry_, deltaTime);
    
    // ESC key to exit
    if (inputManager_.IsKeyPressed(KEY_ESCAPE)) {
        isRunning_ = false;
    }
}
