#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <string>

#include "Core/GameContext.h"
#include "Core/Systems/ISystem.h"
#include "Core/TraceCompat.h"
#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/DeckLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "raylib.h"

#include <entt/entt.hpp>
#include <iterator>
#include <string>

#include "ComponentsNew.h"
#include "Core/GameContext.h"
#include "Core/Systems/ISystem.h"
#include "Core/TraceCompat.h"
#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/DeckLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "new/Domain/TD/LineTD/Components/AttackEventQueue.h"
#include "new/Domain/TD/LineTD/Components/BaseArrivalQueue.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Domain/TD/LineTD/Components/SpawnQueue.h"
#include "new/Domain/TD/LineTD/Managers/WaveManager.h"
#include "new/Domain/TD/LineTD/Systems/BaseArrivalSystem.h"
#include "new/Domain/TD/LineTD/Systems/CombatResolveSystem.h"
#include "new/Domain/TD/LineTD/Systems/CostSystem.h"
#include "new/Domain/TD/LineTD/Systems/DeathCheckSystem.h"
#include "new/Domain/TD/LineTD/Systems/FrontlineSystem.h"
#include "new/Domain/TD/LineTD/Systems/LaneMovementSystem.h"
#include "new/Domain/TD/LineTD/Systems/LineTDSpawnSystem.h"
#include "new/Domain/TD/LineTD/Systems/PlacementSystem.h"
#include "new/Domain/TD/LineTD/Systems/SimpleCombatSystem.h"
#include "new/Domain/TD/LineTD/Systems/WaveSchedulerSystem.h"
#include "new/Game/Components/GameState.h"
#include "new/Game/Components/Placement.h"
#include "new/Game/Components/UnitPreset.h"
#include "new/Game/Systems/HudSystem.h"
#include "new/Game/Systems/RenderSystem.h"

namespace {

using New::Game::Components::UnitPreset;

struct Resolution {
  int width;
  int height;
};

Resolution ChooseInitialResolution() {
  // 許容リストは大きい順に評価し、モニタに収まる最大を選択
  constexpr Resolution allowed[]{
      {1920, 1080},
      {1600, 900},
      {1280, 720},
  };

  const int monitor = GetCurrentMonitor();
  const int monitorWidth = GetMonitorWidth(monitor);
  const int monitorHeight = GetMonitorHeight(monitor);

  for (const auto &res : allowed) {
    if (res.width <= monitorWidth && res.height <= monitorHeight) {
      return res;
    }
  }
  return allowed[std::size(allowed) - 1];
}

bool LoadDefinitions(New::Core::GameContext &context, const char *entityPath,
                     const char *wavePath, const char *abilityPath,
                     const char *stagePath, const char *deckPath) {
  using namespace New::Data::Loaders;
  auto &registry = context.GetDefinitionRegistry();
  registry.Clear();

  EntityLoader entityLoader;
  bool ok = true;
  if (!entityLoader.LoadFromFile(entityPath)) {
    ok = false;
  }
  if (!entityLoader.RegisterTo(registry)) {
    ok = false;
  }

  WaveLoader waveLoader;
  if (!waveLoader.LoadFromFile(wavePath)) {
    ok = false;
  }
  if (!waveLoader.RegisterTo(registry)) {
    ok = false;
  }

  AbilityLoader abilityLoader;
  if (!abilityLoader.LoadFromFile(abilityPath)) {
    ok = false;
  }
  if (!abilityLoader.RegisterTo(registry)) {
    ok = false;
  }

  StageLoader stageLoader;
  if (!stageLoader.LoadFromFile(stagePath)) {
    ok = false;
  }
  if (!stageLoader.RegisterTo(registry)) {
    ok = false;
  }

  DeckLoader deckLoader;
  if (!deckLoader.LoadFromFile(deckPath)) {
    ok = false;
  }
  if (!deckLoader.RegisterTo(registry)) {
    ok = false;
  }

  return ok;
}

void LogDefinitions(const New::Core::GameContext &context) {
  const auto &registry = context.GetDefinitionRegistry();

  auto logEntity = [&](const std::string &id) {
    const auto *e = registry.GetEntity(id);
    if (e) {
      TRACELOG(LOG_INFO, "[Entity] id=%s name=%s health=%d", e->id.c_str(),
               e->name.c_str(), e->health);
    } else {
      TRACELOG(LOG_WARNING, "[Entity] id=%s not found", id.c_str());
    }
  };

  auto logWave = [&](const std::string &id) {
    const auto *w = registry.GetWave(id);
    if (w) {
      TRACELOG(LOG_INFO, "[Wave] id=%s entries=%zu", w->id.c_str(),
               w->entries.size());
      for (size_t i = 0; i < w->entries.size(); ++i) {
        const auto &entry = w->entries[i];
        TRACELOG(LOG_INFO, "  [%zu] enemy=%s delay=%.2f", i,
                 entry.enemyId.c_str(), entry.startDelaySeconds);
      }
    } else {
      TRACELOG(LOG_WARNING, "[Wave] id=%s not found", id.c_str());
    }
  };

  logEntity("enemy_slime");
  logEntity("enemy_goblin");
  logEntity("fallback_entity");

  logWave("wave_1");
  logWave("fallback_wave");

  auto logAbility = [&](const std::string &id) {
    const auto *a = registry.GetAbility(id);
    if (a) {
      TRACELOG(LOG_INFO, "[Ability] id=%s name=%s cost=%d cooldown=%.2f",
               a->id.c_str(), a->name.c_str(), a->cost, a->cooldown);
    } else {
      TRACELOG(LOG_WARNING, "[Ability] id=%s not found", id.c_str());
    }
  };

  logAbility("fireball");
  logAbility("heal");
  logAbility("fallback_ability");
}

class BackgroundGridSystem : public New::Core::ISystem {
public:
  int GetRenderPriority() const override { return 10; }
  std::string GetName() const override { return "BackgroundGridSystem"; }

  void Render(New::Core::GameContext &context) override {
    const int vw = context.GetRenderer().GetVirtualWidth();
    const int vh = context.GetRenderer().GetVirtualHeight();
    const Color lineColor = ColorAlpha(GRAY, 0.25f);
    for (int x = 0; x <= vw; x += 160) {
      DrawLine(x, 0, x, vh, lineColor);
    }
    for (int y = 0; y <= vh; y += 120) {
      DrawLine(0, y, vw, y, lineColor);
    }
  }
};

// Placement request is now stored as a component on battle_state.

class DebugHudSystem : public New::Core::ISystem {
public:
  int GetRenderPriority() const override { return 90; }
  std::string GetName() const override { return "DebugHudSystem"; }

  void Render(New::Core::GameContext &context) override {
    const auto &renderer = context.GetRenderer();
    const auto &input = context.GetInputManager();
    const auto &resources = context.GetResourceManager();
    auto stateView =
        context.GetWorld().View<New::Game::Components::BattleState>();
    const Vector2 mouseScreen = GetMousePosition();
    const Vector2 mouseVirtual = input.ScreenToVirtual(mouseScreen);
    const int fps = GetFPS();

    DrawRectangle(20, 20, 460, 100, ColorAlpha(BLACK, 0.35f));
    DrawRectangleLines(20, 20, 460, 100, ColorAlpha(LIGHTGRAY, 0.6f));

    const bool hasFont = resources.HasDefaultFont();
    const Font *font = hasFont ? &resources.GetDefaultFont() : nullptr;
    const float fontSize = 22.0f;
    const float spacing = 2.0f;

    auto draw = [&](const char *text, float x, float y) {
      if (font) {
        DrawTextEx(*font, text, {x, y}, fontSize, spacing, RAYWHITE);
      } else {
        DrawText(text, static_cast<int>(x), static_cast<int>(y),
                 static_cast<int>(fontSize), RAYWHITE);
      }
    };

    draw(TextFormat("FPS: %d", fps), 36.0f, 32.0f);
    draw(TextFormat("Virtual Mouse: (%.1f, %.1f)", mouseVirtual.x,
                    mouseVirtual.y),
         36.0f, 60.0f);
    draw(TextFormat("Virtual Size: %d x %d", renderer.GetVirtualWidth(),
                    renderer.GetVirtualHeight()),
         36.0f, 86.0f);
    draw(hasFont ? "Font: NotoSansJP (OK)" : "Font: default (fallback)", 36.0f,
         112.0f);
    if (!stateView.empty()) {
      const auto &state =
          stateView.get<New::Game::Components::BattleState>(*stateView.begin());
      if (state.messageTtl > 0.0f && !state.lastMessage.empty()) {
        draw(state.lastMessage.c_str(), 36.0f, 140.0f);
      }
      draw(TextFormat("MinGap: %.0f  Iter: %d  KB(Debug): %.1f", state.minGap,
                      state.frontlineIterations, state.debugKnockback),
           36.0f, 168.0f);
    }
  }
};

class PlacementPreviewSystem : public New::Core::ISystem {
public:
  PlacementPreviewSystem(const New::Data::StageDef *stage,
                         const std::array<UnitPreset, 10> &deckSlots)
      : stage_(stage), deckSlots_(deckSlots) {}

  int GetRenderPriority() const override { return 75; }
  std::string GetName() const override { return "PlacementPreviewSystem"; }

  void Render(New::Core::GameContext &context) override {
    if (!stage_) {
      return;
    }
    auto &world = context.GetWorld();
    auto stateView = world.View<New::Game::Components::BattleState>();
    if (stateView.empty()) {
      return;
    }
    auto &state =
        stateView.get<New::Game::Components::BattleState>(*stateView.begin());
    if (state.victory || state.defeat) {
      return;
    }

    const Vector2 mouseVirtual =
        context.GetInputManager().ScreenToVirtual(GetMousePosition());

    const float laneYDefault = context.GetRenderer().GetVirtualHeight() * 0.5f;
    const auto laneItHighlight =
        std::min_element(stage_->lanes.begin(), stage_->lanes.end(),
                         [&](const auto &a, const auto &b) {
                           return std::abs(a.y - mouseVirtual.y) <
                                  std::abs(b.y - mouseVirtual.y);
                         });
    const bool hasLane = laneItHighlight != stage_->lanes.end();
    const float laneY = hasLane ? laneItHighlight->y : laneYDefault;
    const float laneStartX = hasLane ? laneItHighlight->startX : 0.0f;
    const float laneEndX =
        hasLane ? laneItHighlight->endX
                : static_cast<float>(context.GetRenderer().GetVirtualWidth());

    const int slot = std::clamp(state.selectedSlot, 0, 9);
    const UnitPreset &preset = deckSlots_[slot];

    // マウスYに最も近いレーンをハイライト
    if (hasLane) {
      const auto &lane = *laneItHighlight;
      const float laneHeight = 48.0f; // 簡易ハイライト幅
      DrawRectangle(lane.startX, lane.y - laneHeight * 0.5f,
                    lane.endX - lane.startX, laneHeight,
                    ColorAlpha(SKYBLUE, 0.15f));
      DrawLine(lane.startX, lane.y, lane.endX, lane.y, ColorAlpha(BLUE, 0.35f));
    }

    const float spawnOffset = 32.0f;
    const float spawnX =
        !stage_->lanes.empty()
            ? std::max(laneStartX, laneEndX - spawnOffset)
            : (context.GetRenderer().GetVirtualWidth() - spawnOffset);
    const float spawnY = laneY;

    // 重なりチェック（簡易）
    const float minDistance = 48.0f;
    bool blocked = false;
    auto allyView = world.View<New::Domain::TD::LineTD::Components::Team,
                               New::Domain::TD::LineTD::Components::Lane,
                               New::Core::Components::Transform>();
    for (auto e : allyView) {
      const auto &team =
          allyView.get<New::Domain::TD::LineTD::Components::Team>(e);
      if (team.side != New::Domain::TD::LineTD::Components::TeamSide::Player) {
        continue;
      }
      const auto &lane =
          allyView.get<New::Domain::TD::LineTD::Components::Lane>(e);
      const auto &tr = allyView.get<New::Core::Components::Transform>(e);
      const float endX = !stage_->lanes.empty()
                             ? stage_->lanes.front().endX
                             : context.GetRenderer().GetVirtualWidth();
      if (std::abs(tr.x - endX) < minDistance) {
        blocked = true;
        break;
      }
    }

    const bool cooldown = state.slotCooldowns[slot] > 0.0f;
    const bool costOk = state.cost >= preset.cost;
    Color ghostColor = (!blocked && !cooldown && costOk)
                           ? ColorAlpha(SKYBLUE, 0.35f)
                           : ColorAlpha(RED, 0.35f);
    Color lineColor = (!blocked && !cooldown && costOk)
                          ? ColorAlpha(GREEN, 0.35f)
                          : ColorAlpha(RED, 0.35f);

    const int vw = context.GetRenderer().GetVirtualWidth();
    DrawLine(0, static_cast<int>(spawnY), vw, static_cast<int>(spawnY),
             lineColor);
    DrawRectangle(static_cast<int>(spawnX - 32), static_cast<int>(spawnY - 32),
                  64, 64, ghostColor);
    DrawRectangleLines(static_cast<int>(spawnX - 32),
                       static_cast<int>(spawnY - 32), 64, 64,
                       ColorAlpha(RAYWHITE, 0.6f));
    // 射程の可視化（現在の選択スロット）
    DrawCircleLines(static_cast<int>(spawnX), static_cast<int>(spawnY),
                    preset.attackRange, ColorAlpha(BLUE, 0.4f));

    // プレビューにクールダウン/コストを重ねて表示
    DrawRectangle(spawnX - 70, spawnY - 96, 140, 40, ColorAlpha(BLACK, 0.4f));
    DrawRectangleLines(spawnX - 70, spawnY - 96, 140, 40,
                       ColorAlpha(RAYWHITE, 0.7f));
    DrawText(TextFormat("Cost %d", preset.cost), static_cast<int>(spawnX - 62),
             static_cast<int>(spawnY - 90), 16, YELLOW);
    DrawText(TextFormat("CD %.1fs", state.slotCooldowns[slot]),
             static_cast<int>(spawnX - 62), static_cast<int>(spawnY - 72), 16,
             state.slotCooldowns[slot] > 0.0f ? SKYBLUE : LIGHTGRAY);

    // クリック判定の可視化（スロットパネルとマウス位置）
    {
      const float uiScale = 1.2f;
      const float slotSize = 82.0f * uiScale;
      const float slotSpacing = 10.0f * uiScale;
      const float slotPanelW = slotSize * 5 + slotSpacing * 4;
      const float slotPanelH = slotSize * 2 + slotSpacing * 1;
      const float slotPanelX = context.GetRenderer().GetVirtualWidth() -
                               slotPanelW - 20.0f * uiScale;
      const float slotPanelY = context.GetRenderer().GetVirtualHeight() -
                               slotPanelH - 20.0f * uiScale;

      DrawRectangleLines(slotPanelX, slotPanelY, slotPanelW, slotPanelH,
                         ColorAlpha(YELLOW, 0.6f));
      const float relX = mouseVirtual.x - slotPanelX;
      const float relY = mouseVirtual.y - slotPanelY;
      const int col = static_cast<int>(relX / (slotSize + slotSpacing));
      const int row = static_cast<int>(relY / (slotSize + slotSpacing));
      if (col >= 0 && col < 5 && row >= 0 && row < 2) {
        const float cellX = slotPanelX + col * (slotSize + slotSpacing);
        const float cellY = slotPanelY + row * (slotSize + slotSpacing);
        DrawRectangleLines(cellX, cellY, slotSize, slotSize,
                           ColorAlpha(GREEN, 0.7f));
      }
      // マウス位置のクロスヘア
      DrawLine(mouseVirtual.x - 6, mouseVirtual.y, mouseVirtual.x + 6,
               mouseVirtual.y, ColorAlpha(LIGHTGRAY, 0.6f));
      DrawLine(mouseVirtual.x, mouseVirtual.y - 6, mouseVirtual.x,
               mouseVirtual.y + 6, ColorAlpha(LIGHTGRAY, 0.6f));
    }
  }

private:
  const New::Data::StageDef *stage_;
  const std::array<UnitPreset, 10> deckSlots_;
};

} // namespace

enum class DataMode { Sample, Debug };

int main() {
  constexpr DataMode dataMode = DataMode::Sample;

  // ウィンドウは固定でFHD（1920x1080）にする
  const Resolution initialResolution{1920, 1080};

  // リサイズ禁止（FLAG_WINDOW_RESIZABLEは付けない）。変更は将来の設定メニューで
  // SetWindowSize → input.UpdateScreenSize を呼ぶ方針。
  InitWindow(initialResolution.width, initialResolution.height,
             "SimpleTDCGame - NewArchNext");
  SetWindowMinSize(initialResolution.width, initialResolution.height);
  SetTargetFPS(60);

  New::Core::GameContext context;
  if (!context.Initialize()) {
    CloseWindow();
    return -1;
  }

  const char *entityPath = "assets/definitions/entities_sample.json";
  const char *wavePath = "assets/definitions/waves_sample.json";
  const char *abilityPath = "assets/definitions/abilities_sample.json";
  const char *stagePath = "assets/definitions/stages_sample.json";
  const char *deckPath = "assets/definitions/deck_sample.json";
  if (dataMode == DataMode::Debug) {
    TRACELOG(LOG_WARNING, "DataMode: DEBUG (using *_debug.json)");
    entityPath = "assets/definitions/entities_debug.json";
    wavePath = "assets/definitions/waves_debug.json";
    abilityPath = "assets/definitions/abilities_debug.json";
    stagePath = "assets/definitions/stages_debug.json";
    deckPath = "assets/definitions/deck_debug.json";
  } else {
    TRACELOG(LOG_INFO, "DataMode: SAMPLE");
  }

  const bool dataLoaded = LoadDefinitions(context, entityPath, wavePath,
                                          abilityPath, stagePath, deckPath);
  TRACELOG(LOG_INFO, "Data loaded: %s", dataLoaded ? "OK" : "With fallback");
  LogDefinitions(context);

  auto &registry = context.GetDefinitionRegistry();
  const New::Data::StageDef *stage = registry.GetStage(
      dataMode == DataMode::Debug ? "stage_debug" : "stage_sample");
  if (!stage) {
    stage = registry.GetStage("fallback_stage");
  }
  if (!stage) {
    TRACELOG(LOG_ERROR, "No stage definition found");
    context.Shutdown();
    CloseWindow();
    return -1;
  }

  New::Domain::TD::LineTD::Managers::WaveManager waveManager;
  waveManager.SetStage(stage);
  int currentWaveIndex = 0;
  const auto resolveWave = [&](int idx) -> const New::Data::WaveDef * {
    if (idx < 0 || idx >= static_cast<int>(stage->waves.size())) {
      return nullptr;
    }
    return registry.GetWave(stage->waves[idx]);
  };
  const New::Data::WaveDef *currentWave = resolveWave(currentWaveIndex);
  if (!currentWave) {
    currentWave = registry.GetWave("fallback_wave");
  }
  waveManager.SetWave(currentWave);

  // 10スロットのフォールバックデッキ（メイン3 + アビリティ2 + サブ5）
  const std::array<UnitPreset, 10> fallbackDeck{{
      /*Slot0-2: メイン*/ UnitPreset{30, 180.0f, 20.0f, 260.0f, 0.9f, 0.8f,
                                     24.0f, "single", 1},
      /*Slot1*/
      UnitPreset{40, 220.0f, 24.0f, 260.0f, 1.0f, 1.0f, 26.0f, "single", 1},
      /*Slot2*/
      UnitPreset{50, 180.0f, 30.0f, 280.0f, 1.1f, 1.2f, 28.0f, "multi", 2},
      /*Slot3-4: アビリティ*/
      UnitPreset{60, 260.0f, 22.0f, 240.0f, 0.8f, 1.4f, 22.0f, "single", 1},
      /*Slot4*/
      UnitPreset{80, 320.0f, 28.0f, 300.0f, 1.3f, 1.6f, 30.0f, "pierce", 3},
      /*Slot5-9: サブ*/
      UnitPreset{35, 200.0f, 22.0f, 250.0f, 0.95f, 0.9f, 25.0f, "single", 1},
      /*Slot6*/
      UnitPreset{45, 240.0f, 26.0f, 270.0f, 1.05f, 1.1f, 27.0f, "multi", 2},
      /*Slot7*/
      UnitPreset{55, 190.0f, 28.0f, 290.0f, 1.15f, 1.3f, 29.0f, "single", 1},
      /*Slot8*/
      UnitPreset{65, 280.0f, 24.0f, 250.0f, 0.85f, 1.5f, 23.0f, "pierce", 2},
      /*Slot9*/
      UnitPreset{75, 300.0f, 30.0f, 310.0f, 1.25f, 1.7f, 31.0f, "multi", 3},
  }};

  auto applyDeckFromRegistry = [&](const New::Data::DefinitionRegistry &reg,
                                   const std::string &deckId,
                                   std::array<UnitPreset, 10> &outDeck) {
    outDeck = fallbackDeck;
    const auto *deck = reg.GetDeck(deckId);
    if (!deck) {
      deck = reg.GetDeck("fallback_deck");
    }
    if (!deck) {
      return;
    }
    const size_t count = std::min(deck->slots.size(), outDeck.size());
    for (size_t i = 0; i < count; ++i) {
      const auto &slot = deck->slots[i];
      outDeck[i] =
          UnitPreset{slot.cost,        slot.health,         slot.attackDamage,
                     slot.attackRange, slot.attackCooldown, slot.spawnCooldown,
                     slot.knockback,   slot.attackType,     slot.hitCount};
    }
    for (size_t i = count; i < outDeck.size(); ++i) {
      outDeck[i] = fallbackDeck[i];
    }
  };

  std::array<UnitPreset, 10> deckSlots = fallbackDeck;
  applyDeckFromRegistry(
      registry, dataMode == DataMode::Debug ? "deck_debug" : "deck_sample",
      deckSlots);

  // Battle state entity
  auto &world = context.GetWorld();
  const entt::entity stateEntity = world.CreateEntity("battle_state");
  New::Game::Components::BattleState state{};
  state.playerLife = stage->playerLife;
  state.cost = stage->startingCost;
  state.selectedSlot = 0;
  state.selectedSlotCost = deckSlots[0].cost;
  state.waveIndex = currentWaveIndex;
  state.totalWaves = static_cast<int>(stage->waves.size());
  // デバッグしやすいようにコスト回復を高速化
  state.costRegenPerSec = 15.0f;
  state.waveBonusCost = 50;
  // Stage由来の到達ダメージ/撃破報酬を反映
  state.baseArrivalDamage = std::max(1, stage->baseArrivalDamage);
  state.killReward = std::max(0, stage->killReward);
  state.minGap = stage->minGap > 0.0f ? stage->minGap : 80.0f;
  state.frontlineIterations = std::max(1, stage->frontlineIterations);
  for (int i = 0; i < 10; ++i) {
    state.slotCosts[i] = deckSlots[i].cost;
  }
  world.Add<New::Game::Components::BattleState>(stateEntity, state);
  world.Add<New::Domain::TD::LineTD::Components::BaseArrivalQueue>(
      stateEntity, New::Domain::TD::LineTD::Components::BaseArrivalQueue{});
  world.Add<New::Domain::TD::LineTD::Components::SpawnQueue>(
      stateEntity, New::Domain::TD::LineTD::Components::SpawnQueue{});
  world.Add<New::Domain::TD::LineTD::Components::AttackEventQueue>(
      stateEntity, New::Domain::TD::LineTD::Components::AttackEventQueue{});
  world.Add<New::Game::Components::PlacementRequest>(
      stateEntity, New::Game::Components::PlacementRequest{});

  auto &renderer = context.GetRenderer();
  auto &input = context.GetInputManager();
  auto &systems = context.GetSystemRunner();

  systems.AddSystem<BackgroundGridSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::CostSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::PlacementSystem>(
      stage, deckSlots);
  systems.AddSystem<New::Domain::TD::LineTD::Systems::WaveSchedulerSystem>(
      waveManager);
  systems.AddSystem<New::Domain::TD::LineTD::Systems::LineTDSpawnSystem>(
      waveManager, *stage, registry);
  systems.AddSystem<New::Domain::TD::LineTD::Systems::LaneMovementSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::FrontlineSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::SimpleCombatSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::CombatResolveSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::BaseArrivalSystem>();
  systems.AddSystem<New::Domain::TD::LineTD::Systems::DeathCheckSystem>();
  systems.AddSystem<PlacementPreviewSystem>(stage, deckSlots);
  systems.AddSystem<New::Game::Systems::RenderSystem>();
  systems.AddSystem<New::Game::Systems::HudSystem>();
  systems.AddSystem<DebugHudSystem>();

  if (!systems.Initialize(context)) {
    context.Shutdown();
    CloseWindow();
    return -1;
  }

  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();
    // 入力は常に現在の実解像度を反映し、仮想座標変換を維持
    input.UpdateScreenSize(GetScreenWidth(), GetScreenHeight());

    // デバッグ調整（到達ダメージ / 撃破報酬 / 前線距離 / ノックバック上書き）
    {
      auto stateView = world.View<New::Game::Components::BattleState>();
      if (!stateView.empty()) {
        auto &s = stateView.get<New::Game::Components::BattleState>(
            *stateView.begin());
        if (IsKeyPressed(KEY_F6)) {
          s.baseArrivalDamage = std::max(0, s.baseArrivalDamage - 1);
          s.lastMessage = TextFormat("BaseDmg -> %d", s.baseArrivalDamage);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F7)) {
          s.baseArrivalDamage = std::min(99, s.baseArrivalDamage + 1);
          s.lastMessage = TextFormat("BaseDmg -> %d", s.baseArrivalDamage);
          s.messageTtl = 1.2f;
        }
        if (IsKeyPressed(KEY_F8)) {
          s.killReward = std::max(0, s.killReward - 1);
          s.lastMessage = TextFormat("KillReward -> %d", s.killReward);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F9)) {
          s.killReward = std::min(999, s.killReward + 1);
          s.lastMessage = TextFormat("KillReward -> %d", s.killReward);
          s.messageTtl = 1.2f;
        }
        if (IsKeyPressed(KEY_F10)) {
          s.minGap = std::max(32.0f, s.minGap - 4.0f);
          s.lastMessage = TextFormat("MinGap -> %.0f", s.minGap);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F11)) {
          s.minGap = std::min(160.0f, s.minGap + 4.0f);
          s.lastMessage = TextFormat("MinGap -> %.0f", s.minGap);
          s.messageTtl = 1.2f;
        }
        if (IsKeyPressed(KEY_F12)) {
          s.debugKnockback = std::max(0.0f, s.debugKnockback - 2.0f);
          s.lastMessage = TextFormat("KB(Debug) -> %.1f", s.debugKnockback);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F1)) {
          s.debugKnockback = std::min(60.0f, s.debugKnockback + 2.0f);
          s.lastMessage = TextFormat("KB(Debug) -> %.1f", s.debugKnockback);
          s.messageTtl = 1.2f;
        }
      }
    }

    systems.Update(context, dt);

    // デバッグ調整（到達ダメージ / 撃破報酬）
    {
      auto stateView = world.View<New::Game::Components::BattleState>();
      if (!stateView.empty()) {
        auto &s = stateView.get<New::Game::Components::BattleState>(
            *stateView.begin());
        if (IsKeyPressed(KEY_F6)) {
          s.baseArrivalDamage = std::max(0, s.baseArrivalDamage - 1);
          s.lastMessage = TextFormat("BaseDmg -> %d", s.baseArrivalDamage);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F7)) {
          s.baseArrivalDamage = std::min(99, s.baseArrivalDamage + 1);
          s.lastMessage = TextFormat("BaseDmg -> %d", s.baseArrivalDamage);
          s.messageTtl = 1.2f;
        }
        if (IsKeyPressed(KEY_F8)) {
          s.killReward = std::max(0, s.killReward - 1);
          s.lastMessage = TextFormat("KillReward -> %d", s.killReward);
          s.messageTtl = 1.2f;
        } else if (IsKeyPressed(KEY_F9)) {
          s.killReward = std::min(999, s.killReward + 1);
          s.lastMessage = TextFormat("KillReward -> %d", s.killReward);
          s.messageTtl = 1.2f;
        }
      }
    }

    // ホットキー選択（0-9）
    {
      int slotChange = -1;
      if (IsKeyPressed(KEY_ZERO))
        slotChange = 0;
      else if (IsKeyPressed(KEY_ONE))
        slotChange = 1;
      else if (IsKeyPressed(KEY_TWO))
        slotChange = 2;
      else if (IsKeyPressed(KEY_THREE))
        slotChange = 3;
      else if (IsKeyPressed(KEY_FOUR))
        slotChange = 4;
      else if (IsKeyPressed(KEY_FIVE))
        slotChange = 5;
      else if (IsKeyPressed(KEY_SIX))
        slotChange = 6;
      else if (IsKeyPressed(KEY_SEVEN))
        slotChange = 7;
      else if (IsKeyPressed(KEY_EIGHT))
        slotChange = 8;
      else if (IsKeyPressed(KEY_NINE))
        slotChange = 9;

      if (slotChange >= 0) {
        auto stateView = world.View<New::Game::Components::BattleState>();
        if (!stateView.empty()) {
          auto &s = stateView.get<New::Game::Components::BattleState>(
              *stateView.begin());
          s.selectedSlot = slotChange;
          s.selectedSlotCost = deckSlots[slotChange].cost;
          s.lastMessage = TextFormat("Slot %d selected", slotChange + 1);
          s.messageTtl = 1.5f;
        }
      }
      // データ再読込 (Deck/Stage/Wave/Entity/Ability)
      if (IsKeyPressed(KEY_F2)) {
        // 戦闘中リロードはシステムが古いステージ参照を保持してクラッシュしやすいので抑止
        auto stateView = world.View<New::Game::Components::BattleState>();
        if (!stateView.empty()) {
          auto &s = stateView.get<New::Game::Components::BattleState>(
              *stateView.begin());
          if (!s.victory && !s.defeat) {
            s.lastMessage = "Reload disabled during battle";
            s.messageTtl = 1.5f;
            goto render_loop_continue;
          }
        }

        const char *entityPath =
            dataMode == DataMode::Debug
                ? "assets/definitions/entities_debug.json"
                : "assets/definitions/entities_sample.json";
        const char *wavePath = dataMode == DataMode::Debug
                                   ? "assets/definitions/waves_debug.json"
                                   : "assets/definitions/waves_sample.json";
        const char *abilityPath =
            dataMode == DataMode::Debug
                ? "assets/definitions/abilities_debug.json"
                : "assets/definitions/abilities_sample.json";
        const char *stagePath = dataMode == DataMode::Debug
                                    ? "assets/definitions/stages_debug.json"
                                    : "assets/definitions/stages_sample.json";
        const char *deckPath = dataMode == DataMode::Debug
                                   ? "assets/definitions/deck_debug.json"
                                   : "assets/definitions/deck_sample.json";

        const bool reloaded = LoadDefinitions(context, entityPath, wavePath,
                                              abilityPath, stagePath, deckPath);
        auto &registry = context.GetDefinitionRegistry();
        const New::Data::StageDef *stageNew = registry.GetStage(
            dataMode == DataMode::Debug ? "stage_debug" : "stage_sample");
        if (!stageNew) {
          stageNew = registry.GetStage("fallback_stage");
        }
        if (reloaded && stageNew) {
          applyDeckFromRegistry(registry,
                                dataMode == DataMode::Debug ? "deck_debug"
                                                            : "deck_sample",
                                deckSlots);
          // BattleState を再反映（コスト/HPは既存を維持）
          auto stateView = world.View<New::Game::Components::BattleState>();
          if (!stateView.empty()) {
            auto &s = stateView.get<New::Game::Components::BattleState>(
                *stateView.begin());
            s.baseArrivalDamage = std::max(1, stageNew->baseArrivalDamage);
            s.killReward = std::max(0, stageNew->killReward);
            s.minGap = stageNew->minGap > 0.0f ? stageNew->minGap : s.minGap;
            s.frontlineIterations =
                std::max(1, stageNew->frontlineIterations > 0
                                ? stageNew->frontlineIterations
                                : s.frontlineIterations);
            for (int i = 0; i < 10; ++i) {
              s.slotCosts[i] = deckSlots[i].cost;
            }
            const int sel = std::clamp(s.selectedSlot, 0, 9);
            s.selectedSlotCost = deckSlots[sel].cost;
          }
          // set lastMessage on BattleState instead of legacy placementMessage
          {
            auto stateView2 = world.View<New::Game::Components::BattleState>();
            if (stateView2.begin() != stateView2.end()) {
              auto &s2 = stateView2.get<New::Game::Components::BattleState>(
                  *stateView2.begin());
              s2.lastMessage = "Reloaded definitions";
              s2.messageTtl = 1.5f;
            }
          }
        } else {
          auto stateView3 = world.View<New::Game::Components::BattleState>();
          if (stateView3.begin() != stateView3.end()) {
            auto &s3 = stateView3.get<New::Game::Components::BattleState>(
                *stateView3.begin());
            s3.lastMessage = "Reload failed (see log)";
            s3.messageTtl = 2.0f;
          }
        }
      }
    }

    // マウス入力による設置予約とスロット選択
    {
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        const Vector2 mouseScreen = GetMousePosition();
        const Vector2 mouseVirtual = input.ScreenToVirtual(mouseScreen);

        // スロットUIのクリック判定（右下の5*2グリッド）
        const int vw = renderer.GetVirtualWidth();
        const int vh = renderer.GetVirtualHeight();
        // HudSystem側の値と揃える
        const float uiScale = 1.2f;
        const float slotSize = 82.0f * uiScale;
        const float slotSpacing = 10.0f * uiScale;
        const float slotPanelW = slotSize * 5 + slotSpacing * 4;
        const float slotPanelH = slotSize * 2 + slotSpacing * 1;
        const float slotPanelX = vw - slotPanelW - 20.0f * uiScale;
        const float slotPanelY = vh - slotPanelH - 20.0f * uiScale;

        bool clickedSlot = false;
        if (mouseVirtual.x >= slotPanelX &&
            mouseVirtual.x <= slotPanelX + slotPanelW &&
            mouseVirtual.y >= slotPanelY &&
            mouseVirtual.y <= slotPanelY + slotPanelH) {
          // スロットUI内をクリック
          const float relativeX = mouseVirtual.x - slotPanelX;
          const float relativeY = mouseVirtual.y - slotPanelY;
          const int col =
              static_cast<int>(relativeX / (slotSize + slotSpacing));
          const int row =
              static_cast<int>(relativeY / (slotSize + slotSpacing));
          if (col >= 0 && col < 5 && row >= 0 && row < 2) {
            const int slotIndex = row * 5 + col;
            if (slotIndex >= 0 && slotIndex < 10) {
              auto stateView = world.View<New::Game::Components::BattleState>();
              if (!stateView.empty()) {
                auto &s = stateView.get<New::Game::Components::BattleState>(
                    *stateView.begin());
                s.selectedSlot = slotIndex;
                s.selectedSlotCost = deckSlots[slotIndex].cost;
                s.lastMessage = TextFormat("Slot %d selected", slotIndex + 1);
                s.messageTtl = 1.5f;
                clickedSlot = true;
              }
            }
          }
        }

        // スロットUI以外をクリックした場合は設置予約
        if (!clickedSlot) {
          auto view = world.View<New::Game::Components::PlacementRequest>();
          if (!view.empty()) {
            auto &req = view.get<New::Game::Components::PlacementRequest>(
                *view.begin());
            req.x = mouseVirtual.x;
            req.y = mouseVirtual.y;
            req.valid = true;
          }
        }
      }
    }

    // 勝敗判定
    auto stateView = world.View<New::Game::Components::BattleState>();
    if (!stateView.empty()) {
      auto &state =
          stateView.get<New::Game::Components::BattleState>(*stateView.begin());
      if (!state.victory && !state.defeat) {
        bool hasEnemyAlive = false;
        auto enemyView = world.View<New::Domain::TD::LineTD::Components::Team,
                                    New::Core::Components::Health>();
        for (auto e : enemyView) {
          const auto &team =
              enemyView.get<New::Domain::TD::LineTD::Components::Team>(e);
          const auto &hp = enemyView.get<New::Core::Components::Health>(e);
          if (team.side ==
                  New::Domain::TD::LineTD::Components::TeamSide::Enemy &&
              !hp.IsDead()) {
            hasEnemyAlive = true;
            break;
          }
        }
        if (waveManager.IsFinished() && !hasEnemyAlive) {
          // 次Waveがあれば進行、なければ勝利
          const int nextWaveIndex = state.waveIndex + 1;
          if (nextWaveIndex < state.totalWaves) {
            const New::Data::WaveDef *nextWave = resolveWave(nextWaveIndex);
            if (!nextWave) {
              nextWave = registry.GetWave("fallback_wave");
            }
            waveManager.SetWave(nextWave);
            state.waveIndex = nextWaveIndex;
            state.cost += state.waveBonusCost;
          } else {
            state.victory = true;
          }
        }
      }
    }

    // メッセージTTL更新
    if (!stateView.empty()) {
      auto &s =
          stateView.get<New::Game::Components::BattleState>(*stateView.begin());
      if (s.messageTtl > 0.0f) {
        s.messageTtl = std::max(0.0f, s.messageTtl - dt);
        if (s.messageTtl == 0.0f) {
          s.lastMessage.clear();
        }
      }
    }

  render_loop_continue:
    BeginDrawing();
    {
      ClearBackground(BLACK);

      // 仮想FHD(1920x1080)で描画し、レターボックス付きでスケーリング出力
      renderer.BeginRender();
      renderer.Clear(BLANK);
      systems.Render(context);
      renderer.EndRender();

      renderer.RenderScaled();
    }
    EndDrawing();
  }

  context.Shutdown();
  CloseWindow();
  return 0;
}
