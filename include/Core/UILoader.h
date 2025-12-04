/**
 * @file UILoader.h
 * @brief UI定義JSONローダー
 * 
 * Phase 4B: データ駆動UIシステム
 * JSONファイルからUILayoutDefを読み込む
 */
#pragma once

#include "Core/UIDefinitions.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace Data {

/**
 * @brief UI定義JSONローダー
 */
class UILoader {
public:
    /**
     * @brief JSONファイルからUILayoutDefを読み込む
     */
    static std::optional<UILayoutDef> LoadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "UILoader: Failed to open file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j;
            file >> j;
            return ParseLayout(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "UILoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "UILoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief JSON文字列からUILayoutDefを読み込む
     */
    static std::optional<UILayoutDef> LoadFromString(const std::string& jsonStr) {
        try {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            return ParseLayout(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "UILoader: JSON parse error: " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "UILoader: Error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

private:
    static UILayoutDef ParseLayout(const nlohmann::json& j) {
        UILayoutDef layout;
        
        layout.id = j.value("id", "unnamed");
        layout.name = j.value("name", layout.id);
        layout.baseWidth = j.value("baseWidth", 1920);
        layout.baseHeight = j.value("baseHeight", 1080);
        
        if (j.contains("elements") && j["elements"].is_array()) {
            for (const auto& elemJson : j["elements"]) {
                layout.elements.push_back(ParseElement(elemJson));
            }
        }
        
        if (j.contains("conditions") && j["conditions"].is_object()) {
            for (auto& [key, value] : j["conditions"].items()) {
                layout.conditions[key] = value.get<bool>();
            }
        }
        
        return layout;
    }
    
    static UIElementDef ParseElement(const nlohmann::json& j) {
        UIElementDef elem;
        
        elem.id = j.value("id", "");
        elem.type = ParseElementType(j.value("type", "panel"));
        
        // 位置・サイズ
        elem.x = j.value("x", 0.0f);
        elem.y = j.value("y", 0.0f);
        elem.width = j.value("width", 100.0f);
        elem.height = j.value("height", 50.0f);
        
        // アンカー
        elem.anchor = ParseAnchor(j.value("anchor", "topLeft"));
        elem.pivot = ParseAnchor(j.value("pivot", "topLeft"));
        
        // 色
        elem.backgroundColor = ParseColor(j, "backgroundColor", UIColor{50, 50, 60, 255});
        elem.borderColor = ParseColor(j, "borderColor", UIColor{80, 80, 100, 255});
        elem.borderWidth = j.value("borderWidth", 0.0f);
        elem.cornerRadius = j.value("cornerRadius", 0.0f);
        elem.opacity = j.value("opacity", 1.0f);
        
        // テキスト
        elem.text = j.value("text", "");
        elem.fontId = j.value("fontId", "");
        elem.fontSize = j.value("fontSize", 16);
        elem.textColor = ParseColor(j, "textColor", UIColor{255, 255, 255, 255});
        elem.textAlign = ParseAnchor(j.value("textAlign", "center"));
        
        // 画像
        elem.imageId = j.value("imageId", "");
        
        // プログレスバー
        elem.fillColor = ParseColor(j, "fillColor", UIColor{100, 200, 100, 255});
        elem.bindValue = j.value("bindValue", "");
        elem.vertical = j.value("vertical", false);
        
        // インタラクション
        elem.interactive = j.value("interactive", false);
        elem.onClick = j.value("onClick", "");
        elem.onHover = j.value("onHover", "");
        
        // ホバー時スタイル
        if (j.contains("hoverBackgroundColor")) {
            elem.hoverBackgroundColor = ParseColor(j, "hoverBackgroundColor", UIColor{});
        }
        if (j.contains("hoverBorderColor")) {
            elem.hoverBorderColor = ParseColor(j, "hoverBorderColor", UIColor{});
        }
        
        // 状態
        elem.visible = j.value("visible", true);
        elem.enabled = j.value("enabled", true);
        
        // 繰り返し
        elem.repeatCount = j.value("repeatCount", 0);
        elem.repeatSpacingX = j.value("repeatSpacingX", 0.0f);
        elem.repeatSpacingY = j.value("repeatSpacingY", 0.0f);
        elem.repeatBindArray = j.value("repeatBindArray", "");
        
        // 子要素
        if (j.contains("children") && j["children"].is_array()) {
            for (const auto& childJson : j["children"]) {
                elem.children.push_back(ParseElement(childJson));
            }
        }
        
        return elem;
    }
    
    static UIColor ParseColor(const nlohmann::json& j, const std::string& key, UIColor defaultColor) {
        if (!j.contains(key)) return defaultColor;
        
        const auto& colorJson = j[key];
        
        // 配列形式 [r, g, b] or [r, g, b, a]
        if (colorJson.is_array()) {
            UIColor color;
            if (colorJson.size() >= 3) {
                color.r = colorJson[0].get<unsigned char>();
                color.g = colorJson[1].get<unsigned char>();
                color.b = colorJson[2].get<unsigned char>();
                color.a = colorJson.size() >= 4 ? colorJson[3].get<unsigned char>() : 255;
            }
            return color;
        }
        
        // オブジェクト形式 {r, g, b, a}
        if (colorJson.is_object()) {
            UIColor color;
            color.r = colorJson.value("r", defaultColor.r);
            color.g = colorJson.value("g", defaultColor.g);
            color.b = colorJson.value("b", defaultColor.b);
            color.a = colorJson.value("a", defaultColor.a);
            return color;
        }
        
        // 文字列形式（プリセット）
        if (colorJson.is_string()) {
            return ParseColorPreset(colorJson.get<std::string>());
        }
        
        return defaultColor;
    }
    
    static UIColor ParseColorPreset(const std::string& name) {
        static const std::unordered_map<std::string, UIColor> presets = {
            {"white", {255, 255, 255, 255}},
            {"black", {0, 0, 0, 255}},
            {"red", {230, 41, 55, 255}},
            {"green", {0, 228, 48, 255}},
            {"blue", {0, 121, 241, 255}},
            {"yellow", {253, 249, 0, 255}},
            {"orange", {255, 161, 0, 255}},
            {"gray", {130, 130, 130, 255}},
            {"darkGray", {80, 80, 80, 255}},
            {"lightGray", {200, 200, 200, 255}},
            {"transparent", {0, 0, 0, 0}},
            {"panelDark", {20, 20, 30, 230}},
            {"panelLight", {50, 55, 65, 255}},
            {"border", {80, 80, 100, 255}},
            {"highlight", {100, 150, 255, 255}},
        };
        
        auto it = presets.find(name);
        return it != presets.end() ? it->second : UIColor{255, 255, 255, 255};
    }
};

} // namespace Data
