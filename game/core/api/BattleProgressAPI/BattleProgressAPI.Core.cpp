#include "../BattleProgressAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cmath>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../BattleSetupAPI.hpp"
#include "../GameplayDataAPI.hpp"
#include "../SetupAPI.hpp"
#include "../SceneOverlayControlAPI.hpp"
#include "../../config/SharedContext.hpp"
#include "../../system/TowerEnhancementEffects.hpp"

namespace game {
namespace core {

BattleProgressAPI::BattleProgressAPI()
    : sharedContext_(nullptr),
      ecsAPI_(nullptr),
      gameplayDataAPI_(nullptr),
      setupAPI_(nullptr),
      sceneOverlayAPI_(nullptr),
      battleResult_(BattleResult::InProgress),
      battleTime_(0.0f),
      currentWave_(1),
      totalWaves_(1),
      spawnCursor_(0),
      gold_(500),
      goldMaxCap_(9999),
      goldMaxCurrent_(9999.0f),
      goldMaxGrowthPerSecond_(30.0f),
      goldRegenPerSecond_(10.0f),
      goldRegenAccumulator_(0.0f),
      gameSpeed_(1.0f),
      isPaused_(false),
      gameStateText_("準備中..."),
      isInitialized_(false) {
}

bool BattleProgressAPI::Initialize(SharedContext* sharedContext) {
    sharedContext_ = sharedContext;
    ecsAPI_ = sharedContext ? sharedContext->ecsAPI : nullptr;
    gameplayDataAPI_ = sharedContext ? sharedContext->gameplayDataAPI : nullptr;
    setupAPI_ = sharedContext ? sharedContext->setupAPI : nullptr;
    sceneOverlayAPI_ = sharedContext ? sharedContext->sceneOverlayAPI : nullptr;

    if (!sharedContext_) {
        LOG_ERROR("BattleProgressAPI::Initialize: sharedContext is null");
        isInitialized_ = false;
        return false;
    }
    isInitialized_ = true;
    return true;
}

void BattleProgressAPI::InitializeFromStage() {
    if (sharedContext_ && sharedContext_->battleSetupAPI) {
        sharedContext_->battleSetupData = sharedContext_->battleSetupAPI->BuildBattleSetupData(
            sharedContext_->currentStageId,
            sharedContext_->formationData);
        InitializeFromSetupData(sharedContext_->battleSetupData);
        return;
    }

    // デフォルト
    lane_ = LaneConfig{};
    playerTower_ = TowerState{};
    enemyTower_ = TowerState{};
    currentWave_ = 1;
    totalWaves_ = 1;
    battleTime_ = 0.0f;
    battleResult_ = BattleResult::InProgress;
    gold_ = 500;
    goldMaxCap_ = 9999;
    goldMaxCurrent_ = static_cast<float>(goldMaxCap_);
    goldMaxGrowthPerSecond_ = 30.0f;
    goldRegenPerSecond_ = 10.0f;
    goldRegenAccumulator_ = 0.0f;
    gameSpeed_ = 1.0f;
    isPaused_ = false;
    unitCooldownUntil_.clear();

    // SharedContextからステージ情報を取得
    if (!sharedContext_ || !gameplayDataAPI_ || sharedContext_->currentStageId.empty()) {
        LOG_WARN("No stage selected, using default battle config");
        gameStateText_ = "Battle";
    } else {
        auto stageData = gameplayDataAPI_->GetStageDataById(sharedContext_->currentStageId);
        if (!stageData) {
            LOG_WARN("Stage not found: {}, using default battle config", sharedContext_->currentStageId);
            gameStateText_ = "Battle";
        } else {
            gameStateText_ = stageData->stageName.empty() ? "Battle" : stageData->stageName;

            // lanes[0]
            try {
                if (stageData->data.contains("lanes") && stageData->data["lanes"].is_array() && !stageData->data["lanes"].empty()) {
                    const auto& lane0 = stageData->data["lanes"][0];
                    lane_.y = lane0.value("y", lane_.y);
                    lane_.startX = lane0.value("startX", lane_.startX);
                    lane_.endX = lane0.value("endX", lane_.endX);
                }
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read lanes from stage json: {}", e.what());
            }

            // minGap
            try {
                lane_.minGap = stageData->data.value("minGap", lane_.minGap);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read minGap from stage json: {}", e.what());
            }

            // wave count（後続TODOでWaveLoader接続）
            totalWaves_ = stageData->waveCount > 0 ? stageData->waveCount : 1;
            currentWave_ = 1;
            if (setupAPI_) {
                spawnSchedule_ = setupAPI_->LoadStageSpawnEvents(stageData->data);
            } else {
                spawnSchedule_.clear();
            }
            spawnCursor_ = 0;
            if (!spawnSchedule_.empty()) {
                LOG_INFO("Spawn schedule loaded: {} events", spawnSchedule_.size());
            } else {
                LOG_INFO("Spawn schedule is empty");
            }

            // gold
            try {
                gold_ = stageData->data.value("startingCost", gold_);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read startingCost from stage json: {}", e.what());
            }

            // お財布（最大値）/ 回復（任意キー。未指定ならデフォルト）
            try {
                goldMaxCap_ = stageData->data.value("maxCost", goldMaxCap_);
                goldMaxCap_ = stageData->data.value("maxGold", goldMaxCap_);

                // お財布開始値（無ければ cap の 1/4（最大1000））
                const int defaultStart = std::max(100, std::min(1000, goldMaxCap_ / 4));
                int startMax = stageData->data.value("walletMaxStart", defaultStart);
                startMax = stageData->data.value("startMaxGold", startMax);
                goldMaxCurrent_ = static_cast<float>(std::max(0, std::min(startMax, goldMaxCap_)));

                goldMaxGrowthPerSecond_ = stageData->data.value("walletGrowthPerSecond", goldMaxGrowthPerSecond_);
                goldMaxGrowthPerSecond_ = stageData->data.value("walletMaxGrowthPerSecond", goldMaxGrowthPerSecond_);

                goldRegenPerSecond_ = stageData->data.value("goldRegenPerSecond", goldRegenPerSecond_);
                goldRegenPerSecond_ = stageData->data.value("costRegenPerSecond", goldRegenPerSecond_);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read gold regen config from stage json: {}", e.what());
            }
            const int curMax = std::max(0, static_cast<int>(goldMaxCurrent_));
            gold_ = std::max(0, std::min(gold_, curMax));

            // castle hp
            int playerHp = 1000;
            int enemyHp = 6000;
            try {
                if (stageData->data.contains("castle_hp") && stageData->data["castle_hp"].is_object()) {
                    const auto& chp = stageData->data["castle_hp"];
                    playerHp = chp.value("player_castle_hp", playerHp);
                    enemyHp = chp.value("enemy_castle_hp", enemyHp);
                } else {
                    // 互換フォールバック（旧 playerLife 等）
                    playerHp = stageData->data.value("playerLife", playerHp);
                    enemyHp = stageData->data.value("enemyLife", enemyHp);
                }
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read castle_hp from stage json: {}", e.what());
            }

            playerTower_.maxHp = playerTower_.currentHp = playerHp;
            enemyTower_.maxHp = enemyTower_.currentHp = enemyHp;
        }
    }

    // ===== タワー強化（セーブ永続）を戦闘へ反映 =====
    // - 城HP: 最大値へ乗算（開始時は満タン）
    // - お財布成長/秒、コスト回復/秒: 乗算
    if (gameplayDataAPI_) {
        const auto te = gameplayDataAPI_->GetTowerEnhancements();
        const auto attachments = gameplayDataAPI_->GetTowerAttachments();
        const auto& masters = gameplayDataAPI_->GetAllTowerAttachmentMasters();
        const auto mul = system::CalculateTowerEnhancementMultipliers(te, attachments, masters);

        playerTower_.maxHp = std::max(
            1, static_cast<int>(std::round(static_cast<float>(playerTower_.maxHp) * mul.playerTowerHpMul)));
        playerTower_.currentHp = playerTower_.maxHp;

        goldMaxGrowthPerSecond_ = std::max(0.0f, goldMaxGrowthPerSecond_ * mul.walletGrowthMul);
        goldRegenPerSecond_ = std::max(0.0f, goldRegenPerSecond_ * mul.costRegenMul);
    }

    // ===== 画面レイアウト: 下部ユニットバー直上に戦闘レーンを配置 =====
    // BattleHUDRenderer の BOTTOM_H(=240) を前提に、HUD上端より少し上へ寄せる。
    constexpr float SCREEN_H = 1080.0f;
    constexpr float HUD_BOTTOM_H = 240.0f;
    constexpr float LANE_MARGIN_ABOVE_HUD = 30.0f;
    lane_.y = SCREEN_H - HUD_BOTTOM_H - LANE_MARGIN_ABOVE_HUD;

    // タワー配置（lane start/endに配置）
    enemyTower_.x = lane_.startX;
    enemyTower_.y = lane_.y;
    playerTower_.x = lane_.endX;
    playerTower_.y = lane_.y;

    // 簡易マッピング（後でデータ駆動化可能）
    enemyToCharacterId_.clear();
    enemyToCharacterId_["enemy_slime"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_goblin"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_brute"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_slime_debug"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_ogre_debug"] = "char_sub_orca_001";
    // インラインwaves用（短いID）
    enemyToCharacterId_["goblin"] = "char_sub_hatslime_001";
    enemyToCharacterId_["goblin_boss"] = "char_sub_lanterfish_001";
    enemyToCharacterId_["ogre"] = "char_sub_yodarehaki_001";
    enemyToCharacterId_["ogre_boss"] = "char_sub_chainsword_001";
    enemyToCharacterId_["dragon"] = "char_sub_rainbow_001";
    enemyToCharacterId_["dragon_boss"] = "char_sub_orca_001";
}

void BattleProgressAPI::InitializeFromSetupData(const BattleSetupData& data) {
    lane_ = LaneConfig{};
    playerTower_ = TowerState{};
    enemyTower_ = TowerState{};
    currentWave_ = 1;
    totalWaves_ = 1;
    battleTime_ = 0.0f;
    battleResult_ = BattleResult::InProgress;
    gold_ = 500;
    goldMaxCap_ = 9999;
    goldMaxCurrent_ = static_cast<float>(goldMaxCap_);
    goldMaxGrowthPerSecond_ = 30.0f;
    goldRegenPerSecond_ = 10.0f;
    goldRegenAccumulator_ = 0.0f;
    gameSpeed_ = 1.0f;
    isPaused_ = false;
    unitCooldownUntil_.clear();

    lane_.y = data.lane.y;
    lane_.startX = data.lane.startX;
    lane_.endX = data.lane.endX;
    lane_.minGap = data.lane.minGap;

    playerTower_.currentHp = data.playerTower.currentHp;
    playerTower_.maxHp = data.playerTower.maxHp;
    playerTower_.x = data.playerTower.x;
    playerTower_.y = data.playerTower.y;
    playerTower_.width = data.playerTower.width;
    playerTower_.height = data.playerTower.height;

    enemyTower_.currentHp = data.enemyTower.currentHp;
    enemyTower_.maxHp = data.enemyTower.maxHp;
    enemyTower_.x = data.enemyTower.x;
    enemyTower_.y = data.enemyTower.y;
    enemyTower_.width = data.enemyTower.width;
    enemyTower_.height = data.enemyTower.height;
    currentWave_ = data.currentWave;
    totalWaves_ = data.totalWaves;
    spawnSchedule_ = data.spawnSchedule;
    spawnCursor_ = 0;
    gold_ = data.gold;
    goldMaxCap_ = data.goldMaxCap;
    goldMaxCurrent_ = data.goldMaxCurrent;
    goldMaxGrowthPerSecond_ = data.goldMaxGrowthPerSecond;
    goldRegenPerSecond_ = data.goldRegenPerSecond;
    gameSpeed_ = data.gameSpeed;
    isPaused_ = data.isPaused;
    gameStateText_ = data.gameStateText;

    if (!spawnSchedule_.empty()) {
        LOG_INFO("Spawn schedule loaded: {} events", spawnSchedule_.size());
    } else {
        LOG_INFO("Spawn schedule is empty");
    }

    enemyToCharacterId_.clear();
    enemyToCharacterId_["enemy_slime"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_goblin"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_brute"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_slime_debug"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_ogre_debug"] = "char_sub_orca_001";
    enemyToCharacterId_["goblin"] = "char_sub_hatslime_001";
    enemyToCharacterId_["goblin_boss"] = "char_sub_lanterfish_001";
    enemyToCharacterId_["ogre"] = "char_sub_yodarehaki_001";
    enemyToCharacterId_["ogre_boss"] = "char_sub_chainsword_001";
    enemyToCharacterId_["dragon"] = "char_sub_rainbow_001";
    enemyToCharacterId_["dragon_boss"] = "char_sub_orca_001";
}

int BattleProgressAPI::GetGoldMaxCurrent() const {
    return std::max(0, static_cast<int>(goldMaxCurrent_));
}

void BattleProgressAPI::SetGameSpeed(float speed) {
    gameSpeed_ = speed;
}

void BattleProgressAPI::SetPaused(bool paused) {
    isPaused_ = paused;
}

} // namespace core
} // namespace game
