#pragma once

#include "Core/Systems/ISystem.h"
#include "Core/Components/CoreComponents.h"

namespace Systems {
    // PositionとVelocityを持つエンティティを移動させるシステム
    class MovementSystem : public Core::ISystem {
    public:
        void Update(entt::registry& registry, float deltaTime) override;
    };
}

