#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/GameState.hpp"
#include "../config/SharedContext.hpp"
#include <string>
#include <vector>

namespace game {
namespace core {

// 前方宣言
class InputSystemAPI;
class SceneOverlayControlAPI;

/// @brief タイトル画面専用クラス
///
/// 責務:
/// - タイトル画面の更新・描画
/// - メニュー操作（キーボード/マウス）
/// - メニューアクション実行
/// - 遷移リクエストの通知
///
/// 安全化:
/// - 初期化済みチェック
/// - メニューインデックス範囲チェック
/// - 遷移リクエストは一度だけ有効（取得後にリセット）
class TitleScreen : public IScene {
public:
    TitleScreen();
    ~TitleScreen();

    /// @brief 初期化
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @param ctx SharedContextへのポインタ（所有権なし）
    /// @return 成功時true、失敗時false
    bool Initialize(BaseSystemAPI* systemAPI) override;

    /// @brief 更新処理（入力処理・メニュー更新）
    /// @param deltaTime デルタタイム（秒）
    void Update(float deltaTime) override;

    /// @brief タイトル画面描画
    void Render() override;

    /// @brief クリーンアップ
    void Shutdown() override;

    /// @brief 遷移リクエスト取得
    /// @param nextState 遷移先のステート（出力）
    /// @return 遷移リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    bool RequestTransition(GameState& nextState) override;

    /// @brief 終了リクエスト取得
    /// @return 終了リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    bool RequestQuit() override;

    void SetSharedContext(SharedContext* ctx) override {
        sharedContext_ = ctx;
        inputAPI_ = ctx ? ctx->inputAPI : nullptr;
        sceneOverlayAPI_ = ctx ? ctx->sceneOverlayAPI : nullptr;
    }

private:
    struct Button {
        float x, y, width, height;
        std::string label;
        bool is_hovered = false;
        
        bool IsPointInside(float px, float py) const {
            return (px >= x && px <= x + width &&
                   py >= y && py <= y + height);
        }
    };
    
    // MenuItemはButtonと同じ構造を使用（統一されたボタンサイズのため）

    BaseSystemAPI* systemAPI_;
    InputSystemAPI* inputAPI_;
    SharedContext* sharedContext_;
    SceneOverlayControlAPI* sceneOverlayAPI_;
    bool isInitialized_;
    bool hasTransitionRequest_;
    GameState requestedNextState_;
    bool requestQuit_;

    // UI要素
    Button start_button_;
    Button menu_items_[3];  // ライセンス、設定、終了（統一されたボタンサイズ）
    
    std::string title_text_;
    std::string version_text_;
    
    // リソース
    Texture2D* background_texture_;
    bool has_background_;
    
    // 設定
    const int TITLE_FONT_SIZE = 108;  // 72 * 1.5
    const int BUTTON_FONT_SIZE = 48;  // 32 * 1.5
    const int MENU_FONT_SIZE = 30;    // 20 * 1.5
    const int VERSION_FONT_SIZE = 18; // 12 * 1.5
    
    // 描画ヘルパー
    void RenderBackground();
    void RenderTitle();
    void RenderStartButton();
    void RenderFooterMenu();
    void RenderVersionInfo();
    
    // イベント処理
    void OnStartButtonClick();
    void OnMenuItemClick(int item_index);
    void UpdateHoverStates();
    
    // ユーティリティ
    bool LoadBackgroundImage();
    void DrawGradientBackground();
    
    enum MenuItemType {
        LICENSE = 0,
        SETTINGS = 1,
        QUIT = 2
    };
};

} // namespace core
} // namespace game
