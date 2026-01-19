#include "../GameplayDataAPI.hpp"

namespace game {
namespace core {

const entities::TowerAttachment* GameplayDataAPI::GetTowerAttachment(const std::string& id) const {
    if (!towerAttachmentManager_) {
        return nullptr;
    }
    return towerAttachmentManager_->GetAttachment(id);
}

std::vector<const entities::TowerAttachment*> GameplayDataAPI::GetAllTowerAttachments() const {
    if (!towerAttachmentManager_) {
        return {};
    }
    return towerAttachmentManager_->GetAllAttachments();
}

const entities::TowerAttachmentManager* GameplayDataAPI::GetTowerAttachmentManager() const {
    return towerAttachmentManager_.get();
}

const std::unordered_map<std::string, entities::TowerAttachment>&
GameplayDataAPI::GetAllTowerAttachmentMasters() const {
    static const std::unordered_map<std::string, entities::TowerAttachment> kEmpty;
    if (!towerAttachmentManager_) {
        return kEmpty;
    }
    return towerAttachmentManager_->GetAttachmentMasters();
}

} // namespace core
} // namespace game
