/**
 * @file TDGameScene.cpp
 * @brief TDゲームシーン実装
 *
 * Phase 3: 既存実装統合（TD/Roguelike）
 */

#include "Application/TDGameScene.h"
#include "Core/Components/CoreComponents.h"
#include "Core/GameContext.h"
#include "Core/GameRenderer.h"
#include "Core/World.h"
#include "Domain/TD/Components/TDComponents.h"
#include "Domain/TD/Managers/GameStateManager.h"
#include "Domain/TD/Managers/SpawnManager.h"
#include "Domain/TD/Managers/WaveManager.h"
#include "Domain/TD/Systems/TDSystems.h"


#include <iostream>
#include <raylib.h>

namespace Application {

// TD用グローバル変数（GameContext との統合）
static Domain::TD::Managers::WaveManager *g_waveManager = nullptr;
static Domain::TD::Managers::SpawnManager *g_spawnManager = nullptr;
static Domain::TD::Managers::GameStateManager *g_gameStateManager = nullptr;

TDGameScene::TDGameScene()
    : nextScene_(""), initialized_(false), elapsedTime_(0.0f) {}

void TDGameScene::Initialize(Core::World &world, Core::GameContext &context) {
  if (initialized_) {
    std::cout << "TDGameScene: Already initialized\n";
    return;
  }

  std::cout << "TDGameScene: Initializing...\n";

  // TD固有マネージャーの登録・確認
  if (!context.Has<Domain::TD::Managers::WaveManager>()) {
    context.Emplace<Domain::TD::Managers::WaveManager>();
  }
  g_waveManager = context.TryGet<Domain::TD::Managers::WaveManager>();

  if (!context.Has<Domain::TD::Managers::SpawnManager>()) {
    context.Emplace<Domain::TD::Managers::SpawnManager>();
  }
  g_spawnManager = context.TryGet<Domain::TD::Managers::SpawnManager>();

  if (!context.Has<Domain::TD::Managers::GameStateManager>()) {
    context.Emplace<Domain::TD::Managers::GameStateManager>();
  }
  g_gameStateManager = context.TryGet<Domain::TD::Managers::GameStateManager>();

  // ゲーム状態初期化
  if (g_gameStateManager) {
    g_gameStateManager->Initialize();
    g_gameStateManager->SetBaseHealth(100.0f, 100.0f); // プレイヤー拠点100HP
  }

  // TD固有のシステムを登録
  RegisterTDSystems(context);

  // デフォルトステージを読み込む
  LoadDefaultStage(world, context);

  initialized_ = true;
  elapsedTime_ = 0.0f;

  std::cout << "TDGameScene: Initialized successfully\n";
}

void TDGameScene::RegisterTDSystems(Core::GameContext &context) {
  // TDシステムはUnifiedGameで既に登録されているはずだが、
  // ここで確認・必要に応じて再登録

  std::cout << "TDGameScene: TD systems ready for integration\n";
}

 void TDGameScene::LoadDefaultStage(Core::World &world,
                                    Core::GameContext &context) {
   // TODO: ステージ定義から実際のステージを読み込む
   // 暫定的にデフォルトWaveを作成
 
   std::cout << "TDGameScene: Loading default stage...\n";
 
   // デフォルトステージ定義を作成（ハードコード）
   Data::StageDef stageDef;
   stageDef.id = "stage_1";
   stageDef.name = "Stage 1";
   stageDef.description = "First TD stage";
   stageDef.baseHealth = 100.0f;
   stageDef.enemyBaseHealth = 100.0f;
 
   // Wave 1
   Data::WaveDef wave1;
   wave1.waveNumber = 1;
 
   // Wave エントリ（敵タイプ、出現数、遅延、間隔、レーン）
   Data::EnemySpawnEntry entry1{"goblin", 3, 0.5f, 0.5f, 0};
   Data::EnemySpawnEntry entry2{"goblin", 2, 1.0f, 0.5f, 1};
   wave1.enemies.push_back(entry1);
   wave1.enemies.push_back(entry2);
   stageDef.waves.push_back(wave1);
 
   // Wave 2
   Data::WaveDef wave2;
   wave2.waveNumber = 2;
   Data::EnemySpawnEntry entry3{"orc", 2, 1.0f, 1.0f, 0};
   Data::EnemySpawnEntry entry4{"orc", 2, 1.5f, 1.0f, 2};
   wave2.enemies.push_back(entry3);
   wave2.enemies.push_back(entry4);
   stageDef.waves.push_back(wave2);
 
   // WaveManagerにステージをロード
   if (g_waveManager) {
     g_waveManager->LoadStage(stageDef);
     g_waveManager->StartNextWave(world);
   }
 
   // ゲーム開始
   if (g_gameStateManager) {
     g_gameStateManager->StartGame(world, *g_waveManager);
   }
 
   std::cout << "TDGameScene: Default stage loaded with "
             << stageDef.waves.size() << " waves\n";
 }

void TDGameScene::ProcessInput() {
  // ESCキー: ホームシーンに戻る
  if (IsKeyPressed(KEY_ESCAPE)) {
    nextScene_ = "Home";
    std::cout << "TDGameScene: ESC pressed, returning to Home\n";
  }

  // TODO: タワー配置やアップグレード入力の実装
}

 void TDGameScene::CheckGameEndCondition(Core::World &world,
                                         Core::GameContext &context) {
   if (!g_gameStateManager)
     return;
 
   auto phase = g_gameStateManager->GetPhase();
 
   if (phase == Domain::TD::Managers::GamePhase::Victory) {
     nextScene_ = "Home";
     std::cout << "TDGameScene: Victory!\n";
   } else if (phase == Domain::TD::Managers::GamePhase::Defeat) {
     nextScene_ = "Home";
     std::cout << "TDGameScene: Defeat!\n";
   }
 }

void TDGameScene::Update(Core::World &world, Core::GameContext &context,
                         float deltaTime) {
  if (!initialized_)
    return;

  elapsedTime_ += deltaTime;

  // 入力処理
  ProcessInput();

  // マネージャー更新
  if (g_waveManager) {
    g_waveManager->Update(world, context, deltaTime);
  }

  if (g_spawnManager) {
    g_spawnManager->Update(world, deltaTime);
  }

  if (g_gameStateManager) {
    g_gameStateManager->Update(world, *g_waveManager, deltaTime);
  }

  // ゲーム終了条件チェック
  CheckGameEndCondition(world, context);
}

void TDGameScene::Render(Core::World &world, Core::GameContext &context) {
  if (!initialized_)
    return;

  auto *renderer = context.TryGet<Core::GameRenderer>();
  if (!renderer) {
    DrawText("Renderer not available", 50, 50, 30, WHITE);
    return;
  }

  // 背景描画
  DrawRectangle(0, 0, 1920, 1080, DARKBLUE);

  // タイトル描画
  DrawText("TD Game", 50, 50, 48, WHITE);
  DrawText("Press ESC to return to home", 50, 120, 24, GRAY);

  // ゲーム状態表示
  DrawText("Mode: Tower Defense", 50, 200, 32, YELLOW);
  DrawText(("Elapsed: " + std::to_string(static_cast<int>(elapsedTime_)) + "s")
               .c_str(),
           50, 250, 24, WHITE);

   // Wave情報表示
   if (g_waveManager) {
     int waveNum = g_waveManager->GetCurrentWaveNumber();
     int totalWaves = g_waveManager->GetTotalWaves();
     std::string waveInfo =
         "Wave: " + std::to_string(waveNum) + "/" + std::to_string(totalWaves);
     DrawText(waveInfo.c_str(), 50, 300, 24, WHITE);
   }

  // ゲーム状態表示
  if (g_gameStateManager) {
    float baseHp = g_gameStateManager->GetBaseHealth();
    std::string hpInfo = "Base HP: " + std::to_string(static_cast<int>(baseHp));
    DrawText(hpInfo.c_str(), 50, 350, 24, WHITE);

    // HP バー表示
    DrawRectangle(50, 380, 200, 20, RED);
    DrawRectangle(50, 380, static_cast<int>(200.0f * baseHp / 100.0f), 20,
                  GREEN);
    DrawRectangleLines(50, 380, 200, 20, WHITE);
  }

   // エンティティ描画（簡易版）
   auto view = world.View<Core::Components::Position>();
   for (auto entity : view) {
     const auto &pos = view.get<Core::Components::Position>(entity);

     // 敵ユニットの描画
     if (world.HasAll<Domain::TD::Components::EnemyUnit>(entity)) {
       DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 15, RED);
     }
     // 味方ユニットの描画（AllyUnit コンポーネント）
     else if (world.HasAll<Domain::TD::Components::AllyUnit>(entity)) {
       DrawRectangle(static_cast<int>(pos.x - 15), static_cast<int>(pos.y - 15),
                     30, 30, BLUE);
     }
   }
}

void TDGameScene::Shutdown(Core::World &world, Core::GameContext &context) {
  if (!initialized_)
    return;

  std::cout << "TDGameScene: Shutting down...\n";

  // グローバル参照をクリア
  g_waveManager = nullptr;
  g_spawnManager = nullptr;
  g_gameStateManager = nullptr;

  initialized_ = false;
  nextScene_.clear();

  std::cout << "TDGameScene: Shutdown complete\n";
}

} // namespace Application
