#include "Shared/Simulation/SimulationContext.h"

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Game/Components/NewCoreComponents.h"
#include "Shared/Simulation/Factories/CharacterFactory.h"

namespace Shared::Simulation {

SimulationContext::SimulationContext(Shared::Core::GameContext* context,
                                     Shared::Data::DefinitionRegistry* definitions)
    : context_(context), definitions_(definitions) {
    EnsureFactory();
}

SimulationContext::~SimulationContext() {
    Clear();
}

void SimulationContext::EnsureFactory() {
    if (!character_factory_ && context_ && definitions_) {
        character_factory_ = std::make_unique<CharacterFactory>(*context_, *definitions_);
    }
}

entt::entity SimulationContext::SpawnEntity(const std::string& definitionId,
                                            const Vector2& position,
                                            Game::Components::Team::Type team) {
    EnsureFactory();
    if (!character_factory_) {
        return entt::null;
    }
    return character_factory_->CreateEntity(registry_, definitionId, position, team);
}

void SimulationContext::DestroyEntity(entt::entity entity) {
    if (registry_.valid(entity)) {
        registry_.destroy(entity);
    }
}

std::vector<entt::entity> SimulationContext::FindEntitiesByDefinition(const std::string& definitionId) const {
    std::vector<entt::entity> result;
    auto view = registry_.view<Game::Components::EntityDefId>();
    for (auto entity : view) {
        const auto& def = view.get<Game::Components::EntityDefId>(entity);
        if (def.id == definitionId) {
            result.push_back(entity);
        }
    }
    return result;
}

void SimulationContext::Update(float /*deltaTime*/) {
    // NOTE: Week2以降でSystemsを統合
}

size_t SimulationContext::GetEntityCount() const {
    return registry_.storage<entt::entity>()->size();
}

void SimulationContext::SetEntityPosition(entt::entity entity, const Vector2& position) {
    if (!registry_.valid(entity)) return;
    if (!registry_.all_of<Game::Components::Transform>(entity)) return;
    auto& tf = registry_.get<Game::Components::Transform>(entity);
    tf.x = position.x;
    tf.y = position.y;
}

Vector2 SimulationContext::GetEntityPosition(entt::entity entity) const {
    if (!registry_.valid(entity)) return {0.0f, 0.0f};
    if (!registry_.all_of<Game::Components::Transform>(entity)) return {0.0f, 0.0f};
    const auto& tf = registry_.get<Game::Components::Transform>(entity);
    return {tf.x, tf.y};
}

std::vector<SimulationContext::RenderCommand> SimulationContext::GetRenderCommands(const Rectangle& /*camera_bounds*/) const {
    // TODO: Week2/3で実装（現状は空）
    return {};
}

void SimulationContext::Clear() {
    registry_.clear();
}

bool SimulationContext::ReloadEntity(entt::entity entity, ReloadPolicy policy) {
    if (!registry_.valid(entity) || !definitions_) {
        return false;
    }

    if (!registry_.all_of<Game::Components::EntityDefId>(entity)) {
        return false;
    }

    const auto& idComp = registry_.get<Game::Components::EntityDefId>(entity);
    const auto* def = definitions_->GetEntity(idComp.id);
    if (!def) {
        return false;
    }

    Vector2 pos = GetEntityPosition(entity);
    int prev_hp = 0;
    int prev_max = 0;
    if (registry_.all_of<Game::Components::Stats>(entity)) {
        const auto& s = registry_.get<Game::Components::Stats>(entity);
        prev_hp = s.current_hp;
        prev_max = s.max_hp;
    }

    // Stats更新
    registry_.emplace_or_replace<Game::Components::Stats>(entity,
        def->stats.hp,
        def->stats.hp,
        def->stats.attack,
        def->stats.attack_speed,
        def->stats.range,
        def->stats.move_speed,
        def->stats.knockback);

    auto& stats = registry_.get<Game::Components::Stats>(entity);
    switch (policy) {
    case ReloadPolicy::PreserveState:
        stats.current_hp = std::min(prev_hp, stats.max_hp);
        break;
    case ReloadPolicy::ResetToDefault:
        stats.current_hp = stats.max_hp;
        break;
    case ReloadPolicy::ScaleProportionally:
        if (prev_max > 0) {
            float ratio = static_cast<float>(prev_hp) / static_cast<float>(prev_max);
            stats.current_hp = static_cast<int>(stats.max_hp * ratio);
        }
        break;
    }

    SetEntityPosition(entity, pos);
    return true;
}

size_t SimulationContext::ReloadAllInstances(const std::string& definitionId, ReloadPolicy policy) {
    auto entities = FindEntitiesByDefinition(definitionId);
    size_t count = 0;
    for (auto e : entities) {
        if (ReloadEntity(e, policy)) {
            ++count;
        }
    }
    return count;
}

} // namespace Shared::Simulation

