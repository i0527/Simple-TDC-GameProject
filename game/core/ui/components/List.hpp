#pragma once

#include "../IUIComponent.hpp"
#include <vector>
#include <functional>

namespace game {
namespace core {

class UISystemAPI;

namespace ui {

/// @brief リストアイテム構造体
struct ListItem {
    std::string id;
    std::string label;
    std::string value;
    bool enabled = true;
};

/// @brief リスト型UIコンポーネント
///
/// インベントリ、ランキングなどに使用されるリスト型UI。
/// スクロール、選択、ページネーション機能を提供します。
class List : public IUIComponent {
public:
    List();
    ~List() = default;

    // IUIComponent実装
    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    void SetPosition(float x, float y) override;
    void SetSize(float width, float height) override;
    Rect GetBounds() const override;
    void SetMargin(const Margin& margin) override;

    void SetVisible(bool visible) override;
    bool IsVisible() const override;
    void SetEnabled(bool enabled) override;
    bool IsEnabled() const override;

    // P1: 構造化イベント
    UIEventResult HandleEvent(const UIEvent& ev) override;

    bool OnMouseClick(float x, float y) override;
    bool OnMouseHover(float x, float y) override;
    bool OnKey(int key) override;

    void AddChild(std::shared_ptr<IUIComponent> child) override;
    void RemoveChild(std::shared_ptr<IUIComponent> child) override;

    UIComponentType GetType() const override { return UIComponentType::List; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

    // List固有メソッド
    /// @brief アイテムを追加
    /// @param item 追加するアイテム
    void AddItem(const ListItem& item);

    /// @brief アイテムをすべて削除
    void ClearItems();

    /// @brief アイテムを削除
    /// @param id 削除するアイテムのID
    void RemoveItem(const std::string& id);

    /// @brief 選択インデックスを設定
    /// @param index 選択するインデックス
    void SetSelectedIndex(int index);

    /// @brief 選択インデックスを取得
    /// @return 選択されているインデックス（-1の場合は未選択）
    int GetSelectedIndex() const;

    /// @brief 選択されているアイテムを取得
    /// @return 選択されているアイテムへのポインタ、未選択の場合はnullptr
    const ListItem* GetSelectedItem() const;

    /// @brief アイテムの高さを設定
    /// @param height アイテムの高さ
    void SetItemHeight(float height);

    /// @brief 1ページあたりのアイテム数を設定
    /// @param count アイテム数
    void SetItemsPerPage(int count);

    /// @brief 選択変更コールバックを設定
    /// @param callback 選択が変更されたときに呼び出されるコールバック関数
    void SetOnSelectionChanged(std::function<void(const ListItem&)> callback);

    /// @brief アイテム数を取得
    /// @return アイテム数
    size_t GetItemCount() const;

    /// @brief 画像テクスチャ描画を有効/無効化
    void SetUseTextures(bool useTextures);

    /// @brief 描画用UIシステムAPIを設定
    void SetUISystemAPI(::game::core::UISystemAPI* uiAPI) { uiAPI_ = uiAPI; }

private:
    Rect bounds_;
    Margin margin_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    std::string id_;

    std::vector<ListItem> items_;
    int selectedIndex_;
    int itemsPerPage_;
    float itemHeight_;
    float scrollOffset_;
    bool useTextures_ = true;
    std::function<void(const ListItem&)> onSelectionChanged_;
    ::game::core::UISystemAPI* uiAPI_ = nullptr;
};

} // namespace ui
} // namespace core
} // namespace game
