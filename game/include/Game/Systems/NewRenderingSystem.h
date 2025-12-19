#pragma once

#include <entt/entt.hpp>

#include "Game/Components/NewCoreComponents.h"
#include "Game/Graphics/SpriteRenderer.h"

namespace Game::Systems {

/// @brief 新しい描画システム（統一描画器使用）
class NewRenderingSystem {
public:
    /// @brief エンティティ描画処理
    /// @param registry ECSレジストリ
    void DrawEntities(entt::registry& registry) const;

private:
};

} // namespace Game::Systems
