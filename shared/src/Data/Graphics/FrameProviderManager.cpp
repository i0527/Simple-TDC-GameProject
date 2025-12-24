#include "Data/Graphics/FrameProviderManager.h"

#include "Data/Definitions/EntityDef.h"
#include "Shared/Simulation/Factories/CharacterFactory.h"
#include <iostream>

namespace Shared::Data::Graphics {

FrameProviderManager::FrameProviderManager() {
}

FrameProviderManager::~FrameProviderManager() {
    ClearAll();
}

std::shared_ptr<IFrameProvider> FrameProviderManager::GetProvider(
    const std::string& entityId,
    const Shared::Data::EntityDef& entityDef,
    Shared::Simulation::CharacterFactory& factory) {
    
    // キャッシュを確認
    auto it = providers_.find(entityId);
    if (it != providers_.end()) {
        return it->second;
    }

    // Providerを作成
    auto provider = factory.CreateProvider(entityDef);
    if (!provider) {
        std::cerr << "[FrameProviderManager] Failed to create provider for entity: " 
                  << entityId << std::endl;
        return nullptr;
    }

    // shared_ptrに変換してキャッシュに保存
    std::shared_ptr<IFrameProvider> shared_provider(std::move(provider));
    providers_[entityId] = shared_provider;

    return shared_provider;
}

std::shared_ptr<IFrameProvider> FrameProviderManager::GetCachedProvider(const std::string& entityId) const {
    auto it = providers_.find(entityId);
    if (it != providers_.end()) {
        return it->second;
    }
    return nullptr;
}

void FrameProviderManager::ClearProvider(const std::string& entityId) {
    auto it = providers_.find(entityId);
    if (it != providers_.end()) {
        providers_.erase(it);
    }
}

void FrameProviderManager::ClearAll() {
    providers_.clear();
}

size_t FrameProviderManager::GetProviderCount() const {
    return providers_.size();
}

bool FrameProviderManager::HasProvider(const std::string& entityId) const {
    return providers_.find(entityId) != providers_.end();
}

} // namespace Shared::Data::Graphics

