#include "Game.h"
#include "Components.h"
#include "AnimationSystem.h"
#include "ResourceManager.h"
#include "Scenes/TitleScene.h"
#include "Scenes/HomeScene.h"
#include "Scenes/TDGameScene.h"
#include "Scenes/TDTestGameScene.h"
#include "Scenes/NethackGameScene.h"
#include <iostream>
#include <memory>

// �T���v���V�[������
class SampleScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "Sample Scene Initialized" << std::endl;
        
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();
        
        // === すべてのキャラクターJSONを自動読み込み ===
        imageMgr.LoadAllSpriteSheets("assets/json", "assets/atlas");
        
        // ロードされたスプライトシート一覧を取得
        std::vector<std::string> allSprites = imageMgr.GetAllSpriteSheetNames();
        std::cout << "Loaded " << allSprites.size() << " sprite sheets:" << std::endl;
        for (const auto& name : allSprites) {
            std::cout << "  - " << name << std::endl;
        }
        
        // === cupslime (矢印キー操作) ===
        std::vector<std::string> cupslimeFrames = imageMgr.GetAllFrameNames("cupslime");
        if (!cupslimeFrames.empty()) {
            auto cupslime = registry.create();
            registry.emplace<Components::Position>(cupslime, 300.0f, 300.0f);
            registry.emplace<Components::Velocity>(cupslime, 0.0f, 0.0f);
            registry.emplace<Components::Player>(cupslime);
            registry.emplace<Components::Scale>(cupslime, 1.75f, 1.75f);
            
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
        
        // === yodarehaki (WASD操作) ===
        std::vector<std::string> yodarehakiFrames = imageMgr.GetAllFrameNames("yodarehaki");
        if (!yodarehakiFrames.empty()) {
            auto yodarehaki = registry.create();
            registry.emplace<Components::Position>(yodarehaki, 600.0f, 300.0f);
            registry.emplace<Components::Velocity>(yodarehaki, 0.0f, 0.0f);
            registry.emplace<Components::WASDPlayer>(yodarehaki);
            registry.emplace<Components::Scale>(yodarehaki, 1.75f, 1.75f);
            
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
        // ���͏���
        Core::InputManager& inputManager = Core::InputManager::GetInstance();
        inputManager.Update();
        
        // ���L�[: Player�icupslime���܂ށj�𑀍�
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
        
        // WASD: yodarehaki �𑀍�
        auto wasdView = registry.view<Components::Position, Components::Velocity, Components::WASDPlayer>();
        for (auto entity : wasdView) {
            auto& vel = wasdView.get<Components::Velocity>(entity);
            
            vel.dx = 0.0f;
            vel.dy = 0.0f;
            
            if (IsKeyDown(KEY_D)) vel.dx = 200.0f;   // �E
            if (IsKeyDown(KEY_A)) vel.dx = -200.0f;  // ��
            if (IsKeyDown(KEY_S)) vel.dy = 200.0f;   // ��
            if (IsKeyDown(KEY_W)) vel.dy = -200.0f;  // ��
        }
        
        // �ړ��X�V
        Systems::MovementSystem::Update(registry, deltaTime);
        
        // �A�j���[�V�����X�V
        Systems::AnimationSystem::Update(registry, deltaTime);
    }
    
    void Render(entt::registry& registry) override {
        // スプライト描画(cupslime + yodarehaki)
        Systems::SpriteRenderSystem::Render(registry);
        
        // 効率化：ヘルパー関数を使用
        UI::DrawText(u8"Arrow Keys: Move cupslime (1.75x scale, animated)", {10, 100}, 16, DARKGRAY);
        UI::DrawText(u8"WASD: Move yodarehaki (1.75x scale, animated)", {10, 120}, 16, DARKGRAY);
    }
    
    void Shutdown(entt::registry& registry) override {
        std::cout << "Sample Scene Shutdown" << std::endl;
    }
};

// TestScene作成関数の前方宣言
std::unique_ptr<Core::IScene> CreateTestScene();

::Game::Game() 
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
    
    // UIマネージャーの初期化（日本語フォント対応）
    // フォントパスは assets/fonts/NotoSansJP-Medium.ttf を想定
    uiManager_.Initialize("assets/fonts/NotoSansJP-Medium.ttf", 18.0f);
    
    this->InitializeScenes();
    
    // 起動時はタイトルシーンへ遷移（InitializeScenes()内で設定済み）
}

::Game::~Game() {
    // UIマネージャーの終了
    uiManager_.Shutdown();
    CloseWindow();
}

void ::Game::LoadConfig() {
    try {
        configManager_.LoadConfig("assets/config.json");
        
        // �E�B���h�E�ݒ�̓ǂݍ���
        screenWidth_ = configManager_.GetInt("window.width", 800);
        screenHeight_ = configManager_.GetInt("window.height", 600);
        windowTitle_ = configManager_.GetString("window.title", "Simple TDC Game");
        
        std::cout << "Config loaded: " << screenWidth_ << "x" << screenHeight_ << std::endl;
    } catch (const Core::ConfigException& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        // �f�t�H���g�l���g�p
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // �f�t�H���g�l���g�p
    }
}

void ::Game::InitializeScenes() {
    // �T���v���V�[����o�^
    // タイトルシーン
    sceneManager_.RegisterScene("title", std::make_unique<Scenes::TitleScene>());
    
    // ホームシーン
    sceneManager_.RegisterScene("home", std::make_unique<Scenes::HomeScene>());
    
    // ゲームシーン
    sceneManager_.RegisterScene("td_game", std::make_unique<Scenes::TDGameScene>());
    sceneManager_.RegisterScene("td_test", std::make_unique<Scenes::TDTestGameScene>());
    sceneManager_.RegisterScene("nethack", std::make_unique<Scenes::NethackGameScene>());
    
    // 既存のシーン（後方互換性のため）
    sceneManager_.RegisterScene("sample", std::make_unique<SampleScene>());
    sceneManager_.RegisterScene("test", CreateTestScene());
    
    // 初期シーンをタイトルに設定
    sceneManager_.ChangeScene("title");
    
    std::cout << "Scenes initialized" << std::endl;
}

void ::Game::Run() {
    while (!WindowShouldClose() && isRunning_) {
        float deltaTime = GetFrameTime();
        
        // �V�[���J�ڏ���
        sceneManager_.ProcessSceneChange(registry_);
        
        // ���݂̃V�[�����X�V
        sceneManager_.UpdateCurrentScene(registry_, deltaTime);
        
        // �`��
        Render();
        
        // ESC �L�[�ŏI��
        if (inputManager_.IsKeyPressed(KEY_ESCAPE)) {
            isRunning_ = false;
        }
    }
}

void ::Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // === 1. ゲーム世界・シーン描画 ===
    sceneManager_.RenderCurrentScene(registry_);
    
    // === 2. デバッグ情報表示（効率化版） ===
    UI::DrawText(u8"Simple TDC Game - ESC to Exit", {10, 10}, 20, DARKGRAY);
    DrawFPS(10, 40);
    
    std::string sceneText = "Current Scene: " + sceneManager_.GetCurrentSceneName();
    UI::DrawText(sceneText.c_str(), {10, 70}, 16, DARKGRAY);
    
    // === 3. UIManager描画（raygui + ImGui） ===
    uiManager_.DrawSampleUI();
    
    // === 4. ImGui描画（1回のBegin/Endで全ウィンドウ） ===
    uiManager_.BeginImGui();
    uiManager_.DrawDebugWindow(registry_);
    uiManager_.EndImGui();
    
    EndDrawing();
}

void ::Game::Update(float deltaTime) {
    // シーン変更処理
    sceneManager_.ProcessSceneChange(registry_);
    
    // 現在のシーンを更新
    sceneManager_.UpdateCurrentScene(registry_, deltaTime);
    
    // ESC キーで終了
    if (inputManager_.IsKeyPressed(KEY_ESCAPE)) {
        isRunning_ = false;
    }
}
