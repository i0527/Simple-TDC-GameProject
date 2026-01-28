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

// stages.json の rewardMonsters.monsterId（短縮名）を characters.json の id に解決する
std::string ResolveRewardCharacterId(entities::CharacterManager* characterManager,
                                     const std::string& monsterId) {
    if (!characterManager || monsterId.empty()) {
        return monsterId;
    }
    if (characterManager->HasCharacter(monsterId)) {
        return monsterId;
    }
    // 短縮名 -> キャラID のマッピング（パターンに当てはまらないもの）
    static const std::unordered_map<std::string, std::string> kMonsterIdToCharacterId{
        {"dkurage", "char_sub_poisonjellyfish_001"},
        {"kimokimo", "char_sub_kimoisogin_001"},
        {"mush", "char_sub_mushmeramera_001"},
        {"seaserpentboss", "char_sub_seaserpent_001"},
        {"crystalboss", "char_sub_crystalgolem_001"},
        {"anglerfish", "char_sub_lanterfish_001"},
    };
    auto it = kMonsterIdToCharacterId.find(monsterId);
    if (it != kMonsterIdToCharacterId.end() && characterManager->HasCharacter(it->second)) {
        return it->second;
    }
    // char_sub_XXX_001 の XXX が monsterId と一致するものを探す
    const std::string prefix = "char_sub_";
    const std::string suffix = "_001";
    for (const std::string& id : characterManager->GetAllCharacterIds()) {
        if (id.size() > prefix.size() + suffix.size() &&
            id.compare(0, prefix.size(), prefix) == 0 &&
            id.compare(id.size() - suffix.size(), suffix.size(), suffix) == 0) {
            std::string middle = id.substr(prefix.size(), id.size() - prefix.size() - suffix.size());
            if (middle == monsterId) {
                return id;
            }
        }
    }
    return monsterId;
}

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
    
    // 無限ステージの生存時間を記録
    if (battleStats) {
        lastClearReport_.survivalTime = battleStats->clearTime;
    }

    // ステージ報酬（ゴールド）
    int rewardGold = stage->rewardGold;
    playerDataManager_->AddGold(rewardGold);
    lastClearReport_.rewardGold = rewardGold;
    LOG_INFO("Stage {} cleared: base reward {} gold", stageId, rewardGold);

    // チケット報酬（毎回クリアで付与。JSONの rewardTickets で設定）
    int rewardTickets = stage->rewardTickets;
    if (rewardTickets > 0) {
        playerDataManager_->AddTickets(rewardTickets);
        lastClearReport_.ticketsRewarded = rewardTickets;
        LOG_INFO("Stage {} cleared: reward {} tickets", stageId, rewardTickets);
    }

    // キャラクター報酬処理
    // rewardCharacterOnEveryClearがtrueの場合、毎回クリア時にキャラクター報酬を付与
    // そうでない場合、初回クリア時のみ付与
    bool shouldRewardCharacter = stage->rewardCharacterOnEveryClear || isFirstClear;
    if (shouldRewardCharacter && !stage->rewardMonsters.empty() && characterManager_) {
        for (const auto& rewardMonster : stage->rewardMonsters) {
            std::string characterId = ResolveRewardCharacterId(characterManager_.get(), rewardMonster.monsterId);
            if (!characterManager_->HasCharacter(characterId)) {
                LOG_WARN("Character reward skipped: no master for id '{}' (monsterId '{}')",
                        characterId, rewardMonster.monsterId);
                continue;
            }
            const auto& charState = playerDataManager_->GetCharacterState(characterId);
            auto newCharState = charState;

            if (!charState.unlocked) {
                // 未所持の場合は新規解放（編成・ユニットで利用可能に）
                newCharState.unlocked = true;
                newCharState.level = rewardMonster.level;
                lastClearReport_.newCharacters.push_back(characterId);
                LOG_INFO("Character reward: unlocked character {} (level {})",
                        characterId, rewardMonster.level);
            } else if (stage->rewardCharacterOnEveryClear) {
                // 既に所持・毎回クリア時: レベルを報酬値との最大に
                newCharState.level = std::max(newCharState.level, rewardMonster.level);
                LOG_INFO("Character reward: updated character {} level to {}",
                        characterId, newCharState.level);
            } else {
                // 初回のみ・重複: Lv+10（上限50）
                newCharState.level = std::min(50, newCharState.level + 10);
                LOG_INFO("Character reward: duplicate first-clear, {} Lv+10 -> {}",
                        characterId, newCharState.level);
            }
            playerDataManager_->SetCharacterState(characterId, newCharState);
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
