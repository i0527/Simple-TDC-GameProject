#include "Game.h"
#include "Components.h"
#include <iostream>
#include <memory>

// サンプルシーン実装
class SampleScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "Sample Scene Initialized" << std::endl;
        
        // サンプルプレイヤーエンティティを作成
        auto player = registry.create();
        registry.emplace<Components::Position>(player, 400.0f, 300.0f);
        registry.emplace<Components::Velocity>(player, 0.0f, 0.0f);
        registry.emplace<Components::Renderable>(player, BLUE, 20.0f);
        registry.emplace<Components::Player>(player);
    }
    
    void Update(entt::registry& registry, float deltaTime) override {
        // 入力処理
        Core::InputManager& inputManager = Core::InputManager::GetInstance();
        inputManager.Update();
        
        // プレイヤー移動ロジック
        auto view = registry.view<Components::Position, Components::Velocity, Components::Player>();
        for (auto entity : view) {
            auto& pos = view.get<Components::Position>(entity);
            auto& vel = view.get<Components::Velocity>(entity);
            
            // キー入力で速度を設定
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (inputManager.IsKeyDown(KEY_RIGHT)) vel.dx = 200.0f;
            if (inputManager.IsKeyDown(KEY_LEFT)) vel.dx = -200.0f;
            if (inputManager.IsKeyDown(KEY_DOWN)) vel.dy = 200.0f;
            if (inputManager.IsKeyDown(KEY_UP)) vel.dy = -200.0f;
            
            // 位置を更新
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
        }
    }
    
    void Render(entt::registry& registry) override {
        // エンティティを描画
        auto view = registry.view<Components::Position, Components::Renderable>();
        for (auto entity : view) {
            auto& pos = view.get<Components::Position>(entity);
            auto& renderable = view.get<Components::Renderable>(entity);
            
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), renderable.radius, renderable.color);
        }
    }
    
    void Shutdown(entt::registry& registry) override {
        std::cout << "Sample Scene Shutdown" << std::endl;
    }
};

Game::Game() 
    : sceneManager_(Core::SceneManager::GetInstance()),
      configManager_(Core::ConfigManager::GetInstance()),
      inputManager_(Core::InputManager::GetInstance()),
      isRunning_(true), 
      screenWidth_(800), 
      screenHeight_(600), 
      windowTitle_("Simple TDC Game") {
    
    LoadConfig();
    InitWindow(screenWidth_, screenHeight_, windowTitle_.c_str());
    SetTargetFPS(60);
    InitializeScenes();
    
    // 最初のシーンに変更
    sceneManager_.ChangeScene("sample");
}

Game::~Game() {
    CloseWindow();
}

void Game::LoadConfig() {
    try {
        configManager_.LoadConfig("assets/config.json");
        
        // ウィンドウ設定の読み込み
        screenWidth_ = configManager_.GetInt("window.width", 800);
        screenHeight_ = configManager_.GetInt("window.height", 600);
        windowTitle_ = configManager_.GetString("window.title", "Simple TDC Game");
        
        std::cout << "Config loaded: " << screenWidth_ << "x" << screenHeight_ << std::endl;
    } catch (const Core::ConfigException& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        // デフォルト値を使用
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // デフォルト値を使用
    }
}

void Game::InitializeScenes() {
    // サンプルシーンを登録
    sceneManager_.RegisterScene("sample", std::make_unique<SampleScene>());
    
    std::cout << "Scenes initialized" << std::endl;
}

void Game::Run() {
    while (!WindowShouldClose() && isRunning_) {
        float deltaTime = GetFrameTime();
        
        // シーン遷移処理
        sceneManager_.ProcessSceneChange(registry_);
        
        // 現在のシーンを更新
        sceneManager_.UpdateCurrentScene(registry_, deltaTime);
        
        // 描画
        Render();
        
        // ESC キーで終了
        if (inputManager_.IsKeyPressed(KEY_ESCAPE)) {
            isRunning_ = false;
        }
    }
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // 現在のシーンを描画
    sceneManager_.RenderCurrentScene(registry_);
    
    // デバッグ情報
    DrawText("Simple TDC Game - Use Arrow Keys to Move, ESC to Exit", 10, 10, 20, DARKGRAY);
    DrawFPS(10, 40);
    DrawText(("Current Scene: " + sceneManager_.GetCurrentSceneName()).c_str(), 10, 70, 16, DARKGRAY);
    
    EndDrawing();
}
