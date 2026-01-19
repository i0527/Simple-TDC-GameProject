#pragma once

#include "../../config/SharedContext.hpp"
#include "../../config/GameState.hpp"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/UISystemAPI.hpp"

namespace game {
namespace core {

// 前方宣言
class BaseSystemAPI;
class UISystemAPI;

/// @brief オーバーレイ基底インターフェース
///
/// すべてのオーバーレイが実装する必要があるインターフェース。
/// オーバーレイのライフサイクル管理と状態遷移を提供します。
class IOverlay {
public:
    virtual ~IOverlay() = default;

    /// @brief オーバーレイの初期化
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @param uiAPI UISystemAPIへのポインタ（所有権なし）
    /// @return 成功時true、失敗時false
    virtual bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) = 0;

    /// @brief オーバーレイの更新処理
    /// @param ctx 共有コンテキスト
    /// @param deltaTime デルタタイム（秒）
    virtual void Update(SharedContext& ctx, float deltaTime) = 0;

    /// @brief オーバーレイの描画処理
    /// @param ctx 共有コンテキスト
    virtual void Render(SharedContext& ctx) = 0;

    /// @brief ImGuiを使用したオーバーレイかどうか
    /// @return ImGuiを使用する場合true
    virtual bool IsImGuiOverlay() const { return false; }

    /// @brief オーバーレイのクリーンアップ
    virtual void Shutdown() = 0;

    /// @brief オーバーレイのステートを取得
    /// @return オーバーレイのステート
    virtual OverlayState GetState() const = 0;

    /// @brief クローズリクエストを取得
    /// @return クローズリクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    virtual bool RequestClose() const = 0;

    /// @brief 終了リクエストを取得
    /// @return 終了リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    virtual bool RequestQuit() const { return false; }

    /// @brief ステート遷移リクエストを取得
    /// @param nextState 遷移先のステート（出力）
    /// @return 遷移リクエストがある場合true
    /// @note このメソッドは一度だけ有効。呼び出し後、リクエストはリセットされる
    virtual bool RequestTransition(GameState& nextState) const = 0;
};

} // namespace core
} // namespace game
