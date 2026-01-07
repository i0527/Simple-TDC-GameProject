#pragma once

#include "../IUIComponent.hpp"

namespace game {
namespace core {
namespace ui {

/// @brief パネルUIコンポーネント
///
/// 共通UIとして使用されるパネルコンポーネント。
/// 子要素のレイアウト管理を行います。
class Panel : public IUIComponent {
public:
    Panel();
    ~Panel() = default;

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

    UIComponentType GetType() const override { return UIComponentType::Panel; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

private:
    Rect bounds_;
    Margin margin_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    std::string id_;
};

} // namespace ui
} // namespace core
} // namespace game
