/**
 * @file UISerializer.h
 * @brief UIレイアウト定義のJSONシリアライズ/デシリアライズ
 * 
 * Phase 4: REST API用のシリアライズ機能
 */
#pragma once

#include "Data/Definitions/UILayoutDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief UIレイアウト定義のシリアライザー
 */
class UISerializer {
public:
    /**
     * @brief UILayoutDefをJSONにシリアライズ
     */
    static nlohmann::json Serialize(const UILayoutDef& def);
    
    /**
     * @brief UIElementDefをJSONにシリアライズ
     */
    static nlohmann::json SerializeElement(const UIElementDef& elem);
    
    /**
     * @brief JSONからUILayoutDefをデシリアライズ
     */
    static UILayoutDef Deserialize(const nlohmann::json& j);
    
    /**
     * @brief JSONからUIElementDefをデシリアライズ
     */
    static UIElementDef DeserializeElement(const nlohmann::json& j);

private:
    static std::string AnchorToString(UIAnchor anchor);
    static UIAnchor StringToAnchor(const std::string& str);
    static std::string ElementTypeToString(UIElementType type);
    static UIElementType StringToElementType(const std::string& str);
    static nlohmann::json SerializeColor(const UIColor& color);
    static UIColor DeserializeColor(const nlohmann::json& j);
};

} // namespace Data

