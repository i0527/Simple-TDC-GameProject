#include "Game.h"
#include "Components.h"
#include "AnimationSystem.h"
#include "ResourceManager.h"
#include <iostream>
#include <memory>

// �T���v���V�[������
class SampleScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "Sample Scene Initialized" << std::endl;
        
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();
        
        // ============ cupslime �̓ǂݍ��݂ƕ\�� ============
        const std::string cupslimeJson = "assets/json/cupslime.json";
        const std::string cupslimeImage = "assets/atlas/cupslime.png";
        imageMgr.LoadSpriteSheet("cupslime", cupslimeJson, cupslimeImage);
        
        std::vector<std::string> cupslimeFrames = imageMgr.GetAllFrameNames("cupslime");
        
        if (!cupslimeFrames.empty()) {
            // cupslime �G���e�B�e�B�i���L�[����E�A�j���[�V�����t���j
            auto cupslime = registry.create();
            registry.emplace<Components::Position>(cupslime, 300.0f, 300.0f);
            registry.emplace<Components::Velocity>(cupslime, 0.0f, 0.0f);
            registry.emplace<Components::Player>(cupslime);  // ���L�[����
            
            // 1.75�{�X�P�[�����O
            registry.emplace<Components::Scale>(cupslime, 1.75f, 1.75f);
            
            // �A�j���[�V����
            auto& anim = registry.emplace<Components::SpriteAnimation>(cupslime);
            anim.spriteName = "cupslime";
            anim.frames = cupslimeFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            // �X�v���C�g�t���[��
            auto firstFrameInfo = imageMgr.GetFrameInfo(cupslimeFrames[0]);
            registry.emplace<Components::SpriteFrame>(cupslime, 
                Components::SpriteFrame{cupslimeFrames[0], firstFrameInfo.rect});
            
            // �e�N�X�`���Q��
            registry.emplace<Components::SpriteTexture>(cupslime, 
                Components::SpriteTexture{"cupslime"});
            
            std::cout << "cupslime loaded with " << cupslimeFrames.size() << " frames" << std::endl;
        } else {
            std::cout << "Failed to load cupslime sprite sheet" << std::endl;
        }
        
        // ============ yodarehaki �̓ǂݍ��݂ƕ\�� ============
        const std::string yodarehakiJson = "assets/json/yodarehaki.json";
        const std::string yodarehakiImage = "assets/atlas/yodarehaki.png";
        imageMgr.LoadSpriteSheet("yodarehaki", yodarehakiJson, yodarehakiImage);
        
        std::vector<std::string> yodarehakiFrames = imageMgr.GetAllFrameNames("yodarehaki");
        
        if (!yodarehakiFrames.empty()) {
            // yodarehaki �G���e�B�e�B�iWASD����E�A�j���[�V�����t���j
            auto yodarehaki = registry.create();
            registry.emplace<Components::Position>(yodarehaki, 600.0f, 300.0f);
            registry.emplace<Components::Velocity>(yodarehaki, 0.0f, 0.0f);
            
            // WASD�v���C���[�^�O
            registry.emplace<Components::WASDPlayer>(yodarehaki);
            
            // 1.75�{�X�P�[�����O
            registry.emplace<Components::Scale>(yodarehaki, 1.75f, 1.75f);
            
            // �A�j���[�V����
            auto& anim = registry.emplace<Components::SpriteAnimation>(yodarehaki);
            anim.spriteName = "yodarehaki";
            anim.frames = yodarehakiFrames;
            anim.currentFrameIndex = 0;
            anim.elapsedTime = 0.0f;
            anim.isPlaying = true;
            anim.isLooping = true;
            
            // �X�v���C�g�t���[��
            auto firstFrameInfo = imageMgr.GetFrameInfo(yodarehakiFrames[0]);
            registry.emplace<Components::SpriteFrame>(yodarehaki, 
                Components::SpriteFrame{yodarehakiFrames[0], firstFrameInfo.rect});
            
            // �e�N�X�`���Q��
            registry.emplace<Components::SpriteTexture>(yodarehaki, 
                Components::SpriteTexture{"yodarehaki"});
            
            std::cout << "yodarehaki loaded with " << yodarehakiFrames.size() << " frames" << std::endl;
        } else {
            std::cout << "Failed to load yodarehaki sprite sheet" << std::endl;
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
        // �X�v���C�g�`��icupslime + yodarehaki�j
        Systems::SpriteRenderSystem::Render(registry);
        
        // �����e�L�X�g
        DrawText("Arrow Keys: Move cupslime (1.75x scale, animated)", 10, 100, 16, DARKGRAY);
        DrawText("WASD: Move yodarehaki (1.75x scale, animated)", 10, 120, 16, DARKGRAY);
    }
    
    void Shutdown(entt::registry& registry) override {
        std::cout << "Sample Scene Shutdown" << std::endl;
    }
};

// TestScene作成関数の前方宣言
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
    
    LoadConfig();
    InitWindow(screenWidth_, screenHeight_, windowTitle_.c_str());
    SetTargetFPS(60);
    
    // UIマネージャーの初期化（日本語フォント対応）
    // フォントパスは assets/fonts/NotoSansJP-Medium.otf を想定
    uiManager_.Initialize("assets/fonts/NotoSansJP-Medium.otf", 18.0f);
    
    InitializeScenes();
    
    // 起動時にSampleSceneへ遷移
    sceneManager_.ChangeScene("sample");
}

Game::~Game() {
    // UIマネージャーの終了
    uiManager_.Shutdown();
    CloseWindow();
}

void Game::LoadConfig() {
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

void Game::InitializeScenes() {
    // �T���v���V�[����o�^
    sceneManager_.RegisterScene("sample", std::make_unique<SampleScene>());
    sceneManager_.RegisterScene("test", CreateTestScene());
    
    std::cout << "Scenes initialized" << std::endl;
}

void Game::Run() {
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

void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // === 1. ゲーム世界・シーン描画 ===
    // 現在のシーンを描画
    sceneManager_.RenderCurrentScene(registry_);
    
    // デバッグ情報
    Font defaultFont = uiManager_.GetRayguiFont();
    DrawTextEx(defaultFont, "Simple TDC Game - ESC to Exit", {10, 10}, 20, 1, DARKGRAY);
    DrawFPS(10, 40);
    DrawTextEx(defaultFont, ("Current Scene: " + sceneManager_.GetCurrentSceneName()).c_str(), {10, 70}, 16, 1, DARKGRAY);
    
    // === 2. UIManager描画（raygui + ImGui）===
    // サンプルUIを描画（raygui HUD + ImGui デバッグウィンドウ）
    uiManager_.DrawSampleUI();
    
    EndDrawing();
}
