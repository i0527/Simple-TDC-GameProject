#include "Core/EventSystem.h"
#include <iostream>

namespace Shared::Core {

void EventSystem::Subscribe(const std::string& event_type,
                           std::function<void(const nlohmann::json&)> callback) {
    subscribers_[event_type].push_back(std::move(callback));
}

void EventSystem::Emit(const std::string& event_type, const nlohmann::json& data) {
    auto it = subscribers_.find(event_type);
    if (it == subscribers_.end()) {
        return;
    }

    for (const auto& callback : it->second) {
        try {
            callback(data);
        } catch (const std::exception& e) {
            std::cerr << "Exception in event callback for '" << event_type 
                     << "': " << e.what() << std::endl;
        }
    }
}

void EventSystem::ClearAll() {
    subscribers_.clear();
}

} // namespace Shared::Core
