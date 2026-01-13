#include "BattleHUDRenderer.hpp"

// 標準ライブラリ
#include <algorithm>
#include <sstream>

// プロジェクト内
#include "../../utils/Log.h"
#include "../entities/CharacterManager.hpp"

namespace game {
namespace core {
namespace ui {

namespace {
constexpr float SCREEN_W = 1920.0f;
constexpr float SCREEN_H = 1080.0f;

constexpr float TOP_H = 90.0f;
constexpr float BOTTOM_H = 240.0f;

constexpr float SLOT_W = 200.0f;
constexpr float SLOT_H = 86.0f;
constexpr float SLOT_GAP_X = 24.0f;
constexpr float SLOT_GAP_Y = 18.0f;
constexpr int SLOT_COLS = 5;
constexpr int SLOT_ROWS = 2;
constexpr int SLOT_COUNT = 10;

Color MakeColor(int r, int g, int b, int a = 255) {
    return Color{ static_cast<unsigned char>(r),
                  static_cast<unsigned char>(g),
                  static_cast<unsigned char>(b),
                  static_cast<unsigned char>(a) };
}

} // namespace

BattleHUDRenderer::BattleHUDRenderer(BaseSystemAPI* sysAPI)
    : sysAPI_(sysAPI) {
}

void BattleHUDRenderer::Render(const SharedContext& ctx,
                               int playerTowerHp, int playerTowerMaxHp,
                               int enemyTowerHp, int enemyTowerMaxHp,
                               int gold,
                               int goldMax,
                               float gameSpeed,
                               bool isPaused,
                               float currentTime,
                               const std::unordered_map<std::string, float>& cooldownUntil) {
    (void)gold;
    (void)goldMax;
    (void)currentTime;
    (void)cooldownUntil;
    topButtons_.clear();
    unitSlotButtons_.clear();

    RenderTopBar(playerTowerHp, playerTowerMaxHp, enemyTowerHp, enemyTowerMaxHp, gameSpeed, isPaused);
    RenderBottomBar(ctx, gold, goldMax, currentTime, cooldownUntil);
}

BattleHUDAction BattleHUDRenderer::HandleClick(const SharedContext& ctx,
                                               Vector2 mousePos,
                                               int gold,
                                               float currentTime,
                                               const std::unordered_map<std::string, float>& cooldownUntil) {
    (void)ctx;

    for (const auto& btn : topButtons_) {
        if (IsMouseInRect(mousePos, btn.rect)) {
            return btn.action;
        }
    }

    for (const auto& slot : unitSlotButtons_) {
        if (!slot.unitId.empty() && IsMouseInRect(mousePos, slot.spawnRect)) {
            if (!slot.isEnabled) {
                return BattleHUDAction{};
            }

            // クールダウン
            auto it = cooldownUntil.find(slot.unitId);
            if (it != cooldownUntil.end() && currentTime < it->second) {
                return BattleHUDAction{};
            }

            // ゴールド
            if (gold < slot.costGold) {
                return BattleHUDAction{};
            }

            BattleHUDAction action;
            action.type = BattleHUDActionType::SpawnUnit;
            action.unitId = slot.unitId;
            return action;
        }
    }

    return BattleHUDAction{};
}

void BattleHUDRenderer::RenderTopBar(int playerHp, int playerMaxHp,
                                    int enemyHp, int enemyMaxHp,
                                    float gameSpeed, bool isPaused) {
    // 背景
    sysAPI_->DrawRectangle(0, 0, static_cast<int>(SCREEN_W), static_cast<int>(TOP_H), MakeColor(245, 245, 245));
    sysAPI_->DrawLine(0, static_cast<int>(TOP_H), static_cast<int>(SCREEN_W), static_cast<int>(TOP_H), 2.0f, MakeColor(80, 80, 80));

    // Pause/Resume ボタン（左上）
    Rectangle pauseRect{ 30.0f, 20.0f, 200.0f, 50.0f };
    sysAPI_->DrawRectangleRec(pauseRect, MakeColor(230, 230, 230));
    sysAPI_->DrawRectangleLines(pauseRect.x, pauseRect.y, pauseRect.width, pauseRect.height, 3.0f, MakeColor(80, 80, 80));
    sysAPI_->DrawTextDefault(isPaused ? "Play" : "Pause", static_cast<int>(pauseRect.x + 70), static_cast<int>(pauseRect.y + 14), 22.0f, MakeColor(60, 60, 60));

    RectButton pauseBtn;
    pauseBtn.rect = pauseRect;
    pauseBtn.action.type = BattleHUDActionType::TogglePause;
    topButtons_.push_back(pauseBtn);

    // Speed（右側に小さめ2ボタン）
    const float speedBaseX = 260.0f;
    const float speedY = 20.0f;
    const float speedW = 120.0f;
    const float speedH = 50.0f;
    const float speedGap = 14.0f;

    auto drawSpeedBtn = [&](float x, float targetSpeed) {
        const bool active = std::abs(gameSpeed - targetSpeed) < 0.01f;
        Rectangle r{ x, speedY, speedW, speedH };
        sysAPI_->DrawRectangleRec(r, active ? MakeColor(220, 220, 220) : MakeColor(240, 240, 240));
        sysAPI_->DrawRectangleLines(r.x, r.y, r.width, r.height, 3.0f, MakeColor(80, 80, 80));
        std::ostringstream ss;
        ss << "x" << targetSpeed;
        sysAPI_->DrawTextDefault(ss.str(), static_cast<int>(r.x + 38), static_cast<int>(r.y + 14), 22.0f, MakeColor(60, 60, 60));

        RectButton b;
        b.rect = r;
        b.action.type = BattleHUDActionType::SetSpeed;
        b.action.speed = targetSpeed;
        topButtons_.push_back(b);
    };

    drawSpeedBtn(speedBaseX, 1.0f);
    drawSpeedBtn(speedBaseX + speedW + speedGap, 2.0f);

    // HP表示（右上）
    const float hpX = 560.0f;
    sysAPI_->DrawTextDefault("Enemy HP", static_cast<int>(hpX), 22, 18.0f, MakeColor(40, 40, 40));
    sysAPI_->DrawTextDefault("Player HP", static_cast<int>(hpX), 52, 18.0f, MakeColor(40, 40, 40));

    Rectangle enemyBar{ hpX + 120.0f, 26.0f, 240.0f, 14.0f };
    Rectangle playerBar{ hpX + 120.0f, 56.0f, 240.0f, 14.0f };

    const float ep = SafePct(enemyHp, enemyMaxHp);
    const float pp = SafePct(playerHp, playerMaxHp);

    sysAPI_->DrawRectangleRec(enemyBar, MakeColor(230, 230, 230));
    sysAPI_->DrawRectangleRec(playerBar, MakeColor(230, 230, 230));
    sysAPI_->DrawRectangleRec(Rectangle{ enemyBar.x, enemyBar.y, enemyBar.width * ep, enemyBar.height }, MakeColor(220, 120, 120));
    sysAPI_->DrawRectangleRec(Rectangle{ playerBar.x, playerBar.y, playerBar.width * pp, playerBar.height }, MakeColor(120, 160, 220));
    sysAPI_->DrawRectangleLines(enemyBar.x, enemyBar.y, enemyBar.width, enemyBar.height, 2.0f, MakeColor(80, 80, 80));
    sysAPI_->DrawRectangleLines(playerBar.x, playerBar.y, playerBar.width, playerBar.height, 2.0f, MakeColor(80, 80, 80));
}

void BattleHUDRenderer::RenderBottomBar(const SharedContext& ctx,
                                       int gold,
                                       int goldMax,
                                       float currentTime,
                                       const std::unordered_map<std::string, float>& cooldownUntil) {
    (void)currentTime;

    const float y0 = SCREEN_H - BOTTOM_H;
    sysAPI_->DrawRectangle(0, static_cast<int>(y0), static_cast<int>(SCREEN_W), static_cast<int>(BOTTOM_H), MakeColor(220, 220, 220));
    sysAPI_->DrawLine(0, static_cast<int>(y0), static_cast<int>(SCREEN_W), static_cast<int>(y0), 2.0f, MakeColor(80, 80, 80));

    // ゴールド表示（左）
    std::ostringstream goldText;
    goldText << "Gold: " << gold << " / " << goldMax;
    sysAPI_->DrawTextDefault(goldText.str(), 30, static_cast<int>(y0 + 18), 22.0f, MakeColor(40, 40, 40));

    // 10枠の中心配置
    const float totalW = SLOT_COLS * SLOT_W + (SLOT_COLS - 1) * SLOT_GAP_X;
    const float totalH = SLOT_ROWS * SLOT_H + (SLOT_ROWS - 1) * SLOT_GAP_Y;
    const float startX = (SCREEN_W - totalW) * 0.5f;
    const float startY = y0 + (BOTTOM_H - totalH) * 0.5f;

    // 編成10枠を unitId の配列に展開
    std::vector<std::string> slotUnitIds(SLOT_COUNT, "");
    if (!ctx.formationData.IsEmpty()) {
        for (const auto& slot : ctx.formationData.slots) {
            const int idx = slot.first;
            if (idx >= 0 && idx < SLOT_COUNT) {
                slotUnitIds[idx] = slot.second;
            }
        }
    }

    for (int i = 0; i < SLOT_COUNT; ++i) {
        const int col = i % SLOT_COLS;
        const int row = i / SLOT_COLS;
        Rectangle slotRect{
            startX + col * (SLOT_W + SLOT_GAP_X),
            startY + row * (SLOT_H + SLOT_GAP_Y),
            SLOT_W,
            SLOT_H
        };

        const std::string unitId = slotUnitIds[i];
        const bool hasUnit = !unitId.empty();

        // スロット背景
        sysAPI_->DrawRectangleRec(slotRect, hasUnit ? MakeColor(245, 245, 245) : MakeColor(235, 235, 235));
        sysAPI_->DrawRectangleLines(slotRect.x, slotRect.y, slotRect.width, slotRect.height, 3.0f, MakeColor(80, 80, 80));

        // Spawnボタン（右下）
        Rectangle spawnRect{ slotRect.x + SLOT_W - 88.0f, slotRect.y + SLOT_H - 34.0f, 76.0f, 26.0f };

        int costGold = 0;
        bool enabled = false;
        std::string displayName = hasUnit ? unitId : "Empty";

        if (hasUnit && ctx.characterManager) {
            auto ch = ctx.characterManager->GetCharacterTemplate(unitId);
            if (ch) {
                displayName = ch->name;
                costGold = ch->cost;
                enabled = true;
            }
        }

        // クールダウン中は無効
        if (enabled) {
            auto it = cooldownUntil.find(unitId);
            if (it != cooldownUntil.end() && currentTime < it->second) {
                enabled = false;
            }
        }
        // ゴールド不足も無効
        if (enabled && gold < costGold) {
            enabled = false;
        }

        // 表示
        sysAPI_->DrawTextDefault(displayName, static_cast<int>(slotRect.x + 10), static_cast<int>(slotRect.y + 10), 18.0f, MakeColor(40, 40, 40));
        if (hasUnit) {
            std::ostringstream cs;
            cs << "Cost " << costGold;
            sysAPI_->DrawTextDefault(cs.str(), static_cast<int>(slotRect.x + 10), static_cast<int>(slotRect.y + 34), 16.0f, MakeColor(80, 80, 80));
        }

        sysAPI_->DrawRectangleRec(spawnRect, enabled ? MakeColor(245, 245, 245) : MakeColor(210, 210, 210));
        sysAPI_->DrawRectangleLines(spawnRect.x, spawnRect.y, spawnRect.width, spawnRect.height, 2.0f, MakeColor(80, 80, 80));
        sysAPI_->DrawTextDefault("Spawn", static_cast<int>(spawnRect.x + 10), static_cast<int>(spawnRect.y + 6), 16.0f, MakeColor(60, 60, 60));

        UnitSlotButton slotBtn;
        slotBtn.slotRect = slotRect;
        slotBtn.spawnRect = spawnRect;
        slotBtn.unitId = unitId;
        slotBtn.costGold = costGold;
        slotBtn.isEnabled = enabled;
        unitSlotButtons_.push_back(slotBtn);
    }
}

bool BattleHUDRenderer::IsMouseInRect(Vector2 mouse, Rectangle rect) {
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.width &&
           mouse.y >= rect.y && mouse.y <= rect.y + rect.height;
}

float BattleHUDRenderer::SafePct(int current, int max) {
    if (max <= 0) return 0.0f;
    float v = static_cast<float>(current) / static_cast<float>(max);
    return std::clamp(v, 0.0f, 1.0f);
}

} // namespace ui
} // namespace core
} // namespace game

