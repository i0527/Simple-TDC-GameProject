#pragma once

#include "../IUIComponent.hpp"
#include <functional>

namespace game {
namespace core {

class BaseSystemAPI;
class UISystemAPI;

namespace ui {

/// @brief ボタンUIコンポーネント
///
/// クリック可能なボタンコンポーネント。
/// クリックコールバック機能を提供します。
class Button : public IUIComponent {
public:
    Button();
    ~Button() = default;

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

    UIComponentType GetType() const override { return UIComponentType::Button; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

    // Button固有メソッド
    /// @brief ラベルを設定
    /// @param label ボタンのラベル
    void SetLabel(const std::string& label);

    /// @brief ラベルを取得
    /// @return ボタンのラベル
    const std::string& GetLabel() const;

    /// @brief クリックコールバックを設定
    /// @param callback クリック時に呼び出されるコールバック関数
    void SetOnClickCallback(std::function<void()> callback);

    /// @brief アクションIDを設定（P1: 構造化イベント用）
    /// @param actionId アクションID（例: "start_battle"）
    void SetActionId(const std::string& actionId);

    /// @brief アクションIDを取得
    /// @return アクションID
    const std::string& GetActionId() const;

    /// @brief 描画用UIシステムAPIを設定
    void SetUISystemAPI(::game::core::UISystemAPI* uiAPI) { uiAPI_ = uiAPI; }

    /// @brief オーディオ用システムAPIを設定
    void SetBaseSystemAPI(::game::core::BaseSystemAPI* systemAPI) { baseSystemAPI_ = systemAPI; }

private:
    void PlayClickSound();

    Rect bounds_;
    Margin margin_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    bool isHovered_;
    std::string id_;
    std::string label_;
    std::string actionId_;  // P1: 構造化イベント用
    std::function<void()> onClickCallback_;
    ::game::core::UISystemAPI* uiAPI_ = nullptr;
    ::game::core::BaseSystemAPI* baseSystemAPI_ = nullptr;
};

} // namespace ui
} // namespace core
} // namespace game
