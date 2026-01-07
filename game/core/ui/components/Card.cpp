#include "Card.hpp"
#include "../UIEvent.hpp"
#include "../../../utils/Log.h"
#include <imgui.h>
#include <algorithm>

namespace game {
namespace core {
namespace ui {

Card::Card()
    : bounds_{0.0f, 0.0f, 200.0f, 300.0f}
    , margin_{0.0f, 0.0f, 0.0f, 0.0f}
    , visible_(true)
    , enabled_(true)
    , isHovered_(false)
    , id_("")
    , actionId_("")
{
}

bool Card::Initialize() {
    return true;
}

void Card::Update(float deltaTime) {
    // 子要素の更新
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}

void Card::Render() {
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

    // ImGuiでカードを描画
    ImGui::SetNextWindowPos(absolutePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds_.width, bounds_.height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    if (!enabled_) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(("Card##" + id_).c_str(), nullptr, flags)) {
        // ホバー状態のチェック
        isHovered_ = ImGui::IsWindowHovered();

        // カードの背景
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        ImU32 bgColor = isHovered_ ? IM_COL32(60, 60, 60, 255) : IM_COL32(40, 40, 40, 255);
        drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), bgColor);

        // タイトル
        if (!content_.title.empty()) {
            ImGui::Text("%s", content_.title.c_str());
            ImGui::Separator();
        }

        // 画像（将来実装: テクスチャIDから画像を取得して表示）
        if (!content_.imageId.empty()) {
            // プレースホルダー: 画像表示領域
            ImVec2 imageSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x);
            ImGui::InvisibleButton("##image", imageSize);
            ImGui::SameLine();
        }

        // 説明
        if (!content_.description.empty()) {
            ImGui::TextWrapped("%s", content_.description.c_str());
        }

        // メタデータ
        if (!content_.metadata.empty()) {
            ImGui::Separator();
            for (const auto& [key, value] : content_.metadata) {
                ImGui::Text("%s: %s", key.c_str(), value.c_str());
            }
        }

        // 子要素の描画
        for (auto& child : children_) {
            if (child && child->IsVisible()) {
                child->Render();
            }
        }

        // クリック処理
        if (enabled_ && ImGui::IsItemClicked()) {
            if (onClickCallback_) {
                onClickCallback_();
            }
        }
    }
    ImGui::End();
}

void Card::Shutdown() {
    children_.clear();
    onClickCallback_ = nullptr;
}

void Card::SetPosition(float x, float y) {
    bounds_.x = x;
    bounds_.y = y;
}

void Card::SetSize(float width, float height) {
    bounds_.width = width;
    bounds_.height = height;
}

Rect Card::GetBounds() const {
    return Rect{
        bounds_.x + margin_.left,
        bounds_.y + margin_.top,
        bounds_.width,
        bounds_.height
    };
}

void Card::SetMargin(const Margin& margin) {
    margin_ = margin;
}

void Card::SetVisible(bool visible) {
    visible_ = visible;
}

bool Card::IsVisible() const {
    return visible_;
}

void Card::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Card::IsEnabled() const {
    return enabled_;
}

UIEventResult Card::HandleEvent(const UIEvent& ev) {
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
    case UIEventType::Key:
        // Cardはキーイベントを処理しない
        break;
    case UIEventType::None:
        break;
    }

    // 子要素を持つならここで子へ伝播
    for (auto& child : children_) {
        if (child && child->IsEnabled() && child->IsVisible()) {
            UIEventResult childResult = child->HandleEvent(ev);
            if (childResult.handled) {
                result = childResult;
                return result;
            }
        }
    }

    return result;
}

bool Card::OnMouseClick(float x, float y) {
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

bool Card::OnMouseHover(float x, float y) {
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

bool Card::OnKey(int key) {
    // カードはキー入力を受け付けない
    return false;
}

void Card::AddChild(std::shared_ptr<IUIComponent> child) {
    if (child) {
        children_.push_back(child);
    }
}

void Card::RemoveChild(std::shared_ptr<IUIComponent> child) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&child](const std::shared_ptr<IUIComponent>& c) {
                return c == child;
            }),
        children_.end()
    );
}

const std::string& Card::GetId() const {
    return id_;
}

void Card::SetId(const std::string& id) {
    id_ = id;
}

void Card::SetContent(const CardContent& content) {
    content_ = content;
}

const CardContent& Card::GetContent() const {
    return content_;
}

void Card::SetOnClickCallback(std::function<void()> callback) {
    onClickCallback_ = callback;
}

void Card::SetActionId(const std::string& actionId) {
    actionId_ = actionId;
}

const std::string& Card::GetActionId() const {
    return actionId_;
}

} // namespace ui
} // namespace core
} // namespace game
