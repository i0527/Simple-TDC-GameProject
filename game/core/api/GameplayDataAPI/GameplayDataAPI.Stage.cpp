#include "../GameplayDataAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cstdlib>

// プロジェクト内
#include "../../ecs/entities/StageLoader.hpp"
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

void GameplayDataAPI::MarkStageCleared(const std::string& stageId, int starsEarned) {
    if (!playerDataManager_ || !stageManager_) {
        return;
    }
    auto st = playerDataManager_->GetStageState(stageId);
    st.isCleared = true;
    st.isLocked = false;
    st.starsEarned = std::max(st.starsEarned, std::max(0, starsEarned));
    playerDataManager_->SetStageState(stageId, st);

    auto stage = stageManager_->GetStageDataById(stageId);
    if (stage) {
        for (const auto& unlockId : stage->unlockOnClear) {
            if (!stageManager_->HasStage(unlockId)) {
                continue;
            }
            auto unlockState = playerDataManager_->GetStageState(unlockId);
            unlockState.isLocked = false;
            playerDataManager_->SetStageState(unlockId, unlockState);
        }
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

} // namespace core
} // namespace game
