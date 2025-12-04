/**
 * @file UnifiedGame.cpp
 * @brief 統合ゲームプラットフォーム実装
 * 
 * TD/Roguelike統合プラットフォームの中核実装。
 * ゲームモードの切り替えと統合実行を管理する。
 */

#include "Core/Platform.h"
#include "Application/UnifiedGame.h"
#include "Application/HomeScene.h"
#include "Core/GameRenderer.h"
#include "Core/EntityFactory.h"
#include "Core/SoundManager.h"
#include "Core/EffectManager.h"
#include "Core/HTTPServer.h"
#include "Core/Components/CoreComponents.h"
#include "Game/Components/GameComponents.h"
#include "Domain/TD/Components/TDComponents.h"
#include "Domain/TD/Systems/TDSystems.h"
#include "Domain/TD/Managers/WaveManager.h"
#include "Domain/TD/Managers/SpawnManager.h"
#include "Domain/TD/Managers/GameStateManager.h"
#include "Domain/Roguelike/Components/RoguelikeComponents.h"
#include "Domain/Roguelike/Systems/InputSystem.h"
#include "Domain/Roguelike/Systems/FOVSystem.h"
#include "Domain/Roguelike/Managers/TurnManager.h"
#include "Data/Loaders/CharacterLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/UILoader.h"
#include "Data/Loaders/MapLoader.h"
#include <nlohmann/json.hpp>

#include <iostream>
#include <chrono>

namespace Application {

// ===== 基本システム（静的関数として定義）=====

namespace CoreSystems {

/**
 * @brief ライフタイムシステム
 */
void LifetimeSystem(Core::World& world, Core::GameContext& context, float dt) {
    auto view = world.View<Core::Components::Lifetime>();
    
    for (auto entity : view) {
        auto& lifetime = view.get<Core::Components::Lifetime>(entity);
        lifetime.remaining -= dt;
        
        if (lifetime.remaining <= 0.0f) {
            world.MarkForDestruction(entity);
        }
    }
}

/**
 * @brief 破棄システム
 */
void DestructionSystem(Core::World& world, Core::GameContext& context, float dt) {
    auto view = world.View<Core::Components::MarkedForDestruction>();
    for (auto entity : view) {
        world.MarkForDestruction(entity);
    }
    
    world.FlushDestruction();
}

} // namespace CoreSystems

// ===== UnifiedGame 実装 =====

UnifiedGame::UnifiedGame()
    : world_(std::make_unique<Core::World>())
    , context_(std::make_unique<Core::GameContext>())
    , systemRunner_(std::make_unique<Core::SystemRunner>())
    , definitions_(std::make_unique<Data::DefinitionRegistry>())
    , definitionLoader_(std::make_unique<Data::DefinitionLoader>())
{
}

bool UnifiedGame::Initialize(
    const std::string& definitionsPath,
    bool enableHTTPServer,
    int httpServerPort
) {
    if (initialized_) {
        std::cerr << "UnifiedGame: Already initialized\n";
        return false;
    }
    
    // Raylib初期化
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr int TARGET_FPS = 60;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple TD Game - Unified Platform");
    SetTargetFPS(TARGET_FPS);
    
    // オーディオデバイス初期化
    InitAudioDevice();
    
    // レンダラー初期化
    auto& renderer = context_->Emplace<Core::GameRenderer>();
    renderer.Initialize(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // データ定義読み込み
    try {
        definitionLoader_->LoadAllCharacters(definitionsPath + "/characters");
        definitionLoader_->LoadAllStages(definitionsPath + "/stages");
        definitionLoader_->LoadAllUILayouts(definitionsPath + "/ui");
        definitionLoader_->LoadAllMaps(definitionsPath + "/maps");
        
        // 定義をレジストリに登録
        auto& loader = *definitionLoader_;
        // TODO: ローダーからレジストリへの統合
        
        std::cout << "Definitions loaded from: " << definitionsPath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to load definitions: " << e.what() << "\n";
        // エラーでも続行（デフォルト定義を使用）
    }
    
    // レジストリをコンテキストに登録（コピーで登録）
    context_->Register<Data::DefinitionRegistry>(*definitions_);
    
    // EntityFactory登録（コンテキストからレジストリを取得）
    context_->Register<Core::EntityFactory>(*world_, context_->Get<Data::DefinitionRegistry>());
    
    // SoundManager登録
    auto& soundManager = context_->Emplace<Core::SoundManager>();
    soundManager.SetRegistry(&context_->Get<Data::DefinitionRegistry>());
    
    // EffectManager登録
    auto& effectManager = context_->Emplace<Core::EffectManager>();
    effectManager.SetRegistry(&context_->Get<Data::DefinitionRegistry>());
    
    // HTTPサーバー初期化（オプション）
    enableHTTPServer_ = enableHTTPServer;
    httpServerPort_ = httpServerPort;
    if (enableHTTPServer_) {
        httpServer_ = std::make_unique<Core::HTTPServer>();
        if (httpServer_->Initialize(
            httpServerPort_,
            definitionsPath,
            definitions_.get(),
            definitionLoader_.get()
        )) {
            // 開発モードを有効化（ローカル開発用）
            httpServer_->SetDevelopmentMode(true);
            
            // ファイル変更時のコールバックを設定（ホットリロード）
            httpServer_->SetFileChangedCallback([this](const std::string& filePath) {
                this->OnFileChanged(filePath);
            });
            
            // ゲーム状態取得のコールバックを設定（デバッグ/プレビュー用）
            httpServer_->SetGameStateCallback([this]() {
                return this->GetGameState();
            });
            
            httpServer_->Start();
            std::cout << "HTTP Server started on port " << httpServerPort_ << "\n";
            std::cout << "API available at http://localhost:" << httpServerPort_ << "/api\n";
            std::cout << "WebSocket available at ws://localhost:" << httpServerPort_ << "/ws\n";
            std::cout << "Development mode: ENABLED (security restrictions relaxed)\n";
        } else {
            std::cerr << "Warning: Failed to initialize HTTP server\n";
            httpServer_.reset();
            enableHTTPServer_ = false;
        }
    }
    
    // 基本システム登録
    systemRunner_->Register(Core::SystemPhase::Update, "Lifetime", 
                           CoreSystems::LifetimeSystem, 0);
    systemRunner_->Register(Core::SystemPhase::PostUpdate, "Destruction", 
                           CoreSystems::DestructionSystem, 100);
    
    // シーンを登録
    RegisterScene("Home", std::make_unique<HomeScene>());
    
    // ホームシーンにコールバックを設定
    auto* homeScene = dynamic_cast<HomeScene*>(scenes_["Home"].get());
    if (homeScene) {
        homeScene->SetGameModeCallback([this](GameMode mode) {
            this->SetGameMode(mode);
        });
    }
    
    // 初期シーンを設定
    currentSceneName_ = "Home";
    ChangeScene("Home");
    
    initialized_ = true;
    running_ = false;
    
    std::cout << "UnifiedGame initialized successfully\n";
    return true;
}

void UnifiedGame::SetGameMode(GameMode mode) {
    if (currentMode_ == mode) {
        return;
    }
    
    // 現在のモードをクリア
    ClearCurrentSystems();
    
    // 新しいモードを設定
    currentMode_ = mode;
    
    // 新しいモードを初期化
    InitializeGameMode(mode);
}

void UnifiedGame::InitializeGameMode(GameMode mode) {
    switch (mode) {
        case GameMode::Menu:
            RegisterMenuSystems();
            break;
        case GameMode::TD:
            RegisterTDSystems();
            break;
        case GameMode::Roguelike:
            RegisterRoguelikeSystems();
            break;
    }
    
    std::cout << "Game mode initialized: " << GameModeToString(mode) << "\n";
}

void UnifiedGame::ClearCurrentSystems() {
    // すべてのシステムをクリア
    systemRunner_ = std::make_unique<Core::SystemRunner>();
    
    // 基本システムを再登録
    systemRunner_->Register(Core::SystemPhase::Update, "Lifetime", 
                           CoreSystems::LifetimeSystem, 0);
    systemRunner_->Register(Core::SystemPhase::PostUpdate, "Destruction", 
                           CoreSystems::DestructionSystem, 100);
}

void UnifiedGame::RegisterTDSystems() {
    // TDマネージャーを登録
    auto& waveManager = context_->Emplace<Domain::TD::Managers::WaveManager>();
    auto& spawnManager = context_->Emplace<Domain::TD::Managers::SpawnManager>();
    auto& gameStateManager = context_->Emplace<Domain::TD::Managers::GameStateManager>();
    
    // TDシステムラッパー
    systemRunner_->Register(Core::SystemPhase::Update, "WaveManager",
        [&waveManager](Core::World& world, Core::GameContext& ctx, float dt) {
            waveManager.Update(world, ctx, dt);
        }, 50);
    
    systemRunner_->Register(Core::SystemPhase::Update, "SpawnManager",
        [&spawnManager](Core::World& world, Core::GameContext& ctx, float dt) {
            spawnManager.Update(world, dt);
        }, 51);
    
    systemRunner_->Register(Core::SystemPhase::Update, "GameStateManager",
        [&gameStateManager, &waveManager](Core::World& world, Core::GameContext& ctx, float dt) {
            gameStateManager.Update(world, waveManager, dt);
        }, 52);
    
    // TD戦闘システム
    systemRunner_->Register(Core::SystemPhase::Update, "LaneMovement",
                           Domain::TD::Systems::LaneMovementSystem, 100);
    systemRunner_->Register(Core::SystemPhase::Update, "AttackCooldown",
                           Domain::TD::Systems::AttackCooldownSystem, 120);
    systemRunner_->Register(Core::SystemPhase::Update, "AttackTrigger",
                           Domain::TD::Systems::AttackTriggerSystem, 121);
    systemRunner_->Register(Core::SystemPhase::Update, "AttackExecution",
                           Domain::TD::Systems::AttackExecutionSystem, 122);
    systemRunner_->Register(Core::SystemPhase::Update, "DeathCheck",
                           Domain::TD::Systems::DeathCheckSystem, 200);
    
    std::cout << "TD systems registered\n";
}

void UnifiedGame::RegisterRoguelikeSystems() {
    // Roguelikeマネージャーを登録
    auto& turnManager = context_->Emplace<Domain::Roguelike::Managers::TurnManager>();
    
    // Roguelikeシステム
    // TODO: ターンシステム、FOVシステム等を登録
    
    std::cout << "Roguelike systems registered\n";
}

void UnifiedGame::RegisterMenuSystems() {
    // メニューシステムは後で実装
    std::cout << "Menu systems registered (placeholder)\n";
}

void UnifiedGame::RegisterScene(const std::string& name, std::unique_ptr<IScene> scene) {
    if (!scene) {
        std::cerr << "UnifiedGame: Cannot register null scene: " << name << "\n";
        return;
    }
    
    if (scenes_.find(name) != scenes_.end()) {
        std::cerr << "UnifiedGame: Scene already registered: " << name << "\n";
        return;
    }
    
    scenes_[name] = std::move(scene);
    std::cout << "UnifiedGame: Scene registered: " << name << "\n";
}

void UnifiedGame::ChangeScene(const std::string& name) {
    if (scenes_.find(name) == scenes_.end()) {
        std::cerr << "UnifiedGame: Scene not found: " << name << "\n";
        return;
    }
    
    // 現在のシーンを終了
    if (!currentSceneName_.empty() && scenes_.find(currentSceneName_) != scenes_.end()) {
        scenes_[currentSceneName_]->Shutdown(*world_, *context_);
    }
    
    // 新しいシーンに切り替え
    currentSceneName_ = name;
    
    // 新しいシーンを初期化
    if (scenes_.find(currentSceneName_) != scenes_.end()) {
        scenes_[currentSceneName_]->Initialize(*world_, *context_);
        std::cout << "UnifiedGame: Scene changed to: " << currentSceneName_ << "\n";
    }
}

void UnifiedGame::Run() {
    if (!initialized_) {
        std::cerr << "UnifiedGame: Not initialized. Call Initialize() first.\n";
        return;
    }
    
    // ホームシーンで開始（Menuモード）
    currentMode_ = GameMode::Menu;
    InitializeGameMode(GameMode::Menu);
    
    running_ = true;
    
    while (running_ && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        Update(deltaTime);
        Render();
    }
    
    Shutdown();
}

void UnifiedGame::Update(float deltaTime) {
    if (!running_ || !initialized_) return;
    
    // イベント処理
    world_->ProcessEvents();
    
    // 現在のシーンを更新
    if (!currentSceneName_.empty() && scenes_.find(currentSceneName_) != scenes_.end()) {
        scenes_[currentSceneName_]->Update(*world_, *context_, deltaTime);
        
        // シーン遷移チェック
        std::string nextScene = scenes_[currentSceneName_]->GetNextScene();
        if (!nextScene.empty()) {
            if (nextScene == "Exit") {
                running_ = false;
            } else {
                ChangeScene(nextScene);
                scenes_[currentSceneName_]->ClearNextScene();
            }
            return;
        }
    }
    
    // システム実行（Render以外）
    systemRunner_->RunPhase(Core::SystemPhase::PreUpdate, *world_, *context_, deltaTime);
    systemRunner_->RunPhase(Core::SystemPhase::Update, *world_, *context_, deltaTime);
    systemRunner_->RunPhase(Core::SystemPhase::PostUpdate, *world_, *context_, deltaTime);
}

void UnifiedGame::Render() {
    if (!running_ || !initialized_) return;
    
    auto* renderer = context_->TryGet<Core::GameRenderer>();
    if (!renderer) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Renderer not initialized", 10, 10, 20, WHITE);
        EndDrawing();
        return;
    }
    
    // FHDレンダリング開始（BeginTextureModeが呼ばれる）
    renderer->BeginRender();
    
    // 現在のシーンを描画
    if (!currentSceneName_.empty() && scenes_.find(currentSceneName_) != scenes_.end()) {
        scenes_[currentSceneName_]->Render(*world_, *context_);
    }
    
    // 描画システム実行（FHD座標で描画）
    systemRunner_->RunPhase(Core::SystemPhase::Render, *world_, *context_, 0.0f);
    
    // FHDレンダリング終了（EndTextureMode + BeginDrawing + DrawTexturePro + EndDrawingが呼ばれる）
    renderer->EndRender();
}

void UnifiedGame::Shutdown() {
    if (!initialized_) return;
    
    running_ = false;
    
    // 現在のシーンを終了
    if (!currentSceneName_.empty() && scenes_.find(currentSceneName_) != scenes_.end()) {
        scenes_[currentSceneName_]->Shutdown(*world_, *context_);
    }
    
    // すべてのシーンをクリア
    scenes_.clear();
    
    // システムクリア
    ClearCurrentSystems();
    
    // コンテキストクリーンアップ
    context_.reset();
    
    // Raylib終了
    CloseAudioDevice();
    CloseWindow();
    
    initialized_ = false;
    std::cout << "UnifiedGame shutdown complete\n";
}

void UnifiedGame::OnFileChanged(const std::string& filePath) {
    std::cout << "UnifiedGame: File changed: " << filePath << "\n";
    
    // ファイルタイプに応じて再読み込み
    if (filePath.find("ui") != std::string::npos || filePath.find(".ui.json") != std::string::npos) {
        // UIレイアウト定義の再読み込み
        try {
            definitionLoader_->LoadAllUILayouts("assets/definitions/ui");
            std::cout << "UI layouts reloaded\n";
            
            // WebSocket経由でUIエディターに通知（リアルタイム更新）
            if (httpServer_) {
                nlohmann::json notification;
                notification["file"] = filePath;
                notification["reloaded"] = true;
                httpServer_->BroadcastToClients("ui_reloaded", notification.dump());
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to reload UI layouts: " << e.what() << "\n";
        }
    } else if (filePath.find("character") != std::string::npos || filePath.find(".character.json") != std::string::npos) {
        // キャラクター定義の再読み込み
        try {
            definitionLoader_->LoadAllCharacters("assets/definitions/characters");
            std::cout << "Characters reloaded\n";
            
            // SSE経由でクライアントに通知（リアルタイム更新）
            if (httpServer_) {
                nlohmann::json notification;
                notification["file"] = filePath;
                notification["reloaded"] = true;
                httpServer_->BroadcastToClients("character_reloaded", notification.dump());
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to reload characters: " << e.what() << "\n";
        }
    } else if (filePath.find("stage") != std::string::npos || filePath.find(".stage.json") != std::string::npos) {
        // ステージ定義の再読み込み
        try {
            definitionLoader_->LoadAllStages("assets/definitions/stages");
            std::cout << "Stages reloaded\n";
            
            // SSE経由でクライアントに通知（リアルタイム更新）
            if (httpServer_) {
                nlohmann::json notification;
                notification["file"] = filePath;
                notification["reloaded"] = true;
                httpServer_->BroadcastToClients("stage_reloaded", notification.dump());
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to reload stages: " << e.what() << "\n";
        }
    }
}

nlohmann::json UnifiedGame::GetGameState() const {
    nlohmann::json state;
    
    // 基本情報
    state["mode"] = GameModeToString(currentMode_);
    state["scene"] = currentSceneName_;
    state["entityCount"] = world_->EntityCount();
    
    // エンティティ情報（簡易版）
    nlohmann::json entities;
    entities["total"] = world_->EntityCount();
    
    // TDモードの場合の追加情報
    if (currentMode_ == GameMode::TD) {
        nlohmann::json tdState;
        
        // WaveManagerの情報を取得
        if (context_->Has<Domain::TD::Managers::WaveManager>()) {
            const auto& waveManager = context_->Get<Domain::TD::Managers::WaveManager>();
            // WaveManagerの状態を取得（実装に応じて調整）
            tdState["waveManager"] = nlohmann::json::object();
            // TODO: WaveManagerにGetState()メソッドを追加して詳細情報を取得
        }
        
        // GameStateManagerの情報を取得
        if (context_->Has<Domain::TD::Managers::GameStateManager>()) {
            const auto& gameStateManager = context_->Get<Domain::TD::Managers::GameStateManager>();
            // GameStateManagerの状態を取得（実装に応じて調整）
            tdState["gameStateManager"] = nlohmann::json::object();
            // TODO: GameStateManagerにGetState()メソッドを追加して詳細情報を取得
        }
        
        // TDエンティティの統計
        nlohmann::json tdEntities;
        // TODO: TDコンポーネントを持つエンティティの統計を取得
        tdState["entities"] = tdEntities;
        
        state["td"] = tdState;
    }
    
    // Roguelikeモードの場合の追加情報
    if (currentMode_ == GameMode::Roguelike) {
        nlohmann::json roguelikeState;
        
        // TurnManagerの情報を取得
        if (context_->Has<Domain::Roguelike::Managers::TurnManager>()) {
            const auto& turnManager = context_->Get<Domain::Roguelike::Managers::TurnManager>();
            // TurnManagerの状態を取得（実装に応じて調整）
            roguelikeState["turnManager"] = nlohmann::json::object();
            // TODO: TurnManagerにGetState()メソッドを追加して詳細情報を取得
        }
        
        // Roguelikeエンティティの統計
        nlohmann::json roguelikeEntities;
        // TODO: Roguelikeコンポーネントを持つエンティティの統計を取得
        roguelikeState["entities"] = roguelikeEntities;
        
        state["roguelike"] = roguelikeState;
    }
    
    state["entities"] = entities;
    state["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return state;
}

} // namespace Application

