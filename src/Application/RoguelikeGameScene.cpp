/**
 * @file RoguelikeGameScene.cpp
 * @brief ローグライクゲームシーン実装
 *
 * Phase 3: 既存実装統合（TD/Roguelike）
 */

#include "Application/RoguelikeGameScene.h"
#include "Core/GameContext.h"
#include "Core/GameRenderer.h"
#include "Core/World.h"
#include "Domain/Roguelike/Components/RoguelikeComponents.h"
#include "Domain/Roguelike/Managers/TurnManager.h"
#include "Domain/Roguelike/Systems/AISystem.h"
#include "Domain/Roguelike/Systems/ActionSystem.h"
#include "Domain/Roguelike/Systems/CombatSystem.h"
#include "Domain/Roguelike/Systems/FOVSystem.h"
#include "Domain/Roguelike/Systems/HungerSystem.h"
#include "Domain/Roguelike/Systems/InputSystem.h"
#include "Domain/Roguelike/Systems/ItemSystem.h"
#include "Domain/Roguelike/Systems/LevelUpSystem.h"

#include <iostream>
#include <random>
#include <raylib.h>

namespace Application {

// ローグライク用グローバル変数（GameContext との統合）
static Domain::Roguelike::Managers::TurnManager *g_turnManager = nullptr;
static Domain::Roguelike::Components::MapData *g_mapData = nullptr;
static entt::entity g_playerEntity = entt::null;

RoguelikeGameScene::RoguelikeGameScene()
    : nextScene_(""), initialized_(false), turnCount_(0) {}

void RoguelikeGameScene::Initialize(Core::World &world,
                                    Core::GameContext &context) {
  if (initialized_) {
    std::cout << "RoguelikeGameScene: Already initialized\n";
    return;
  }

  std::cout << "RoguelikeGameScene: Initializing...\n";

  // ターンマネージャーを GameContext に登録
  if (!context.Has<Domain::Roguelike::Managers::TurnManager>()) {
    context.Emplace<Domain::Roguelike::Managers::TurnManager>();
  }
  g_turnManager = context.TryGet<Domain::Roguelike::Managers::TurnManager>();

  // マップデータを初期化
  mapData_ = std::make_unique<Domain::Roguelike::Components::MapData>();
  mapData_->Initialize(80, 25); // 80x25 のダンジョン
  g_mapData = mapData_.get();

  // GameContext にマップデータを登録（後でアクセス可能に）
  context.Register<Domain::Roguelike::Components::MapData>(*mapData_);

  // ダンジョンを生成
  GenerateDungeon(world, context);

  // Roguelike固有システムを登録
  RegisterRoguelikeSystems(context);

  initialized_ = true;
  turnCount_ = 0;

  std::cout << "RoguelikeGameScene: Initialized successfully\n";
}

void RoguelikeGameScene::RegisterRoguelikeSystems(Core::GameContext &context) {
  // Note: SystemRunnerはUnifiedGameで既に存在するため、
  // システムはUpdate()内で静的関数として呼び出す

  std::cout << "RoguelikeGameScene: Roguelike systems ready for integration\n";
}

void RoguelikeGameScene::GenerateDungeon(Core::World &world,
                                         Core::GameContext &context) {
  if (!g_mapData)
    return;

  std::cout << "RoguelikeGameScene: Generating dungeon...\n";

  // 簡易ダンジョン生成: ランダムな部屋配置
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> roomDist(3, 8);
  std::uniform_int_distribution<int> posDist(2, 20);

  // 背景を全て壁で埋める
  for (int y = 0; y < g_mapData->height; ++y) {
    for (int x = 0; x < g_mapData->width; ++x) {
      g_mapData->At(x, y).type = Domain::Roguelike::Components::TileType::Wall;
    }
  }

  // ランダムに部屋を生成
  for (int i = 0; i < 5; ++i) {
    int roomW = roomDist(rng);
    int roomH = roomDist(rng);
    int roomX = posDist(rng) * 5;
    int roomY = posDist(rng) * 2;

    // 部屋の範囲外チェック
    if (roomX + roomW >= g_mapData->width - 2)
      continue;
    if (roomY + roomH >= g_mapData->height - 2)
      continue;

    // 床を配置
    for (int y = roomY; y < roomY + roomH; ++y) {
      for (int x = roomX; x < roomX + roomW; ++x) {
        g_mapData->At(x, y).type =
            Domain::Roguelike::Components::TileType::Floor;
      }
    }
  }

  // プレイヤーを配置（マップの最初の床タイルに）
  entt::entity player = entt::null;
  for (int y = 0; y < g_mapData->height; ++y) {
    for (int x = 0; x < g_mapData->width; ++x) {
      if (g_mapData->At(x, y).type ==
          Domain::Roguelike::Components::TileType::Floor) {
        // プレイヤーエンティティ作成
        player = world.Create();
        world.Emplace<Domain::Roguelike::Components::GridPosition>(player, x,
                                                                   y);
        world.Emplace<Domain::Roguelike::Components::TurnActor>(
            player, true); // isPlayer = true
        world.Emplace<Domain::Roguelike::Components::ActionCommand>(player);
        world.Emplace<Domain::Roguelike::Components::Stats>(player);

        g_mapData->At(x, y).occupant = player;
        g_playerEntity = player;
        std::cout << "Player placed at (" << x << ", " << y << ")\n";
        goto player_placed;
      }
    }
  }

player_placed:

  // 敵を配置（ランダムな床タイルに）
  int monsterCount = 3;
  for (int i = 0; i < monsterCount && i < 10; ++i) {
    int monsterX =
        std::uniform_int_distribution<int>(0, g_mapData->width - 1)(rng);
    int monsterY =
        std::uniform_int_distribution<int>(0, g_mapData->height - 1)(rng);

    if (g_mapData->At(monsterX, monsterY).type !=
        Domain::Roguelike::Components::TileType::Floor)
      continue;
    if (g_mapData->At(monsterX, monsterY).occupant != entt::null)
      continue;

    entt::entity monster = world.Create();
    world.Emplace<Domain::Roguelike::Components::GridPosition>(
        monster, monsterX, monsterY);
    world.Emplace<Domain::Roguelike::Components::TurnActor>(
        monster, false); // isPlayer = false
    world.Emplace<Domain::Roguelike::Components::ActionCommand>(monster);
    world.Emplace<Domain::Roguelike::Components::Stats>(monster);
    world.Emplace<Domain::Roguelike::Components::AI>(monster);

    g_mapData->At(monsterX, monsterY).occupant = monster;
  }

  std::cout << "RoguelikeGameScene: Dungeon generated with " << monsterCount
            << " monsters\n";
}

void RoguelikeGameScene::ProcessInput() {
  // ESCキー: ホームシーンに戻る
  if (IsKeyPressed(KEY_ESCAPE)) {
    nextScene_ = "Home";
    std::cout << "RoguelikeGameScene: ESC pressed, returning to Home\n";
  }
}

void RoguelikeGameScene::CheckGameEndCondition(Core::World &world,
                                               Core::GameContext &context) {
  if (!g_turnManager || g_playerEntity == entt::null)
    return;

  // プレイヤーHP確認
  auto *playerStats =
      world.TryGet<Domain::Roguelike::Components::Stats>(g_playerEntity);
  if (playerStats && playerStats->IsDead()) {
    g_turnManager->SetGameOver();
    nextScene_ = "Home"; // TODO: ゲームオーバー画面を実装
    std::cout << "RoguelikeGameScene: Game Over\n";
  }
}

void RoguelikeGameScene::Update(Core::World &world, Core::GameContext &context,
                                float deltaTime) {
  if (!initialized_)
    return;

  // 入力処理
  ProcessInput();

  // ターン管理更新
  if (g_turnManager) {
    auto state = g_turnManager->Update(world.Registry());

    // ターン処理
    switch (state) {
    case Domain::Roguelike::Managers::TurnManager::State::AwaitingInput: {
      // プレイヤー入力処理
      Domain::Roguelike::Systems::InputSystem::ProcessInput(world.Registry());
      break;
    }

    case Domain::Roguelike::Managers::TurnManager::State::ProcessingTurns: {
      // 現在のアクターの行動を実行
      entt::entity currentActor = g_turnManager->GetCurrentActor();
      if (currentActor != entt::null && world.Valid(currentActor)) {
        auto *cmd = world.TryGet<Domain::Roguelike::Components::ActionCommand>(
            currentActor);
        if (cmd &&
            cmd->type !=
                Domain::Roguelike::Components::ActionCommand::Type::None) {
          // 行動を実行
          Domain::Roguelike::Systems::ActionSystem::ExecuteAction(
              world.Registry(), *g_mapData, currentActor);
          g_turnManager->CompleteAction(world.Registry());
        } else if (world.HasAll<Domain::Roguelike::Components::AI>(
                       currentActor)) {
          // AI 敵の行動決定
          Domain::Roguelike::Systems::AISystem::DecideAction(
              world.Registry(), *g_mapData, currentActor, g_playerEntity);
          if (auto *aiCmd =
                  world.TryGet<Domain::Roguelike::Components::ActionCommand>(
                      currentActor)) {
            if (aiCmd->type !=
                Domain::Roguelike::Components::ActionCommand::Type::None) {
              Domain::Roguelike::Systems::ActionSystem::ExecuteAction(
                  world.Registry(), *g_mapData, currentActor);
              g_turnManager->CompleteAction(world.Registry());
            }
          }
        }
      }
      break;
    }

    case Domain::Roguelike::Managers::TurnManager::State::GameOver:
      CheckGameEndCondition(world, context);
      break;

    default:
      break;
    }
  }

  // ゲーム終了条件チェック
  CheckGameEndCondition(world, context);

  turnCount_ = g_turnManager ? g_turnManager->GetTurnCount() : 0;
}

void RoguelikeGameScene::Render(Core::World &world,
                                Core::GameContext &context) {
  if (!initialized_)
    return;

  auto *renderer = context.TryGet<Core::GameRenderer>();
  if (!renderer) {
    DrawText("Renderer not available", 50, 50, 30, WHITE);
    return;
  }

  // 背景描画
  DrawRectangle(0, 0, 1920, 1080, DARKGRAY);

  // タイトル描画
  DrawText("Roguelike Game", 50, 50, 48, WHITE);
  DrawText("Press ESC to return to home", 50, 120, 24, GRAY);

  // ゲーム状態表示
  DrawText("Mode: Roguelike", 50, 200, 32, YELLOW);
  DrawText(("Turn: " + std::to_string(turnCount_)).c_str(), 50, 250, 24, WHITE);

  if (g_mapData) {
    // マップ描画
    int gridSize = 30;
    int startX = 300;
    int startY = 50;

    for (int y = 0; y < g_mapData->height; ++y) {
      for (int x = 0; x < g_mapData->width; ++x) {
        const auto &tile = g_mapData->At(x, y);
        Color tileColor = DARKGRAY;

        switch (tile.type) {
        case Domain::Roguelike::Components::TileType::Floor:
          tileColor = DARKBLUE;
          break;
        case Domain::Roguelike::Components::TileType::Wall:
          tileColor = BLACK;
          break;
        default:
          tileColor = DARKGRAY;
        }

        DrawRectangle(startX + x * gridSize, startY + y * gridSize, gridSize,
                      gridSize, tileColor);
        DrawRectangleLines(startX + x * gridSize, startY + y * gridSize,
                           gridSize, gridSize, GRAY);
      }
    }

    // エンティティ描画
    auto view = world.View<Domain::Roguelike::Components::GridPosition>();
    for (auto entity : view) {
      const auto &pos =
          view.get<Domain::Roguelike::Components::GridPosition>(entity);
      Color entityColor = YELLOW;

      if (world.HasAll<Domain::Roguelike::Components::AI>(entity)) {
        entityColor = RED; // 敵
      } else if (world.HasAll<Domain::Roguelike::Components::TurnActor>(
                     entity)) {
        auto *actor =
            world.TryGet<Domain::Roguelike::Components::TurnActor>(entity);
        if (actor && actor->isPlayer) {
          entityColor = WHITE; // プレイヤー
        }
      }

      DrawCircle(startX + pos.x * gridSize + gridSize / 2,
                 startY + pos.y * gridSize + gridSize / 2, gridSize / 3,
                 entityColor);
    }
  }

  // UI情報描画
  if (g_turnManager) {
    std::string stateStr = "State: ";
    switch (g_turnManager->GetState()) {
    case Domain::Roguelike::Managers::TurnManager::State::AwaitingInput:
      stateStr += "Awaiting Input";
      break;
    case Domain::Roguelike::Managers::TurnManager::State::ProcessingTurns:
      stateStr += "Processing Turns";
      break;
    case Domain::Roguelike::Managers::TurnManager::State::GameOver:
      stateStr += "Game Over";
      break;
    default:
      stateStr += "Unknown";
    }
    DrawText(stateStr.c_str(), 50, 300, 20, WHITE);
  }
}

void RoguelikeGameScene::Shutdown(Core::World &world,
                                  Core::GameContext &context) {
  if (!initialized_)
    return;

  std::cout << "RoguelikeGameScene: Shutting down...\n";

  // MapDataをクリア（メモリ解放）
  mapData_.reset();
  
  // グローバル参照をクリア
  g_turnManager = nullptr;
  g_mapData = nullptr;
  g_playerEntity = entt::null;

  initialized_ = false;
  nextScene_.clear();
  turnCount_ = 0;

  std::cout << "RoguelikeGameScene: Shutdown complete\n";
}

} // namespace Application
