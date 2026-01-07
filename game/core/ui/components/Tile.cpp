#include "Tile.hpp"
#include "../UIEvent.hpp"
#include "../../../utils/Log.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace game {
namespace core {
namespace ui {

Tile::Tile()
    : bounds_{0.0f, 0.0f, 400.0f, 400.0f}
    , margin_{0.0f, 0.0f, 0.0f, 0.0f}
    , visible_(true)
    , enabled_(true)
    , id_("")
    , cols_(3)
    , rows_(3)
    , tileWidth_(100.0f)
    , tileHeight_(100.0f)
    , selectedIndex_(-1)
{
}

bool Tile::Initialize() {
    return true;
}

void Tile::Update(float deltaTime) {
    // 子要素の更新
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}

void Tile::Render() {
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

    // ImGuiでタイルグリッドを描画
    ImGui::SetNextWindowPos(absolutePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds_.width, bounds_.height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    if (!enabled_) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(("Tile##" + id_).c_str(), nullptr, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // グリッドの計算
        float spacing = 10.0f;
        float availableWidth = windowSize.x - spacing * (cols_ + 1);
        float availableHeight = windowSize.y - spacing * (rows_ + 1);
        float actualTileWidth = availableWidth / cols_;
        float actualTileHeight = availableHeight / rows_;

        // タイルサイズが設定されている場合はそれを使用
        if (tileWidth_ > 0 && tileHeight_ > 0) {
            actualTileWidth = tileWidth_;
            actualTileHeight = tileHeight_;
        }

        // タイルを描画
        for (size_t i = 0; i < tiles_.size(); ++i) {
            const auto& tile = tiles_[i];
            int row = static_cast<int>(i) / cols_;
            int col = static_cast<int>(i) % cols_;

            if (row >= rows_) {
                break; // グリッドの範囲外
            }

            float x = windowPos.x + spacing + col * (actualTileWidth + spacing);
            float y = windowPos.y + spacing + row * (actualTileHeight + spacing);

            bool isSelected = (static_cast<int>(i) == selectedIndex_);
            bool isEnabled = tile.enabled && enabled_;

            // タイルの背景色
            ImU32 bgColor;
            if (!isEnabled) {
                bgColor = IM_COL32(50, 50, 50, 255);
            } else if (isSelected) {
                bgColor = IM_COL32(100, 150, 200, 255);
            } else {
                bgColor = IM_COL32(70, 70, 70, 255);
            }

            // タイルの矩形を描画
            ImVec2 tileMin(x, y);
            ImVec2 tileMax(x + actualTileWidth, y + actualTileHeight);
            drawList->AddRectFilled(tileMin, tileMax, bgColor);
            drawList->AddRect(tileMin, tileMax, IM_COL32(150, 150, 150, 255), 0.0f, 0, 2.0f);

            // タイルのラベル
            if (!tile.label.empty()) {
                ImVec2 textSize = ImGui::CalcTextSize(tile.label.c_str());
                ImVec2 textPos(
                    x + (actualTileWidth - textSize.x) * 0.5f,
                    y + (actualTileHeight - textSize.y) * 0.5f
                );
                drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), tile.label.c_str());
            }

            // 画像（将来実装: テクスチャIDから画像を取得して表示）
            if (!tile.imageId.empty()) {
                // プレースホルダー: 画像表示領域
                // 実際の実装では、BaseSystemAPIからテクスチャを取得して表示
            }

            // クリック判定用のInvisibleButton
            ImGui::SetCursorPos(ImVec2(x - windowPos.x, y - windowPos.y));
            std::string buttonId = "##tile_" + std::to_string(i);
            if (ImGui::InvisibleButton(buttonId.c_str(), ImVec2(actualTileWidth, actualTileHeight))) {
                if (isEnabled && static_cast<int>(i) != selectedIndex_) {
                    int oldIndex = selectedIndex_;
                    selectedIndex_ = static_cast<int>(i);
                    
                    if (onTileSelected_ && oldIndex != selectedIndex_) {
                        onTileSelected_(tile);
                    }
                }
            }
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

void Tile::Shutdown() {
    children_.clear();
    tiles_.clear();
    onTileSelected_ = nullptr;
}

void Tile::SetPosition(float x, float y) {
    bounds_.x = x;
    bounds_.y = y;
}

void Tile::SetSize(float width, float height) {
    bounds_.width = width;
    bounds_.height = height;
}

Rect Tile::GetBounds() const {
    return Rect{
        bounds_.x + margin_.left,
        bounds_.y + margin_.top,
        bounds_.width,
        bounds_.height
    };
}

void Tile::SetMargin(const Margin& margin) {
    margin_ = margin;
}

void Tile::SetVisible(bool visible) {
    visible_ = visible;
}

bool Tile::IsVisible() const {
    return visible_;
}

void Tile::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Tile::IsEnabled() const {
    return enabled_;
}

UIEventResult Tile::HandleEvent(const UIEvent& ev) {
    UIEventResult result;

    if (!visible_ || !enabled_ || tiles_.empty()) {
        return result;
    }

    switch (ev.type) {
    case UIEventType::Click: {
        const Rect r = GetBounds();
        const bool inside =
            ev.x >= r.x && ev.x <= r.x + r.width &&
            ev.y >= r.y && ev.y <= r.y + r.height;

        if (inside) {
            // クリック位置からタイルインデックスを計算
            float relativeX = ev.x - r.x;
            float relativeY = ev.y - r.y;
            
            float spacing = 10.0f;
            float availableWidth = r.width - spacing * (cols_ + 1);
            float availableHeight = r.height - spacing * (rows_ + 1);
            float actualTileWidth = availableWidth / cols_;
            float actualTileHeight = availableHeight / rows_;
            
            if (tileWidth_ > 0 && tileHeight_ > 0) {
                actualTileWidth = tileWidth_;
                actualTileHeight = tileHeight_;
            }
            
            int col = static_cast<int>((relativeX - spacing) / (actualTileWidth + spacing));
            int row = static_cast<int>((relativeY - spacing) / (actualTileHeight + spacing));
            int clickedIndex = row * cols_ + col;
            
            if (clickedIndex >= 0 && clickedIndex < static_cast<int>(tiles_.size()) && row < rows_) {
                const auto& tile = tiles_[clickedIndex];
                if (tile.enabled && static_cast<int>(clickedIndex) != selectedIndex_) {
                    int oldIndex = selectedIndex_;
                    selectedIndex_ = clickedIndex;
                    
                    result.handled = true;
                    result.componentId = id_;
                    result.actionId = "select_tile:" + tile.id;
                    
                    if (onTileSelected_ && oldIndex != selectedIndex_) {
                        onTileSelected_(tile);
                    }
                } else if (tile.enabled) {
                    result.handled = true;
                    result.componentId = id_;
                    result.actionId = "select_tile:" + tile.id;
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
    case UIEventType::Key:
        // Tileはキーイベントを処理しない
        break;
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

bool Tile::OnMouseClick(float x, float y) {
    if (!enabled_ || !visible_ || tiles_.empty()) {
        return false;
    }

    Rect bounds = GetBounds();
    if (x >= bounds.x && x <= bounds.x + bounds.width &&
        y >= bounds.y && y <= bounds.y + bounds.height) {
        // クリック位置からタイルインデックスを計算
        float relativeX = x - bounds.x;
        float relativeY = y - bounds.y;

        float spacing = 10.0f;
        float availableWidth = bounds.width - spacing * (cols_ + 1);
        float availableHeight = bounds.height - spacing * (rows_ + 1);
        float actualTileWidth = availableWidth / cols_;
        float actualTileHeight = availableHeight / rows_;

        if (tileWidth_ > 0 && tileHeight_ > 0) {
            actualTileWidth = tileWidth_;
            actualTileHeight = tileHeight_;
        }

        int col = static_cast<int>((relativeX - spacing) / (actualTileWidth + spacing));
        int row = static_cast<int>((relativeY - spacing) / (actualTileHeight + spacing));

        if (col >= 0 && col < cols_ && row >= 0 && row < rows_) {
            int clickedIndex = row * cols_ + col;
            if (clickedIndex >= 0 && clickedIndex < static_cast<int>(tiles_.size())) {
                if (tiles_[clickedIndex].enabled) {
                    int oldIndex = selectedIndex_;
                    selectedIndex_ = clickedIndex;
                    
                    if (onTileSelected_ && oldIndex != selectedIndex_) {
                        onTileSelected_(tiles_[selectedIndex_]);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool Tile::OnMouseHover(float x, float y) {
    if (!visible_) {
        return false;
    }

    Rect bounds = GetBounds();
    return (x >= bounds.x && x <= bounds.x + bounds.width &&
            y >= bounds.y && y <= bounds.y + bounds.height);
}

bool Tile::OnKey(int key) {
    // タイルはキー入力を受け付けない（将来、方向キーで選択移動を実装可能）
    return false;
}

void Tile::AddChild(std::shared_ptr<IUIComponent> child) {
    if (child) {
        children_.push_back(child);
    }
}

void Tile::RemoveChild(std::shared_ptr<IUIComponent> child) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&child](const std::shared_ptr<IUIComponent>& c) {
                return c == child;
            }),
        children_.end()
    );
}

const std::string& Tile::GetId() const {
    return id_;
}

void Tile::SetId(const std::string& id) {
    id_ = id;
}

void Tile::AddTile(const TileData& data) {
    tiles_.push_back(data);
}

void Tile::RemoveTile(const std::string& id) {
    tiles_.erase(
        std::remove_if(tiles_.begin(), tiles_.end(),
            [&id](const TileData& tile) {
                return tile.id == id;
            }),
        tiles_.end()
    );

    // 選択インデックスの調整
    if (selectedIndex_ >= static_cast<int>(tiles_.size())) {
        selectedIndex_ = static_cast<int>(tiles_.size()) - 1;
    }
}

void Tile::SetGridSize(int cols, int rows) {
    cols_ = cols;
    rows_ = rows;
}

void Tile::SetTileSize(float width, float height) {
    tileWidth_ = width;
    tileHeight_ = height;
}

void Tile::SetSelectedIndex(int index) {
    if (index >= -1 && index < static_cast<int>(tiles_.size())) {
        int oldIndex = selectedIndex_;
        selectedIndex_ = index;
        
        if (onTileSelected_ && oldIndex != selectedIndex_ && selectedIndex_ >= 0) {
            onTileSelected_(tiles_[selectedIndex_]);
        }
    }
}

int Tile::GetSelectedIndex() const {
    return selectedIndex_;
}

const TileData* Tile::GetSelectedTile() const {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(tiles_.size())) {
        return &tiles_[selectedIndex_];
    }
    return nullptr;
}

void Tile::SetOnTileSelected(std::function<void(const TileData&)> callback) {
    onTileSelected_ = callback;
}

} // namespace ui
} // namespace core
} // namespace game
