#pragma once

#include "../api/BaseSystemAPI.hpp"
#include <string>
#include <vector>
#include <raylib.h>

namespace game {
namespace core {
namespace ui {

/// @brief ユニットリストアイテム
struct UnitListItem {
    std::string unitId;
    std::string displayName;
    int currentCount;
    int maxCount;
    int costGold;
    bool isSelected;
};

/// @brief ゲームシーンHUD描画クラス
///
/// 責務:
/// - Top HUD（Wave、HP、Gold、ゲーム速度、ボタン）
/// - Left Panel（ミニマップ・背景表示）
/// - Right Panel（ユニット選択パネル）
/// - Field UI（選択・ホバー表示）
class HUDRenderer {
public:
    /// @brief コンストラクタ
    /// @param sysAPI BaseSystemAPIポインタ
    explicit HUDRenderer(BaseSystemAPI* sysAPI);
    
    ~HUDRenderer() = default;

    // ========== Top HUD ==========

    /// @brief Top HUDを描画
    /// @param wave 現在のウェーブ番号
    /// @param totalWaves 総ウェーブ数
    /// @param hp 現在のHP
    /// @param maxHp 最大HP
    /// @param gold 現在のゴールド
    /// @param gameSpeed ゲーム速度（0.5, 1.0, 2.0）
    /// @param isPaused ポーズ中かどうか
    /// @param gameStateText ゲーム状態テキスト（例："準備中..."）
    void RenderTopHUD(int wave, int totalWaves, int hp, int maxHp,
                     int gold, float gameSpeed, bool isPaused,
                     const std::string& gameStateText);

    // ========== Left Panel ==========

    /// @brief Left Panel（ミニマップ）を描画
    /// @param fieldOriginX フィールド原点X座標
    /// @param fieldOriginY フィールド原点Y座標
    /// @param fieldWidth フィールド幅（ピクセル）
    /// @param fieldHeight フィールド高さ（ピクセル）
    void RenderLeftPanel(float fieldOriginX, float fieldOriginY,
                        float fieldWidth, float fieldHeight);

    // ========== Right Panel ==========

    /// @brief Right Panel（ユニット選択）を描画
    /// @param units ユニットリスト
    /// @param selectedUnitId 選択中のユニットID（空文字列なら未選択）
    void RenderRightPanel(const std::vector<UnitListItem>& units,
                         const std::string& selectedUnitId);

    // ========== Field UI ==========

    /// @brief フィールドUI（ホバー・選択表示）を描画
    /// @param hoverGx ホバー中のグリッドX座標（-1なら無効）
    /// @param hoverGy ホバー中のグリッドY座標（-1なら無効）
    /// @param selectGx 選択中のグリッドX座標（-1なら無効）
    /// @param selectGy 選択中のグリッドY座標（-1なら無効）
    /// @param isPlaceable ホバー位置が配置可能かどうか
    /// @param cellSize セルサイズ（ピクセル）
    /// @param fieldOriginX フィールド原点X座標
    /// @param fieldOriginY フィールド原点Y座標
    void RenderFieldUI(int hoverGx, int hoverGy, int selectGx, int selectGy,
                      bool isPlaceable, int cellSize,
                      float fieldOriginX, float fieldOriginY);

    // ========== ボタン検出 ==========

    /// @brief Top HUDのボタンがクリックされたかチェック
    /// @param mouseX マウスX座標
    /// @param mouseY マウスY座標
    /// @return クリックされたボタン名（"speed_0.5", "speed_1.0", "speed_2.0", "pause", "exit", ""）
    std::string CheckTopHUDButtonClick(float mouseX, float mouseY);

private:
    BaseSystemAPI* sysAPI_;

    /// @brief プログレスバーを描画
    void DrawBar(float x, float y, float width, float height,
                float current, float max, Color fillColor, Color bgColor);

    /// @brief テキストを描画（デフォルトフォント使用）
    void DrawText(float x, float y, const std::string& text,
                 int fontSize, Color color);

    /// @brief 中央揃えテキストを描画
    void DrawTextCentered(float centerX, float y, const std::string& text,
                         int fontSize, Color color);

    /// @brief ボタンを描画
    void DrawButton(float x, float y, float width, float height,
                   const std::string& label, bool isActive, Color baseColor);

    /// @brief 矩形内にマウスがあるかチェック
    bool IsMouseInRect(float mouseX, float mouseY,
                      float rectX, float rectY, float rectW, float rectH);

    // Top HUDボタン矩形（キャッシュ）
    struct ButtonRect {
        float x, y, width, height;
        std::string id;
    };
    std::vector<ButtonRect> topHudButtons_;
};

} // namespace ui
} // namespace core
} // namespace game
