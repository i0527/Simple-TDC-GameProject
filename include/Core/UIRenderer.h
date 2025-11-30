/**
 * @file UIRenderer.h
 * @brief JSON定義ベースUIレンダラー
 * 
 * Phase 4B: データ駆動UIシステム
 * UILayoutDefからUI要素を描画し、インタラクションを処理
 */
#pragma once

#include "Core/UIDefinitions.h"
#include "Core/GameRenderer.h"
#include "Core/FallbackRenderer.h"
#include "Core/Platform.h"

#include <unordered_map>
#include <functional>
#include <string>
#include <any>
#include <iostream>
#include <regex>
#include <algorithm>

namespace Core {

/**
 * @brief UIイベントデータ
 */
struct UIEvent {
    std::string elementId;
    std::string eventType;      // "click", "hover", "release"
    Vector2 mousePosition;
    int repeatIndex = -1;       // 繰り返し要素のインデックス
};

/**
 * @brief UIバインディング - 動的値の取得
 */
using UIValueGetter = std::function<float(const std::string& bindPath)>;
using UIStringGetter = std::function<std::string(const std::string& bindPath)>;
using UIEventHandler = std::function<void(const UIEvent& event)>;

/**
 * @brief UI要素のランタイム状態
 */
struct UIElementState {
    bool isHovered = false;
    bool isPressed = false;
    Rectangle bounds{};         // 計算済み境界
};

/**
 * @brief JSON定義ベースUIレンダラー
 * 
 * 使用例:
 * @code
 * UIRenderer ui;
 * ui.SetLayout(myLayoutDef);
 * 
 * // 値バインディング
 * ui.SetValueGetter([&](const std::string& path) {
 *     if (path == "player.hp_percent") return player.hp / player.maxHp;
 *     return 0.0f;
 * });
 * 
 * // イベントハンドラ
 * ui.SetEventHandler([&](const UIEvent& e) {
 *     if (e.eventType == "click" && e.elementId == "slot") {
 *         SpawnUnit(e.repeatIndex);
 *     }
 * });
 * 
 * // ゲームループ
 * ui.Update(mouseWorldPos);
 * ui.Render();
 * @endcode
 */
class UIRenderer {
public:
    UIRenderer() = default;
    
    /**
     * @brief レイアウトを設定
     */
    void SetLayout(const Data::UILayoutDef& layout) {
        layout_ = &layout;
        elementStates_.clear();
        
        // 全要素の状態を初期化
        InitializeElementStates(layout.elements, "");
    }
    
    /**
     * @brief 値取得関数を設定
     */
    void SetValueGetter(UIValueGetter getter) {
        valueGetter_ = std::move(getter);
    }
    
    /**
     * @brief 文字列取得関数を設定
     */
    void SetStringGetter(UIStringGetter getter) {
        stringGetter_ = std::move(getter);
    }
    
    /**
     * @brief イベントハンドラを設定
     */
    void SetEventHandler(UIEventHandler handler) {
        eventHandler_ = std::move(handler);
    }
    
    /**
     * @brief 更新（マウス入力処理）
     * @param mouseWorldPos FHD座標でのマウス位置
     */
    void Update(Vector2 mouseWorldPos) {
        if (!layout_) return;
        
        mousePos_ = mouseWorldPos;
        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        
        // 全要素のホバー状態をリセット
        for (auto& [id, state] : elementStates_) {
            bool wasHovered = state.isHovered;
            state.isHovered = false;
            
            if (CheckCollisionPointRec(mousePos_, state.bounds)) {
                state.isHovered = true;
            }
        }
        
        // 要素のインタラクション処理
        ProcessInteraction(layout_->elements, "", mousePressed, mouseReleased);
    }
    
    /**
     * @brief 描画
     */
    void Render() {
        if (!layout_) return;
        
        RenderElements(layout_->elements, 0, 0, "");
    }
    
    /**
     * @brief 要素の表示/非表示を設定
     */
    void SetElementVisible(const std::string& elementId, bool visible) {
        visibilityOverrides_[elementId] = visible;
    }
    
    /**
     * @brief 条件を設定
     */
    void SetCondition(const std::string& conditionName, bool value) {
        conditions_[conditionName] = value;
    }
    
    /**
     * @brief 要素の状態を取得
     */
    const UIElementState* GetElementState(const std::string& elementId) const {
        auto it = elementStates_.find(elementId);
        return it != elementStates_.end() ? &it->second : nullptr;
    }

private:
    void InitializeElementStates(const std::vector<Data::UIElementDef>& elements, const std::string& parentId) {
        for (const auto& elem : elements) {
            std::string fullId = parentId.empty() ? elem.id : (parentId + "." + elem.id);
            
            if (elem.repeatCount > 0) {
                for (int i = 0; i < elem.repeatCount; ++i) {
                    std::string repeatId = fullId + "[" + std::to_string(i) + "]";
                    elementStates_[repeatId] = UIElementState{};
                }
            } else {
                elementStates_[fullId] = UIElementState{};
            }
            
            // 子要素も初期化
            InitializeElementStates(elem.children, fullId);
        }
    }
    
    void ProcessInteraction(
        const std::vector<Data::UIElementDef>& elements, 
        const std::string& parentId,
        bool mousePressed, 
        bool mouseReleased
    ) {
        for (const auto& elem : elements) {
            if (!IsElementVisible(elem)) continue;
            
            std::string fullId = parentId.empty() ? elem.id : (parentId + "." + elem.id);
            
            if (elem.repeatCount > 0) {
                for (int i = 0; i < elem.repeatCount; ++i) {
                    std::string repeatId = fullId + "[" + std::to_string(i) + "]";
                    auto it = elementStates_.find(repeatId);
                    if (it != elementStates_.end() && elem.interactive) {
                        ProcessElementInteraction(elem, it->second, repeatId, mousePressed, mouseReleased, i);
                    }
                }
            } else {
                auto it = elementStates_.find(fullId);
                if (it != elementStates_.end() && elem.interactive) {
                    ProcessElementInteraction(elem, it->second, fullId, mousePressed, mouseReleased, -1);
                }
            }
            
            // 子要素
            ProcessInteraction(elem.children, fullId, mousePressed, mouseReleased);
        }
    }
    
    void ProcessElementInteraction(
        const Data::UIElementDef& elem,
        UIElementState& state,
        const std::string& elementId,
        bool mousePressed,
        bool mouseReleased,
        int repeatIndex
    ) {
        // enabled=falseの場合はインタラクション無効
        if (!elem.enabled) {
            state.isHovered = false;
            state.isPressed = false;
            return;
        }
        
        if (state.isHovered) {
            // ホバーイベント
            if (!elem.onHover.empty() && eventHandler_) {
                // ホバーは毎フレーム発火しないように調整可能
            }
            
            // クリックイベント
            if (mousePressed && !elem.onClick.empty() && eventHandler_) {
                state.isPressed = true;
                UIEvent event;
                event.elementId = elem.id;
                event.eventType = "click";
                event.mousePosition = mousePos_;
                event.repeatIndex = repeatIndex;
                eventHandler_(event);
            }
        }
        
        if (mouseReleased) {
            state.isPressed = false;
        }
    }
    
    void RenderElements(
        const std::vector<Data::UIElementDef>& elements, 
        float parentX, 
        float parentY,
        const std::string& parentId
    ) {
        for (const auto& elem : elements) {
            if (!IsElementVisible(elem)) continue;
            
            std::string fullId = parentId.empty() ? elem.id : (parentId + "." + elem.id);
            
            if (elem.repeatCount > 0) {
                // 繰り返し描画
                for (int i = 0; i < elem.repeatCount; ++i) {
                    float offsetX = i * elem.repeatSpacingX;
                    float offsetY = i * elem.repeatSpacingY;
                    std::string repeatId = fullId + "[" + std::to_string(i) + "]";
                    
                    RenderSingleElement(elem, parentX + offsetX, parentY + offsetY, repeatId, i);
                }
            } else {
                RenderSingleElement(elem, parentX, parentY, fullId, -1);
            }
        }
    }
    
    void RenderSingleElement(
        const Data::UIElementDef& elem, 
        float parentX, 
        float parentY,
        const std::string& elementId,
        int repeatIndex
    ) {
        // 位置計算
        float anchorX, anchorY;
        Data::GetAnchorOffset(elem.anchor, 
            static_cast<float>(FHD::RENDER_WIDTH), 
            static_cast<float>(FHD::RENDER_HEIGHT), 
            anchorX, anchorY);
        
        float pivotX, pivotY;
        Data::GetPivotOffset(elem.pivot, elem.width, elem.height, pivotX, pivotY);
        
        float finalX = parentX + anchorX + elem.x - pivotX;
        float finalY = parentY + anchorY + elem.y - pivotY;
        
        Rectangle bounds = {finalX, finalY, elem.width, elem.height};
        
        // 状態を更新
        auto it = elementStates_.find(elementId);
        if (it != elementStates_.end()) {
            it->second.bounds = bounds;
            it->second.isHovered = CheckCollisionPointRec(mousePos_, bounds);
        }
        
        bool isHovered = it != elementStates_.end() && it->second.isHovered;
        bool isPressed = it != elementStates_.end() && it->second.isPressed;
        
        // 描画
        switch (elem.type) {
            case Data::UIElementType::Panel:
                RenderPanel(elem, bounds, isHovered, isPressed);
                break;
            case Data::UIElementType::Text:
                RenderText(elem, bounds, repeatIndex);
                break;
            case Data::UIElementType::Button:
                RenderButton(elem, bounds, isHovered, isPressed);
                break;
            case Data::UIElementType::ProgressBar:
                RenderProgressBar(elem, bounds, repeatIndex);
                break;
            case Data::UIElementType::Slot:
                RenderSlot(elem, bounds, isHovered, isPressed, repeatIndex);
                break;
            case Data::UIElementType::Container:
            case Data::UIElementType::Image:
                RenderPanel(elem, bounds, isHovered, isPressed);
                break;
        }
        
        // 子要素を描画
        RenderElements(elem.children, finalX, finalY, elementId);
    }
    
    void RenderPanel(const Data::UIElementDef& elem, Rectangle bounds, bool isHovered, bool isPressed) {
        Color bgColor = elem.backgroundColor.ToRaylib();
        
        if (isHovered && elem.hoverBackgroundColor) {
            bgColor = elem.hoverBackgroundColor->ToRaylib();
        }
        
        bgColor.a = static_cast<unsigned char>(bgColor.a * elem.opacity);
        
        // 角丸対応
        if (elem.cornerRadius > 0) {
            DrawRectangleRounded(bounds, elem.cornerRadius / std::min(bounds.width, bounds.height), 8, bgColor);
        } else {
            DrawRectangleRec(bounds, bgColor);
        }
        
        if (elem.borderWidth > 0) {
            Color borderColor = elem.borderColor.ToRaylib();
            if (isHovered && elem.hoverBorderColor) {
                borderColor = elem.hoverBorderColor->ToRaylib();
            }
            if (elem.cornerRadius > 0) {
                DrawRectangleRoundedLinesEx(bounds, elem.cornerRadius / std::min(bounds.width, bounds.height), 8, elem.borderWidth, borderColor);
            } else {
                DrawRectangleLinesEx(bounds, elem.borderWidth, borderColor);
            }
        }
    }
    
    void RenderText(const Data::UIElementDef& elem, Rectangle bounds, int repeatIndex) {
        std::string text = elem.text;
        
        // ${variable} 形式の動的テキスト展開
        text = ExpandVariables(text, repeatIndex);
        
        // バインディングがあれば値を取得（bindValueは完全置換用）
        if (!elem.bindValue.empty() && stringGetter_) {
            std::string bindPath = elem.bindValue;
            if (repeatIndex >= 0) {
                // 配列インデックスを置換
                size_t pos = bindPath.find("[]");
                if (pos != std::string::npos) {
                    bindPath.replace(pos, 2, "[" + std::to_string(repeatIndex) + "]");
                }
            }
            text = stringGetter_(bindPath);
        }
        
        Color textColor = elem.textColor.ToRaylib();
        textColor.a = static_cast<unsigned char>(textColor.a * elem.opacity);
        
        // テキスト配置計算
        int textWidth = MeasureText(text.c_str(), elem.fontSize);
        float textX = bounds.x;
        float textY = bounds.y;
        
        switch (elem.textAlign) {
            case Data::UIAnchor::TopCenter:
            case Data::UIAnchor::Center:
            case Data::UIAnchor::BottomCenter:
                textX = bounds.x + (bounds.width - textWidth) / 2;
                break;
            case Data::UIAnchor::TopRight:
            case Data::UIAnchor::MiddleRight:
            case Data::UIAnchor::BottomRight:
                textX = bounds.x + bounds.width - textWidth;
                break;
            default:
                break;
        }
        
        switch (elem.textAlign) {
            case Data::UIAnchor::MiddleLeft:
            case Data::UIAnchor::Center:
            case Data::UIAnchor::MiddleRight:
                textY = bounds.y + (bounds.height - elem.fontSize) / 2;
                break;
            case Data::UIAnchor::BottomLeft:
            case Data::UIAnchor::BottomCenter:
            case Data::UIAnchor::BottomRight:
                textY = bounds.y + bounds.height - elem.fontSize;
                break;
            default:
                break;
        }
        
        DrawText(text.c_str(), static_cast<int>(textX), static_cast<int>(textY), elem.fontSize, textColor);
    }
    
    void RenderButton(const Data::UIElementDef& elem, Rectangle bounds, bool isHovered, bool isPressed) {
        // enabled=falseの場合は暗く表示
        float dimFactor = elem.enabled ? 1.0f : 0.5f;
        
        // 背景
        Color bgColor = elem.backgroundColor.ToRaylib();
        if (!elem.enabled) {
            // 無効状態
            bgColor.r = static_cast<unsigned char>(bgColor.r * dimFactor);
            bgColor.g = static_cast<unsigned char>(bgColor.g * dimFactor);
            bgColor.b = static_cast<unsigned char>(bgColor.b * dimFactor);
        } else if (isPressed) {
            bgColor = Color{
                static_cast<unsigned char>(bgColor.r * 0.7f),
                static_cast<unsigned char>(bgColor.g * 0.7f),
                static_cast<unsigned char>(bgColor.b * 0.7f),
                bgColor.a
            };
        } else if (isHovered) {
            if (elem.hoverBackgroundColor) {
                bgColor = elem.hoverBackgroundColor->ToRaylib();
            } else {
                bgColor = Color{
                    static_cast<unsigned char>(std::min(255, bgColor.r + 30)),
                    static_cast<unsigned char>(std::min(255, bgColor.g + 30)),
                    static_cast<unsigned char>(std::min(255, bgColor.b + 30)),
                    bgColor.a
                };
            }
        }
        
        // 角丸対応
        if (elem.cornerRadius > 0) {
            DrawRectangleRounded(bounds, elem.cornerRadius / std::min(bounds.width, bounds.height), 8, bgColor);
        } else {
            DrawRectangleRec(bounds, bgColor);
        }
        
        // 枠線
        if (elem.borderWidth > 0) {
            Color borderColor = elem.borderColor.ToRaylib();
            if (isHovered && elem.hoverBorderColor) {
                borderColor = elem.hoverBorderColor->ToRaylib();
            }
            if (elem.cornerRadius > 0) {
                DrawRectangleRoundedLinesEx(bounds, elem.cornerRadius / std::min(bounds.width, bounds.height), 8, elem.borderWidth, borderColor);
            } else {
                DrawRectangleLinesEx(bounds, elem.borderWidth, borderColor);
            }
        }
        
        // テキスト
        if (!elem.text.empty()) {
            RenderText(elem, bounds, -1);
        }
    }
    
    void RenderProgressBar(const Data::UIElementDef& elem, Rectangle bounds, int repeatIndex) {
        // 背景
        RenderPanel(elem, bounds, false, false);
        
        // 値取得
        float value = 1.0f;
        if (!elem.bindValue.empty() && valueGetter_) {
            std::string bindPath = elem.bindValue;
            if (repeatIndex >= 0) {
                size_t pos = bindPath.find("[]");
                if (pos != std::string::npos) {
                    bindPath.replace(pos, 2, "[" + std::to_string(repeatIndex) + "]");
                }
            }
            value = valueGetter_(bindPath);
            value = std::clamp(value, 0.0f, 1.0f);
        }
        
        // フィル
        Color fillColor = elem.fillColor.ToRaylib();
        Rectangle fillBounds;
        
        if (elem.vertical) {
            float fillHeight = bounds.height * value;
            fillBounds = {
                bounds.x,
                bounds.y + bounds.height - fillHeight,
                bounds.width,
                fillHeight
            };
        } else {
            fillBounds = {
                bounds.x,
                bounds.y,
                bounds.width * value,
                bounds.height
            };
        }
        
        DrawRectangleRec(fillBounds, fillColor);
    }
    
    void RenderSlot(const Data::UIElementDef& elem, Rectangle bounds, bool isHovered, bool isPressed, int repeatIndex) {
        // スロットはボタンと同様だが、追加でインデックス表示など
        RenderButton(elem, bounds, isHovered, isPressed);
        
        // スロットインデックス表示
        if (repeatIndex >= 0) {
            char indexStr[4];
            snprintf(indexStr, sizeof(indexStr), "%d", repeatIndex + 1);
            DrawText(indexStr, 
                     static_cast<int>(bounds.x + bounds.width - 15), 
                     static_cast<int>(bounds.y + bounds.height - 18), 
                     14, Color{180, 180, 180, 255});
        }
    }
    
    bool IsElementVisible(const Data::UIElementDef& elem) const {
        if (!elem.visible) return false;
        
        auto it = visibilityOverrides_.find(elem.id);
        if (it != visibilityOverrides_.end()) {
            return it->second;
        }
        
        return true;
    }
    
    /**
     * @brief 動的テキスト ${variable} を展開
     */
    std::string ExpandVariables(const std::string& text, int repeatIndex) const {
        if (text.find("${") == std::string::npos) {
            return text;  // 変数がなければそのまま返す
        }
        
        std::string result = text;
        std::regex varPattern(R"(\$\{([^}]+)\})");
        std::smatch match;
        
        std::string::const_iterator searchStart = result.cbegin();
        std::string output;
        size_t lastPos = 0;
        
        while (std::regex_search(searchStart, result.cend(), match, varPattern)) {
            size_t matchPos = static_cast<size_t>(match.position()) + (searchStart - result.cbegin());
            output += result.substr(lastPos, matchPos - lastPos);
            
            std::string varName = match[1].str();
            std::string replacement;
            
            // 繰り返しインデックスを置換
            if (repeatIndex >= 0) {
                size_t bracketPos = varName.find("[]");
                if (bracketPos != std::string::npos) {
                    varName.replace(bracketPos, 2, "[" + std::to_string(repeatIndex) + "]");
                }
            }
            
            // 値を取得
            if (stringGetter_) {
                replacement = stringGetter_(varName);
            } else if (valueGetter_) {
                float val = valueGetter_(varName);
                // 整数として表示できる場合は整数で
                if (val == static_cast<int>(val)) {
                    replacement = std::to_string(static_cast<int>(val));
                } else {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%.1f", val);
                    replacement = buf;
                }
            } else {
                replacement = match[0].str();  // 展開できない場合はそのまま
            }
            
            output += replacement;
            lastPos = matchPos + match[0].length();
            searchStart = result.cbegin() + lastPos;
        }
        
        output += result.substr(lastPos);
        return output;
    }
    
    const Data::UILayoutDef* layout_ = nullptr;
    std::unordered_map<std::string, UIElementState> elementStates_;
    std::unordered_map<std::string, bool> visibilityOverrides_;
    std::unordered_map<std::string, bool> conditions_;
    
    UIValueGetter valueGetter_;
    UIStringGetter stringGetter_;
    UIEventHandler eventHandler_;
    
    Vector2 mousePos_{};
};

} // namespace Core
