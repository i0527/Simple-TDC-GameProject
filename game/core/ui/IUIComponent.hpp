#pragma once

#include <string>
#include <vector>
#include <memory>
#include "UIEvent.hpp"

namespace game {
namespace core {
namespace ui {

/// @brief 矩形領域を表す構造体
struct Rect {
    float x;
    float y;
    float width;
    float height;
};

/// @brief マージンを表す構造体
struct Margin {
    float top;
    float right;
    float bottom;
    float left;
};

/// @brief UIコンポーネントの種類
enum class UIComponentType {
    Card,
    List,
    Tile,
    Panel,
    Button,
    Text,
    Image,
};

/// @brief UIコンポーネント基底インターフェース
///
/// すべてのUIコンポーネントが実装する必要があるインターフェース。
/// ライフサイクル、レイアウト、表示制御、イベント処理、子要素管理を提供します。
class IUIComponent {
public:
    virtual ~IUIComponent() = default;

    // ライフサイクル
    /// @brief コンポーネントの初期化
    /// @return 成功時true、失敗時false
    virtual bool Initialize() = 0;

    /// @brief コンポーネントの更新処理
    /// @param deltaTime デルタタイム（秒）
    virtual void Update(float deltaTime) = 0;

    /// @brief コンポーネントの描画処理
    /// @note 描画バックエンド経由で描画（現在はImGui直接呼び出し）
    virtual void Render() = 0;

    /// @brief コンポーネントのクリーンアップ
    virtual void Shutdown() = 0;

    // レイアウト
    /// @brief 位置を設定
    /// @param x X座標
    /// @param y Y座標
    virtual void SetPosition(float x, float y) = 0;

    /// @brief サイズを設定
    /// @param width 幅
    /// @param height 高さ
    virtual void SetSize(float width, float height) = 0;

    /// @brief 境界矩形を取得
    /// @return 境界矩形
    virtual Rect GetBounds() const = 0;

    /// @brief マージンを設定
    /// @param margin マージン
    virtual void SetMargin(const Margin& margin) = 0;

    // 表示制御
    /// @brief 表示状態を設定
    /// @param visible 表示する場合true
    virtual void SetVisible(bool visible) = 0;

    /// @brief 表示状態を取得
    /// @return 表示されている場合true
    virtual bool IsVisible() const = 0;

    /// @brief 有効状態を設定
    /// @param enabled 有効にする場合true
    virtual void SetEnabled(bool enabled) = 0;

    /// @brief 有効状態を取得
    /// @return 有効な場合true
    virtual bool IsEnabled() const = 0;

    // イベント（P1: 構造化イベント）
    /// @brief UIイベントを処理（P1: 構造化イベント）
    /// @param ev UIイベント
    /// @return イベント処理結果
    /// @note 呼び出し側（Overlay）が UIEvent を投げて、UIEventResult で結果を受け取る。
    ///       子要素を持つコンポーネント（Panel 等）は内部で子へ伝播する。
    virtual UIEventResult HandleEvent(const UIEvent& ev) = 0;

    // イベント（旧API - 後方互換性のため残す）
    /// @brief マウスクリックイベント（旧API）
    /// @param x クリック位置のX座標
    /// @param y クリック位置のY座標
    /// @return イベントを処理した場合true
    /// @deprecated HandleEvent() の使用を推奨
    virtual bool OnMouseClick(float x, float y) = 0;

    /// @brief マウスホバーイベント（旧API）
    /// @param x マウス位置のX座標
    /// @param y マウス位置のY座標
    /// @return イベントを処理した場合true
    /// @deprecated HandleEvent() の使用を推奨
    virtual bool OnMouseHover(float x, float y) = 0;

    /// @brief キー入力イベント（旧API）
    /// @param key キーコード
    /// @return イベントを処理した場合true
    /// @deprecated HandleEvent() の使用を推奨
    virtual bool OnKey(int key) = 0;

    // 子要素管理
    /// @brief 子要素を追加
    /// @param child 子要素へのshared_ptr
    virtual void AddChild(std::shared_ptr<IUIComponent> child) = 0;

    /// @brief 子要素を削除
    /// @param child 削除する子要素へのshared_ptr
    virtual void RemoveChild(std::shared_ptr<IUIComponent> child) = 0;

    // その他
    /// @brief コンポーネントの種類を取得
    /// @return コンポーネントの種類
    virtual UIComponentType GetType() const = 0;

    /// @brief IDを取得
    /// @return コンポーネントのID
    virtual const std::string& GetId() const = 0;

    /// @brief IDを設定
    /// @param id コンポーネントのID
    virtual void SetId(const std::string& id) = 0;
};

} // namespace ui
} // namespace core
} // namespace game
