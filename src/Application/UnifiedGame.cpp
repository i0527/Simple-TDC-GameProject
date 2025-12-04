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
#include "Application/TDGameScene.h"
#include "Application/RoguelikeGameScene.h"
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
#include <filesystem>

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
    , definitionLoader_(std::make_unique<Data::DefinitionLoader>(*definitions_))
{
    // SystemRunnerはRunPhase/RunAll呼び出し時にworld_とcontext_を引数で受け取る
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
        std::cout << "UnifiedGame: Loading definitions from: " << definitionsPath << "\n";
        
        std::cout << "UnifiedGame: Loading characters...\n";
        definitionLoader_->LoadAllCharacters(definitionsPath + "/characters");
        
        std::cout << "UnifiedGame: Loading stages...\n";
        definitionLoader_->LoadAllStages(definitionsPath + "/stages");
        
        std::cout << "UnifiedGame: Loading UI layouts...\n";
        definitionLoader_->LoadAllUILayouts(definitionsPath + "/ui");
        
        // マップ定義の読み込みはオプショナル（Roguelike用）
        std::string mapsPath = definitionsPath + "/maps";
        if (std::filesystem::exists(mapsPath)) {
            std::cout << "UnifiedGame: Loading maps from: " << mapsPath << "\n";
            definitionLoader_->LoadAllMaps(mapsPath);
        } else {
            std::cout << "UnifiedGame: ℹ️ Maps directory not found at: " << mapsPath 
                      << " - Roguelike will generate dungeons procedurally\n";
        }
        
        // 定義をレジストリに登録
        auto& loader = *definitionLoader_;
        // TODO: ローダーからレジストリへの統合
        
        std::cout << "UnifiedGame: ✅ All available definitions loaded successfully\n";
    } catch (const std::exception& e) {
        std::cerr << "UnifiedGame: ❌ Failed to load definitions: " << e.what() << "\n";
        // エラーでも続行(デフォルト定義を使用)
        std::cerr << "UnifiedGame: ⚠️ Continuing with default definitions\n";
    }
    
    // EntityFactory登録（レジストリへの参照を渡す）
    context_->Register<Core::EntityFactory>(*world_, *definitions_);
    
    // SoundManager登録
    auto& soundManager = context_->Emplace<Core::SoundManager>();
    soundManager.SetRegistry(definitions_.get());
    
    // EffectManager登録
    auto& effectManager = context_->Emplace<Core::EffectManager>();
    effectManager.SetRegistry(definitions_.get());
    
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

            // エンティティ操作のコールバックを設定（開発者モード用）
            httpServer_->SetEntityOperationCallbacks(
                [this]() { return this->GetEntities(); },
                [this](const std::string& id) { return this->GetEntity(id); },
                [this](const nlohmann::json& config) { return this->CreateEntity(config); },
                [this](const std::string& id, const nlohmann::json& config) { return this->UpdateEntity(id, config); },
                [this](const std::string& id) { return this->DeleteEntity(id); },
                [this](const std::string& id, const nlohmann::json& params) { return this->SpawnEntity(id, params); },
                [this](const std::string& id, const nlohmann::json& stats) { return this->SetEntityStats(id, stats); },
                [this](const std::string& id, const nlohmann::json& aiConfig) { return this->SetEntityAI(id, aiConfig); },
                [this](const std::string& id, const nlohmann::json& skills) { return this->SetEntitySkills(id, skills); }
            );

            // スクリーンショット取得のコールバックを設定（開発者モード用）
            httpServer_->SetScreenshotCallback([this]() {
                return this->GetScreenshot();
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
    RegisterScene("TDGame", std::make_unique<TDGameScene>());
    RegisterScene("Roguelike", std::make_unique<RoguelikeGameScene>());
    
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
    
    // モードに対応するシーンに切り替え
    switch (mode) {
        case GameMode::Menu:
            ChangeScene("Home");
            break;
        case GameMode::TD:
            ChangeScene("TDGame");
            break;
        case GameMode::Roguelike:
            ChangeScene("Roguelike");
            break;
    }
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
    
    // エンティティ情報（詳細版）
    nlohmann::json entities;
    entities["total"] = world_->EntityCount();
    
    // TDエンティティの統計
    int tdEntityCount = 0;
    int tdAllyCount = 0;
    int tdEnemyCount = 0;
    auto tdView = world_->View<Domain::TD::Components::Unit>();
    for (auto entity : tdView) {
        tdEntityCount++;
        const auto& unit = tdView.get<Domain::TD::Components::Unit>(entity);
        if (unit.isEnemy) {
            tdEnemyCount++;
        } else {
            tdAllyCount++;
        }
    }
    
    // Roguelikeエンティティの統計
    int rogueEntityCount = 0;
    int roguePlayerCount = 0;
    int rogueMonsterCount = 0;
    auto rogueView = world_->View<Domain::Roguelike::Components::GridPosition>();
    for (auto entity : rogueView) {
        rogueEntityCount++;
        if (world_->HasAll<Domain::Roguelike::Components::TurnActor>(entity)) {
            const auto* actor = world_->TryGet<Domain::Roguelike::Components::TurnActor>(entity);
            if (actor && actor->isPlayer) {
                roguePlayerCount++;
            } else {
                rogueMonsterCount++;
            }
        }
    }
    
    entities["td"] = nlohmann::json::object();
    entities["td"]["total"] = tdEntityCount;
    entities["td"]["allies"] = tdAllyCount;
    entities["td"]["enemies"] = tdEnemyCount;
    
    entities["roguelike"] = nlohmann::json::object();
    entities["roguelike"]["total"] = rogueEntityCount;
    entities["roguelike"]["players"] = roguePlayerCount;
    entities["roguelike"]["monsters"] = rogueMonsterCount;
    
    // TDモードの場合の追加情報
    if (currentMode_ == GameMode::TD) {
        nlohmann::json tdState;
        
        // WaveManagerの情報を取得
        if (context_->Has<Domain::TD::Managers::WaveManager>()) {
            const auto& waveManager = context_->Get<Domain::TD::Managers::WaveManager>();
            tdState["waveManager"] = nlohmann::json::object();
            tdState["waveManager"]["currentWave"] = waveManager.GetCurrentWaveNumber();
            tdState["waveManager"]["totalWaves"] = waveManager.GetTotalWaves();
            tdState["waveManager"]["laneCount"] = waveManager.GetLaneCount();
        }
        
        // GameStateManagerの情報を取得
        if (context_->Has<Domain::TD::Managers::GameStateManager>()) {
            const auto& gameStateManager = context_->Get<Domain::TD::Managers::GameStateManager>();
            tdState["gameStateManager"] = nlohmann::json::object();
            tdState["gameStateManager"]["baseHealth"] = gameStateManager.GetBaseHealth();
            tdState["gameStateManager"]["enemyBaseHealth"] = gameStateManager.GetEnemyBaseHealth();
            tdState["gameStateManager"]["phase"] = static_cast<int>(gameStateManager.GetPhase());
        }
        
        // SpawnManagerの情報を取得
        if (context_->Has<Domain::TD::Managers::SpawnManager>()) {
            const auto& spawnManager = context_->Get<Domain::TD::Managers::SpawnManager>();
            tdState["spawnManager"] = nlohmann::json::object();
            tdState["spawnManager"]["currentCost"] = spawnManager.GetCurrentCost();
            tdState["spawnManager"]["maxCost"] = spawnManager.GetMaxCost();
        }
        
        state["td"] = tdState;
    }
    
    // Roguelikeモードの場合の追加情報
    if (currentMode_ == GameMode::Roguelike) {
        nlohmann::json roguelikeState;
        
        // TurnManagerの情報を取得
        if (context_->Has<Domain::Roguelike::Managers::TurnManager>()) {
            const auto& turnManager = context_->Get<Domain::Roguelike::Managers::TurnManager>();
            roguelikeState["turnManager"] = nlohmann::json::object();
            roguelikeState["turnManager"]["turnCount"] = turnManager.GetTurnCount();
            roguelikeState["turnManager"]["state"] = static_cast<int>(turnManager.GetState());
            roguelikeState["turnManager"]["awaitingInput"] = turnManager.IsAwaitingInput();
        }
        
        state["roguelike"] = roguelikeState;
    }
    
    state["entities"] = entities;
    state["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return state;
}

nlohmann::json UnifiedGame::GetEntities() const {
    nlohmann::json result = nlohmann::json::array();
    
    // 全エンティティを走査
    auto view = world_->View<Core::Components::Identity>();
    for (auto entity : view) {
        const auto& identity = view.get<Core::Components::Identity>(entity);
        
        nlohmann::json entityJson;
        entityJson["id"] = std::to_string(static_cast<uint32_t>(entity));
        entityJson["entityId"] = identity.id;
        entityJson["type"] = identity.type;
        entityJson["name"] = identity.name;
        
        // 位置情報
        if (auto* pos = world_->TryGet<Core::Components::Position>(entity)) {
            entityJson["position"] = nlohmann::json::object();
            entityJson["position"]["x"] = pos->x;
            entityJson["position"]["y"] = pos->y;
        }
        
        // TDコンポーネント
        if (auto* stats = world_->TryGet<Domain::TD::Components::Stats>(entity)) {
            entityJson["td"] = nlohmann::json::object();
            entityJson["td"]["stats"] = nlohmann::json::object();
            entityJson["td"]["stats"]["maxHealth"] = stats->maxHealth;
            entityJson["td"]["stats"]["currentHealth"] = stats->currentHealth;
            entityJson["td"]["stats"]["attack"] = stats->attack;
            entityJson["td"]["stats"]["defense"] = stats->defense;
            entityJson["td"]["stats"]["moveSpeed"] = stats->moveSpeed;
        }
        
        // Roguelikeコンポーネント
        if (auto* rogueStats = world_->TryGet<Domain::Roguelike::Components::Stats>(entity)) {
            entityJson["roguelike"] = nlohmann::json::object();
            entityJson["roguelike"]["stats"] = nlohmann::json::object();
            entityJson["roguelike"]["stats"]["maxHp"] = rogueStats->maxHp;
            entityJson["roguelike"]["stats"]["hp"] = rogueStats->hp;
            entityJson["roguelike"]["stats"]["attack"] = rogueStats->attack;
            entityJson["roguelike"]["stats"]["defense"] = rogueStats->defense;
            entityJson["roguelike"]["stats"]["level"] = rogueStats->level;
        }
        
        result.push_back(entityJson);
    }
    
    return result;
}

nlohmann::json UnifiedGame::GetEntity(const std::string& entityId) const {
    // entityIdは文字列形式のentt::entity ID
    try {
        uint32_t entityValue = std::stoul(entityId);
        entt::entity entity = static_cast<entt::entity>(entityValue);
        
        if (!world_->Valid(entity)) {
            return nlohmann::json();
        }
        
        nlohmann::json entityJson;
        entityJson["id"] = entityId;
        
        // Identity
        if (auto* identity = world_->TryGet<Core::Components::Identity>(entity)) {
            entityJson["entityId"] = identity->id;
            entityJson["type"] = identity->type;
            entityJson["name"] = identity->name;
        }
        
        // Position
        if (auto* pos = world_->TryGet<Core::Components::Position>(entity)) {
            entityJson["position"] = nlohmann::json::object();
            entityJson["position"]["x"] = pos->x;
            entityJson["position"]["y"] = pos->y;
        }
        
        // TDコンポーネント
        if (auto* stats = world_->TryGet<Domain::TD::Components::Stats>(entity)) {
            entityJson["td"] = nlohmann::json::object();
            entityJson["td"]["stats"] = nlohmann::json::object();
            entityJson["td"]["stats"]["maxHealth"] = stats->maxHealth;
            entityJson["td"]["stats"]["currentHealth"] = stats->currentHealth;
            entityJson["td"]["stats"]["attack"] = stats->attack;
            entityJson["td"]["stats"]["defense"] = stats->defense;
            entityJson["td"]["stats"]["moveSpeed"] = stats->moveSpeed;
            entityJson["td"]["stats"]["attackInterval"] = stats->attackInterval;
        }
        
        if (auto* unit = world_->TryGet<Domain::TD::Components::Unit>(entity)) {
            if (!entityJson.contains("td")) {
                entityJson["td"] = nlohmann::json::object();
            }
            entityJson["td"]["unit"] = nlohmann::json::object();
            entityJson["td"]["unit"]["definitionId"] = unit->definitionId;
            entityJson["td"]["unit"]["isEnemy"] = unit->isEnemy;
            entityJson["td"]["unit"]["level"] = unit->level;
        }
        
        // Roguelikeコンポーネント
        if (auto* rogueStats = world_->TryGet<Domain::Roguelike::Components::Stats>(entity)) {
            entityJson["roguelike"] = nlohmann::json::object();
            entityJson["roguelike"]["stats"] = nlohmann::json::object();
            entityJson["roguelike"]["stats"]["maxHp"] = rogueStats->maxHp;
            entityJson["roguelike"]["stats"]["hp"] = rogueStats->hp;
            entityJson["roguelike"]["stats"]["attack"] = rogueStats->attack;
            entityJson["roguelike"]["stats"]["defense"] = rogueStats->defense;
            entityJson["roguelike"]["stats"]["level"] = rogueStats->level;
            entityJson["roguelike"]["stats"]["experience"] = rogueStats->experience;
        }
        
        if (auto* gridPos = world_->TryGet<Domain::Roguelike::Components::GridPosition>(entity)) {
            if (!entityJson.contains("roguelike")) {
                entityJson["roguelike"] = nlohmann::json::object();
            }
            entityJson["roguelike"]["gridPosition"] = nlohmann::json::object();
            entityJson["roguelike"]["gridPosition"]["x"] = gridPos->x;
            entityJson["roguelike"]["gridPosition"]["y"] = gridPos->y;
        }
        
        return entityJson;
    } catch (const std::exception&) {
        return nlohmann::json();
    }
}

nlohmann::json UnifiedGame::CreateEntity(const nlohmann::json& config) const {
    // テスト用エンティティ作成
    auto entity = world_->Create();
    
    // 基本コンポーネント
    std::string entityId = config.value("entityId", "test_entity_" + std::to_string(static_cast<uint32_t>(entity)));
    std::string type = config.value("type", "unit");
    std::string name = config.value("name", "Test Entity");
    
    world_->Emplace<Core::Components::Identity>(entity, entityId, type, name);
    
    // 位置
    float x = config.value("position", nlohmann::json::object()).value("x", 0.0f);
    float y = config.value("position", nlohmann::json::object()).value("y", 0.0f);
    world_->Emplace<Core::Components::Position>(entity, x, y);
    
    // TDコンポーネント（指定がある場合）
    if (config.contains("td")) {
        auto& tdConfig = config["td"];
        if (tdConfig.contains("stats")) {
            auto& statsConfig = tdConfig["stats"];
            Domain::TD::Components::Stats stats;
            stats.maxHealth = statsConfig.value("maxHealth", 100.0f);
            stats.currentHealth = statsConfig.value("currentHealth", stats.maxHealth);
            stats.attack = statsConfig.value("attack", 10.0f);
            stats.defense = statsConfig.value("defense", 0.0f);
            stats.moveSpeed = statsConfig.value("moveSpeed", 50.0f);
            stats.attackInterval = statsConfig.value("attackInterval", 1.0f);
            world_->Emplace<Domain::TD::Components::Stats>(entity, stats);
        }
        
        if (tdConfig.contains("unit")) {
            auto& unitConfig = tdConfig["unit"];
            Domain::TD::Components::Unit unit;
            unit.definitionId = unitConfig.value("definitionId", "");
            unit.isEnemy = unitConfig.value("isEnemy", false);
            unit.level = unitConfig.value("level", 1);
            world_->Emplace<Domain::TD::Components::Unit>(entity, unit);
            
            // 陣営タグ
            if (unit.isEnemy) {
                world_->Emplace<Domain::TD::Components::EnemyUnit>(entity);
            } else {
                world_->Emplace<Domain::TD::Components::AllyUnit>(entity);
            }
        }
    }
    
    // Roguelikeコンポーネント（指定がある場合）
    if (config.contains("roguelike")) {
        auto& rogueConfig = config["roguelike"];
        if (rogueConfig.contains("stats")) {
            auto& statsConfig = rogueConfig["stats"];
            Domain::Roguelike::Components::Stats stats;
            stats.maxHp = statsConfig.value("maxHp", 10);
            stats.hp = statsConfig.value("hp", stats.maxHp);
            stats.attack = statsConfig.value("attack", 1);
            stats.defense = statsConfig.value("defense", 0);
            stats.level = statsConfig.value("level", 1);
            stats.experience = statsConfig.value("experience", 0);
            world_->Emplace<Domain::Roguelike::Components::Stats>(entity, stats);
        }
        
        if (rogueConfig.contains("gridPosition")) {
            auto& posConfig = rogueConfig["gridPosition"];
            int gridX = posConfig.value("x", 0);
            int gridY = posConfig.value("y", 0);
            world_->Emplace<Domain::Roguelike::Components::GridPosition>(entity, gridX, gridY);
        }
    }
    
    nlohmann::json result;
    result["success"] = true;
    result["entityId"] = std::to_string(static_cast<uint32_t>(entity));
    result["message"] = "Entity created";
    
    return result;
}

nlohmann::json UnifiedGame::UpdateEntity(const std::string& entityId, const nlohmann::json& config) const {
    try {
        uint32_t entityValue = std::stoul(entityId);
        entt::entity entity = static_cast<entt::entity>(entityValue);
        
        if (!world_->Valid(entity)) {
            nlohmann::json error;
            error["success"] = false;
            error["error"] = "Entity not found";
            return error;
        }
        
        // 位置更新
        if (config.contains("position")) {
            auto* pos = world_->TryGet<Core::Components::Position>(entity);
            if (pos) {
                pos->x = config["position"].value("x", pos->x);
                pos->y = config["position"].value("y", pos->y);
            }
        }
        
        // TDステータス更新
        if (config.contains("td") && config["td"].contains("stats")) {
            auto* stats = world_->TryGet<Domain::TD::Components::Stats>(entity);
            if (stats) {
                auto& statsConfig = config["td"]["stats"];
                if (statsConfig.contains("maxHealth")) stats->maxHealth = statsConfig["maxHealth"];
                if (statsConfig.contains("currentHealth")) stats->currentHealth = statsConfig["currentHealth"];
                if (statsConfig.contains("attack")) stats->attack = statsConfig["attack"];
                if (statsConfig.contains("defense")) stats->defense = statsConfig["defense"];
                if (statsConfig.contains("moveSpeed")) stats->moveSpeed = statsConfig["moveSpeed"];
                if (statsConfig.contains("attackInterval")) stats->attackInterval = statsConfig["attackInterval"];
            }
        }
        
        // Roguelikeステータス更新
        if (config.contains("roguelike") && config["roguelike"].contains("stats")) {
            auto* stats = world_->TryGet<Domain::Roguelike::Components::Stats>(entity);
            if (stats) {
                auto& statsConfig = config["roguelike"]["stats"];
                if (statsConfig.contains("maxHp")) stats->maxHp = statsConfig["maxHp"];
                if (statsConfig.contains("hp")) stats->hp = statsConfig["hp"];
                if (statsConfig.contains("attack")) stats->attack = statsConfig["attack"];
                if (statsConfig.contains("defense")) stats->defense = statsConfig["defense"];
                if (statsConfig.contains("level")) stats->level = statsConfig["level"];
                if (statsConfig.contains("experience")) stats->experience = statsConfig["experience"];
            }
        }
        
        nlohmann::json result;
        result["success"] = true;
        result["entityId"] = entityId;
        result["message"] = "Entity updated";
        
        return result;
    } catch (const std::exception& e) {
        nlohmann::json error;
        error["success"] = false;
        error["error"] = e.what();
        return error;
    }
}

bool UnifiedGame::DeleteEntity(const std::string& entityId) const {
    try {
        uint32_t entityValue = std::stoul(entityId);
        entt::entity entity = static_cast<entt::entity>(entityValue);
        
        if (!world_->Valid(entity)) {
            return false;
        }
        
        world_->Destroy(entity);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

nlohmann::json UnifiedGame::SpawnEntity(const std::string& entityId, const nlohmann::json& spawnParams) const {
    // エンティティ定義IDからエンティティを作成
    std::string definitionId = spawnParams.value("definitionId", entityId);
    float x = spawnParams.value("x", 0.0f);
    float y = spawnParams.value("y", 0.0f);
    bool isEnemy = spawnParams.value("isEnemy", false);
    int level = spawnParams.value("level", 1);
    
    // EntityFactoryを使用してエンティティを作成
    if (context_->Has<Core::EntityFactory>()) {
        auto& factory = context_->Get<Core::EntityFactory>();
        entt::entity entity = factory.CreateCharacter(definitionId, x, y, isEnemy, level);
        
        nlohmann::json result;
        result["success"] = true;
        result["entityId"] = std::to_string(static_cast<uint32_t>(entity));
        result["message"] = "Entity spawned";
        
        return result;
    }
    
    nlohmann::json error;
    error["success"] = false;
    error["error"] = "EntityFactory not available";
    return error;
}

nlohmann::json UnifiedGame::SetEntityStats(const std::string& entityId, const nlohmann::json& stats) const {
    try {
        uint32_t entityValue = std::stoul(entityId);
        entt::entity entity = static_cast<entt::entity>(entityValue);
        
        if (!world_->Valid(entity)) {
            nlohmann::json error;
            error["success"] = false;
            error["error"] = "Entity not found";
            return error;
        }
        
        // TDステータス設定
        if (auto* tdStats = world_->TryGet<Domain::TD::Components::Stats>(entity)) {
            if (stats.contains("maxHealth")) tdStats->maxHealth = stats["maxHealth"];
            if (stats.contains("currentHealth")) tdStats->currentHealth = stats["currentHealth"];
            if (stats.contains("attack")) tdStats->attack = stats["attack"];
            if (stats.contains("defense")) tdStats->defense = stats["defense"];
            if (stats.contains("moveSpeed")) tdStats->moveSpeed = stats["moveSpeed"];
            if (stats.contains("attackInterval")) tdStats->attackInterval = stats["attackInterval"];
        }
        
        // Roguelikeステータス設定
        if (auto* rogueStats = world_->TryGet<Domain::Roguelike::Components::Stats>(entity)) {
            if (stats.contains("maxHp")) rogueStats->maxHp = stats["maxHp"];
            if (stats.contains("hp")) rogueStats->hp = stats["hp"];
            if (stats.contains("attack")) rogueStats->attack = stats["attack"];
            if (stats.contains("defense")) rogueStats->defense = stats["defense"];
            if (stats.contains("level")) rogueStats->level = stats["level"];
            if (stats.contains("experience")) rogueStats->experience = stats["experience"];
        }
        
        nlohmann::json result;
        result["success"] = true;
        result["entityId"] = entityId;
        result["message"] = "Entity stats updated";
        
        return result;
    } catch (const std::exception& e) {
        nlohmann::json error;
        error["success"] = false;
        error["error"] = e.what();
        return error;
    }
}

nlohmann::json UnifiedGame::SetEntityAI(const std::string& entityId, const nlohmann::json& aiConfig) const {
    // AI設定は将来実装（現在はプレースホルダー）
    nlohmann::json result;
    result["success"] = true;
    result["entityId"] = entityId;
    result["message"] = "AI configuration updated (placeholder)";
    result["note"] = "AI configuration will be implemented in future phases";
    
    return result;
}

nlohmann::json UnifiedGame::SetEntitySkills(const std::string& entityId, const nlohmann::json& skills) const {
    // スキル設定は将来実装（現在はプレースホルダー）
    nlohmann::json result;
    result["success"] = true;
    result["entityId"] = entityId;
    result["message"] = "Skills updated (placeholder)";
    result["note"] = "Skill system will be implemented in future phases";
    
    return result;
}

std::string UnifiedGame::GetScreenshot() const {
    // TODO: 実装予定
    // GameRendererからRenderTextureを取得し、画像をエクスポートしてBase64エンコード
    // 現在はプレースホルダーとして空文字列を返す
    
    // 将来的な実装例:
    // auto* renderer = context_->TryGet<Core::GameRenderer>();
    // if (!renderer) return "";
    // 
    // const auto& renderTarget = renderer->GetRenderTarget();
    // Image image = LoadImageFromTexture(renderTarget.texture);
    // ImageFlipVertical(&image);  // RenderTextureはY反転されているため
    // 
    // // PNGエンコード（外部ライブラリが必要: stb_image_write等）
    // // Base64エンコード
    // 
    // UnloadImage(image);
    // return base64EncodedData;
    
    return "";  // プレースホルダー
}

} // namespace Application

