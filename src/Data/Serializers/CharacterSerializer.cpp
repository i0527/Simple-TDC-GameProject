/**
 * @file CharacterSerializer.cpp
 * @brief キャラクター定義のJSONシリアライズ/デシリアライズ実装
 */

#include "Data/Serializers/CharacterSerializer.h"
#include "Data/Loaders/CharacterLoader.h"
#include <sstream>

namespace Data {

std::string CharacterSerializer::RarityToString(Rarity rarity) {
    switch (rarity) {
        case Rarity::Normal: return "normal";
        case Rarity::Rare: return "rare";
        case Rarity::SuperRare: return "superRare";
        case Rarity::UberRare: return "uberRare";
        case Rarity::Legend: return "legend";
        default: return "normal";
    }
}

Rarity CharacterSerializer::StringToRarity(const std::string& str) {
    if (str == "normal") return Rarity::Normal;
    if (str == "rare") return Rarity::Rare;
    if (str == "super_rare" || str == "superRare") return Rarity::SuperRare;
    if (str == "uber_rare" || str == "uberRare") return Rarity::UberRare;
    if (str == "legend") return Rarity::Legend;
    return Rarity::Normal;
}

std::string CharacterSerializer::GameModeTypeToString(GameModeType type) {
    switch (type) {
        case GameModeType::TD: return "td";
        case GameModeType::Roguelike: return "roguelike";
        case GameModeType::Both: return "both";
        default: return "both";
    }
}

GameModeType CharacterSerializer::StringToGameModeType(const std::string& str) {
    if (str == "td") return GameModeType::TD;
    if (str == "roguelike") return GameModeType::Roguelike;
    if (str == "both") return GameModeType::Both;
    return GameModeType::Both;
}

std::string CharacterSerializer::AttackTypeToString(AttackType type) {
    switch (type) {
        case AttackType::Single: return "single";
        case AttackType::Area: return "area";
        case AttackType::Wave: return "wave";
        default: return "single";
    }
}

AttackType CharacterSerializer::StringToAttackType(const std::string& str) {
    if (str == "single") return AttackType::Single;
    if (str == "area") return AttackType::Area;
    if (str == "wave") return AttackType::Wave;
    return AttackType::Single;
}

nlohmann::json CharacterSerializer::SerializeRect(const Rect& rect) {
    nlohmann::json j;
    j["x"] = rect.x;
    j["y"] = rect.y;
    j["width"] = rect.width;
    j["height"] = rect.height;
    return j;
}

Rect CharacterSerializer::DeserializeRect(const nlohmann::json& j) {
    Rect r;
    r.x = j.value("x", 0.0f);
    r.y = j.value("y", 0.0f);
    r.width = j.value("width", 0.0f);
    r.height = j.value("height", 0.0f);
    return r;
}

nlohmann::json CharacterSerializer::SerializeAnimationDef(const AnimationDef& anim) {
    nlohmann::json j;
    j["name"] = anim.name;
    j["loop"] = anim.loop;
    if (!anim.nextAnimation.empty()) {
        j["nextAnimation"] = anim.nextAnimation;
    }
    
    j["frames"] = nlohmann::json::array();
    for (const auto& frame : anim.frames) {
        nlohmann::json frameJson;
        frameJson["index"] = frame.index;
        frameJson["duration"] = frame.duration;
        if (!frame.tag.empty()) {
            frameJson["tag"] = frame.tag;
        }
        j["frames"].push_back(frameJson);
    }
    
    return j;
}

AnimationDef CharacterSerializer::DeserializeAnimationDef(const nlohmann::json& j) {
    AnimationDef anim;
    anim.name = j.value("name", "");
    anim.loop = j.value("loop", true);
    anim.nextAnimation = j.value("nextAnimation", "");
    
    if (j.contains("frames") && j["frames"].is_array()) {
        for (const auto& frameJson : j["frames"]) {
            FrameDef frame;
            frame.index = frameJson.value("index", 0);
            frame.duration = frameJson.value("duration", 0.1f);
            frame.tag = frameJson.value("tag", "");
            anim.frames.push_back(frame);
        }
    }
    
    return anim;
}

nlohmann::json CharacterSerializer::Serialize(const CharacterDef& def) {
    nlohmann::json j;
    
    // 識別
    j["id"] = def.id;
    j["name"] = def.name;
    if (!def.description.empty()) {
        j["description"] = def.description;
    }
    j["rarity"] = RarityToString(def.rarity);
    
    if (!def.traits.empty()) {
        j["traits"] = nlohmann::json::array();
        for (const auto& trait : def.traits) {
            j["traits"].push_back(trait);
        }
    }
    
    // ゲームモード
    j["gameMode"] = GameModeTypeToString(def.gameMode);
    
    // ビジュアル
    nlohmann::json visual;
    nlohmann::json sprite;
    sprite["atlasPath"] = def.visual.sprite.atlasPath;
    sprite["jsonPath"] = def.visual.sprite.jsonPath;
    visual["sprite"] = sprite;
    
    visual["frameWidth"] = def.visual.frameWidth;
    visual["frameHeight"] = def.visual.frameHeight;
    visual["framesPerRow"] = def.visual.framesPerRow;
    visual["scale"] = def.visual.scale;
    visual["defaultAnimation"] = def.visual.defaultAnimation;
    
    if (!def.visual.animations.empty()) {
        visual["animations"] = nlohmann::json::object();
        for (const auto& [name, anim] : def.visual.animations) {
            visual["animations"][name] = SerializeAnimationDef(anim);
        }
    }
    
    j["visual"] = visual;
    
    // ステータス
    nlohmann::json stats;
    stats["hp"] = def.stats.hp;
    stats["attack"] = def.stats.attack;
    stats["defense"] = def.stats.defense;
    stats["moveSpeed"] = def.stats.moveSpeed;
    stats["attackInterval"] = def.stats.attackInterval;
    stats["knockbackResist"] = def.stats.knockbackResist;
    j["stats"] = stats;
    
    // 戦闘
    nlohmann::json combat;
    combat["attackType"] = AttackTypeToString(def.combat.attackType);
    combat["attackRange"] = def.combat.attackRange;
    combat["attackCooldown"] = def.combat.attackCooldown;
    combat["hitbox"] = SerializeRect(def.combat.hitbox);
    combat["attackRangeArea"] = SerializeRect(def.combat.attackRangeArea);
    combat["criticalChance"] = def.combat.criticalChance;
    combat["criticalMultiplier"] = def.combat.criticalMultiplier;
    combat["attackCount"] = def.combat.attackCount;
    j["combat"] = combat;
    
    // TD固有設定
    nlohmann::json td;
    td["cost"] = def.td.cost;
    td["rechargeTime"] = def.td.rechargeTime;
    td["laneMovement"] = def.td.laneMovement;
    td["laneSpeed"] = def.td.laneSpeed;
    j["td"] = td;
    
    // Roguelike固有設定
    nlohmann::json roguelike;
    roguelike["gridMovement"] = def.roguelike.gridMovement;
    roguelike["turnBased"] = def.roguelike.turnBased;
    roguelike["movementCost"] = def.roguelike.movementCost;
    j["roguelike"] = roguelike;
    
    // スキル
    if (!def.skillIds.empty()) {
        j["skillIds"] = nlohmann::json::array();
        for (const auto& skillId : def.skillIds) {
            j["skillIds"].push_back(skillId);
        }
    }
    
    // 成長率
    j["healthGrowth"] = def.healthGrowth;
    j["attackGrowth"] = def.attackGrowth;
    
    return j;
}

CharacterDef CharacterSerializer::Deserialize(const nlohmann::json& j) {
    CharacterDef def;
    
    // 識別
    def.id = j.value("id", "");
    def.name = j.value("name", def.id);
    def.description = j.value("description", "");
    def.rarity = StringToRarity(j.value("rarity", "normal"));
    
    if (j.contains("traits") && j["traits"].is_array()) {
        for (const auto& trait : j["traits"]) {
            def.traits.push_back(trait.get<std::string>());
        }
    }
    
    // ゲームモード
    def.gameMode = StringToGameModeType(j.value("gameMode", "both"));
    
    // ビジュアル
    if (j.contains("visual")) {
        const auto& visual = j["visual"];
        
        if (visual.contains("sprite")) {
            const auto& sprite = visual["sprite"];
            def.visual.sprite.atlasPath = sprite.value("atlasPath", "");
            def.visual.sprite.jsonPath = sprite.value("jsonPath", "");
        }
        
        def.visual.frameWidth = visual.value("frameWidth", 64);
        def.visual.frameHeight = visual.value("frameHeight", 64);
        def.visual.framesPerRow = visual.value("framesPerRow", 8);
        def.visual.scale = visual.value("scale", 1.0f);
        def.visual.defaultAnimation = visual.value("defaultAnimation", "idle");
        
        if (visual.contains("animations") && visual["animations"].is_object()) {
            for (const auto& [name, animJson] : visual["animations"].items()) {
                auto anim = DeserializeAnimationDef(animJson);
                anim.name = name; // 名前を確実に設定
                def.visual.animations[name] = std::move(anim);
            }
        }
    }
    
    // ステータス
    if (j.contains("stats")) {
        const auto& stats = j["stats"];
        def.stats.hp = stats.value("hp", 100.0f);
        def.stats.attack = stats.value("attack", 15.0f);
        def.stats.defense = stats.value("defense", 5.0f);
        def.stats.moveSpeed = stats.value("moveSpeed", 120.0f);
        def.stats.attackInterval = stats.value("attackInterval", 1.5f);
        def.stats.knockbackResist = stats.value("knockbackResist", 0.0f);
    }
    
    // 戦闘
    if (j.contains("combat")) {
        const auto& combat = j["combat"];
        def.combat.attackType = StringToAttackType(combat.value("attackType", "single"));
        def.combat.attackRange = combat.value("attackRange", 50.0f);
        def.combat.attackCooldown = combat.value("attackCooldown", 1.5f);
        
        if (combat.contains("hitbox")) {
            def.combat.hitbox = DeserializeRect(combat["hitbox"]);
        }
        
        if (combat.contains("attackRangeArea")) {
            def.combat.attackRangeArea = DeserializeRect(combat["attackRangeArea"]);
        }
        
        def.combat.criticalChance = combat.value("criticalChance", 0.0f);
        def.combat.criticalMultiplier = combat.value("criticalMultiplier", 1.5f);
        def.combat.attackCount = combat.value("attackCount", 1);
    }
    
    // TD固有設定
    if (j.contains("td")) {
        const auto& td = j["td"];
        def.td.cost = td.value("cost", 100.0f);
        def.td.rechargeTime = td.value("rechargeTime", 3.0f);
        def.td.laneMovement = td.value("laneMovement", true);
        def.td.laneSpeed = td.value("laneSpeed", 50.0f);
    }
    
    // Roguelike固有設定
    if (j.contains("roguelike")) {
        const auto& roguelike = j["roguelike"];
        def.roguelike.gridMovement = roguelike.value("gridMovement", true);
        def.roguelike.turnBased = roguelike.value("turnBased", true);
        def.roguelike.movementCost = roguelike.value("movementCost", 1);
    }
    
    // スキル
    if (j.contains("skillIds") && j["skillIds"].is_array()) {
        for (const auto& skillId : j["skillIds"]) {
            def.skillIds.push_back(skillId.get<std::string>());
        }
    }
    
    // 成長率
    def.healthGrowth = j.value("healthGrowth", 1.1f);
    def.attackGrowth = j.value("attackGrowth", 1.1f);
    
    return def;
}

} // namespace Data

