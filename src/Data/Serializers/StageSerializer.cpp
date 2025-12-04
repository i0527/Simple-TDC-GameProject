/**
 * @file StageSerializer.cpp
 * @brief ステージ定義のJSONシリアライズ/デシリアライズ実装
 */

#include "Data/Serializers/StageSerializer.h"
#include "Data/Loaders/StageLoader.h"

namespace Data {

nlohmann::json StageSerializer::SerializeEnemySpawnEntry(const EnemySpawnEntry& entry) {
    nlohmann::json j;
    j["characterId"] = entry.characterId;
    j["count"] = entry.count;
    j["delay"] = entry.delay;
    j["interval"] = entry.interval;
    j["lane"] = entry.lane;
    return j;
}

EnemySpawnEntry StageSerializer::DeserializeEnemySpawnEntry(const nlohmann::json& j) {
    EnemySpawnEntry entry;
    entry.characterId = j.value("characterId", "");
    entry.count = j.value("count", 1);
    entry.delay = j.value("delay", 0.0f);
    entry.interval = j.value("interval", 1.0f);
    entry.lane = j.value("lane", 0);
    return entry;
}

nlohmann::json StageSerializer::SerializeWaveDef(const WaveDef& wave) {
    nlohmann::json j;
    j["waveNumber"] = wave.waveNumber;
    j["duration"] = wave.duration;
    j["triggerCondition"] = wave.triggerCondition;
    
    j["enemies"] = nlohmann::json::array();
    for (const auto& enemy : wave.enemies) {
        j["enemies"].push_back(SerializeEnemySpawnEntry(enemy));
    }
    
    return j;
}

WaveDef StageSerializer::DeserializeWaveDef(const nlohmann::json& j) {
    WaveDef wave;
    wave.waveNumber = j.value("waveNumber", 0);
    wave.duration = j.value("duration", 30.0f);
    wave.triggerCondition = j.value("triggerCondition", "time");
    
    if (j.contains("enemies") && j["enemies"].is_array()) {
        for (const auto& enemyJson : j["enemies"]) {
            wave.enemies.push_back(DeserializeEnemySpawnEntry(enemyJson));
        }
    }
    
    return wave;
}

nlohmann::json StageSerializer::Serialize(const StageDef& def) {
    nlohmann::json j;
    
    // 基本情報
    j["id"] = def.id;
    j["name"] = def.name;
    if (!def.description.empty()) {
        j["description"] = def.description;
    }
    if (!def.backgroundPath.empty()) {
        j["backgroundPath"] = def.backgroundPath;
    }
    
    // Waves
    j["waves"] = nlohmann::json::array();
    for (const auto& wave : def.waves) {
        j["waves"].push_back(SerializeWaveDef(wave));
    }
    
    // 勝利条件
    j["baseHealth"] = def.baseHealth;
    j["enemyBaseHealth"] = def.enemyBaseHealth;
    j["timeLimit"] = def.timeLimit;
    
    // 報酬
    j["clearReward"] = def.clearReward;
    j["firstClearBonus"] = def.firstClearBonus;
    
    if (!def.dropCharacterIds.empty()) {
        j["dropCharacterIds"] = nlohmann::json::array();
        for (const auto& charId : def.dropCharacterIds) {
            j["dropCharacterIds"].push_back(charId);
        }
    }
    
    // コスト
    j["startingCost"] = def.startingCost;
    j["costRegenRate"] = def.costRegenRate;
    j["maxCost"] = def.maxCost;
    
    // レーン設定
    j["laneCount"] = def.laneCount;
    j["laneHeight"] = def.laneHeight;
    
    // 難易度調整
    j["enemyHealthMultiplier"] = def.enemyHealthMultiplier;
    j["enemyAttackMultiplier"] = def.enemyAttackMultiplier;
    
    return j;
}

StageDef StageSerializer::Deserialize(const nlohmann::json& j) {
    StageDef def;
    
    // 基本情報
    def.id = j.value("id", "");
    def.name = j.value("name", def.id);
    def.description = j.value("description", "");
    def.backgroundPath = j.value("backgroundPath", "");
    
    // Waves
    if (j.contains("waves") && j["waves"].is_array()) {
        for (const auto& waveJson : j["waves"]) {
            def.waves.push_back(DeserializeWaveDef(waveJson));
        }
    }
    
    // 勝利条件
    def.baseHealth = j.value("baseHealth", 1000.0f);
    def.enemyBaseHealth = j.value("enemyBaseHealth", 1000.0f);
    def.timeLimit = j.value("timeLimit", 0.0f);
    
    // 報酬
    def.clearReward = j.value("clearReward", 100);
    def.firstClearBonus = j.value("firstClearBonus", 50);
    
    if (j.contains("dropCharacterIds") && j["dropCharacterIds"].is_array()) {
        for (const auto& charId : j["dropCharacterIds"]) {
            def.dropCharacterIds.push_back(charId.get<std::string>());
        }
    }
    
    // コスト
    def.startingCost = j.value("startingCost", 500.0f);
    def.costRegenRate = j.value("costRegenRate", 10.0f);
    def.maxCost = j.value("maxCost", 9999.0f);
    
    // レーン設定
    def.laneCount = j.value("laneCount", 1);
    def.laneHeight = j.value("laneHeight", 100.0f);
    
    // 難易度調整
    def.enemyHealthMultiplier = j.value("enemyHealthMultiplier", 1.0f);
    def.enemyAttackMultiplier = j.value("enemyAttackMultiplier", 1.0f);
    
    return def;
}

} // namespace Data

