#include "Button.hpp"
#include "../UIEvent.hpp"
#include "../../../utils/Log.h"
#include <imgui.h>
#include <algorithm>

namespace game {
namespace core {
namespace ui {

Button::Button()
    : bounds_{0.0f, 0.0f, 100.0f, 30.0f}
    , margin_{0.0f, 0.0f, 0.0f, 0.0f}
    , visible_(true)
    , enabled_(true)
    , isHovered_(false)
    , id_("")
    , label_("Button")
    , actionId_("")
{
}

bool Button::Initialize() {
    return true;
}

void Button::Update(float deltaTime) {
    // 子要素の更新
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}

void Button::Render() {
    if (!visible_) {
        return;
    }

    // 親ウィンドウの位置を取得（親ウィンドウが存在する場合）
    // ImGui::GetWindowPos()は現在のウィンドウ（親ウィンドウ）の位置を返す
    ImVec2 absolutePos;
    ImVec2 parentPos = ImGui::GetWindowPos();
    // 親ウィンドウが存在する場合（位置が(0,0)でない場合）、親ウィンドウの位置を基準に絶対座標を計算
    if (parentPos.x != 0.0f || parentPos.y != 0.0f) {
        absolutePos = ImVec2(
            bounds_.x + margin_.left + parentPos.x,
            bounds_.y + margin_.top + parentPos.y
        );
    } else {
        // 親ウィンドウが存在しない場合、絶対座標を使用
        absolutePos = ImVec2(bounds_.x + margin_.left, bounds_.y + margin_.top);
    }

    // ImGuiでボタンを描画
    ImGui::SetNextWindowPos(absolutePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds_.width, bounds_.height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

    if (!enabled_) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(("Button##" + id_).c_str(), nullptr, flags)) {
        // ボタンのスタイル
        if (!enabled_) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        } else if (isHovered_) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }

        // ボタンを描画
        if (ImGui::Button(label_.c_str(), ImVec2(bounds_.width, bounds_.height))) {
            if (enabled_ && onClickCallback_) {
                onClickCallback_();
            }
        }

        // ホバー状態のチェック
        isHovered_ = ImGui::IsItemHovered();

        if (!enabled_ || isHovered_) {
            ImGui::PopStyleColor(3);
        }

        // 子要素の描画
        for (auto& child : children_) {
            if (child && child->IsVisible()) {
                child->Render();
            }
        }
    }
    ImGui::End();
}

void Button::Shutdown() {
    children_.clear();
    onClickCallback_ = nullptr;
}

void Button::SetPosition(float x, float y) {
    bounds_.x = x;
    bounds_.y = y;
}

void Button::SetSize(float width, float height) {
    bounds_.width = width;
    bounds_.height = height;
}

Rect Button::GetBounds() const {
    return Rect{
        bounds_.x + margin_.left,
        bounds_.y + margin_.top,
        bounds_.width,
        bounds_.height
    };
}

void Button::SetMargin(const Margin& margin) {
    margin_ = margin;
}

void Button::SetVisible(bool visible) {
    visible_ = visible;
}

bool Button::IsVisible() const {
    return visible_;
}

void Button::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Button::IsEnabled() const {
    return enabled_;
}

UIEventResult Button::HandleEvent(const UIEvent& ev) {
    UIEventResult result;

    if (!visible_ || !enabled_) {
        return result;
    }

    switch (ev.type) {
    case UIEventType::Click: {
        const Rect r = GetBounds();
        const bool inside =
            ev.x >= r.x && ev.x <= r.x + r.width &&
            ev.y >= r.y && ev.y <= r.y + r.height;

        if (inside) {
            result.handled = true;
            result.componentId = id_;
            result.actionId = actionId_;
            if (onClickCallback_) {
                onClickCallback_();
            }
        }
        break;
    }
    case UIEventType::Hover: {
        const Rect r = GetBounds();
        const bool inside =
            ev.x >= r.x && ev.x <= r.x + r.width &&
            ev.y >= r.y && ev.y <= r.y + r.height;
        isHovered_ = inside;
        if (inside) {
            result.handled = true;
            result.componentId = id_;
        }
        break;
    }
    case UIEventType::Key: {
        // ボタンはEnter/Spaceキーでクリック可能
        if (ev.key == 32 || ev.key == 257) { // Space or Enter
            result.handled = true;
            result.componentId = id_;
            result.actionId = actionId_;
            if (onClickCallback_) {
                onClickCallback_();
            }
        }
        break;
    }
    case UIEventType::None:
        break;
    }

    // 子要素を持つならここで子へ伝播（現在はButtonは子要素を持たない想定）

    return result;
}

bool Button::OnMouseClick(float x, float y) {
    if (!enabled_ || !visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        if (onClickCallback_) {
            onClickCallback_();
        }
        return true;
    }
    return false;
}

bool Button::OnMouseHover(float x, float y) {
    if (!visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        isHovered_ = true;
        return true;
    }
    isHovered_ = false;
    return false;
}

bool Button::OnKey(int key) {
    // ボタンはEnter/Spaceキーでクリック可能
    if (!enabled_ || !visible_) {
        return false;
    }

    if (key == 32 || key == 257) { // Space or Enter
        if (onClickCallback_) {
            onClickCallback_();
        }
        return true;
    }
    return false;
}

void Button::AddChild(std::shared_ptr<IUIComponent> child) {
    if (child) {
        children_.push_back(child);
    }
}

void Button::RemoveChild(std::shared_ptr<IUIComponent> child) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&child](const std::shared_ptr<IUIComponent>& c) {
                return c == child;
            }),
        children_.end()
    );
}

const std::string& Button::GetId() const {
    return id_;
}

void Button::SetId(const std::string& id) {
    id_ = id;
}

void Button::SetLabel(const std::string& label) {
    label_ = label;
}

const std::string& Button::GetLabel() const {
    return label_;
}

void Button::SetOnClickCallback(std::function<void()> callback) {
    onClickCallback_ = callback;
}

void Button::SetActionId(const std::string& actionId) {
    actionId_ = actionId;
}

const std::string& Button::GetActionId() const {
    return actionId_;
}

} // namespace ui
} // namespace core
} // namespace game
