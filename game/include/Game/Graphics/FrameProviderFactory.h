#pragma once

#include "Data/Graphics/IFrameProvider.h"
#include "Data/Definitions/EntityDef.h"
#include <memory>

namespace Game::Graphics {

/**
 * @brief FrameProviderファクトリー
 * 
 * EntityDefに基づいて適切なProviderを生成
 * 開発モード（use_dev_mode）と本番モード（AsepriteJsonAtlasProvider）を切り替え
 */
class FrameProviderFactory {
public:
    /**
     * @brief EntityDefからProviderを生成
     * @param def エンティティ定義
     * @return Provider（所有権を返す）、失敗時はnullptr
     */
    static std::unique_ptr<Shared::Data::Graphics::IFrameProvider> Create(
        const Shared::Data::EntityDef& def
    );
};

} // namespace Game::Graphics

