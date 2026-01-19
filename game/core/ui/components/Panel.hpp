#pragma once

#include "../IUIComponent.hpp"

namespace game {
namespace core {

class UISystemAPI;

namespace ui {

/// @brief 繝代ロ繝ｫUI繧ｳ繝ｳ繝昴・繝阪Φ繝・
///
/// 蜈ｱ騾啅I縺ｨ縺励※菴ｿ逕ｨ縺輔ｌ繧九ヱ繝阪Ν繧ｳ繝ｳ繝昴・繝阪Φ繝医・
/// 蟄占ｦ∫ｴ縺ｮ繝ｬ繧､繧｢繧ｦ繝育ｮ｡逅・ｒ陦後＞縺ｾ縺吶・
class Panel : public IUIComponent {
public:
    Panel();
    ~Panel() = default;

    // IUIComponent螳溯｣・
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

    // P1: 讒矩蛹悶う繝吶Φ繝・
    UIEventResult HandleEvent(const UIEvent& ev) override;

    bool OnMouseClick(float x, float y) override;
    bool OnMouseHover(float x, float y) override;
    bool OnKey(int key) override;

    void AddChild(std::shared_ptr<IUIComponent> child) override;
    void RemoveChild(std::shared_ptr<IUIComponent> child) override;

    UIComponentType GetType() const override { return UIComponentType::Panel; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

    /// @brief 繝ｫ繝ｼ繝医ヱ繝阪Ν・郁ｦｪ繧ｦ繧｣繝ｳ繝峨え繧呈戟縺溘↑縺・ｼ峨°縺ｩ縺・°繧定ｨｭ螳・
    void SetRoot(bool isRoot) { isRoot_ = isRoot; }
    bool IsRoot() const { return isRoot_; }

    /// @brief 謠冗判逕ｨUI繧ｷ繧ｹ繝・ΒAPI繧定ｨｭ螳・
    void SetUISystemAPI(::game::core::UISystemAPI* uiAPI) { uiAPI_ = uiAPI; }

private:
    Rect bounds_;
    Margin margin_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    bool isRoot_ = false;
    std::string id_;
    ::game::core::UISystemAPI* uiAPI_ = nullptr;
};

} // namespace ui
} // namespace core
} // namespace game
