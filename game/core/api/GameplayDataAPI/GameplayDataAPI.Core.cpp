#include "../GameplayDataAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"

namespace game {
namespace core {

bool GameplayDataAPI::Initialize(const std::string& characterJsonPath,
                                 const std::string& itemPassiveJsonPath,
                                 const std::string& stageJsonPath,
                                 const std::string& playerSavePath,
                                 const std::string& towerAttachmentJsonPath) {
    isInitialized_ = false;
    characterJsonPath_ = characterJsonPath;
    itemPassiveJsonPath_ = itemPassiveJsonPath;
    stageJsonPath_ = stageJsonPath;
    playerSavePath_ = playerSavePath;
    towerAttachmentJsonPath_ = towerAttachmentJsonPath;

    characterManager_ = std::make_unique<entities::CharacterManager>();
    if (!characterManager_->Initialize(characterJsonPath)) {
        LOG_WARN("GameplayDataAPI: CharacterManager initialization failed, using fallback");
    }

    itemPassiveManager_ = std::make_unique<entities::ItemPassiveManager>();
    if (!itemPassiveManager_->Initialize(itemPassiveJsonPath)) {
        LOG_WARN("GameplayDataAPI: ItemPassiveManager initialization failed, using fallback");
    }

    stageManager_ = std::make_unique<entities::StageManager>();
    if (!stageManager_->Initialize(stageJsonPath)) {
        LOG_WARN("GameplayDataAPI: StageManager initialization failed, using fallback");
    } else {
        LOG_INFO("GameplayDataAPI: StageManager initialized with {} stages", stageManager_->GetStageCount());
    }

    towerAttachmentManager_ = std::make_unique<entities::TowerAttachmentManager>();
    if (!towerAttachmentManager_->Initialize(towerAttachmentJsonPath)) {
        LOG_WARN("GameplayDataAPI: TowerAttachmentManager initialization failed, using fallback");
    }

    playerDataManager_ = std::make_unique<PlayerDataManager>();
    if (!playerDataManager_->LoadOrCreate(playerSavePath, *characterManager_,
                                          *itemPassiveManager_, *stageManager_)) {
        LOG_WARN("GameplayDataAPI: PlayerDataManager initialization failed, using defaults");
    }

    isInitialized_ = true;
    return true;
}

void GameplayDataAPI::Shutdown() {
    if (characterManager_) {
        characterManager_->Shutdown();
        characterManager_.reset();
    }
    if (itemPassiveManager_) {
        itemPassiveManager_->Shutdown();
        itemPassiveManager_.reset();
    }
    if (stageManager_) {
        stageManager_->Shutdown();
        stageManager_.reset();
    }
    if (towerAttachmentManager_) {
        towerAttachmentManager_->Shutdown();
        towerAttachmentManager_.reset();
    }
    playerDataManager_.reset();
    isInitialized_ = false;
}

} // namespace core
} // namespace game
