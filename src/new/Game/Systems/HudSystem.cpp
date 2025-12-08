#include "new/Game/Systems/HudSystem.h"

#include "Core/GameContext.h"
#include "Core/GameRenderer.h"
#include "Core/ResourceManager.h"
#include "Core/World.h"
#include "new/Game/Components/GameState.h"
#include "raylib.h"

namespace New::Game::Systems {

using New::Game::Components::BattleState;
using New::Game::Components::SlotType;

void HudSystem::Render(Core::GameContext &context) {
  auto &world = context.GetWorld();
  auto view = world.View<BattleState>();
  if (view.empty()) {
    return;
  }

  const auto entity = *view.begin();
  const auto &state = view.get<BattleState>(entity);

  const float uiScale = 1.2f; // 全体を約20%スケールアップ

  const bool hasFont = context.GetResourceManager().HasDefaultFont();
  const Font *font =
      hasFont ? &context.GetResourceManager().GetDefaultFont() : nullptr;
  const float fontSize = 28.0f * uiScale;
  const float spacing = 1.0f;
  const float smallFontSize = 20.0f * uiScale;

  auto drawText = [&](const char *text, float x, float y, Color color,
                      float size) {
    if (font) {
      DrawTextEx(*font, text, {x, y}, size, spacing, color);
    } else {
      DrawText(text, static_cast<int>(x), static_cast<int>(y),
               static_cast<int>(size), color);
    }
  };

  const float panelX = 20.0f * uiScale;
  const float panelY = 140.0f * uiScale;
  const float panelW = 420.0f * uiScale;
  const float panelH = 180.0f * uiScale;

  DrawRectangle(panelX, panelY, panelW, panelH, ColorAlpha(BLACK, 0.6f));
  DrawRectangleLines(panelX, panelY, panelW, panelH,
                     ColorAlpha(LIGHTGRAY, 0.85f));

  drawText(TextFormat("Life: %d", state.playerLife), panelX + 16.0f,
           panelY + 16.0f, RAYWHITE, fontSize);
  drawText(TextFormat("Cost: %d", state.cost), panelX + 16.0f, panelY + 48.0f,
           RAYWHITE, fontSize);
  drawText(TextFormat("Wave: %d / %d", state.waveIndex + 1, state.totalWaves),
           panelX + 16.0f, panelY + 80.0f, RAYWHITE, fontSize);
  drawText(TextFormat("Slot: %d (Cost %d)", state.selectedSlot + 1,
                      state.selectedSlotCost),
           panelX + 16.0f, panelY + 112.0f, RAYWHITE, fontSize);
  const float slotCd =
      state.slotCooldowns[std::clamp(state.selectedSlot, 0, 9)];
  if (slotCd > 0.0f) {
    drawText(TextFormat("Cooldown: %.1fs", slotCd), panelX + 220.0f,
             panelY + 112.0f, SKYBLUE, fontSize);
  }
  if (state.messageTtl > 0.0f && !state.lastMessage.empty()) {
    drawText(TextFormat("Msg: %s", state.lastMessage.c_str()), panelX + 16.0f,
             panelY + 144.0f, YELLOW, fontSize);
  }
  drawText(
      TextFormat("BaseDmg: %d  KillReward: %d  MinGap: %.0f  KB(Debug): %.1f",
                 state.baseArrivalDamage, state.killReward, state.minGap,
                 state.debugKnockback),
      panelX + 16.0f, panelY + 176.0f, LIGHTGRAY, fontSize);

  if (state.victory) {
    drawText("VICTORY", panelX + 220.0f, panelY + 16.0f, GREEN, fontSize);
  } else if (state.defeat) {
    drawText("DEFEAT", panelX + 220.0f, panelY + 16.0f, RED, fontSize);
  }

  // 右下に5*2のスロットUIを描画
  const int vw = context.GetRenderer().GetVirtualWidth();
  const int vh = context.GetRenderer().GetVirtualHeight();
  const float slotSize = 82.0f * uiScale;
  const float slotSpacing = 10.0f * uiScale;
  const float slotPanelW = slotSize * 5 + slotSpacing * 4;
  const float slotPanelH = slotSize * 2 + slotSpacing * 1;
  const float slotPanelX = vw - slotPanelW - 20.0f * uiScale;
  const float slotPanelY = vh - slotPanelH - 20.0f * uiScale;

  // 背景パネル
  DrawRectangle(slotPanelX, slotPanelY, slotPanelW, slotPanelH,
                ColorAlpha(BLACK, 0.5f));
  DrawRectangleLines(slotPanelX, slotPanelY, slotPanelW, slotPanelH,
                     ColorAlpha(LIGHTGRAY, 0.8f));

  // 10スロットを5*2で描画
  for (int i = 0; i < 10; ++i) {
    const int col = i % 5;
    const int row = i / 5;
    const float slotX = slotPanelX + col * (slotSize + slotSpacing);
    const float slotY = slotPanelY + row * (slotSize + slotSpacing);

    const bool isSelected = (state.selectedSlot == i);
    const bool isCooldown = state.slotCooldowns[i] > 0.0f;
    const bool canAfford = state.cost >= state.slotCosts[i];
    const bool isMainLimit =
        (state.slotTypes[i] == SlotType::Main &&
         state.slotSummonLimits[i] > 0 &&
         state.slotSummonCounts[i] >= state.slotSummonLimits[i]);

    // スロット背景色
    Color slotBgColor = ColorAlpha(DARKGRAY, 0.7f);
    if (isSelected) {
      slotBgColor = ColorAlpha(YELLOW, 0.5f);
    } else if (isCooldown || !canAfford || isMainLimit) {
      slotBgColor = ColorAlpha(RED, 0.4f);
    } else {
      slotBgColor = ColorAlpha(GREEN, 0.3f);
    }

    DrawRectangle(slotX, slotY, slotSize, slotSize, slotBgColor);
    DrawRectangleLinesEx({slotX, slotY, slotSize, slotSize}, 3.0f,
                         isSelected ? YELLOW : ColorAlpha(WHITE, 0.8f));

    // スロット番号（0-9）
    drawText(TextFormat("%d", i), slotX + 4.0f, slotY + 4.0f, WHITE,
             smallFontSize);

    // コスト表示
    if (state.slotCosts[i] > 0) {
      drawText(TextFormat("%d", state.slotCosts[i]), slotX + 6.0f,
               slotY + slotSize - 22.0f, YELLOW, smallFontSize);
    }

    // クールダウン表示
    if (isCooldown) {
      const float cd = state.slotCooldowns[i];
      drawText(TextFormat("%.1f", cd), slotX + slotSize / 2.0f - 20.0f,
               slotY + slotSize / 2.0f, SKYBLUE, smallFontSize);
    }

    // メインスロットの召喚数制限表示
    if (state.slotTypes[i] == SlotType::Main && state.slotSummonLimits[i] > 0) {
      drawText(TextFormat("%d/%d", state.slotSummonCounts[i],
                          state.slotSummonLimits[i]),
               slotX + slotSize - 40.0f, slotY + 4.0f, ORANGE, smallFontSize);
    }

    // スロットタイプ表示（簡易）
    const char *typeLabel = "";
    if (state.slotTypes[i] == SlotType::Main) {
      typeLabel = "M";
    } else if (state.slotTypes[i] == SlotType::Ability) {
      typeLabel = "A";
    } else if (state.slotTypes[i] == SlotType::Sub) {
      typeLabel = "S";
    }
    if (typeLabel[0] != '\0') {
      drawText(typeLabel, slotX + slotSize - 20.0f, slotY + slotSize - 20.0f,
               LIGHTGRAY, smallFontSize);
    }
  }
}

} // namespace New::Game::Systems
