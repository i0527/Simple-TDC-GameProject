#include "SceneManager.h"
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "Components.h"
#include "AnimationSystem.h"
#include <raylib.h>
#include <entt/entt.hpp>
#include <iostream>
#include <vector>

// TestScene: JSONファイルとテクスチャアトラス(スプライトシート)の読み込みテスト
// ・ConfigManager から値を取得してログ出力
// ・ImageManager で Aseprite 形式 JSON + PNG を読み込み
// ・Frame 情報を EnTT コンポーネント（SpriteAnimation, SpriteFrame, SpriteTexture）に統合
// ・AnimationSystem でアニメーションループを管理
// ・InputSystem と MovementSystem でキー入力に応じて移動

class TestScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "TestScene Initialize" << std::endl;

        // Config 値の取得テスト
        Core::ConfigManager& cfg = Core::ConfigManager::GetInstance();
        int w = cfg.GetInt("window.width", 0);
        int h = cfg.GetInt("window.height", 0);
        std::string title = cfg.GetString("window.title", "none");
        std::cout << "Config window.width=" << w << " window.height=" << h << " window.title=" << title << std::endl;

        // スプライトシート読み込みテスト
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();

        // cupslime スプライトシート読み込み
        const std::string spriteJson = "assets/json/cupslime.json";
        const std::string spriteImage = "assets/atlas/cupslime.png";
        imageMgr.LoadSpriteSheet("cupslime", spriteJson, spriteImage);

        // 読み込み済みフレーム名を取得
        std::vector<std::string> frameNames = imageMgr.GetAllFrameNames("cupslime");
        if (frameNames.empty()) {
            std::cout << "No frames loaded for cupslime, using fallback circle entity." << std::endl;
            // フォールバック: 円を描画するエンティティ
            auto entity = registry.create();
            registry.emplace<Components::Position>(entity, 400.f, 300.f);
            registry.emplace<Components::Renderable>(entity, RED, 30.f);
            fallback_ = true;
            return;
        }

        fallback_ = false;
        
        // フレーム情報をログ出力
        std::cout << "Loaded " << frameNames.size() << " frames:" << std::endl;
        for (size_t i = 0; i < frameNames.size(); ++i) {
            auto info = imageMgr.GetFrameInfo(frameNames[i]);
            std::cout << "  Frame " << i << ": " << frameNames[i] 
                     << " [" << info.rect.x << ", " << info.rect.y 
                     << ", " << info.rect.width << ", " << info.rect.height << "]"
                     << " duration: " << info.duration << "ms" << std::endl;
        }

        // アニメーション付きエンティティを生成
        auto entity = registry.create();
        registry.emplace<Components::Position>(entity, 400.f, 300.f);
        
        // Velocity コンポーネント：キー入力で更新される速度
        registry.emplace<Components::Velocity>(entity, 0.0f, 0.0f);
        
        // Player タグ：InputSystem がこれを見てキー入力処理
        registry.emplace<Components::Player>(entity);
        
        // SpriteAnimation コンポーネント：アニメーション制御
        auto& anim = registry.emplace<Components::SpriteAnimation>(entity);
        anim.spriteName = "cupslime";
        anim.frames = frameNames;
        anim.currentFrameIndex = 0;
        anim.elapsedTime = 0.0f;
        anim.isPlaying = true;
        anim.isLooping = true;
        
        // SpriteFrame コンポーネント：フレーム情報
        auto firstFrameInfo = imageMgr.GetFrameInfo(frameNames[0]);
        registry.emplace<Components::SpriteFrame>(entity, 
            Components::SpriteFrame{frameNames[0], firstFrameInfo.rect});
        
        // SpriteTexture コンポーネント：テクスチャ参照
        registry.emplace<Components::SpriteTexture>(entity, 
            Components::SpriteTexture{"cupslime"});
        
        animatedEntity_ = entity;
        
        std::cout << "TestScene: Entity with animation, input, and movement components created." << std::endl;
    }

    void Update(entt::registry& registry, float deltaTime) override {
        if (fallback_) return;
        
        // キー入力処理（テスト用、InputManager を介さない）
        Systems::InputSystem::Update(registry);
        
        // 移動更新
        Systems::MovementSystem::Update(registry, deltaTime);
        
        // アニメーション更新
        Systems::AnimationSystem::Update(registry, deltaTime);
    }

    void Render(entt::registry& registry) override {
        if (fallback_) {
            auto view = registry.view<Components::Position, Components::Renderable>();
            for (auto e : view) {
                auto& pos = view.get<Components::Position>(e);
                auto& rend = view.get<Components::Renderable>(e);
                DrawCircle((int)pos.x, (int)pos.y, rend.radius, rend.color);
            }
            DrawText("Fallback: No sprite sheet found", 10, 120, 16, DARKGRAY);
            return;
        }

        // スプライト描画
        Systems::SpriteRenderSystem::Render(registry);

        // デバッグ情報表示
        if (auto* anim = registry.try_get<Components::SpriteAnimation>(animatedEntity_)) {
            if (!anim->frames.empty()) {
                DrawText(("Frame: " + anim->frames[anim->currentFrameIndex] + 
                         " [" + std::to_string(anim->currentFrameIndex) + "/" + 
                         std::to_string(anim->frames.size()) + "]").c_str(), 10, 100, 16, DARKGRAY);
            }
        }

        if (auto* pos = registry.try_get<Components::Position>(animatedEntity_)) {
            DrawText(("Position: (" + std::to_string((int)pos->x) + ", " + 
                     std::to_string((int)pos->y) + ")").c_str(), 10, 120, 16, DARKGRAY);
        }

        if (auto* vel = registry.try_get<Components::Velocity>(animatedEntity_)) {
            DrawText(("Velocity: (" + std::to_string((int)vel->dx) + ", " + 
                     std::to_string((int)vel->dy) + ")").c_str(), 10, 140, 16, DARKGRAY);
        }
        
        DrawText("TestScene: cupslime with keyboard control (Arrow Keys)", 10, 80, 20, DARKGRAY);
        DrawText("Use Arrow Keys (UP/DOWN/LEFT/RIGHT) to move", 10, 160, 16, DARKGRAY);
    }

    void Shutdown(entt::registry& registry) override {
        std::cout << "TestScene Shutdown" << std::endl;
    }

private:
    bool fallback_{true};
    entt::entity animatedEntity_{entt::null};
};

// シーン登録用ヘルパー関数（Game::InitializeScenes から呼び出し）
std::unique_ptr<Core::IScene> CreateTestScene() {
    return std::make_unique<TestScene>();
}