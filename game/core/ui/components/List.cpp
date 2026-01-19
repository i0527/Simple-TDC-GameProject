#include "List.hpp"
#include "../UIEvent.hpp"
#include "../../api/UISystemAPI.hpp"
#include "../UiAssetKeys.hpp"
#include "../../../utils/Log.h"
#include <imgui.h>
#include <algorithm>
#include <cstdint>

namespace game {
namespace core {
namespace ui {

List::List()
    : bounds_{0.0f, 0.0f, 300.0f, 400.0f}
    , margin_{0.0f, 0.0f, 0.0f, 0.0f}
    , visible_(true)
    , enabled_(true)
    , id_("")
    , selectedIndex_(-1)
    , itemsPerPage_(10)
    , itemHeight_(30.0f)
    , scrollOffset_(0.0f)
{
}

bool List::Initialize() {
    return true;
}

void List::Update(float deltaTime) {
    // 子要素の更新
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}

void List::Render() {
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

    // ImGuiでリストを描画
    ImGui::SetNextWindowPos(absolutePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds_.width, bounds_.height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

    if (!enabled_) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(("List##" + id_).c_str(), nullptr, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        if (uiAPI_) {
            Texture2D* bgTexture = uiAPI_->GetTexturePtr(UiAssetKeys::FantasyPanelLight);
            if (bgTexture && bgTexture->id != 0) {
                ImTextureID texId = static_cast<ImTextureID>(static_cast<uintptr_t>(bgTexture->id));
                drawList->AddImage(texId, windowPos,
                                   ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y));
            }
            Texture2D* borderTexture = uiAPI_->GetTexturePtr(UiAssetKeys::FantasyBorderLight);
            if (borderTexture && borderTexture->id != 0) {
                ImTextureID borderId = static_cast<ImTextureID>(static_cast<uintptr_t>(borderTexture->id));
                drawList->AddImage(borderId, windowPos,
                                   ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y));
            }
        }

        ImGui::BeginChild("##list_content", ImVec2(0, 0), false,
                          ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);

        ImDrawList* childDrawList = ImGui::GetWindowDrawList();
        float availableWidth = ImGui::GetContentRegionAvail().x;

        for (size_t i = 0; i < items_.size(); ++i) {
            const auto& item = items_[i];
            bool isSelected = (static_cast<int>(i) == selectedIndex_);
            bool isEnabled = item.enabled && enabled_;

            ImVec2 itemPos = ImGui::GetCursorScreenPos();
            ImVec2 itemSize = ImVec2(availableWidth, itemHeight_);
            std::string buttonId = "##list_item_" + std::to_string(i);
            bool clicked = ImGui::InvisibleButton(buttonId.c_str(), itemSize);
            bool hovered = ImGui::IsItemHovered();

            const char* textureKey = UiAssetKeys::ButtonSecondaryNormal;
            if (!isEnabled) {
                textureKey = UiAssetKeys::ButtonSecondaryNormal;
            } else if (isSelected) {
                textureKey = UiAssetKeys::ButtonPrimaryHover;
            } else if (hovered) {
                textureKey = UiAssetKeys::ButtonSecondaryHover;
            }

            Color textColor = Color{230, 230, 230, 255};
            if (uiAPI_) {
                Texture2D* texture = uiAPI_->GetTexturePtr(textureKey);
                if (texture && texture->id != 0) {
                    ImTextureID texId = static_cast<ImTextureID>(static_cast<uintptr_t>(texture->id));
                    childDrawList->AddImage(texId, itemPos,
                                            ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y));
                }
                textColor = uiAPI_->GetReadableTextColor(textureKey);
            }

            std::string label = item.label;
            if (!item.value.empty()) {
                label += " - " + item.value;
            }
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            ImVec2 textPos = ImVec2(itemPos.x + 12.0f, itemPos.y + (itemSize.y - textSize.y) * 0.5f);
            unsigned char textAlpha = isEnabled ? 255 : 160;
            childDrawList->AddText(textPos, IM_COL32(textColor.r, textColor.g, textColor.b, textAlpha), label.c_str());

            if (clicked && isEnabled && static_cast<int>(i) != selectedIndex_) {
                int oldIndex = selectedIndex_;
                selectedIndex_ = static_cast<int>(i);
                if (onSelectionChanged_ && oldIndex != selectedIndex_) {
                    onSelectionChanged_(item);
                }
            }

            if (itemsPerPage_ > 0 && static_cast<int>(i) >= itemsPerPage_ - 1) {
                break;
            }
        }

        ImGui::EndChild();

        // 子要素の描画
        for (auto& child : children_) {
            if (child && child->IsVisible()) {
                child->Render();
            }
        }
    }
    ImGui::End();
}

void List::Shutdown() {
    children_.clear();
    items_.clear();
    onSelectionChanged_ = nullptr;
}

void List::SetPosition(float x, float y) {
    bounds_.x = x;
    bounds_.y = y;
}

void List::SetSize(float width, float height) {
    bounds_.width = width;
    bounds_.height = height;
}

Rect List::GetBounds() const {
    return Rect{
        bounds_.x + margin_.left,
        bounds_.y + margin_.top,
        bounds_.width,
        bounds_.height
    };
}

void List::SetMargin(const Margin& margin) {
    margin_ = margin;
}

void List::SetVisible(bool visible) {
    visible_ = visible;
}

bool List::IsVisible() const {
    return visible_;
}

void List::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool List::IsEnabled() const {
    return enabled_;
}

UIEventResult List::HandleEvent(const UIEvent& ev) {
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
            // クリック位置からアイテムインデックスを計算
            float relativeY = ev.y - r.y;
            int clickedIndex = static_cast<int>(relativeY / itemHeight_);
            
            if (clickedIndex >= 0 && clickedIndex < static_cast<int>(items_.size())) {
                if (items_[clickedIndex].enabled) {
                    int oldIndex = selectedIndex_;
                    selectedIndex_ = clickedIndex;
                    
                    result.handled = true;
                    result.componentId = id_;
                    result.actionId = "select_item:" + items_[selectedIndex_].id;
                    
                    if (onSelectionChanged_ && oldIndex != selectedIndex_) {
                        onSelectionChanged_(items_[selectedIndex_]);
                    }
                }
            } else {
                result.handled = true;
                result.componentId = id_;
            }
        }
        break;
    }
    case UIEventType::Hover: {
        const Rect r = GetBounds();
        const bool inside =
            ev.x >= r.x && ev.x <= r.x + r.width &&
            ev.y >= r.y && ev.y <= r.y + r.height;
        if (inside) {
            result.handled = true;
            result.componentId = id_;
        }
        break;
    }
    case UIEventType::Key: {
        // 上下キーで選択を移動
        if (ev.key == 264 || ev.key == 265) { // Down or Up
            int newIndex = selectedIndex_;
            if (ev.key == 264) { // Down
                newIndex = (selectedIndex_ < static_cast<int>(items_.size()) - 1) ? selectedIndex_ + 1 : selectedIndex_;
            } else { // Up
                newIndex = (selectedIndex_ > 0) ? selectedIndex_ - 1 : selectedIndex_;
            }
            
            if (newIndex != selectedIndex_ && newIndex >= 0 && newIndex < static_cast<int>(items_.size())) {
                if (items_[newIndex].enabled) {
                    int oldIndex = selectedIndex_;
                    selectedIndex_ = newIndex;
                    
                    result.handled = true;
                    result.componentId = id_;
                    result.actionId = "select_item:" + items_[selectedIndex_].id;
                    
                    if (onSelectionChanged_ && oldIndex != selectedIndex_) {
                        onSelectionChanged_(items_[selectedIndex_]);
                    }
                }
            }
        }
        break;
    }
    case UIEventType::None:
        break;
    }

    // 子要素にイベントを伝播
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

bool List::OnMouseClick(float x, float y) {
    if (!enabled_ || !visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        // クリック位置からアイテムインデックスを計算
        float relativeY = y - bounds.y;
        int clickedIndex = static_cast<int>(relativeY / itemHeight_);
        
        if (clickedIndex >= 0 && clickedIndex < static_cast<int>(items_.size())) {
            if (items_[clickedIndex].enabled) {
                int oldIndex = selectedIndex_;
                selectedIndex_ = clickedIndex;
                
                if (onSelectionChanged_ && oldIndex != selectedIndex_) {
                    onSelectionChanged_(items_[selectedIndex_]);
                }
                return true;
            }
        }
    }
    return false;
}

bool List::OnMouseHover(float x, float y) {
    if (!visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    return (x >= bounds.x && x <= bounds.x + bounds.width &&
            y >= bounds.y && y <= bounds.y + bounds.height);
}

bool List::OnKey(int key) {
    if (!enabled_ || !visible_ || items_.empty()) {
        return false;
    }

    // 上下キーで選択を移動
    if (key == 265) { // Up arrow
        if (selectedIndex_ > 0) {
            SetSelectedIndex(selectedIndex_ - 1);
            return true;
        }
    } else if (key == 264) { // Down arrow
        if (selectedIndex_ < static_cast<int>(items_.size()) - 1) {
            SetSelectedIndex(selectedIndex_ + 1);
            return true;
        }
    }

    return false;
}

void List::AddChild(std::shared_ptr<IUIComponent> child) {
    if (child) {
        children_.push_back(child);
    }
}

void List::RemoveChild(std::shared_ptr<IUIComponent> child) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&child](const std::shared_ptr<IUIComponent>& c) {
                return c == child;
            }),
        children_.end()
    );
}

const std::string& List::GetId() const {
    return id_;
}

void List::SetId(const std::string& id) {
    id_ = id;
}

void List::AddItem(const ListItem& item) {
    items_.push_back(item);
}

void List::ClearItems() {
    items_.clear();
    selectedIndex_ = -1;
}

void List::RemoveItem(const std::string& id) {
    items_.erase(
        std::remove_if(items_.begin(), items_.end(),
            [&id](const ListItem& item) {
                return item.id == id;
            }),
        items_.end()
    );

    // 選択インデックスの調整
    if (selectedIndex_ >= static_cast<int>(items_.size())) {
        selectedIndex_ = static_cast<int>(items_.size()) - 1;
    }
}

void List::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(items_.size())) {
        int oldIndex = selectedIndex_;
        selectedIndex_ = index;
        
        if (onSelectionChanged_ && oldIndex != selectedIndex_ && selectedIndex_ >= 0) {
            onSelectionChanged_(items_[selectedIndex_]);
        }
    }
}

int List::GetSelectedIndex() const {
    return selectedIndex_;
}

const ListItem* List::GetSelectedItem() const {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        return &items_[selectedIndex_];
    }
    return nullptr;
}

void List::SetItemHeight(float height) {
    itemHeight_ = height;
}

void List::SetItemsPerPage(int count) {
    itemsPerPage_ = count;
}

void List::SetOnSelectionChanged(std::function<void(const ListItem&)> callback) {
    onSelectionChanged_ = callback;
}

size_t List::GetItemCount() const {
    return items_.size();
}

} // namespace ui
} // namespace core
} // namespace game
