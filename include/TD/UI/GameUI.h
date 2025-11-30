/**
 * @file GameUI.h
 * @brief ゲームUI描画クラス
 * 
 * Phase 3: UI/UX改善
 * - デッキスロット（クリック対応）
 * - HPバー
 * - コスト/クールダウンゲージ
 * - ウェーブ情報
 * 
 * Phase 4A: FHD固定レンダリング対応
 * - すべての座標を1920x1080固定で指定
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/GameRenderer.h"
#include "TD/Managers/WaveManager.h"
#include "TD/Managers/SpawnManager.h"
#include "TD/Managers/GameStateManager.h"
#include "TD/Components/TDComponents.h"
#include "Core/Components/CoreComponents.h"

#include "Core/Platform.h"
#include <string>
#include <functional>

namespace TD::UI {

// FHD定数への短縮エイリアス
namespace FHD = Core::FHD;
namespace FHDUI = Core::FHD::UI;

/**
 * @brief デッキスロットUI情報
 */
struct DeckSlotUI {
    Rectangle bounds;
    int slotIndex;
    bool isHovered;
    bool isPressed;
};

/**
 * @brief ゲームUI管理クラス
 */
class GameUI {
public:
    // スロットクリック時のコールバック
    using SlotClickCallback = std::function<void(int slotIndex)>;
    
    GameUI() = default;
    
    /**
     * @brief 初期化（FHD固定解像度）
     */
    void Initialize() {
        // FHD固定座標を使用
        renderWidth_ = FHD::RENDER_WIDTH;
        renderHeight_ = FHD::RENDER_HEIGHT;
        
        // デッキスロット位置を計算
        UpdateDeckSlotPositions(5);
    }
    
    /**
     * @brief 初期化（互換性のため維持）
     * @deprecated Initialize()を使用してください
     */
    void Initialize(int /*screenWidth*/, int /*screenHeight*/) {
        Initialize();
    }
    
    /**
     * @brief スロットクリックコールバックを設定
     */
    void SetSlotClickCallback(SlotClickCallback callback) {
        slotClickCallback_ = std::move(callback);
    }
    
    /**
     * @brief 入力処理（クリック検出）
     * @param mouseWorldPos ワールド座標に変換済みのマウス位置
     */
    void HandleInput(Vector2 mouseWorldPos) {
        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        for (auto& slot : deckSlots_) {
            slot.isHovered = CheckCollisionPointRec(mouseWorldPos, slot.bounds);
            slot.isPressed = slot.isHovered && mousePressed;
            
            if (slot.isPressed && slotClickCallback_) {
                slotClickCallback_(slot.slotIndex);
            }
        }
        
        // 選択中スロットの更新（数字キー）
        if (IsKeyPressed(KEY_ONE)) selectedSlot_ = 0;
        else if (IsKeyPressed(KEY_TWO)) selectedSlot_ = 1;
        else if (IsKeyPressed(KEY_THREE)) selectedSlot_ = 2;
        else if (IsKeyPressed(KEY_FOUR)) selectedSlot_ = 3;
        else if (IsKeyPressed(KEY_FIVE)) selectedSlot_ = 4;
    }
    
    /**
     * @brief 入力処理（旧API互換）
     * @deprecated HandleInput(Vector2)を使用してください
     */
    void HandleInput() {
        HandleInput(GetMousePosition());
    }
    
    /**
     * @brief 上部UI描画（Wave、コスト、拠点HP）
     */
    void DrawTopBar(
        const WaveManager& waves, 
        const SpawnManager& spawns, 
        const GameStateManager& gameState
    ) {
        // 背景パネル（FHD座標）
        DrawRectangle(
            static_cast<int>(FHDUI::TOP_BAR_X), 
            static_cast<int>(FHDUI::TOP_BAR_Y), 
            static_cast<int>(FHDUI::TOP_BAR_WIDTH), 
            static_cast<int>(FHDUI::TOP_BAR_HEIGHT), 
            Fade(Color{20, 20, 30, 255}, 0.9f)
        );
        DrawRectangleLinesEx(
            Rectangle{FHDUI::TOP_BAR_X, FHDUI::TOP_BAR_Y, FHDUI::TOP_BAR_WIDTH, FHDUI::TOP_BAR_HEIGHT}, 
            2, Color{60, 60, 80, 255}
        );
        
        // Wave情報
        DrawWaveInfo(waves, static_cast<int>(FHDUI::TOP_BAR_X + 10), static_cast<int>(FHDUI::TOP_BAR_Y + 5));
        
        // コストゲージ
        DrawCostGauge(spawns, static_cast<int>(FHDUI::TOP_BAR_X + 200), static_cast<int>(FHDUI::TOP_BAR_Y + 5));
        
        // 拠点HP
        DrawBaseHealthBars(gameState, static_cast<int>(FHDUI::TOP_BAR_X + 420), static_cast<int>(FHDUI::TOP_BAR_Y + 5));
    }
    
    /**
     * @brief デッキスロット描画
     */
    void DrawDeckSlots(
        const SpawnManager& spawns, 
        Core::GameContext& ctx
    ) {
        const auto& deck = spawns.GetDeck();
        UpdateDeckSlotPositions(static_cast<int>(deck.size()));
        
        // 背景パネル（FHD座標）
        float panelWidth = deck.size() * FHDUI::DECK_SLOT_SPACING + 20.0f;
        float panelX = (static_cast<float>(FHD::RENDER_WIDTH) - panelWidth) / 2.0f;
        DrawRectangle(
            static_cast<int>(panelX), 
            static_cast<int>(FHDUI::DECK_PANEL_Y), 
            static_cast<int>(panelWidth), 
            90, 
            Fade(Color{20, 20, 30, 255}, 0.85f)
        );
        
        for (size_t i = 0; i < deck.size() && i < deckSlots_.size(); ++i) {
            const auto& slot = deck[i];
            auto& uiSlot = deckSlots_[i];
            
            DrawSingleSlot(uiSlot, slot, spawns, ctx, static_cast<int>(i));
        }
    }
    
    /**
     * @brief ユニット上のHPバー描画システム
     */
    void DrawUnitHealthBars(Core::World& world) {
        using namespace TD::Components;
        using namespace Core::Components;
        
        auto view = world.View<Position, Stats, Unit>();
        
        for (auto entity : view) {
            const auto& pos = view.get<Position>(entity);
            const auto& stats = view.get<Stats>(entity);
            
            // 死亡中はスキップ
            if (world.HasAll<Dying>(entity)) continue;
            
            float hpPercent = stats.currentHealth / stats.maxHealth;
            bool isEnemy = world.HasAll<EnemyUnit>(entity);
            
            DrawUnitHPBar(pos.x, pos.y - 30.0f, 40.0f, hpPercent, isEnemy);
        }
    }
    
    /**
     * @brief レーン背景描画
     */
    void DrawLaneBackgrounds(const WaveManager& waves) {
        int laneCount = waves.GetLaneCount();
        float laneHeight = waves.GetLaneHeight();
        
        for (int i = 0; i < laneCount; ++i) {
            float laneY = waves.GetLaneY(i, static_cast<float>(FHD::RENDER_HEIGHT));
            
            // 交互に色を変える
            Color laneColor = (i % 2 == 0) 
                ? Fade(Color{200, 220, 200, 255}, 0.3f)
                : Fade(Color{180, 200, 180, 255}, 0.3f);
            
            DrawRectangle(
                static_cast<int>(FHD::BATTLEFIELD_LEFT), 
                static_cast<int>(laneY - laneHeight / 2), 
                static_cast<int>(FHD::BATTLEFIELD_RIGHT - FHD::BATTLEFIELD_LEFT), 
                static_cast<int>(laneHeight),
                laneColor
            );
            
            // レーン番号
            char laneNum[8];
            snprintf(laneNum, sizeof(laneNum), "%d", i + 1);
            Core::RDrawText(laneNum, static_cast<int>(FHD::BATTLEFIELD_LEFT + 3), static_cast<int>(laneY - 8), 16, DARKGRAY);
        }
    }
    
    /**
     * @brief 拠点描画
     */
    void DrawBases(const GameStateManager& gameState) {
        // 敵拠点（左）
        DrawBase(
            static_cast<int>(FHD::BASE_LEFT_X), 
            FHD::RENDER_HEIGHT / 2, 
            static_cast<int>(FHD::BASE_WIDTH), 
            static_cast<int>(FHD::BASE_HEIGHT), 
            gameState.GetEnemyBaseHealthPercent(), 
            true
        );
        
        // プレイヤー拠点（右）
        DrawBase(
            static_cast<int>(FHD::BASE_RIGHT_X), 
            FHD::RENDER_HEIGHT / 2, 
            static_cast<int>(FHD::BASE_WIDTH), 
            static_cast<int>(FHD::BASE_HEIGHT), 
            gameState.GetBaseHealthPercent(), 
            false
        );
    }
    
    /**
     * @brief 操作ヘルプ描画
     */
    void DrawControlsHelp() {
        const char* helpText = "1-5 or Click: Summon | P: Pause | R: Restart | ESC: Quit";
        int textWidth = MeasureText(helpText, 14);
        int x = (FHD::RENDER_WIDTH - textWidth) / 2;
        
        DrawRectangle(x - 10, FHD::RENDER_HEIGHT - 18, textWidth + 20, 18, 
                      Fade(Color{20, 20, 30, 255}, 0.8f));
        Core::RDrawText(helpText, x, FHD::RENDER_HEIGHT - 16, 14, LIGHTGRAY);
    }
    
    /**
     * @brief ゲーム状態オーバーレイ描画
     */
    void DrawGameStateOverlay(const GameStateManager& gameState) {
        auto phase = gameState.GetPhase();
        
        if (phase == GamePhase::Paused) {
            DrawOverlayPanel("PAUSED", "Press P to Resume", Color{50, 50, 80, 255});
        } else if (phase == GamePhase::Victory) {
            char timeText[64];
            snprintf(timeText, sizeof(timeText), "Time: %.1f seconds", gameState.GetElapsedTime());
            DrawOverlayPanel("VICTORY!", timeText, Color{30, 120, 30, 255}, "Press R to Restart");
        } else if (phase == GamePhase::Defeat) {
            DrawOverlayPanel("DEFEAT", "Your base was destroyed!", Color{150, 30, 30, 255}, "Press R to Restart");
        }
    }
    
    /**
     * @brief 選択中スロットを取得
     */
    int GetSelectedSlot() const { return selectedSlot_; }
    
    /**
     * @brief 選択中スロットを設定
     */
    void SetSelectedSlot(int slot) { selectedSlot_ = slot; }

private:
    void UpdateDeckSlotPositions(int slotCount) {
        deckSlots_.resize(slotCount);
        
        float slotWidth = FHDUI::DECK_SLOT_WIDTH;
        float slotHeight = FHDUI::DECK_SLOT_HEIGHT;
        float spacing = FHDUI::DECK_SLOT_SPACING;
        float totalWidth = slotCount * spacing;
        float startX = (static_cast<float>(FHD::RENDER_WIDTH) - totalWidth) / 2.0f + 7.5f;
        float y = FHDUI::DECK_SLOT_Y;
        
        for (int i = 0; i < slotCount; ++i) {
            deckSlots_[i].bounds = {
                startX + i * spacing, 
                y, 
                slotWidth, 
                slotHeight
            };
            deckSlots_[i].slotIndex = i;
            deckSlots_[i].isHovered = false;
            deckSlots_[i].isPressed = false;
        }
    }
    
    void DrawSingleSlot(
        const DeckSlotUI& uiSlot, 
        const DeckSlot& slot,
        const SpawnManager& spawns,
        Core::GameContext& ctx,
        int index
    ) {
        Rectangle bounds = uiSlot.bounds;
        
        // 背景色決定
        Color bgColor;
        if (!slot.isReady) {
            bgColor = Color{60, 60, 70, 255};
        } else if (index == selectedSlot_) {
            bgColor = Color{80, 100, 140, 255};
        } else if (uiSlot.isHovered) {
            bgColor = Color{70, 80, 100, 255};
        } else {
            bgColor = Color{50, 55, 65, 255};
        }
        
        DrawRectangleRec(bounds, bgColor);
        
        // 枠線
        Color borderColor = (index == selectedSlot_) 
            ? Color{100, 150, 255, 255} 
            : Color{80, 80, 100, 255};
        DrawRectangleLinesEx(bounds, 2, borderColor);
        
        // キャラクター名
        std::string displayName = slot.characterId;
        if (displayName.length() > 12) {
            displayName = displayName.substr(0, 10) + "..";
        }
        Core::RDrawText(displayName.c_str(), 
                 static_cast<int>(bounds.x + 5), 
                 static_cast<int>(bounds.y + 5), 
                 12, WHITE);
        
        // コスト
        auto cost = spawns.GetCharacterCost(index, ctx);
        if (cost.has_value()) {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%.0f", cost.value());
            
            Color costColor = (spawns.GetCurrentCost() >= cost.value()) 
                ? YELLOW : Color{180, 80, 80, 255};
            Core::RDrawText(costStr, 
                     static_cast<int>(bounds.x + 5), 
                     static_cast<int>(bounds.y + 22), 
                     18, costColor);
        }
        
        // クールダウンオーバーレイ
        if (!slot.isReady) {
            // 暗いオーバーレイ
            DrawRectangleRec(bounds, Fade(BLACK, 0.5f));
            
            // クールダウン表示
            char cdStr[16];
            snprintf(cdStr, sizeof(cdStr), "%.1fs", slot.cooldownRemaining);
            int textW = MeasureText(cdStr, 20);
            Core::RDrawText(cdStr, 
                     static_cast<int>(bounds.x + bounds.width/2 - textW/2), 
                     static_cast<int>(bounds.y + bounds.height/2 - 10), 
                     20, RED);
        }
        
        // キーバインド
        char keyStr[4];
        snprintf(keyStr, sizeof(keyStr), "%d", index + 1);
        Core::RDrawText(keyStr, 
                 static_cast<int>(bounds.x + bounds.width - 15), 
                 static_cast<int>(bounds.y + bounds.height - 18), 
                 14, LIGHTGRAY);
    }
    
    void DrawWaveInfo(const WaveManager& waves, int x, int y) {
        char waveText[32];
        snprintf(waveText, sizeof(waveText), "Wave %d/%d", 
                 waves.GetCurrentWaveNumber(), waves.GetTotalWaves());
        Core::RDrawText(waveText, x, y, 22, WHITE);
        
        // Wave進行バー
        float progress = static_cast<float>(waves.GetCurrentWaveNumber()) / 
                        static_cast<float>(waves.GetTotalWaves());
        DrawRectangle(x, y + 28, 150, 8, Color{40, 40, 50, 255});
        DrawRectangle(x, y + 28, static_cast<int>(150 * progress), 8, Color{100, 180, 255, 255});
        DrawRectangleLinesEx(Rectangle{(float)x, (float)(y + 28), 150, 8}, 1, Color{80, 80, 100, 255});
    }
    
    void DrawCostGauge(const SpawnManager& spawns, int x, int y) {
        // コスト数値
        char costText[32];
        snprintf(costText, sizeof(costText), "Cost: %.0f", spawns.GetCurrentCost());
        Core::RDrawText(costText, x, y, 20, YELLOW);
        
        // コストゲージ
        float costPercent = spawns.GetCurrentCost() / spawns.GetMaxCost();
        DrawRectangle(x, y + 25, 180, 12, Color{40, 40, 50, 255});
        DrawRectangle(x, y + 25, static_cast<int>(180 * costPercent), 12, Color{255, 200, 50, 255});
        DrawRectangleLinesEx(Rectangle{(float)x, (float)(y + 25), 180, 12}, 1, Color{80, 80, 100, 255});
        
        // 最大コスト
        char maxText[16];
        snprintf(maxText, sizeof(maxText), "/%.0f", spawns.GetMaxCost());
        Core::RDrawText(maxText, x + 185, y + 23, 14, GRAY);
    }
    
    void DrawBaseHealthBars(const GameStateManager& gameState, int x, int y) {
        // プレイヤー拠点HP
        Core::RDrawText("Base:", x, y, 14, LIGHTGRAY);
        float playerHp = gameState.GetBaseHealthPercent();
        DrawRectangle(x, y + 18, 80, 10, Color{40, 40, 50, 255});
        DrawRectangle(x, y + 18, static_cast<int>(80 * playerHp), 10, BLUE);
        
        // 敵拠点HP
        Core::RDrawText("Enemy:", x, y + 32, 14, LIGHTGRAY);
        float enemyHp = gameState.GetEnemyBaseHealthPercent();
        DrawRectangle(x, y + 50, 80, 10, Color{40, 40, 50, 255});
        DrawRectangle(x, y + 50, static_cast<int>(80 * enemyHp), 10, RED);
    }
    
    void DrawUnitHPBar(float x, float y, float width, float hpPercent, bool isEnemy) {
        float height = 6.0f;
        float halfWidth = width / 2.0f;
        
        // 背景
        DrawRectangle(
            static_cast<int>(x - halfWidth), 
            static_cast<int>(y), 
            static_cast<int>(width), 
            static_cast<int>(height), 
            Color{30, 30, 30, 200}
        );
        
        // HP
        Color hpColor = isEnemy ? RED : GREEN;
        if (hpPercent < 0.25f) hpColor = Color{255, 100, 50, 255};
        else if (hpPercent < 0.5f) hpColor = YELLOW;
        
        DrawRectangle(
            static_cast<int>(x - halfWidth), 
            static_cast<int>(y), 
            static_cast<int>(width * hpPercent), 
            static_cast<int>(height), 
            hpColor
        );
        
        // 枠
        DrawRectangleLines(
            static_cast<int>(x - halfWidth), 
            static_cast<int>(y), 
            static_cast<int>(width), 
            static_cast<int>(height), 
            Color{60, 60, 60, 200}
        );
    }
    
    void DrawBase(int x, int y, int width, int height, float hpPercent, bool isEnemy) {
        int halfHeight = height / 2;
        
        // 拠点本体
        Color baseColor = isEnemy ? Color{80, 20, 20, 255} : Color{20, 40, 80, 255};
        DrawRectangle(x, y - halfHeight, width, height, baseColor);
        
        // HP表示（縦ゲージ）
        int hpBarWidth = width - 10;
        int hpBarHeight = height - 10;
        int hpX = x + 5;
        int hpY = y - halfHeight + 5;
        
        DrawRectangle(hpX, hpY, hpBarWidth, hpBarHeight, Color{20, 20, 20, 255});
        
        int filledHeight = static_cast<int>(hpBarHeight * hpPercent);
        int emptyHeight = hpBarHeight - filledHeight;
        
        Color hpColor = isEnemy ? RED : BLUE;
        DrawRectangle(hpX, hpY + emptyHeight, hpBarWidth, filledHeight, hpColor);
        
        // ラベル
        const char* label = isEnemy ? "ENEMY" : "BASE";
        int labelWidth = MeasureText(label, 10);
        Core::RDrawText(label, x + width/2 - labelWidth/2, y + halfHeight + 5, 10, WHITE);
    }
    
    void DrawOverlayPanel(
        const char* title, 
        const char* message, 
        Color bgColor,
        const char* subMessage = nullptr
    ) {
        // 画面全体を暗く
        DrawRectangle(0, 0, FHD::RENDER_WIDTH, FHD::RENDER_HEIGHT, Fade(BLACK, 0.5f));
        
        // パネル（FHD座標中央）
        int panelWidth = 500;
        int panelHeight = subMessage ? 180 : 140;
        int panelX = FHD::RENDER_WIDTH / 2 - panelWidth / 2;
        int panelY = FHD::RENDER_HEIGHT / 2 - panelHeight / 2;
        
        DrawRectangle(panelX, panelY, panelWidth, panelHeight, bgColor);
        DrawRectangleLinesEx(
            Rectangle{(float)panelX, (float)panelY, (float)panelWidth, (float)panelHeight}, 
            3, WHITE
        );
        
        // タイトル（FHD用に大きめ）
        int titleWidth = MeasureText(title, 56);
        Core::RDrawText(title, FHD::RENDER_WIDTH / 2 - titleWidth / 2, panelY + 20, 56, WHITE);
        
        // メッセージ
        int msgWidth = MeasureText(message, 24);
        Core::RDrawText(message, FHD::RENDER_WIDTH / 2 - msgWidth / 2, panelY + 90, 24, LIGHTGRAY);
        
        // サブメッセージ
        if (subMessage) {
            int subWidth = MeasureText(subMessage, 20);
            Core::RDrawText(subMessage, FHD::RENDER_WIDTH / 2 - subWidth / 2, panelY + 130, 20, GRAY);
        }
    }
    
    // FHD固定座標
    int renderWidth_ = FHD::RENDER_WIDTH;
    int renderHeight_ = FHD::RENDER_HEIGHT;
    int selectedSlot_ = 0;
    
    std::vector<DeckSlotUI> deckSlots_;
    SlotClickCallback slotClickCallback_;
};

} // namespace TD::UI
