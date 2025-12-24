#include "Game/Systems/AnimationSystem.h"

#include "Data/Graphics/IFrameProvider.h"
#include "Game/Components/NewCoreComponents.h"

namespace Game::Systems {

void AnimationSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Game::Components::Animation, Game::Components::Sprite>();

    for (auto entity : view) {
        auto& anim = view.get<Game::Components::Animation>(entity);
        auto& sprite = view.get<Game::Components::Sprite>(entity);

        if (!anim.isPlaying) continue;

        // Providerを取得（新しいID参照方式を優先、後方互換性のため古い方式もサポート）
        Shared::Data::Graphics::IFrameProvider* provider = nullptr;
        
        if (!sprite.provider_id.empty() && frame_provider_manager_) {
            // 新しい方式: FrameProviderManagerからキャッシュされたProviderを取得
            auto shared_provider = frame_provider_manager_->GetCachedProvider(sprite.provider_id);
            if (shared_provider) {
                provider = shared_provider.get();
            }
        } else if (sprite.provider) {
            // 後方互換性: 既存のproviderポインタを使用
            provider = sprite.provider;
        }

        // providerがnullptrの場合は、PNGベースの描画として扱う
        // PNGファイルは単一画像なので、フレームアニメーションは不要
        // frameIndexを0に固定してスキップ
        if (!provider) {
            anim.frameIndex = 0;
            continue;
        }
        
        anim.elapsedTime += deltaTime;

        // providerを使用してフレームアニメーションを更新
        float fps = provider->GetClipFps(anim.currentClip);
        if (fps <= 0.0f) fps = 12.0f;
        float frameDuration = 1.0f / fps;
        int nextFrameIndex = (int)(anim.elapsedTime / frameDuration);
        int frameCount = provider->GetFrameCount(anim.currentClip);
        if (frameCount <= 0) {
            anim.frameIndex = 0;
            continue;
        }

        if (nextFrameIndex >= frameCount) {
            if (provider->IsLooping(anim.currentClip)) {
                anim.elapsedTime = 0.0f;
                anim.frameIndex = 0;
            } else {
                anim.isPlaying = false;
                anim.frameIndex = frameCount - 1;
            }
        } else {
            anim.frameIndex = nextFrameIndex;
        }
    }
}

} // namespace Game::Systems
