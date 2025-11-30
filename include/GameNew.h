/**
 * @file GameNew.h
 * @brief 新アーキテクチャによるゲームクラス
 * 
 * データ駆動設計を中心とした新しいゲーム管理クラス。
 * GameContext, World, SystemRunner を統合して使用。
 * Phase 4A: FHD固定レンダリング対応
 */
#pragma once

#include "Core/GameContext.h"
#include "Core/World.h"
#include "Core/SystemRunner.h"
#include "Core/GameRenderer.h"
#include "Core/Events.h"
#include "Core/Definitions.h"
#include "Core/DefinitionRegistry.h"
#include "Core/DefinitionLoader.h"
#include "Core/EntityFactory.h"
#include "Core/SoundManager.h"
#include "Core/EffectManager.h"
#include "TD/Managers/WaveManager.h"
#include "TD/Managers/SpawnManager.h"
#include "TD/Managers/GameStateManager.h"

#include "Core/Platform.h"
#include <string>
#include <memory>
#include <functional>

/**
 * @brief 新アーキテクチャによるゲームクラス
 * 
 * 使用例:
 * @code
 * GameNew game;
 * 
 * // 初期化（ウィンドウ作成前に呼び出し可能な設定）
 * game.SetWindowSize(1280, 720);
 * game.SetWindowTitle("Tower Defense");
 * 
 * // ゲーム開始
 * game.Run();
 * @endcode
 */
class GameNew {
public:
    GameNew();
    ~GameNew();
    
    // コピー禁止
    GameNew(const GameNew&) = delete;
    GameNew& operator=(const GameNew&) = delete;
    
    // ===== 設定 =====
    
    void SetWindowSize(int width, int height) {
        screenWidth_ = width;
        screenHeight_ = height;
    }
    
    void SetWindowTitle(const std::string& title) {
        windowTitle_ = title;
    }
    
    void SetTargetFPS(int fps) {
        targetFPS_ = fps;
    }
    
    void SetDefinitionsPath(const std::string& path) {
        definitionsPath_ = path;
    }
    
    // ===== アクセサ =====
    
    Core::GameContext& Context() { return context_; }
    Core::World& World() { return world_; }
    Core::SystemRunner& Systems() { return systemRunner_; }
    Core::GameRenderer& Renderer() { return renderer_; }
    Data::DefinitionRegistry& Definitions() { return context_.Get<Data::DefinitionRegistry>(); }
    Core::EntityFactory& Factory() { return context_.Get<Core::EntityFactory>(); }
    TD::WaveManager& Waves() { return context_.Get<TD::WaveManager>(); }
    TD::SpawnManager& Spawns() { return context_.Get<TD::SpawnManager>(); }
    TD::GameStateManager& GameState() { return context_.Get<TD::GameStateManager>(); }
    Core::SoundManager& Sounds() { return context_.Get<Core::SoundManager>(); }
    Core::EffectManager& Effects() { return context_.Get<Core::EffectManager>(); }
    
    // ウィンドウサイズ（実際のウィンドウ）
    int ScreenWidth() const { return screenWidth_; }
    int ScreenHeight() const { return screenHeight_; }
    
    // レンダリング解像度（FHD固定: 1920x1080）
    int RenderWidth() const { return Core::GameRenderer::RENDER_WIDTH; }
    int RenderHeight() const { return Core::GameRenderer::RENDER_HEIGHT; }
    
    // マウス座標（FHD空間）
    Vector2 GetMouseWorldPosition() const { return renderer_.GetMouseWorldPosition(); }
    
    bool IsRunning() const { return isRunning_; }
    
    // ===== ゲームループ =====
    
    /**
     * @brief ゲームを開始
     */
    void Run();
    
    /**
     * @brief ゲームを終了
     */
    void Quit() { isRunning_ = false; }
    
    // ===== 拡張ポイント =====
    
    using InitCallback = std::function<void(GameNew&)>;
    using UpdateCallback = std::function<void(GameNew&, float)>;
    using RenderCallback = std::function<void(GameNew&)>;
    using ShutdownCallback = std::function<void(GameNew&)>;
    
    /**
     * @brief 初期化コールバックを設定
     * システム登録やシーン設定などに使用
     */
    void OnInit(InitCallback callback) {
        initCallback_ = std::move(callback);
    }
    
    /**
     * @brief 更新コールバックを設定
     */
    void OnUpdate(UpdateCallback callback) {
        updateCallback_ = std::move(callback);
    }
    
    /**
     * @brief 描画コールバックを設定（システム描画後）
     */
    void OnRender(RenderCallback callback) {
        renderCallback_ = std::move(callback);
    }
    
    /**
     * @brief シャットダウンコールバックを設定
     */
    void OnShutdown(ShutdownCallback callback) {
        shutdownCallback_ = std::move(callback);
    }

private:
    void Initialize();
    void InitializeServices();
    void LoadDefinitions();
    void RegisterCoreSystems();
    void Update(float deltaTime);
    void Render();
    void Shutdown();
    
    // Core層
    Core::GameContext context_;
    Core::World world_;
    Core::SystemRunner systemRunner_;
    Core::GameRenderer renderer_;
    
    // 設定
    int screenWidth_ = 1280;
    int screenHeight_ = 720;
    int targetFPS_ = 60;
    std::string windowTitle_ = "Simple TD Game";
    std::string definitionsPath_ = "assets/definitions";
    
    bool isRunning_ = false;
    bool isInitialized_ = false;
    
    // コールバック
    InitCallback initCallback_;
    UpdateCallback updateCallback_;
    RenderCallback renderCallback_;
    ShutdownCallback shutdownCallback_;
};
