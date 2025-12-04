/**
 * @file UISerializer.cpp
 * @brief UIレイアウト定義のJSONシリアライズ/デシリアライズ実装
 */

#include "Data/Serializers/UISerializer.h"
#include "Data/Loaders/UILoader.h"
#include <sstream>
#include <iomanip>

namespace Data {

std::string UISerializer::AnchorToString(UIAnchor anchor) {
    switch (anchor) {
        case UIAnchor::TopLeft: return "topLeft";
        case UIAnchor::TopCenter: return "topCenter";
        case UIAnchor::TopRight: return "topRight";
        case UIAnchor::MiddleLeft: return "middleLeft";
        case UIAnchor::Center: return "center";
        case UIAnchor::MiddleRight: return "middleRight";
        case UIAnchor::BottomLeft: return "bottomLeft";
        case UIAnchor::BottomCenter: return "bottomCenter";
        case UIAnchor::BottomRight: return "bottomRight";
        default: return "topLeft";
    }
}

UIAnchor UISerializer::StringToAnchor(const std::string& str) {
    if (str == "topLeft" || str == "top_left") return UIAnchor::TopLeft;
    if (str == "topCenter" || str == "top_center") return UIAnchor::TopCenter;
    if (str == "topRight" || str == "top_right") return UIAnchor::TopRight;
    if (str == "middleLeft" || str == "middle_left") return UIAnchor::MiddleLeft;
    if (str == "center") return UIAnchor::Center;
    if (str == "middleRight" || str == "middle_right") return UIAnchor::MiddleRight;
    if (str == "bottomLeft" || str == "bottom_left") return UIAnchor::BottomLeft;
    if (str == "bottomCenter" || str == "bottom_center") return UIAnchor::BottomCenter;
    if (str == "bottomRight" || str == "bottom_right") return UIAnchor::BottomRight;
    return UIAnchor::TopLeft;
}

std::string UISerializer::ElementTypeToString(UIElementType type) {
    switch (type) {
        case UIElementType::Panel: return "panel";
        case UIElementType::Text: return "text";
        case UIElementType::Image: return "image";
        case UIElementType::Button: return "button";
        case UIElementType::ProgressBar: return "progressBar";
        case UIElementType::Slot: return "slot";
        case UIElementType::Container: return "container";
        default: return "panel";
    }
}

UIElementType UISerializer::StringToElementType(const std::string& str) {
    if (str == "panel") return UIElementType::Panel;
    if (str == "text") return UIElementType::Text;
    if (str == "image") return UIElementType::Image;
    if (str == "button") return UIElementType::Button;
    if (str == "progressBar" || str == "progress_bar") return UIElementType::ProgressBar;
    if (str == "slot") return UIElementType::Slot;
    if (str == "container") return UIElementType::Container;
    return UIElementType::Panel;
}

nlohmann::json UISerializer::SerializeColor(const UIColor& color) {
    nlohmann::json j;
    j["r"] = color.r;
    j["g"] = color.g;
    j["b"] = color.b;
    j["a"] = color.a;
    return j;
}

UIColor UISerializer::DeserializeColor(const nlohmann::json& j) {
    UIColor color{255, 255, 255, 255};
    if (j.is_object()) {
        color.r = j.value("r", 255);
        color.g = j.value("g", 255);
        color.b = j.value("b", 255);
        color.a = j.value("a", 255);
    }
    return color;
}

nlohmann::json UISerializer::SerializeElement(const UIElementDef& elem) {
    nlohmann::json j;
    
    j["id"] = elem.id;
    j["type"] = ElementTypeToString(elem.type);
    
    // 位置・サイズ
    j["position"] = nlohmann::json{{"x", elem.x}, {"y", elem.y}};
    j["size"] = nlohmann::json{{"width", elem.width}, {"height", elem.height}};
    
    // アンカー
    j["anchor"] = AnchorToString(elem.anchor);
    j["pivot"] = AnchorToString(elem.pivot);
    
    // 見た目
    j["backgroundColor"] = SerializeColor(elem.backgroundColor);
    j["borderColor"] = SerializeColor(elem.borderColor);
    j["borderWidth"] = elem.borderWidth;
    j["cornerRadius"] = elem.cornerRadius;
    j["opacity"] = elem.opacity;
    
    // テキスト
    if (!elem.text.empty()) j["text"] = elem.text;
    if (!elem.fontId.empty()) j["fontId"] = elem.fontId;
    j["fontSize"] = elem.fontSize;
    j["textColor"] = SerializeColor(elem.textColor);
    j["textAlign"] = AnchorToString(elem.textAlign);
    
    // 画像
    if (!elem.imageId.empty()) j["imageId"] = elem.imageId;
    
    // プログレスバー
    j["fillColor"] = SerializeColor(elem.fillColor);
    if (!elem.bindValue.empty()) j["bind"] = elem.bindValue;
    j["vertical"] = elem.vertical;
    
    // インタラクション
    j["interactive"] = elem.interactive;
    if (!elem.onClick.empty()) j["onClick"] = elem.onClick;
    if (!elem.onHover.empty()) j["onHover"] = elem.onHover;
    
    // ホバー時スタイル
    if (elem.hoverBackgroundColor.has_value()) {
        j["hoverBackgroundColor"] = SerializeColor(*elem.hoverBackgroundColor);
    }
    if (elem.hoverBorderColor.has_value()) {
        j["hoverBorderColor"] = SerializeColor(*elem.hoverBorderColor);
    }
    
    // 状態
    j["visible"] = elem.visible;
    j["enabled"] = elem.enabled;
    
    // 子要素
    if (!elem.children.empty()) {
        j["children"] = nlohmann::json::array();
        for (const auto& child : elem.children) {
            j["children"].push_back(SerializeElement(child));
        }
    }
    
    // 繰り返し表示
    if (elem.repeatCount > 0) {
        j["repeatCount"] = elem.repeatCount;
        j["repeatSpacingX"] = elem.repeatSpacingX;
        j["repeatSpacingY"] = elem.repeatSpacingY;
        if (!elem.repeatBindArray.empty()) {
            j["repeatBindArray"] = elem.repeatBindArray;
        }
    }
    
    return j;
}

nlohmann::json UISerializer::Serialize(const UILayoutDef& def) {
    nlohmann::json j;
    
    j["id"] = def.id;
    j["name"] = def.name;
    j["baseWidth"] = def.baseWidth;
    j["baseHeight"] = def.baseHeight;
    
    // 要素
    j["elements"] = nlohmann::json::array();
    for (const auto& elem : def.elements) {
        j["elements"].push_back(SerializeElement(elem));
    }
    
    // 条件
    if (!def.conditions.empty()) {
        j["conditions"] = nlohmann::json::object();
        for (const auto& [key, value] : def.conditions) {
            j["conditions"][key] = value;
        }
    }
    
    return j;
}

UIElementDef UISerializer::DeserializeElement(const nlohmann::json& j) {
    UIElementDef elem;
    
    elem.id = j.value("id", "");
    elem.type = StringToElementType(j.value("type", "panel"));
    
    // 位置・サイズ
    if (j.contains("position")) {
        const auto& pos = j["position"];
        elem.x = pos.value("x", 0.0f);
        elem.y = pos.value("y", 0.0f);
    }
    if (j.contains("size")) {
        const auto& size = j["size"];
        elem.width = size.value("width", 100.0f);
        elem.height = size.value("height", 50.0f);
    }
    
    // アンカー
    if (j.contains("anchor")) {
        elem.anchor = StringToAnchor(j["anchor"].get<std::string>());
    }
    if (j.contains("pivot")) {
        elem.pivot = StringToAnchor(j["pivot"].get<std::string>());
    }
    
    // 見た目
    if (j.contains("backgroundColor")) {
        elem.backgroundColor = DeserializeColor(j["backgroundColor"]);
    }
    if (j.contains("borderColor")) {
        elem.borderColor = DeserializeColor(j["borderColor"]);
    }
    elem.borderWidth = j.value("borderWidth", 0.0f);
    elem.cornerRadius = j.value("cornerRadius", 0.0f);
    elem.opacity = j.value("opacity", 1.0f);
    
    // テキスト
    elem.text = j.value("text", "");
    elem.fontId = j.value("fontId", "");
    elem.fontSize = j.value("fontSize", 16);
    if (j.contains("textColor")) {
        elem.textColor = DeserializeColor(j["textColor"]);
    }
    if (j.contains("textAlign")) {
        elem.textAlign = StringToAnchor(j["textAlign"].get<std::string>());
    }
    
    // 画像
    elem.imageId = j.value("imageId", "");
    
    // プログレスバー
    if (j.contains("fillColor")) {
        elem.fillColor = DeserializeColor(j["fillColor"]);
    }
    elem.bindValue = j.value("bind", "");
    elem.vertical = j.value("vertical", false);
    
    // インタラクション
    elem.interactive = j.value("interactive", false);
    elem.onClick = j.value("onClick", "");
    elem.onHover = j.value("onHover", "");
    
    // ホバー時スタイル
    if (j.contains("hoverBackgroundColor")) {
        elem.hoverBackgroundColor = DeserializeColor(j["hoverBackgroundColor"]);
    }
    if (j.contains("hoverBorderColor")) {
        elem.hoverBorderColor = DeserializeColor(j["hoverBorderColor"]);
    }
    
    // 状態
    elem.visible = j.value("visible", true);
    elem.enabled = j.value("enabled", true);
    
    // 子要素
    if (j.contains("children") && j["children"].is_array()) {
        for (const auto& childJson : j["children"]) {
            elem.children.push_back(DeserializeElement(childJson));
        }
    }
    
    // 繰り返し表示
    elem.repeatCount = j.value("repeatCount", 0);
    elem.repeatSpacingX = j.value("repeatSpacingX", 0.0f);
    elem.repeatSpacingY = j.value("repeatSpacingY", 0.0f);
    elem.repeatBindArray = j.value("repeatBindArray", "");
    
    return elem;
}

UILayoutDef UISerializer::Deserialize(const nlohmann::json& j) {
    UILayoutDef def;
    
    def.id = j.value("id", "");
    def.name = j.value("name", def.id);
    def.baseWidth = j.value("baseWidth", 1920);
    def.baseHeight = j.value("baseHeight", 1080);
    
    // 要素
    if (j.contains("elements") && j["elements"].is_array()) {
        for (const auto& elemJson : j["elements"]) {
            def.elements.push_back(DeserializeElement(elemJson));
        }
    }
    
    // 条件
    if (j.contains("conditions") && j["conditions"].is_object()) {
        for (const auto& [key, value] : j["conditions"].items()) {
            def.conditions[key] = value.get<bool>();
        }
    }
    
    return def;
}

} // namespace Data

