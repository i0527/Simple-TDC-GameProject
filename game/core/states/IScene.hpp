#pragma once

#include "../api/BaseSystemAPI.hpp"
#include "../config/GameState.hpp"

namespace game {
namespace core {

// 前方宣言
class BaseSystemAPI;
struct SharedContext;

/// @brief 独立シーン基底インターフェース
///
/// タイトル画面、ホーム画面、ゲーム画面などの独立シーンが実装するインターフェース。
/// 現時点では将来の拡張用として定義。
/// TitleScreenは既存実装を維持し、将来的にこのインターフェースを実装する可能性がある。
class IScene {
public:
    virtual ~IScene() = default;

    /// @brief シーンの初期化
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @return 成功時true、失敗時false
    virtual bool Initialize(BaseSystemAPI* systemAPI) = 0;

    /// @brief シーンの更新処理
    /// @param deltaTime デルタタイム（秒）
    virtual void Update(float deltaTime) = 0;

    /// @brief シーンの描画処理
    virtual void Render() = 0;

    /// @brief オーバーレイ描画（シーン内のUI重ね描画）
    virtual void RenderOverlay() {}

    /// @brief HUD描画（シーン上のHUD/UI）
    virtual void RenderHUD() {}

    /// @brief ImGui描画（ImGuiフレーム内）
    virtual void RenderImGui() {}

    /// @brief シーンのクリーンアップ
    virtual void Shutdown() = 0;

    /// @brief SharedContext設定（必要な場合のみ使用）
    /// @param ctx SharedContextへのポインタ（所有権なし）
    virtual void SetSharedContext(SharedContext* ctx) {}

    /// @brief 遷移リクエストを取得
    /// @param nextState 遷移先のステート（出力）
    /// @return 遷移リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    virtual bool RequestTransition(GameState& nextState) = 0;

    /// @brief 終了リクエストを取得
    /// @return 終了リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    virtual bool RequestQuit() = 0;
};

} // namespace core
} // namespace game
