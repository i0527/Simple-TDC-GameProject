#include "TowerAttachmentManager.hpp"
#include "TowerAttachmentLoader.hpp"
#include "../../../utils/Log.h"

namespace game {
namespace core {
namespace entities {

TowerAttachmentManager::TowerAttachmentManager() {
}

TowerAttachmentManager::~TowerAttachmentManager() {
    Shutdown();
}

bool TowerAttachmentManager::Initialize(const std::string& json_path) {
    if (!json_path.empty()) {
        if (TowerAttachmentLoader::LoadFromJSON(json_path, attachment_masters_)) {
            return true;
        }
        LOG_WARN("TowerAttachmentManager: JSON load failed, falling back to hardcoded data");
    }

    TowerAttachmentLoader::LoadHardcoded(attachment_masters_);
    return true;
}

const TowerAttachment* TowerAttachmentManager::GetAttachment(const std::string& id) const {
    auto it = attachment_masters_.find(id);
    if (it != attachment_masters_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const TowerAttachment*> TowerAttachmentManager::GetAllAttachments() const {
    std::vector<const TowerAttachment*> result;
    result.reserve(attachment_masters_.size());
    for (const auto& [id, attachment] : attachment_masters_) {
        (void)id;
        result.push_back(&attachment);
    }
    return result;
}

void TowerAttachmentManager::SetMasters(
    const std::unordered_map<std::string, TowerAttachment>& masters) {
    attachment_masters_ = masters;
}

void TowerAttachmentManager::Shutdown() {
    attachment_masters_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
