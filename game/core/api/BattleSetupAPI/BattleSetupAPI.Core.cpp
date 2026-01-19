#include "../BattleSetupAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cmath>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../GameplayDataAPI.hpp"
#include "../SetupAPI.hpp"
#include "../../system/TowerEnhancementEffects.hpp"

namespace game {
namespace core {

BattleSetupAPI::BattleSetupAPI()
    : gameplayDataAPI_(nullptr),
      setupAPI_(nullptr),
      sharedContext_(nullptr),
      isInitialized_(false) {
}

bool BattleSetupAPI::Initialize(GameplayDataAPI* gameplayDataAPI,
                                SetupAPI* setupAPI,
                                SharedContext* sharedContext) {
    if (!gameplayDataAPI || !setupAPI || !sharedContext) {
        LOG_ERROR("BattleSetupAPI::Initialize: invalid argument(s)");
        return false;
    }

    gameplayDataAPI_ = gameplayDataAPI;
    setupAPI_ = setupAPI;
    sharedContext_ = sharedContext;
    sharedContext_->battleSetupAPI = this;

    isInitialized_ = true;
    return true;
}

BattleSetupData BattleSetupAPI::BuildBattleSetupData(const std::string& stageId,
                                                     const FormationData& formation) const {
    BattleSetupData data;
    data.stageId = stageId;
    data.formationSlots = formation.slots;

    if (!isInitialized_ || !gameplayDataAPI_) {
        LOG_WARN("BattleSetupAPI::BuildBattleSetupData: not initialized");
        return data;
    }

    if (stageId.empty()) {
        LOG_WARN("BattleSetupAPI::BuildBattleSetupData: stageId is empty");
        return data;
    }

    auto stageData = gameplayDataAPI_->GetStageDataById(stageId);
    if (!stageData) {
        LOG_WARN("Stage not found: {}, using default battle config", stageId);
        return data;
    }

    data.hasValidStage = true;
    data.stageName = stageData->stageName;
    data.gameStateText = stageData->stageName.empty() ? "Battle" : stageData->stageName;
    data.totalWaves = stageData->waveCount > 0 ? stageData->waveCount : 1;
    data.currentWave = 1;

    if (setupAPI_) {
        data.spawnSchedule = setupAPI_->LoadStageSpawnEvents(stageData->data);
    }

    // lanes[0]
    try {
        if (stageData->data.contains("lanes") && stageData->data["lanes"].is_array() && !stageData->data["lanes"].empty()) {
            const auto& lane0 = stageData->data["lanes"][0];
            data.lane.y = lane0.value("y", data.lane.y);
            data.lane.startX = lane0.value("startX", data.lane.startX);
            data.lane.endX = lane0.value("endX", data.lane.endX);
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_WARN("JSON parse error (lanes): {}", e.what());
    } catch (const std::exception& e) {
        LOG_WARN("Failed to read lanes from stage json: {}", e.what());
    }

    // minGap
    try {
        data.lane.minGap = stageData->data.value("minGap", data.lane.minGap);
    } catch (const nlohmann::json::parse_error& e) {
        LOG_WARN("JSON parse error (minGap): {}", e.what());
    } catch (const std::exception& e) {
        LOG_WARN("Failed to read minGap from stage json: {}", e.what());
    }

    // gold
    try {
        data.gold = stageData->data.value("startingCost", data.gold);
    } catch (const nlohmann::json::parse_error& e) {
        LOG_WARN("JSON parse error (startingCost): {}", e.what());
    } catch (const std::exception& e) {
        LOG_WARN("Failed to read startingCost from stage json: {}", e.what());
    }

    // お財布（最大値）/ 回復
    try {
        data.goldMaxCap = stageData->data.value("maxCost", data.goldMaxCap);
        data.goldMaxCap = stageData->data.value("maxGold", data.goldMaxCap);

        const int defaultStart = std::max(100, std::min(1000, data.goldMaxCap / 4));
        int startMax = stageData->data.value("walletMaxStart", defaultStart);
        startMax = stageData->data.value("startMaxGold", startMax);
        data.goldMaxCurrent = static_cast<float>(std::max(0, std::min(startMax, data.goldMaxCap)));

        data.goldMaxGrowthPerSecond = stageData->data.value("walletGrowthPerSecond", data.goldMaxGrowthPerSecond);
        data.goldMaxGrowthPerSecond = stageData->data.value("walletMaxGrowthPerSecond", data.goldMaxGrowthPerSecond);

        data.goldRegenPerSecond = stageData->data.value("goldRegenPerSecond", data.goldRegenPerSecond);
        data.goldRegenPerSecond = stageData->data.value("costRegenPerSecond", data.goldRegenPerSecond);
    } catch (const nlohmann::json::parse_error& e) {
        LOG_WARN("JSON parse error (gold config): {}", e.what());
    } catch (const std::exception& e) {
        LOG_WARN("Failed to read gold regen config from stage json: {}", e.what());
    }

    const int curMax = std::max(0, static_cast<int>(data.goldMaxCurrent));
    data.gold = std::max(0, std::min(data.gold, curMax));

    // castle hp
    int playerHp = 1000;
    int enemyHp = 6000;
    try {
        if (stageData->data.contains("castle_hp") && stageData->data["castle_hp"].is_object()) {
            const auto& chp = stageData->data["castle_hp"];
            playerHp = chp.value("player_castle_hp", playerHp);
            enemyHp = chp.value("enemy_castle_hp", enemyHp);
        } else {
            playerHp = stageData->data.value("playerLife", playerHp);
            enemyHp = stageData->data.value("enemyLife", enemyHp);
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_WARN("JSON parse error (castle_hp): {}", e.what());
    } catch (const std::exception& e) {
        LOG_WARN("Failed to read castle_hp from stage json: {}", e.what());
    }

    data.playerTower.maxHp = data.playerTower.currentHp = playerHp;
    data.enemyTower.maxHp = data.enemyTower.currentHp = enemyHp;

    // タワー強化（セーブ永続）を戦闘へ反映
    if (gameplayDataAPI_) {
        const auto te = gameplayDataAPI_->GetTowerEnhancements();
        const auto attachments = gameplayDataAPI_->GetTowerAttachments();
        const auto& masters = gameplayDataAPI_->GetAllTowerAttachmentMasters();
        const auto mul = system::CalculateTowerEnhancementMultipliers(te, attachments, masters);

        data.playerTower.maxHp = std::max(
            1, static_cast<int>(std::round(static_cast<float>(data.playerTower.maxHp) * mul.playerTowerHpMul)));
        data.playerTower.currentHp = data.playerTower.maxHp;

        data.goldMaxGrowthPerSecond = std::max(0.0f, data.goldMaxGrowthPerSecond * mul.walletGrowthMul);
        data.goldRegenPerSecond = std::max(0.0f, data.goldRegenPerSecond * mul.costRegenMul);
    }

    // 画面レイアウト（下部ユニットバー直上に戦闘レーン）
    constexpr float SCREEN_H = 1080.0f;
    constexpr float HUD_BOTTOM_H = 240.0f;
    constexpr float LANE_MARGIN_ABOVE_HUD = 30.0f;
    data.lane.y = SCREEN_H - HUD_BOTTOM_H - LANE_MARGIN_ABOVE_HUD;

    // タワー配置（lane start/endに配置）
    data.enemyTower.x = data.lane.startX;
    data.enemyTower.y = data.lane.y;
    data.playerTower.x = data.lane.endX;
    data.playerTower.y = data.lane.y;

    return data;
}

} // namespace core
} // namespace game
