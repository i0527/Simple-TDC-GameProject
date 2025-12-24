#pragma once

#include <entt/entt.hpp>

#include "Data/Graphics/FrameProviderManager.h"
#include "Game/Components/NewCoreComponents.h"

namespace Game::Systems {

/// @brief 新しいアニメーションシステム（クリップベース）
class AnimationSystem {
public:
    AnimationSystem() : frame_provider_manager_(nullptr) {}
    
    /// @brief FrameProviderManagerを設定
    /// @param frameProviderManager FrameProviderManagerへの参照（nullptr可、後方互換性のため）
    void SetFrameProviderManager(Shared::Data::Graphics::FrameProviderManager* frameProviderManager) {
        frame_provider_manager_ = frameProviderManager;
    }
    
    /// @brief アニメーション更新処理
    /// @param registry ECSレジストリ
    /// @param deltaTime デルタ時間（秒）
    void Update(entt::registry& registry, float deltaTime);

private:
    Shared::Data::Graphics::FrameProviderManager* frame_provider_manager_;
};

} // namespace Game::Systems
