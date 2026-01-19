#include "GameSystem.hpp"
#include "../../utils/Log.h"
#include <rlImGui.h>
#include "../ui/UiAssetKeys.hpp"

namespace game {
namespace core {
GameSystem::GameSystem()
    : systemAPI_(nullptr), audioAPI_(nullptr), ecsAPI_(std::make_unique<ECSystemAPI>()),
      inputAPI_(std::make_unique<InputSystemAPI>()),
      uiAPI_(std::make_unique<UISystemAPI>()),
      sceneOverlayAPI_(std::make_unique<SceneOverlayControlAPI>()),
      currentState_(GameState::Initializing), requestShutdown_(false) {}

int GameSystem::Initialize() {
  if (!InitializeBaseSystem()) {
    return 1;
  }
  if (!InitializeAudio()) {
    return 1;
  }
  if (!InitializeInput()) {
    return 1;
  }
  if (!InitializeUI()) {
    return 1;
  }

  InitializeGameplayData();
  SetupSharedContext();
  sharedContext_.currentState = currentState_;

  if (!InitializeDebugUI()) {
    return 1;
  }

  // RaylibのESCキーによる終亁E��無効匁E
  inputAPI_->SetExitKey(0);

  if (!InitializeSetup()) {
    return 1;
  }
  if (!InitializeBattleSetup()) {
    return 1;
  }
  if (!InitializeSceneOverlay()) {
    return 1;
  }
  if (!InitializeBattleProgress()) {
    return 1;
  }
  if (!InitializeScenes()) {
    return 1;
  }

  if (!sceneOverlayAPI_->InitializeState(GameState::Initializing)) {
    LOG_ERROR("Failed to initialize Initializing state");
    return 1;
  }

  LOG_INFO("Game initialization completed successfully");
  return 0;
}

int GameSystem::Run() {
  if (!systemAPI_) {
    LOG_ERROR("GameSystem not initialized! Call Initialize() first.");
    return 1;
  }

  LOG_INFO("Entering main game loop");

  // メインルーチE
  while (!systemAPI_->Window().WindowShouldClose() && !requestShutdown_) {
    float deltaTime = systemAPI_->Timing().GetFrameTime();
    sharedContext_.deltaTime = deltaTime;
    sharedContext_.currentState = currentState_;

    // 入力状態�E更新
    inputAPI_->UpdateInput();

    // オーチE��オ更新
    if (audioAPI_) {
      audioAPI_->Update(deltaTime);
    }

    // スチE�Eトに応じた更新
    {
      const SceneOverlayUpdateResult updateResult =
          sceneOverlayAPI_->Update(currentState_, deltaTime);
      if (updateResult.requestShutdown) {
        requestShutdown_ = true;
      }
      if (updateResult.hasTransition) {
        transitionTo(updateResult.nextState);
      }
    }

    // ===== 描画フェーズ =====
    systemAPI_->Render().BeginRender();

    sceneOverlayAPI_->Render(currentState_);

    // UIカーソル追従！ESカーソルは残す�E�E
    {
      auto mouse = inputAPI_->GetMousePosition();
      systemAPI_->Render().DrawUiCursor(ui::UiAssetKeys::CursorPointer, mouse,
                                        Vec2{2.0f, 2.0f}, 1.0f, WHITE);
    }

    systemAPI_->Render().EndRender();
    
    // 画面描画�E�EenderTexture + ImGUI�E�E
    // EndFrame()冁E��BeginDrawing()が呼ばれ、RenderTexture描画の後に
    // 自動的にImGUI描画フレームが開始�E終亁E��れる
    // HomeScreenのオーバレイをImGuiフレーム冁E��描画するためのコールバック
    systemAPI_->Render().EndFrame([this]() {
        sceneOverlayAPI_->RenderImGui(currentState_);
        // タイトル画面のオーバ�Eレイ�E�EicenseOverlay、SettingsOverlay�E��E
        // Raylibの描画APIを使用するため、ImGuiフレームは不要E
    });
  }

  LOG_INFO("Main game loop ended");
  return 0;
}

void GameSystem::transitionTo(GameState newState) {
  // 同じ状態への遷移を防止（リトライ時は再初期化）
  if (currentState_ == newState) {
    if (newState == GameState::Game) {
      float prevSpeed = 1.0f;
      if (sharedContext_.battleProgressAPI) {
        prevSpeed = sharedContext_.battleProgressAPI->GetGameSpeed();
      }
      LOG_INFO("Reinitializing Game state (retry)");
      sceneOverlayAPI_->CleanupState(currentState_);
      if (!sceneOverlayAPI_->InitializeState(newState)) {
        LOG_ERROR("Failed to reinitialize state: {}", static_cast<int>(newState));
        requestShutdown_ = true;
      }
      if (sharedContext_.battleProgressAPI) {
        sharedContext_.battleProgressAPI->SetGameSpeed(prevSpeed);
      }
      sharedContext_.currentState = currentState_;
      return;
    }
    LOG_WARN("Already in state: {}", static_cast<int>(newState));
    return;
  }

  LOG_INFO("State transition: {} -> {}", static_cast<int>(currentState_),
           static_cast<int>(newState));

  // 現在のスチE�Eト�EクリーンアチE�E
  sceneOverlayAPI_->CleanupState(currentState_);

  // 新しいスチE�Eト�E初期匁E
  if (!sceneOverlayAPI_->InitializeState(newState)) {
    LOG_ERROR("Failed to initialize state: {}", static_cast<int>(newState));
    // エラー時�E終亁E
    requestShutdown_ = true;
    return;
  }

  currentState_ = newState;
  sharedContext_.currentState = currentState_;
  LOG_INFO("State transitioned to: {}", static_cast<int>(newState));
}

void GameSystem::Shutdown() {
  LOG_INFO("=== Game Shutdown ===");

  ShutdownScenes();
  ShutdownBattleProgress();
  ShutdownDebugUI();
  ShutdownBattleSetup();
  ShutdownSetup();
  ShutdownGameplayData();
  ShutdownSceneOverlay();
  ShutdownUI();
  ShutdownInput();
  ShutdownAudio();
  ShutdownBaseSystem();

  // 注愁E systemAPI_->Shutdown() 冁E�� ShutdownLogSystem() が呼ばれ、E
  // spdlog::shutdown() によりすべてのロガーが破棁E��れるため、E
  // こ�E時点以降ではログ出力を行わなぁE��と
}

bool GameSystem::InitializeBaseSystem() {
  // BaseSystemAPIの作�Eと初期化（ログシスチE��も�E動的に初期化される�E�E
  systemAPI_ = std::make_unique<BaseSystemAPI>();
  if (!systemAPI_->Initialize(Resolution::FHD)) {
    LOG_ERROR("Failed to initialize BaseSystemAPI!");
    return false;
  }
  LOG_INFO("=== tower of defense - Game Initialization ===");
  LOG_INFO("BaseSystemAPI initialized with resolution FHD");
  return true;
}

bool GameSystem::InitializeAudio() {
  audioAPI_ = std::make_unique<AudioControlAPI>();
  if (!audioAPI_->Initialize(systemAPI_.get())) {
    LOG_ERROR("Failed to initialize AudioControlAPI!");
    return false;
  }
  return true;
}

bool GameSystem::InitializeInput() {
  if (!inputAPI_->Initialize()) {
    LOG_ERROR("Failed to initialize InputSystemAPI!");
    return false;
  }
  return true;
}

bool GameSystem::InitializeUI() {
  if (!uiAPI_->Initialize(systemAPI_.get())) {
    LOG_ERROR("Failed to initialize UISystemAPI!");
    return false;
  }
  return true;
}

void GameSystem::InitializeGameplayData() {
  // GameplayDataAPIの作�E�E��E期化はSetupAPIで行う�E�E
  gameplayDataAPI_ = std::make_unique<GameplayDataAPI>();
}

void GameSystem::SetupSharedContext() {
  sharedContext_.systemAPI = systemAPI_.get();
  sharedContext_.audioAPI = audioAPI_.get();
  sharedContext_.ecsAPI = ecsAPI_.get();
  sharedContext_.inputAPI = inputAPI_.get();
  sharedContext_.uiAPI = uiAPI_.get();
  sharedContext_.gameplayDataAPI = gameplayDataAPI_.get();
  sharedContext_.currentStageId = "";  // 初期状態では空
}

bool GameSystem::InitializeDebugUI() {
  debugUIAPI_ = std::make_unique<DebugUIAPI>();
  if (!debugUIAPI_->Initialize(&sharedContext_)) {
    LOG_ERROR("Failed to initialize DebugUIAPI!");
    return false;
  }
  sharedContext_.debugUIAPI = debugUIAPI_.get();
  return true;
}

bool GameSystem::InitializeSetup() {
  setupAPI_ = std::make_unique<SetupAPI>();
  if (!setupAPI_->Initialize(systemAPI_.get(), gameplayDataAPI_.get(), ecsAPI_.get(), &sharedContext_)) {
    LOG_ERROR("Failed to initialize SetupAPI!");
    return false;
  }
  sharedContext_.setupAPI = setupAPI_.get();
  return true;
}

bool GameSystem::InitializeBattleSetup() {
  battleSetupAPI_ = std::make_unique<BattleSetupAPI>();
  if (!battleSetupAPI_->Initialize(gameplayDataAPI_.get(), setupAPI_.get(), &sharedContext_)) {
    LOG_ERROR("Failed to initialize BattleSetupAPI!");
    return false;
  }
  sharedContext_.battleSetupAPI = battleSetupAPI_.get();
  return true;
}

bool GameSystem::InitializeSceneOverlay() {
  if (!sceneOverlayAPI_->Initialize(systemAPI_.get(), uiAPI_.get(), &sharedContext_)) {
    LOG_ERROR("Failed to initialize SceneOverlayControlAPI!");
    return false;
  }
  sharedContext_.sceneOverlayAPI = sceneOverlayAPI_.get();
  return true;
}

bool GameSystem::InitializeBattleProgress() {
  battleProgressAPI_ = std::make_unique<BattleProgressAPI>();
  if (!battleProgressAPI_->Initialize(&sharedContext_)) {
    LOG_ERROR("Failed to initialize BattleProgressAPI!");
    return false;
  }
  sharedContext_.battleProgressAPI = battleProgressAPI_.get();
  return true;
}

bool GameSystem::InitializeScenes() {
  // InitSceneの作�E�E�EnitializingスチE�Eトで使用�E�E
  initScene_ = std::make_unique<states::InitScene>();

  // TitleScreenの作�E�E�まだ初期化しなぁE��リソース初期化完亁E��に初期化！E
  titleScreen_ = std::make_unique<TitleScreen>();

  // HomeScreenの作�E�E�まだ初期化しなぁE��E�E移時に初期化！E
  homeScreen_ = std::make_unique<states::HomeScreen>();

  // GameSceneの作�E�E�まだ初期化しなぁE��E�E移時に初期化！E
  gameScene_ = std::make_unique<states::GameScene>();

  // EditorSceneの作�E�E�まだ初期化しなぁE��E�E移時に初期化！E
  editorScene_ = std::make_unique<states::EditorScene>();

  // Scene/Overlay制御APIへシーンを登録
  sceneOverlayAPI_->RegisterScene(GameState::Title, titleScreen_.get());
  sceneOverlayAPI_->RegisterScene(GameState::Home, homeScreen_.get());
  sceneOverlayAPI_->RegisterScene(GameState::Game, gameScene_.get());
  sceneOverlayAPI_->RegisterScene(GameState::Editor, editorScene_.get());
  sceneOverlayAPI_->RegisterScene(GameState::Initializing, initScene_.get());
  return true;
}

void GameSystem::ShutdownScenes() {
  if (sceneOverlayAPI_) {
    sceneOverlayAPI_->ShutdownAllScenes();
  }

  initScene_.reset();
  titleScreen_.reset();
  homeScreen_.reset();
  gameScene_.reset();
  editorScene_.reset();
}

void GameSystem::ShutdownBattleProgress() {
  if (battleProgressAPI_) {
    battleProgressAPI_.reset();
  }
  sharedContext_.battleProgressAPI = nullptr;
}

void GameSystem::ShutdownDebugUI() {
  if (debugUIAPI_) {
    debugUIAPI_->Shutdown();
    debugUIAPI_.reset();
  }
  sharedContext_.debugUIAPI = nullptr;
}

void GameSystem::ShutdownBattleSetup() {
  if (battleSetupAPI_) {
    battleSetupAPI_.reset();
  }
  sharedContext_.battleSetupAPI = nullptr;
}

void GameSystem::ShutdownSetup() {
  if (setupAPI_) {
    setupAPI_.reset();
  }
  sharedContext_.setupAPI = nullptr;
}

void GameSystem::ShutdownGameplayData() {
  if (gameplayDataAPI_) {
    gameplayDataAPI_->Shutdown();
    gameplayDataAPI_.reset();
  }
  sharedContext_.gameplayDataAPI = nullptr;
}

void GameSystem::ShutdownSceneOverlay() {
  if (sceneOverlayAPI_) {
    sceneOverlayAPI_->Shutdown();
    sceneOverlayAPI_.reset();
  }
  sharedContext_.sceneOverlayAPI = nullptr;
}

void GameSystem::ShutdownUI() {
  if (uiAPI_) {
    uiAPI_->Shutdown();
    uiAPI_.reset();
  }
  sharedContext_.uiAPI = nullptr;
}

void GameSystem::ShutdownInput() {
  if (inputAPI_) {
    inputAPI_->Shutdown();
    inputAPI_.reset();
  }
  sharedContext_.inputAPI = nullptr;
}

void GameSystem::ShutdownAudio() {
  if (audioAPI_) {
    audioAPI_->Shutdown();
    audioAPI_.reset();
  }
  sharedContext_.audioAPI = nullptr;
}

void GameSystem::ShutdownBaseSystem() {
  if (systemAPI_) {
    LOG_INFO("Shutting down BaseSystemAPI");
    systemAPI_->Shutdown();
    systemAPI_.reset();
  }
  sharedContext_.systemAPI = nullptr;
}

void GameSystem::RequestTransition(GameState newState) {
    transitionTo(newState);
}
} // namespace core
} // namespace game
