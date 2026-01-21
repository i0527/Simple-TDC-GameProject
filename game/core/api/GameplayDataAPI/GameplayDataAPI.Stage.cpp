#include "../GameplayDataAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cstdlib>

// プロジェクト内
#include "../../ecs/entities/StageLoader.hpp"
#include "../../api/BattleProgressAPI.hpp"
#include "../../../utils/Log.h"

namespace game {
namespace core {

namespace {
const std::unordered_map<std::string, entities::StageData> kEmptyStageMap{};

void ApplyStageState(PlayerDataManager* playerDataManager,
                     entities::StageData& stage) {
    if (!playerDataManager) {
        return;
    }
    const auto st = playerDataManager->GetStageState(stage.id);
    stage.isCleared = st.isCleared;
    stage.isLocked = st.isLocked;
    stage.starsEarned = st.starsEarned;
    if (stage.isCleared) {
        stage.isLocked = false;
    }
}
} // namespace

std::shared_ptr<entities::StageData> GameplayDataAPI::GetStageDataById(
    const std::string& stageId) {
    if (!stageManager_) {
        return nullptr;
    }
    auto stage = stageManager_->GetStageDataById(stageId);
    if (stage) {
        ApplyStageState(playerDataManager_.get(), *stage);
    }
    return stage;
}

std::shared_ptr<entities::StageData> GameplayDataAPI::GetStageData(int stageNumber) {
    if (!stageManager_) {
        return nullptr;
    }
    auto stage = stageManager_->GetStageData(stageNumber);
    if (stage) {
        ApplyStageState(playerDataManager_.get(), *stage);
    }
    return stage;
}

std::vector<entities::StageData> GameplayDataAPI::GetAllStageData() const {
    if (!stageManager_) {
        return {};
    }
    auto stages = stageManager_->GetAllStageData();
    for (auto& stage : stages) {
        ApplyStageState(playerDataManager_.get(), stage);
    }
    return stages;
}

std::vector<std::string> GameplayDataAPI::GetAllStageIds() const {
    if (!stageManager_) {
        return {};
    }
    return stageManager_->GetAllStageIds();
}

bool GameplayDataAPI::HasStage(const std::string& stageId) const {
    if (!stageManager_) {
        return false;
    }
    return stageManager_->HasStage(stageId);
}

size_t GameplayDataAPI::GetStageCount() const {
    if (!stageManager_) {
        return 0;
    }
    return stageManager_->GetStageCount();
}

const std::unordered_map<std::string, entities::StageData>&
GameplayDataAPI::GetAllStages() const {
    if (!stageManager_) {
        return kEmptyStageMap;
    }
    return stageManager_->GetAllStages();
}

bool GameplayDataAPI::SaveStageMasters(
    const std::unordered_map<std::string, entities::StageData>& stages) {
    if (!stageManager_) {
        return false;
    }
    if (stageJsonPath_.empty()) {
        LOG_WARN("GameplayDataAPI::SaveStageMasters: stageJsonPath is empty");
        return false;
    }
    if (!entities::StageLoader::SaveToJSON(stageJsonPath_, stages)) {
        return false;
    }
    stageManager_->SetMasters(stages);
    return true;
}

PlayerDataManager::PlayerSaveData::StageState GameplayDataAPI::GetStageState(
    const std::string& stageId) const {
    if (!playerDataManager_) {
        return PlayerDataManager::PlayerSaveData::StageState();
    }
    return playerDataManager_->GetStageState(stageId);
}

void GameplayDataAPI::SetStageState(
    const std::string& stageId,
    const PlayerDataManager::PlayerSaveData::StageState& state) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetStageState(stageId, state);
}

void GameplayDataAPI::MarkStageCleared(const std::string& stageId, int starsEarned,
                                      const BattleProgressAPI::BattleStats* battleStats) {
    if (!playerDataManager_ || !stageManager_) {
        return;
    }
    
    auto st = playerDataManager_->GetStageState(stageId);
    const bool isFirstClear = !st.isCleared;
    st.isCleared = true;
    st.isLocked = false;
    st.starsEarned = std::max(st.starsEarned, std::max(0, starsEarned));
    playerDataManager_->SetStageState(stageId, st);

    auto stage = stageManager_->GetStageDataById(stageId);
    if (!stage) {
        playerDataManager_->Save();
        return;
    }

    // 報酬レポートをリセット
    lastClearReport_ = StageClearReport();

    // ステージ報酬（ゴールド）
    int rewardGold = stage->rewardGold;
    playerDataManager_->AddGold(rewardGold);
    LOG_INFO("Stage {} cleared: base reward {} gold", stageId, rewardGold);

    // チケット報酬（毎回クリアで付与）
    int rewardTickets = stage->rewardTickets;
    if (rewardTickets > 0) {
        playerDataManager_->AddTickets(rewardTickets);
        lastClearReport_.ticketsRewarded = rewardTickets;
        LOG_INFO("Stage {} cleared: reward {} tickets", stageId, rewardTickets);
    }

    // 初回クリア報酬（キャラクター） - 既に所持しているキャラクターは除外
    if (isFirstClear && !stage->rewardMonsters.empty()) {
        for (const auto& rewardMonster : stage->rewardMonsters) {
            const auto& charState = playerDataManager_->GetCharacterState(rewardMonster.monsterId);
            if (!charState.unlocked) {
                // 未所持のキャラクターのみ付与
                auto newCharState = charState;
                newCharState.unlocked = true;
                newCharState.level = rewardMonster.level;
                playerDataManager_->SetCharacterState(rewardMonster.monsterId, newCharState);
                // レポートに新規キャラを追加
                lastClearReport_.newCharacters.push_back(rewardMonster.monsterId);
                LOG_INFO("First clear reward: unlocked character {} (level {})", 
                        rewardMonster.monsterId, rewardMonster.level);
            } else {
                LOG_INFO("First clear reward: character {} already owned, skipping", 
                        rewardMonster.monsterId);
            }
        }
    }

    // クエスト条件チェックと報酬付与
    if (battleStats && !stage->bonusConditions.empty()) {
        for (const auto& condition : stage->bonusConditions) {
            // conditionTypeが空の場合は従来のボーナス条件（時間のみ）として処理
            if (condition.conditionType.empty()) {
                // 旧形式の条件（descriptionから推測）はスキップ
                continue;
            }

            bool conditionMet = false;
            int actualValue = 0;

            // 条件チェック
            if (condition.conditionType == "tower_hp_percent") {
                // 城のHPがX%以上
                if (battleStats->playerTowerMaxHp > 0) {
                    actualValue = (battleStats->playerTowerHp * 100) / battleStats->playerTowerMaxHp;
                }
                if (condition.conditionOperator == "gte") {
                    conditionMet = (actualValue >= condition.conditionValue);
                } else if (condition.conditionOperator == "lte") {
                    conditionMet = (actualValue <= condition.conditionValue);
                } else if (condition.conditionOperator == "eq") {
                    conditionMet = (actualValue == condition.conditionValue);
                }
            } else if (condition.conditionType == "unit_count") {
                // 召喚したユニット数がX以下
                actualValue = battleStats->spawnedUnitCount;
                if (condition.conditionOperator == "lte") {
                    conditionMet = (actualValue <= condition.conditionValue);
                } else if (condition.conditionOperator == "gte") {
                    conditionMet = (actualValue >= condition.conditionValue);
                } else if (condition.conditionOperator == "eq") {
                    conditionMet = (actualValue == condition.conditionValue);
                }
            } else if (condition.conditionType == "gold_spent") {
                // 使用したゴールドがX以下
                actualValue = battleStats->totalGoldSpent;
                if (condition.conditionOperator == "lte") {
                    conditionMet = (actualValue <= condition.conditionValue);
                } else if (condition.conditionOperator == "gte") {
                    conditionMet = (actualValue >= condition.conditionValue);
                } else if (condition.conditionOperator == "eq") {
                    conditionMet = (actualValue == condition.conditionValue);
                }
            } else if (condition.conditionType == "clear_time") {
                // クリア時間がX秒以内
                actualValue = static_cast<int>(battleStats->clearTime);
                if (condition.conditionOperator == "lte") {
                    conditionMet = (actualValue <= condition.conditionValue);
                } else if (condition.conditionOperator == "gte") {
                    conditionMet = (actualValue >= condition.conditionValue);
                }
            }

            // 条件達成時は報酬を付与
            if (conditionMet) {
                if (condition.rewardType == "gold") {
                    playerDataManager_->AddGold(condition.rewardValue);
                    LOG_INFO("Quest completed: {} (actual: {}) -> {} gold", 
                            condition.description, actualValue, condition.rewardValue);
                } else if (condition.rewardType == "item") {
                    // アイテム報酬の処理（将来実装）
                    LOG_INFO("Quest completed: {} (actual: {}) -> item reward (not implemented)", 
                            condition.description, actualValue);
                }
            } else {
                LOG_DEBUG("Quest not met: {} (required: {}, actual: {})", 
                         condition.description, condition.conditionValue, actualValue);
            }
        }
    }

    // ステージ解放処理
    for (const auto& unlockId : stage->unlockOnClear) {
        if (!stageManager_->HasStage(unlockId)) {
            continue;
        }
        auto unlockState = playerDataManager_->GetStageState(unlockId);
        unlockState.isLocked = false;
        playerDataManager_->SetStageState(unlockId, unlockState);
    }
    
    playerDataManager_->Save();
}

std::string GameplayDataAPI::GetPreferredNextStageId(
    const std::string& stageId) const {
    if (!playerDataManager_ || !stageManager_) {
        return "";
    }

    auto stage = stageManager_->GetStageDataById(stageId);
    if (!stage) {
        return "";
    }

    for (const auto& unlockId : stage->unlockOnClear) {
        if (!stageManager_->HasStage(unlockId)) {
            continue;
        }
        const auto st = playerDataManager_->GetStageState(unlockId);
        if (!st.isLocked) {
            return unlockId;
        }
    }

    // 後方互換: unlockOnClearが無い場合は数値IDの次を返す
    if (!stage->unlockOnClear.empty()) {
        return "";
    }
    char* endPtr = nullptr;
    long cur = std::strtol(stageId.c_str(), &endPtr, 10);
    if (!endPtr || *endPtr != '\0') {
        return "";
    }
    const std::string nextId = std::to_string(static_cast<int>(cur) + 1);
    if (!stageManager_->HasStage(nextId)) {
        return "";
    }
    const auto nextState = playerDataManager_->GetStageState(nextId);
    if (nextState.isLocked) {
        return "";
    }
    return nextId;
}

const StageClearReport& GameplayDataAPI::GetLastStageClearReport() const {
    return lastClearReport_;
}

} // namespace core
} // namespace game
