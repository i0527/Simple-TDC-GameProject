#include "Game.h"
#include "Components.h"
#include "AnimationSystem.h"
#include "ResourceManager.h"
#include <iostream>
#include <memory>

// サンプルシーン実装
class SampleScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "Sample Scene Initialized" << std::endl;
        
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();
        
        // ============ cupslime の読み込みと表示 ============
        const std::string cupslimeJson = "assets/json/cupslime.json";
        const std::string cupslimeImage = "assets/atlas/cupslime.png";
        imageMgr.LoadSpriteSheet("cupslime", cupslimeJson, cupslimeImage);
        
        std::vector<std::string> cupslimeFrames = imageMgr.GetAllFrameNames("cupslime");
        
        if (!cupslimeFrames.empty()) {
            // cupslime エンティティ（矢印キー操作・アニメーション付き）
            auto cupslime = registry.create();
            registry.emplace<Components::Position>(cupslime, 300.0f, 300.0f);
            registry.emplace<Components::Velocity>(cupslime, 0.0f, 0.0f);
            registry.emplace<Components::Player>(cupslime);  // 矢印キー操作
            
            // 1.75倍スケーリング
            registry.emplace<Components::Scale>(cupslime, 1.75f, 1.75f);
            
            // アニメーション
            auto& anim = registry.emplace<Components::SpriteAnimation>(cupslime);
            anim.spriteName = "cupslime";
            anim.frames = cupslimeFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            // スプライトフレーム
            auto firstFrameInfo = imageMgr.GetFrameInfo(cupslimeFrames[0]);
            registry.emplace<Components::SpriteFrame>(cupslime, 
                Components::SpriteFrame{cupslimeFrames[0], firstFrameInfo.rect});
            
            // テクスチャ参照
            registry.emplace<Components::SpriteTexture>(cupslime, 
                Components::SpriteTexture{"cupslime"});
            
            std::cout << "cupslime loaded with " << cupslimeFrames.size() << " frames" << std::endl;
        } else {
            std::cout << "Failed to load cupslime sprite sheet" << std::endl;
        }
        
        // ============ yodarehaki の読み込みと表示 ============
        const std::string yodarehakiJson = "assets/json/yodarehaki.json";
        const std::string yodarehakiImage = "assets/atlas/yodarehaki.png";
        imageMgr.LoadSpriteSheet("yodarehaki", yodarehakiJson, yodarehakiImage);
        
        std::vector<std::string> yodarehakiFrames = imageMgr.GetAllFrameNames("yodarehaki");
        
        if (!yodarehakiFrames.empty()) {
            // yodarehaki エンティティ（WASD操作・アニメーション付き）
            auto yodarehaki = registry.create();
            registry.emplace<Components::Position>(yodarehaki, 600.0f, 300.0f);
            registry.emplace<Components::Velocity>(yodarehaki, 0.0f, 0.0f);
            
            // WASDプレイヤータグ
            registry.emplace<Components::WASDPlayer>(yodarehaki);
            
            // 1.75倍スケーリング
            registry.emplace<Components::Scale>(yodarehaki, 1.75f, 1.75f);
            
            // アニメーション
            auto& anim = registry.emplace<Components::SpriteAnimation>(yodarehaki);
            anim.spriteName = "yodarehaki";
            anim.frames = yodarehakiFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            // スプライトフレーム
            auto firstFrameInfo = imageMgr.GetFrameInfo(yodarehakiFrames[0]);
            registry.emplace<Components::SpriteFrame>(yodarehaki, 
                Components::SpriteFrame{yodarehakiFrames[0], firstFrameInfo.rect});
            
            // テクスチャ参照
            registry.emplace<Components::SpriteTexture>(yodarehaki, 
                Components::SpriteTexture{"yodarehaki"});
            
            std::cout << "yodarehaki loaded with " << yodarehakiFrames.size() << " frames" << std::endl;
        } else {
            std::cout << "Failed to load yodarehaki sprite sheet" << std::endl;
        }
    }
    
    void Update(entt::registry& registry, float deltaTime) override {
        // 入力処理
        Core::InputManager& inputManager = Core::InputManager::GetInstance();
        inputManager.Update();
        
        // 矢印キー: Player（cupslimeを含む）を操作
        auto arrowView = registry.view<Components::Position, Components::Velocity, Components::Player>();
        for (auto entity : arrowView) {
            auto& vel = arrowView.get<Components::Velocity>(entity);
            
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (inputManager.IsKeyDown(KEY_RIGHT)) vel.dx = 200.0f;
            if (inputManager.IsKeyDown(KEY_LEFT)) vel.dx = -200.0f;
            if (inputManager.IsKeyDown(KEY_DOWN)) vel.dy = 200.0f;
            if (inputManager.IsKeyDown(KEY_UP)) vel.dy = -200.0f;
        }
        
        // WASD: yodarehaki を操作
        auto wasdView = registry.view<Components::Position, Components::Velocity, Components::WASDPlayer>();
        for (auto entity : wasdView) {
            auto& vel = wasdView.get<Components::Velocity>(entity);
            
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (IsKeyDown(KEY_D)) vel.dx = 200.0f;   // 右
            if (IsKeyDown(KEY_A)) vel.dx = -200.0f;  // 左
            if (IsKeyDown(KEY_S)) vel.dy = 200.0f;   // 下
            if (IsKeyDown(KEY_W)) vel.dy = -200.0f;  // 上
        }
        
        // 移動更新
        Systems::MovementSystem::Update(registry, deltaTime);
        
        // アニメーション更新
        Systems::AnimationSystem::Update(registry, deltaTime);
    }
    
    void Render(entt::registry& registry) override {
        // スプライト描画（cupslime + yodarehaki）
        Systems::SpriteRenderSystem::Render(registry);
        
        // 説明テキスト
        DrawText("Arrow Keys: Move cupslime (1.75x scale, animated)", 10, 100, 16, DARKGRAY);
        DrawText("WASD: Move yodarehaki (1.75x scale, animated)", 10, 120, 16, DARKGRAY);
    }
    
    void Shutdown(entt::registry& registry) override {
        std::cout << "Sample Scene Shutdown" << std::endl;
    }
};

// TestScene生成関数の前方宣言
std::unique_ptr<Core::IScene> CreateTestScene();

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
    
    // 起動時にSampleSceneへ遷移
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
    sceneManager_.RegisterScene("test", CreateTestScene());
    
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
    DrawText("Simple TDC Game - ESC to Exit", 10, 10, 20, DARKGRAY);
    DrawFPS(10, 40);
    DrawText(("Current Scene: " + sceneManager_.GetCurrentSceneName()).c_str(), 10, 70, 16, DARKGRAY);
    
    EndDrawing();
}
