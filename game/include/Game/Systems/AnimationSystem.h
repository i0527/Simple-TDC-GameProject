#pragma once

#include <entt/entt.hpp>

#include "Game/Components/NewCoreComponents.h"

namespace Game::Systems {

/// @brief 新しいアニメーションシステム（クリップベース）
class AnimationSystem {
public:
    /// @brief アニメーション更新処理
    /// @param registry ECSレジストリ
    /// @param deltaTime デルタ時間（秒）
    void Update(entt::registry& registry, float deltaTime);

private:
};

} // namespace Game::Systems
