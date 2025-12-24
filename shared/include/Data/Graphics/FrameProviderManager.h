#pragma once

#include "IFrameProvider.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Shared::Data {
struct EntityDef;
}

namespace Shared::Simulation {
class CharacterFactory;
}

namespace Shared::Data::Graphics {

/// @brief IFrameProviderのリソース管理システム
/// 
/// エンティティIDベースでProviderを管理し、shared_ptrによる自動ライフサイクル管理を提供
/// 同じエンティティIDでProviderを共有することで、メモリ使用量を削減
class FrameProviderManager {
public:
    FrameProviderManager();
    ~FrameProviderManager();

    /// @brief エンティティIDからProviderを取得（存在しない場合は作成）
    /// @param entityId エンティティID
    /// @param entityDef エンティティ定義
    /// @param factory CharacterFactory（Provider作成用）
    /// @return Providerのshared_ptr（作成失敗時はnullptr）
    std::shared_ptr<IFrameProvider> GetProvider(
        const std::string& entityId,
        const Shared::Data::EntityDef& entityDef,
        Shared::Simulation::CharacterFactory& factory);

    /// @brief キャッシュされたProviderを取得（存在しない場合はnullptr）
    /// @param entityId エンティティID
    /// @return Providerのshared_ptr（存在しない場合はnullptr）
    std::shared_ptr<IFrameProvider> GetCachedProvider(const std::string& entityId) const;

    /// @brief 特定のエンティティIDのProviderをクリア
    /// @param entityId エンティティID
    void ClearProvider(const std::string& entityId);

    /// @brief すべてのProviderをクリア
    void ClearAll();

    /// @brief 管理中のProvider数を取得
    /// @return Provider数
    size_t GetProviderCount() const;

    /// @brief 特定のエンティティIDのProviderが存在するか確認
    /// @param entityId エンティティID
    /// @return 存在する場合true
    bool HasProvider(const std::string& entityId) const;

private:
    std::unordered_map<std::string, std::shared_ptr<IFrameProvider>> providers_;
};

} // namespace Shared::Data::Graphics

