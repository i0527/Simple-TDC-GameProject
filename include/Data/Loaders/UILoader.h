/**
 * @file UILoader.h
 * @brief UIレイアウト定義ローダー
 * 
 * JSONからUIレイアウト定義を読み込み、DefinitionRegistryに登録する
 */
#pragma once

#include "Data/Loaders/DataLoaderBase.h"
#include "Data/Definitions/UILayoutDef.h"
#include "Core/Platform.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief UIレイアウト定義ローダー
 */
class UILoader : public DataLoaderBase {
public:
    explicit UILoader(DefinitionRegistry& registry)
        : DataLoaderBase(registry) {}
    
    /**
     * @brief 単一のUIレイアウト定義ファイルを読み込み
     */
    bool LoadUILayout(const std::string& filePath) {
        try {
            auto jsonData = LoadJsonFile(filePath);
            if (jsonData.is_null()) return false;
            
            auto def = ParseUILayoutDef(jsonData);
            if (def.id.empty()) {
                def.id = GetFileNameWithoutExtension(filePath);
            }
            registry_.RegisterUILayout(std::move(def));
            return true;
        } catch (const std::exception& e) {
            errorHandler_(filePath, e.what());
            return false;
        }
    }
    
    /**
     * @brief ディレクトリ内の全UIレイアウト定義を読み込み
     */
    int LoadAllUILayouts(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".ui.json",
            [this](const std::string& path) { return LoadUILayout(path); });
    }
    
    /**
     * @brief JSONからUILayoutDefをパース
     */
    UILayoutDef ParseUILayoutDef(const json& j) {
        UILayoutDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.baseWidth = GetOr(j, "baseWidth", 1920);
        def.baseHeight = GetOr(j, "baseHeight", 1080);
        
        // 要素
        if (j.contains("elements") && j["elements"].is_array()) {
            for (const auto& elemJson : j["elements"]) {
                def.elements.push_back(ParseUIElementDef(elemJson));
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
    
    /**
     * @brief JSONからUIElementDefをパース
     */
    UIElementDef ParseUIElementDef(const json& j) {
        UIElementDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.type = ParseElementType(GetOr<std::string>(j, "type", "panel"));
        
        // 位置・サイズ
        if (j.contains("position")) {
            const auto& pos = j["position"];
            def.x = GetOr(pos, "x", 0.0f);
            def.y = GetOr(pos, "y", 0.0f);
        }
        
        if (j.contains("size")) {
            const auto& size = j["size"];
            def.width = GetOr(size, "width", 100.0f);
            def.height = GetOr(size, "height", 50.0f);
        }
        
        // アンカー
        if (j.contains("anchor")) {
            def.anchor = ParseAnchor(GetOr<std::string>(j, "anchor", "topLeft"));
        }
        
        if (j.contains("pivot")) {
            def.pivot = ParseAnchor(GetOr<std::string>(j, "pivot", "topLeft"));
        }
        
        // 見た目
        if (j.contains("backgroundColor")) {
            def.backgroundColor = ParseColor(j["backgroundColor"]);
        }
        
        if (j.contains("borderColor")) {
            def.borderColor = ParseColor(j["borderColor"]);
        }
        
        def.borderWidth = GetOr(j, "borderWidth", 0.0f);
        def.cornerRadius = GetOr(j, "cornerRadius", 0.0f);
        def.opacity = GetOr(j, "opacity", 1.0f);
        
        // テキスト
        def.text = GetOr<std::string>(j, "text", "");
        def.fontId = GetOr<std::string>(j, "fontId", "");
        def.fontSize = GetOr(j, "fontSize", 16);
        
        if (j.contains("textColor")) {
            def.textColor = ParseColor(j["textColor"]);
        }
        
        if (j.contains("textAlign")) {
            def.textAlign = ParseAnchor(GetOr<std::string>(j, "textAlign", "center"));
        }
        
        // 画像
        def.imageId = GetOr<std::string>(j, "imageId", "");
        
        // プログレスバー
        if (j.contains("fillColor")) {
            def.fillColor = ParseColor(j["fillColor"]);
        }
        
        def.bindValue = GetOr<std::string>(j, "bind", "");
        def.vertical = GetOr(j, "vertical", false);
        
        // インタラクション
        def.interactive = GetOr(j, "interactive", false);
        def.onClick = GetOr<std::string>(j, "onClick", "");
        def.onHover = GetOr<std::string>(j, "onHover", "");
        
        // 状態
        def.visible = GetOr(j, "visible", true);
        def.enabled = GetOr(j, "enabled", true);
        
        // 子要素
        if (j.contains("children") && j["children"].is_array()) {
            for (const auto& childJson : j["children"]) {
                def.children.push_back(ParseUIElementDef(childJson));
            }
        }
        
        return def;
    }

private:
    template<typename T>
    T GetOr(const json& j, const std::string& key, T defaultValue) {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    UIAnchor ParseAnchor(const std::string& str) {
        if (str == "top_left" || str == "topLeft") return UIAnchor::TopLeft;
        if (str == "top_center" || str == "topCenter") return UIAnchor::TopCenter;
        if (str == "top_right" || str == "topRight") return UIAnchor::TopRight;
        if (str == "middle_left" || str == "middleLeft") return UIAnchor::MiddleLeft;
        if (str == "center") return UIAnchor::Center;
        if (str == "middle_right" || str == "middleRight") return UIAnchor::MiddleRight;
        if (str == "bottom_left" || str == "bottomLeft") return UIAnchor::BottomLeft;
        if (str == "bottom_center" || str == "bottomCenter") return UIAnchor::BottomCenter;
        if (str == "bottom_right" || str == "bottomRight") return UIAnchor::BottomRight;
        return UIAnchor::TopLeft;
    }
    
    UIElementType ParseElementType(const std::string& str) {
        if (str == "panel") return UIElementType::Panel;
        if (str == "text") return UIElementType::Text;
        if (str == "image") return UIElementType::Image;
        if (str == "button") return UIElementType::Button;
        if (str == "progressBar" || str == "progress_bar") return UIElementType::ProgressBar;
        if (str == "slot") return UIElementType::Slot;
        if (str == "container") return UIElementType::Container;
        return UIElementType::Panel;
    }
    
    UIColor ParseColor(const json& j) {
        UIColor color{255, 255, 255, 255};
        
        if (j.is_string()) {
            // "#RRGGBB" or "#RRGGBBAA"形式
            std::string colorStr = j.get<std::string>();
            if (colorStr.length() >= 7 && colorStr[0] == '#') {
                // 簡易的な16進数パース
                // TODO: より堅牢な実装
            }
        } else if (j.is_object()) {
            color.r = GetOr(j, "r", 255);
            color.g = GetOr(j, "g", 255);
            color.b = GetOr(j, "b", 255);
            color.a = GetOr(j, "a", 255);
        }
        
        return color;
    }
};

} // namespace Data

