#pragma once

#include "../IUIComponent.hpp"
#include <map>
#include <functional>

namespace game {
namespace core {
namespace ui {

/// @brief カードコンテンツ構造体
struct CardContent {
    std::string title;
    std::string description;
    std::string imageId;      // テクスチャID
    std::map<std::string, std::string> metadata;  // 追加情報
};

/// @brief カード型UIコンポーネント
///
/// キャラクター紹介、ガチャ結果などに使用されるカード型UI。
/// タイトル、説明、画像、メタデータを表示できます。
class Card : public IUIComponent {
public:
    Card();
    ~Card() = default;

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

    UIComponentType GetType() const override { return UIComponentType::Card; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

    // Card固有メソッド
    /// @brief カードコンテンツを設定
    /// @param content カードコンテンツ
    void SetContent(const CardContent& content);

    /// @brief カードコンテンツを取得
    /// @return カードコンテンツ
    const CardContent& GetContent() const;

    /// @brief クリックコールバックを設定
    /// @param callback クリック時に呼び出されるコールバック関数
    void SetOnClickCallback(std::function<void()> callback);

    /// @brief アクションIDを設定（P1: 構造化イベント用）
    /// @param actionId アクションID（例: "select_card"）
    void SetActionId(const std::string& actionId);

    /// @brief アクションIDを取得
    /// @return アクションID
    const std::string& GetActionId() const;

private:
    Rect bounds_;
    Margin margin_;
    CardContent content_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    bool isHovered_;
    std::function<void()> onClickCallback_;
    std::string id_;
    std::string actionId_;  // P1: 構造化イベント用
};

} // namespace ui
} // namespace core
} // namespace game
