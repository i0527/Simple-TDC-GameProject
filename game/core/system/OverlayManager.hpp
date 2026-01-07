#pragma once

#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../states/overlays/IOverlay.hpp"
#include <memory>
#include <vector>

namespace game {
namespace core {

// 前方宣言
class BaseSystemAPI;

/// @brief オーバーレイ管理クラス
///
/// オーバーレイのスタック管理（LIFO）を行います。
/// 最上層のオーバーレイのみUpdateを実行し、すべてのオーバーレイをRenderします。
class OverlayManager {
public:
    OverlayManager();
    ~OverlayManager();

    /// @brief オーバーレイをスタックに追加
    /// @param state オーバーレイのステート
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @return 成功時true、失敗時false
    bool PushOverlay(OverlayState state, BaseSystemAPI* systemAPI);

    /// @brief 最上層のオーバーレイをスタックから削除
    void PopOverlay();

    /// @brief すべてのオーバーレイをスタックから削除
    void PopAllOverlays();

    /// @brief オーバーレイの更新処理
    /// @param ctx 共有コンテキスト
    /// @param deltaTime デルタタイム（秒）
    /// @note 最上層のオーバーレイのみ更新し、クローズ/遷移リクエストを処理
    void Update(SharedContext& ctx, float deltaTime);

    /// @brief オーバーレイの描画処理
    /// @param ctx 共有コンテキスト
    /// @note すべてのオーバーレイを描画（奥のものが見えるように）
    void Render(SharedContext& ctx);

    /// @brief オーバーレイのクリーンアップ
    void Shutdown();

    /// @brief スタックが空かどうか
    /// @return 空の場合true
    bool IsEmpty() const;

    /// @brief スタック内のオーバーレイ数
    /// @return オーバーレイ数
    size_t Count() const;

    /// @brief 最上層のオーバーレイを取得
    /// @return 最上層のオーバーレイへのポインタ、空の場合はnullptr
    IOverlay* GetTopOverlay() const;

    /// @brief 指定されたステートのオーバーレイがアクティブかどうか
    /// @param state 確認するオーバーレイステート
    /// @return アクティブな場合true
    bool IsOverlayActive(OverlayState state) const;

    // P0: 遷移リクエスト
    /// @brief 遷移リクエストがあるかどうか
    /// @return 遷移リクエストがある場合true
    bool HasTransitionRequest() const { return hasTransitionRequest_; }

    /// @brief 遷移リクエストされたステートを取得
    /// @return 遷移先のステート
    GameState GetRequestedTransition() const { return requestedTransition_; }

    /// @brief 遷移リクエストをクリア
    void ClearTransitionRequest() { hasTransitionRequest_ = false; }

private:
    std::vector<std::unique_ptr<IOverlay>> stack_;

    // P0: オーバーレイからの遷移要求をバッファ
    GameState requestedTransition_ = GameState::Initializing;
    bool hasTransitionRequest_ = false;

    /// @brief オーバーレイインスタンスを作成
    /// @param state オーバーレイのステート
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @return 作成されたオーバーレイのunique_ptr、失敗時はnullptr
    /// @note フェーズ3で各オーバーレイクラスが実装されたら、ここでインスタンス化
    std::unique_ptr<IOverlay> CreateOverlay(OverlayState state, BaseSystemAPI* systemAPI);
};

} // namespace core
} // namespace game
