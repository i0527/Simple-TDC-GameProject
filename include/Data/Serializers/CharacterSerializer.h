/**
 * @file CharacterSerializer.h
 * @brief キャラクター定義のJSONシリアライズ/デシリアライズ
 * 
 * Phase 4: REST API用のシリアライズ機能
 */
#pragma once

#include "Data/Definitions/CharacterDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief キャラクター定義のシリアライザー
 */
class CharacterSerializer {
public:
    /**
     * @brief CharacterDefをJSONにシリアライズ
     */
    static nlohmann::json Serialize(const CharacterDef& def);
    
    /**
     * @brief JSONからCharacterDefをデシリアライズ
     */
    static CharacterDef Deserialize(const nlohmann::json& j);

private:
    static std::string RarityToString(Rarity rarity);
    static Rarity StringToRarity(const std::string& str);
    static std::string GameModeTypeToString(GameModeType type);
    static GameModeType StringToGameModeType(const std::string& str);
    static std::string AttackTypeToString(AttackType type);
    static AttackType StringToAttackType(const std::string& str);
    static nlohmann::json SerializeRect(const Rect& rect);
    static Rect DeserializeRect(const nlohmann::json& j);
    static nlohmann::json SerializeAnimationDef(const AnimationDef& anim);
    static AnimationDef DeserializeAnimationDef(const nlohmann::json& j);
};

} // namespace Data

