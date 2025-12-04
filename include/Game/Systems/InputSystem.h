#pragma once

#include "Core/Systems/ISystem.h"
#include "Core/Components/CoreComponents.h"

namespace Systems {
    // キーボード入力をVelocityに反映させるシンプルなシステム
    class InputSystem : public Core::ISystem {
    public:
        void ProcessInput(entt::registry& registry) override;
    };
}

