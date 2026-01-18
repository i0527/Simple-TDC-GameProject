#include "GameSystem.hpp"
#include "../../utils/Log.h"
#include <raylib.h>
#include <rlImGui.h>

namespace game {
namespace core {
GameSystem::GameSystem()
    : systemAPI_(nullptr), gameAPI_(std::make_unique<GameModuleAPI>()),
      overlayManager_(std::make_unique<OverlayManager>()),
      currentState_(GameState::Initializing), requestShutdown_(false) {}

int GameSystem::Initialize() {
  // BaseSystemAPIの作成と初期化（ログシステムも自動的に初期化される）
  systemAPI_ = std::make_unique<BaseSystemAPI>();
  if (!systemAPI_->Initialize(Resolution::FHD)) {
    LOG_ERROR("Failed to initialize BaseSystemAPI!");
    return 1;
  }
  LOG_INFO("=== Cat Tower Defense - Game Initialization ===");
  LOG_INFO("BaseSystemAPI initialized with resolution FHD");

  // CharacterManagerの初期化
  characterManager_ = std::make_unique<entities::CharacterManager>();
  if (!characterManager_->Initialize("data/characters.json")) {
    LOG_WARN("CharacterManager initialization failed, continuing with hardcoded data");
  } else {
    LOG_INFO("CharacterManager initialized with {} characters", characterManager_->GetCharacterCount());
  }

  // ItemPassiveManagerの初期化
  itemPassiveManager_ = std::make_unique<entities::ItemPassiveManager>();
  if (!itemPassiveManager_->Initialize("data/item_passive.json")) {
    LOG_WARN("ItemPassiveManager initialization failed, continuing with hardcoded data");
  }

  // StageManagerの初期化
  stageManager_ = std::make_unique<entities::StageManager>();
  if (!stageManager_->Initialize("data/stages.json")) {
    LOG_WARN("StageManager initialization failed, using default stages");
  } else {
    LOG_INFO("StageManager initialized with {} stages", stageManager_->GetStageCount());
  }

  // PlayerDataManagerの初期化（単一JSONロード）
  playerDataManager_ = std::make_unique<PlayerDataManager>();
  if (!playerDataManager_->LoadOrCreate("data/saves/player_save.json",
                                        *characterManager_,
                                        *itemPassiveManager_)) {
    LOG_WARN("PlayerDataManager initialization failed, continuing with defaults");
  }

  // SharedContextの設定
  sharedContext_.systemAPI = systemAPI_.get();
  sharedContext_.gameAPI = gameAPI_.get();
  sharedContext_.playerDataManager = playerDataManager_.get();
  sharedContext_.characterManager = characterManager_.get();
  sharedContext_.itemPassiveManager = itemPassiveManager_.get();
  sharedContext_.stageManager = stageManager_.get();
  sharedContext_.overlayManager = overlayManager_.get();
  sharedContext_.currentStageId = "";  // 初期状態では空

  // セーブ内容をSharedContext/マスターへ反映
  if (sharedContext_.playerDataManager) {
    sharedContext_.playerDataManager->ApplyToSharedContext(sharedContext_);

    // 既存UIが参照する is_discovered/level を最低限反映
    const auto& save = sharedContext_.playerDataManager->GetSaveData();
    for (const auto& [id, st] : save.characters) {
      characterManager_->SetCharacterDiscovered(id, st.unlocked);
      characterManager_->SetCharacterLevel(id, st.level);
    }
  }

  // RaylibのESCキーによる終了を無効化
  systemAPI_->SetExitKey(0);

  // ModuleSystemの初期化
  moduleSystem_ = std::make_unique<ModuleSystem>(gameAPI_.get());
  RegisterModules();

  if (!moduleSystem_->Initialize(sharedContext_)) {
    LOG_ERROR("Failed to initialize modules!");
    return 1;
  }

  // ResourceInitializerの初期化
  resourceInitializer_ = std::make_unique<ResourceInitializer>();
  if (!resourceInitializer_->Initialize(systemAPI_.get())) {
    LOG_ERROR("Failed to initialize ResourceInitializer!");
    return 1;
  }

  // TitleScreenの作成（まだ初期化しない、リソース初期化完了後に初期化）
  titleScreen_ = std::make_unique<TitleScreen>();
  
  // HomeScreenの作成（まだ初期化しない、遷移時に初期化）
  homeScreen_ = std::make_unique<states::HomeScreen>();
  
  // GameSceneの作成（まだ初期化しない、遷移時に初期化）
  gameScene_ = std::make_unique<states::GameScene>();

  LOG_INFO("Game initialization completed successfully");
  return 0;
}

int GameSystem::Run() {
  if (!systemAPI_) {
    LOG_ERROR("GameSystem not initialized! Call Initialize() first.");
    return 1;
  }

  LOG_INFO("Entering main game loop");

  // メインループ
  while (!systemAPI_->WindowShouldClose() && !requestShutdown_) {
    float deltaTime = systemAPI_->GetFrameTime();
    sharedContext_.deltaTime = deltaTime;

    // 入力状態の更新
    systemAPI_->UpdateInput();

    // ステートに応じた更新
    switch (currentState_) {
    case GameState::Initializing:
      if (resourceInitializer_->Update(deltaTime)) {
        if (resourceInitializer_->IsCompleted()) {
          transitionTo(GameState::Title);
        } else if (resourceInitializer_->ShouldShutdown()) {
          // エラー表示時間が5秒を超えた場合、終了
          LOG_ERROR(
              "Initialization failed, closing application after 5 seconds");
          requestShutdown_ = true;
        }
      }
      break;

    case GameState::Title:
      if (titleScreen_) {
        titleScreen_->Update(deltaTime);
        moduleSystem_->Update(sharedContext_, deltaTime);

        // オーバーレイの更新（最上層のみ）
        overlayManager_->Update(sharedContext_, deltaTime);

        // P0: オーバーレイによる遷移リクエスト処理
        if (overlayManager_->HasTransitionRequest()) {
          GameState next = overlayManager_->GetRequestedTransition();
          overlayManager_->PopAllOverlays();
          transitionTo(next);
          overlayManager_->ClearTransitionRequest();
        }

        // 終了リクエストチェック
        if (titleScreen_->RequestQuit()) {
          LOG_INFO("QUIT requested from TitleScreen");
          requestShutdown_ = true;
        }

        // 遷移リクエストチェック（将来的に他のシーンへの遷移用）
        GameState nextState;
        if (titleScreen_->RequestTransition(nextState)) {
          transitionTo(nextState);
        }
      }
      break;

    case GameState::Home:
      if (homeScreen_) {
        // SharedContextを設定
        homeScreen_->SetSharedContext(&sharedContext_);
        homeScreen_->Update(deltaTime);
        moduleSystem_->Update(sharedContext_, deltaTime);
        overlayManager_->Update(sharedContext_, deltaTime);
        
        // P0: オーバーレイによる遷移リクエスト処理
        if (overlayManager_->HasTransitionRequest()) {
          GameState next = overlayManager_->GetRequestedTransition();
          overlayManager_->PopAllOverlays();
          transitionTo(next);
          overlayManager_->ClearTransitionRequest();
        }
        
        // HomeScreenからの遷移リクエスト処理
        GameState nextState;
        if (homeScreen_->RequestTransition(nextState)) {
          transitionTo(nextState);
        }
      }
      break;

    case GameState::Game:
      if (gameScene_) {
        gameScene_->SetSharedContext(&sharedContext_);
        gameScene_->Update(deltaTime);
        moduleSystem_->Update(sharedContext_, deltaTime);
        overlayManager_->Update(sharedContext_, deltaTime);
        
        // P0: オーバーレイによる遷移リクエスト処理
        if (overlayManager_->HasTransitionRequest()) {
          GameState next = overlayManager_->GetRequestedTransition();
          overlayManager_->PopAllOverlays();
          transitionTo(next);
          overlayManager_->ClearTransitionRequest();
        }
        
        // GameSceneからの遷移リクエスト処理
        GameState nextState;
        if (gameScene_->RequestTransition(nextState)) {
          transitionTo(nextState);
        }
      }
      break;
    }

    // ===== 描画フェーズ =====
    systemAPI_->BeginRender();

    switch (currentState_) {
    case GameState::Initializing:
      resourceInitializer_->Render();
      break;

    case GameState::Title:
      if (titleScreen_) {
        titleScreen_->Render();
      }
      moduleSystem_->Render(sharedContext_);
      // オーバーレイの描画（すべてのオーバーレイを描画）
      overlayManager_->Render(sharedContext_);
      break;

    case GameState::Home:
      if (homeScreen_) {
        homeScreen_->Render();
      }
      moduleSystem_->Render(sharedContext_);
      overlayManager_->Render(sharedContext_);
      break;

    case GameState::Game:
      if (gameScene_) {
        gameScene_->Render();
      }
      moduleSystem_->Render(sharedContext_);
      overlayManager_->Render(sharedContext_);
      break;
    }

    systemAPI_->EndRender();
    
    // 画面描画（RenderTexture + ImGUI）
    // EndFrame()内でBeginDrawing()が呼ばれ、RenderTexture描画の後に
    // 自動的にImGUI描画フレームが開始・終了される
    // HomeScreenのオーバレイをImGuiフレーム内で描画するためのコールバック
    systemAPI_->EndFrame([this]() {
        if (currentState_ == GameState::Home && homeScreen_) {
            homeScreen_->RenderImGui();
        }
        // タイトル画面のオーバーレイ（LicenseOverlay、SettingsOverlay）は
        // Raylibの描画APIを使用するため、ImGuiフレームは不要
    });
  }

  LOG_INFO("Main game loop ended");
  return 0;
}

void GameSystem::transitionTo(GameState newState) {
  // 同じ状態への遷移を防止
  if (currentState_ == newState) {
    LOG_WARN("Already in state: {}", static_cast<int>(newState));
    return;
  }

  LOG_INFO("State transition: {} -> {}", static_cast<int>(currentState_),
           static_cast<int>(newState));

  // 現在のステートのクリーンアップ
  cleanupCurrentState();

  // 新しいステートの初期化
  if (!initializeState(newState)) {
    LOG_ERROR("Failed to initialize state: {}", static_cast<int>(newState));
    // エラー時は終了
    requestShutdown_ = true;
    return;
  }

  currentState_ = newState;
  LOG_INFO("State transitioned to: {}", static_cast<int>(newState));
}

void GameSystem::cleanupCurrentState() {
  switch (currentState_) {
  case GameState::Initializing:
    // ResourceInitializerはGameSystemのShutdownでクリーンアップ
    break;

  case GameState::Title:
    if (titleScreen_) {
      titleScreen_->Shutdown();
    }
    // オーバーレイをすべて削除
    overlayManager_->PopAllOverlays();
    break;

  case GameState::Home:
    if (homeScreen_) {
      homeScreen_->Shutdown();
    }
    overlayManager_->PopAllOverlays();
    break;

  case GameState::Game:
    if (gameScene_) {
      gameScene_->Shutdown();
    }
    overlayManager_->PopAllOverlays();
    break;
  }
}

bool GameSystem::initializeState(GameState state) {
  switch (state) {
  case GameState::Initializing:
    // ResourceInitializerはInitialize()で既に初期化済み
    return true;

  case GameState::Title:
    LOG_INFO("Initializing Title state...");
    if (titleScreen_) {
      LOG_INFO("TitleScreen object exists, calling Initialize()...");
      bool result = titleScreen_->Initialize(systemAPI_.get(), &sharedContext_);
      LOG_INFO("TitleScreen::Initialize() returned: {}", result);
      return result;
    }
    LOG_ERROR("TitleScreen object is null!");
    return false;

  case GameState::Home:
    if (homeScreen_) {
      if (!homeScreen_->Initialize(systemAPI_.get())) {
        LOG_ERROR("Failed to initialize HomeScreen");
        return false;
      }
      homeScreen_->SetSharedContext(&sharedContext_);
      LOG_INFO("Home state initialized");
      return true;
    }
    return false;

  case GameState::Game:
    if (gameScene_) {
      if (!gameScene_->Initialize(systemAPI_.get())) {
        LOG_ERROR("Failed to initialize GameScene");
        return false;
      }
      gameScene_->SetSharedContext(&sharedContext_);
      LOG_INFO("Game state initialized");
      return true;
    }
    return false;
  }
  return false;
}

void GameSystem::Shutdown() {
  LOG_INFO("=== Game Shutdown ===");

  // 現在のステートのクリーンアップ
  cleanupCurrentState();

  // TitleScreenのクリーンアップ
  if (titleScreen_) {
    titleScreen_->Shutdown();
    titleScreen_.reset();
  }
  
  // HomeScreenのクリーンアップ
  if (homeScreen_) {
    homeScreen_->Shutdown();
    homeScreen_.reset();
  }
  
  // GameSceneのクリーンアップ
  if (gameScene_) {
    gameScene_->Shutdown();
    gameScene_.reset();
  }

  // ResourceInitializerのクリーンアップ
  if (resourceInitializer_) {
    resourceInitializer_->Reset();
    resourceInitializer_.reset();
  }

  // CharacterManagerのクリーンアップ
  if (characterManager_) {
    characterManager_->Shutdown();
    characterManager_.reset();
  }

  // ItemPassiveManagerのクリーンアップ
  if (itemPassiveManager_) {
    itemPassiveManager_->Shutdown();
    itemPassiveManager_.reset();
  }

  // オーバーレイのシャットダウン
  if (overlayManager_) {
    overlayManager_->Shutdown();
    overlayManager_.reset();
  }

  // モジュールのシャットダウン
  if (moduleSystem_) {
    moduleSystem_->Shutdown(sharedContext_);
    moduleSystem_.reset();
  }

  if (systemAPI_) {
    LOG_INFO("Shutting down BaseSystemAPI");
    systemAPI_->Shutdown();
    systemAPI_.reset();
  }

  // 注意: systemAPI_->Shutdown() 内で ShutdownLogSystem() が呼ばれ、
  // spdlog::shutdown() によりすべてのロガーが破棄されるため、
  // この時点以降ではログ出力を行わないこと
}

void GameSystem::RegisterModules() {
  // モジュール登録
  // 将来的にdefineModules.hppで定義されたモジュールを登録
  // 現在は空（モジュール実装時に追加）
  // 例:
  // #include "ecs/defineModules.hpp"
  // moduleSystem_->RegisterModule<MovementModule>();
  // moduleSystem_->RegisterModule<RenderModule>();
}

void GameSystem::RequestTransition(GameState newState) {
    transitionTo(newState);
}
} // namespace core
} // namespace game
