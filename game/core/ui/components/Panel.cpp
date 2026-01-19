#include "Panel.hpp"
#include "../UIEvent.hpp"
#include "../../../utils/Log.h"
#include <imgui.h>
#include <algorithm>

namespace game {
namespace core {
namespace ui {

Panel::Panel()
    : bounds_{0.0f, 0.0f, 400.0f, 300.0f}
    , margin_{0.0f, 0.0f, 0.0f, 0.0f}
    , visible_(true)
    , enabled_(true)
    , id_("")
{
}

bool Panel::Initialize() {
    return true;
}

void Panel::Update(float deltaTime) {
    // 子要素の更新
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}

void Panel::Render() {
    if (!visible_) {
        return;
    }

    // 親ウィンドウの位置を取得（親ウィンドウが存在する場合）
    // ImGui::GetWindowPos()は現在のウィンドウ（親ウィンドウ）の位置を返す
    // Panel::Render()が呼ばれる時点では、親ウィンドウがBegin()されている状態なので、
    // ImGui::GetWindowPos()は親ウィンドウの位置を返す
    ImVec2 absolutePos;
    ImVec2 parentPos(0.0f, 0.0f);
    
    // ルートパネルでない場合のみ親ウィンドウの位置を取得
    if (!isRoot_) {
        // 安全のためにコンテキストと現在のウィンドウをチェック（内部的にはCurrentWindowをチェック）
        // ただし、ルートパネルでないという指定がある場合は親が存在することを期待する
        parentPos = ImGui::GetWindowPos();
    }

    // 親ウィンドウの位置を基準に絶対座標を計算
    absolutePos = ImVec2(
        bounds_.x + margin_.left + parentPos.x,
        bounds_.y + margin_.top + parentPos.y
    );

    ImGui::SetNextWindowPos(absolutePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds_.width, bounds_.height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (!enabled_) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(("Panel##" + id_).c_str(), nullptr, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        ImU32 bgColor = IM_COL32(50, 50, 50, 255);
        drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), bgColor);

        // 子要素の描画
        for (auto& child : children_) {
            if (child && child->IsVisible()) {
                child->Render();
            }
        }
    }
    ImGui::End();
}

void Panel::Shutdown() {
    children_.clear();
}

void Panel::SetPosition(float x, float y) {
    bounds_.x = x;
    bounds_.y = y;
}

void Panel::SetSize(float width, float height) {
    bounds_.width = width;
    bounds_.height = height;
}

Rect Panel::GetBounds() const {
    return Rect{
        bounds_.x + margin_.left,
        bounds_.y + margin_.top,
        bounds_.width,
        bounds_.height
    };
}

void Panel::SetMargin(const Margin& margin) {
    margin_ = margin;
}

void Panel::SetVisible(bool visible) {
    visible_ = visible;
}

bool Panel::IsVisible() const {
    return visible_;
}

void Panel::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Panel::IsEnabled() const {
    return enabled_;
}

UIEventResult Panel::HandleEvent(const UIEvent& ev) {
    UIEventResult result;

    if (!visible_ || !enabled_) {
        return result;
    }

    // 子要素にイベントを伝播（子要素が処理した場合は結果を返す）
    for (auto& child : children_) {
        if (child && child->IsEnabled() && child->IsVisible()) {
            UIEventResult childResult = child->HandleEvent(ev);
            if (childResult.handled) {
                result = childResult;
                return result;
            }
        }
    }

    // 子要素が処理しなかった場合、パネル自身の領域内かチェック
    const Rect r = GetBounds();
    const bool inside =
        ev.x >= r.x && ev.x <= r.x + r.width &&
        ev.y >= r.y && ev.y <= r.y + r.height;

    if (inside && (ev.type == UIEventType::Click || ev.type == UIEventType::Hover)) {
        result.handled = true;
        result.componentId = id_;
    }

    return result;
}

bool Panel::OnMouseClick(float x, float y) {
    if (!enabled_ || !visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        // 子要素にクリックイベントを伝播
        for (auto& child : children_) {
            if (child && child->IsEnabled() && child->OnMouseClick(x, y)) {
                return true;
            }
        }
        return true;
    }
    return false;
}

bool Panel::OnMouseHover(float x, float y) {
    if (!visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        // 子要素にホバーイベントを伝播
        for (auto& child : children_) {
            if (child && child->OnMouseHover(x, y)) {
                return true;
            }
        }
        return true;
    }
    return false;
}

bool Panel::OnKey(int key) {
    if (!enabled_ || !visible_) {
        return false;
    }

    // 子要素にキーイベントを伝播
    for (auto& child : children_) {
        if (child && child->IsEnabled() && child->OnKey(key)) {
            return true;
        }
    }
    return false;
}

void Panel::AddChild(std::shared_ptr<IUIComponent> child) {
    if (child) {
        children_.push_back(child);
    }
}

void Panel::RemoveChild(std::shared_ptr<IUIComponent> child) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&child](const std::shared_ptr<IUIComponent>& c) {
                return c == child;
            }),
        children_.end()
    );
}

const std::string& Panel::GetId() const {
    return id_;
}

void Panel::SetId(const std::string& id) {
    id_ = id;
}

} // namespace ui
} // namespace core
} // namespace game
