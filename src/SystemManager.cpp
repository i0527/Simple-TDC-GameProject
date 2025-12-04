#include "Core/Systems/SystemManager.h"

namespace Core {

void SystemManager::AddSystem(std::unique_ptr<ISystem> system) {
    systems_.push_back(std::move(system));
}

void SystemManager::ProcessInput(entt::registry& registry) {
    for (auto& system : systems_) {
        system->ProcessInput(registry);
    }
}

void SystemManager::Update(entt::registry& registry, float deltaTime) {
    for (auto& system : systems_) {
        system->Update(registry, deltaTime);
    }
}

void SystemManager::Render(entt::registry& registry) {
    for (auto& system : systems_) {
        system->Render(registry);
    }
}

} // namespace Core
