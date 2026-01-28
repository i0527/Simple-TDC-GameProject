#include "BattleHUDRenderer.hpp"

// 標準ライブラリ
#include <algorithm>
#include <sstream>

// プロジェクト�E
#include "../../utils/Log.h"
#include "../api/GameplayDataAPI.hpp"
#include "OverlayColors.hpp"

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
                               const std::unordered_map<std::string, float>& cooldownUntil,
                               bool isInfiniteStage) {
    (void)gold;
    (void)goldMax;
    (void)currentTime;
    (void)cooldownUntil;
    topButtons_.clear();
    unitSlotButtons_.clear();

    RenderTopBar(playerTowerHp, playerTowerMaxHp, enemyTowerHp, enemyTowerMaxHp, gameSpeed, isPaused, isInfiniteStage);
    RenderBottomBar(ctx, gold, goldMax, currentTime, cooldownUntil);
}

BattleHUDAction BattleHUDRenderer::HandleClick(const SharedContext& ctx,
                                               Vec2 mousePos,
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
        // 出撁E�Eタンは廁E��し、スロチE��全体をタチE�Eで出撁E
        if (!slot.unitId.empty() && IsMouseInRect(mousePos, slot.slotRect)) {
            if (!slot.isEnabled) {
                return BattleHUDAction{};
            }

            // クールダウン
            auto it = cooldownUntil.find(slot.unitId);
            if (it != cooldownUntil.end() && currentTime < it->second) {
                return BattleHUDAction{};
            }

            // ゴールチE
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
                                    float gameSpeed, bool isPaused,
                                    bool isInfiniteStage) {
    (void)playerHp;
    (void)playerMaxHp;
    (void)enemyHp;
    (void)enemyMaxHp;

    // 背景
    sysAPI_->Render().DrawRectangle(0, 0, static_cast<int>(SCREEN_W),
                                    static_cast<int>(TOP_H),
                                    ToCoreColor(OverlayColors::PANEL_BG_SECONDARY));
    sysAPI_->Render().DrawLine(0, static_cast<int>(TOP_H),
                               static_cast<int>(SCREEN_W),
                               static_cast<int>(TOP_H), 2.0f,
                               ToCoreColor(OverlayColors::BORDER_DEFAULT));

    // Pause/Resume ボタン�E�左上！E
    Rect pauseRect{ 30.0f, 20.0f, 200.0f, 50.0f };
    sysAPI_->Render().DrawRectangleRec(pauseRect,
                                       ToCoreColor(OverlayColors::BUTTON_SECONDARY));
    sysAPI_->Render().DrawRectangleLines(pauseRect.x, pauseRect.y,
                                         pauseRect.width, pauseRect.height, 3.0f,
                                         ToCoreColor(OverlayColors::BORDER_DEFAULT));
    sysAPI_->Render().DrawTextDefault(
        isPaused ? "再開" : "一時停止", static_cast<int>(pauseRect.x + 60),
        static_cast<int>(pauseRect.y + 14), 22.0f,
        ToCoreColor(OverlayColors::TEXT_DARK));

    RectButton pauseBtn;
    pauseBtn.rect = pauseRect;
    pauseBtn.action.type = BattleHUDActionType::TogglePause;
    topButtons_.push_back(pauseBtn);

    // Speed�E�右側に小さめEボタン�E�E
    const float speedBaseX = 260.0f;
    const float speedY = 20.0f;
    const float speedW = 120.0f;
    const float speedH = 50.0f;
    const float speedGap = 14.0f;

    auto drawSpeedBtn = [&](float x, float targetSpeed) {
        const bool active = std::abs(gameSpeed - targetSpeed) < 0.01f;
        Rect r{ x, speedY, speedW, speedH };
        sysAPI_->Render().DrawRectangleRec(
            r, active ? ToCoreColor(OverlayColors::CARD_BG_SELECTED)
                      : ToCoreColor(OverlayColors::CARD_BG_NORMAL));
        sysAPI_->Render().DrawRectangleLines(
            r.x, r.y, r.width, r.height, 3.0f,
            ToCoreColor(OverlayColors::BORDER_DEFAULT));
        std::ostringstream ss;
        ss << "x" << targetSpeed;
        sysAPI_->Render().DrawTextDefault(
            ss.str(), static_cast<int>(r.x + 38), static_cast<int>(r.y + 14),
            22.0f, ToCoreColor(OverlayColors::TEXT_PRIMARY));

        RectButton b;
        b.rect = r;
        b.action.type = BattleHUDActionType::SetSpeed;
        b.action.speed = targetSpeed;
        topButtons_.push_back(b);
    };

    drawSpeedBtn(speedBaseX, 1.0f);
    drawSpeedBtn(speedBaseX + (speedW + speedGap) * 1, 2.0f);
    drawSpeedBtn(speedBaseX + (speedW + speedGap) * 2, 4.0f);
    drawSpeedBtn(speedBaseX + (speedW + speedGap) * 3, 6.0f);
    
    // ギブアップボタン（無限ステージの場合のみ表示）
    if (isInfiniteStage) {
        const float giveUpX = SCREEN_W - 250.0f;
        const float giveUpY = 20.0f;
        const float giveUpW = 200.0f;
        const float giveUpH = 50.0f;
        Rect giveUpRect{ giveUpX, giveUpY, giveUpW, giveUpH };
        sysAPI_->Render().DrawRectangleRec(giveUpRect,
                                           ToCoreColor(OverlayColors::DANGER_RED));
        sysAPI_->Render().DrawRectangleLines(giveUpRect.x, giveUpRect.y,
                                             giveUpRect.width, giveUpRect.height, 3.0f,
                                             ToCoreColor(OverlayColors::BORDER_DEFAULT));
        sysAPI_->Render().DrawTextDefault(
            "ギブアップ", static_cast<int>(giveUpRect.x + 50),
            static_cast<int>(giveUpRect.y + 14), 22.0f,
            ToCoreColor(OverlayColors::TEXT_PRIMARY));
        
        RectButton giveUpBtn;
        giveUpBtn.rect = giveUpRect;
        giveUpBtn.action.type = BattleHUDActionType::GiveUp;
        topButtons_.push_back(giveUpBtn);
    }
}

void BattleHUDRenderer::RenderBottomBar(const SharedContext& ctx,
                                       int gold,
                                       int goldMax,
                                       float currentTime,
                                       const std::unordered_map<std::string, float>& cooldownUntil) {
    (void)currentTime;

    const float y0 = SCREEN_H - BOTTOM_H;
    sysAPI_->Render().DrawRectangle(0, static_cast<int>(y0),
                                    static_cast<int>(SCREEN_W),
                                    static_cast<int>(BOTTOM_H),
                                    ToCoreColor(OverlayColors::PANEL_BG_SECONDARY));
    sysAPI_->Render().DrawLine(0, static_cast<int>(y0),
                               static_cast<int>(SCREEN_W),
                               static_cast<int>(y0), 2.0f,
                               ToCoreColor(OverlayColors::BORDER_DEFAULT));

    // ゴールド表示�E�左�E�E
    // ゴールド表示（右上に大きく表示）
    std::ostringstream goldText;
    goldText << "Gold: " << gold << " / " << goldMax;
    Vec2 goldTextSize = sysAPI_->Render().MeasureTextDefaultCore(goldText.str(), 48.0f, 1.0f);
    float goldX = SCREEN_W - goldTextSize.x - 30.0f;  // 右端から30px余白
    float goldY = y0 + (BOTTOM_H - goldTextSize.y) * 0.5f;  // 垂直中央
    sysAPI_->Render().DrawTextDefault(goldText.str(), static_cast<int>(goldX),
                                      static_cast<int>(goldY), 48.0f,  // 28.0f → 48.0f（大きく）
                                      ToCoreColor(OverlayColors::TEXT_GOLD));

    // 10枠の中忁E�E置
    const float totalW = SLOT_COLS * SLOT_W + (SLOT_COLS - 1) * SLOT_GAP_X;
    const float totalH = SLOT_ROWS * SLOT_H + (SLOT_ROWS - 1) * SLOT_GAP_Y;
    const float startX = (SCREEN_W - totalW) * 0.5f;
    const float startY = y0 + (BOTTOM_H - totalH) * 0.5f;

    // 編戁E0枠めEunitId の配�Eに展開
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
        Rect slotRect{
            startX + col * (SLOT_W + SLOT_GAP_X),
            startY + row * (SLOT_H + SLOT_GAP_Y),
            SLOT_W,
            SLOT_H
        };

        const std::string unitId = slotUnitIds[i];
        const bool hasUnit = !unitId.empty();

        int costGold = 0;
        bool enabled = false;
        std::string displayName = hasUnit ? unitId : "Empty";
        std::string iconPath;

        bool is_unlocked = true;
        if (hasUnit && ctx.gameplayDataAPI) {
            auto ch = ctx.gameplayDataAPI->GetCharacterTemplate(unitId);
            if (ch) {
                // 未所持チェック
                const auto st = ctx.gameplayDataAPI->GetCharacterState(unitId);
                is_unlocked = st.unlocked;
                
                displayName = ch->name;
                costGold = ch->cost;
                enabled = true;
                iconPath = ch->icon_path;
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

        // スロチE��背景
        sysAPI_->Render().DrawRectangleRec(
            slotRect, hasUnit ? ToCoreColor(OverlayColors::CARD_BG_NORMAL)
                              : ToCoreColor(OverlayColors::PANEL_BG_PRIMARY));

        // portraitを薄く背景に敷く（誰が誰か判別しやすくする�E�E
        if (hasUnit && !iconPath.empty()) {
            void* texturePtr = sysAPI_->Resource().GetTexture(iconPath);
            if (texturePtr) {
                Texture2D* texture = static_cast<Texture2D*>(texturePtr);
                if (texture && texture->id != 0) {
                    Rect src{0.0f, 0.0f, static_cast<float>(texture->width),
                             static_cast<float>(texture->height)};
                    const float pad = 6.0f;
                    const float maxW = std::max(0.0f, slotRect.width - pad * 2.0f);
                    const float maxH = std::max(0.0f, slotRect.height - pad * 2.0f);
                    const float scale = std::min(maxW / static_cast<float>(texture->width),
                                                 maxH / static_cast<float>(texture->height));
                    const float drawW = static_cast<float>(texture->width) * scale;
                    const float drawH = static_cast<float>(texture->height) * scale;
                    Rect dst{
                        slotRect.x + (slotRect.width - drawW) * 0.5f,
                        slotRect.y + (slotRect.height - drawH) * 0.5f,
                        drawW,
                        drawH
                    };
                    ColorRGBA tint{255, 255, 255, 70};
                    sysAPI_->Render().DrawTexturePro(*texture, src, dst,
                                                     Vec2{0.0f, 0.0f}, 0.0f,
                                                     tint);
                }
            }
        }

        // 枠線（�E撁E��能なら緑で強調�E�E
        const ColorRGBA border = enabled ? ToCoreColor(OverlayColors::SUCCESS_GREEN)
                                          : ToCoreColor(OverlayColors::BORDER_DEFAULT);
        const float borderW = enabled ? 4.0f : 3.0f;
        sysAPI_->Render().DrawRectangleLines(slotRect.x, slotRect.y,
                                             slotRect.width, slotRect.height,
                                             borderW, border);

        // 表示（未所持の場合は名前とコストを非表示）
        if (hasUnit && is_unlocked) {
            sysAPI_->Render().DrawTextDefault(
                displayName, static_cast<int>(slotRect.x + 10),
                static_cast<int>(slotRect.y + 8), 32.0f,  // 20.0f → 32.0f（大きく）
                ToCoreColor(OverlayColors::TEXT_PRIMARY));
            std::ostringstream cs;
            cs << "Cost " << costGold;
            sysAPI_->Render().DrawTextDefault(
                cs.str(), static_cast<int>(slotRect.x + 10),
                static_cast<int>(slotRect.y + 40), 28.0f,  // 20.0f → 28.0f、位置も調整
                ToCoreColor(OverlayColors::TEXT_ACCENT));
        } else if (hasUnit && !is_unlocked) {
            // 未所持の場合はロックアイコンのみ表示
            sysAPI_->Render().DrawTextDefault(
                "🔒", static_cast<int>(slotRect.x + slotRect.width - 25),
                static_cast<int>(slotRect.y + 8), 32.0f,  // 20.0f → 32.0f（大きく）
                ToCoreColor(OverlayColors::TEXT_MUTED));
        }

        UnitSlotButton slotBtn;
        slotBtn.slotRect = slotRect;
        slotBtn.unitId = unitId;
        slotBtn.costGold = costGold;
        slotBtn.isEnabled = enabled;
        unitSlotButtons_.push_back(slotBtn);
    }
}

bool BattleHUDRenderer::IsMouseInRect(Vec2 mouse, Rect rect) {
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

