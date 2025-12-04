#pragma once

#include "Core/Systems/ISystem.h"
#include "Core/Components/CoreComponents.h"

namespace Systems {
    // デバッグ用の描画システム（プリミティブを描画）
    class RenderSystem : public Core::ISystem {
    public:
        void Render(entt::registry& registry) override;
    };
}

