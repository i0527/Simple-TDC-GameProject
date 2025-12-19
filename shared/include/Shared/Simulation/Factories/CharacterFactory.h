#pragma once

#include "Data/DefinitionRegistry.h"
#include "Game/Components/NewCoreComponents.h"
#include "Game/Graphics/AsepriteJsonAtlasProvider.h"
#include "Game/Graphics/GridSheetProvider.h"
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Shared::Core {
class GameContext;
}

namespace Shared::Data {
struct EntityDef;
namespace Graphics {
class IFrameProvider;
}
}

namespace Shared::Simulation {

/// @brief エンティティ作成工場（ゲーム/エディタ共有）
class CharacterFactory {
public:
    CharacterFactory(Shared::Core::GameContext& context,
                     Shared::Data::DefinitionRegistry& definitions);

    /// @brief 定義IDからエンティティを作成
    entt::entity CreateEntity(entt::registry& registry,
                              const std::string& entityId,
                              const Vector2& position,
                              Game::Components::Team::Type team);

    /// @brief エンティティ定義からProviderを生成
    std::unique_ptr<Shared::Data::Graphics::IFrameProvider> CreateProvider(
        const Shared::Data::EntityDef& entityDef);

private:
    // Provider種類
    enum class ProviderType { GridSheet, AsepriteJson, Unknown };

    ProviderType DetectProviderType(const Shared::Data::EntityDef& entityDef);

    std::unique_ptr<GridSheetProvider> CreateGridSheetProvider(
        const Shared::Data::EntityDef& entityDef);

    std::unique_ptr<AsepriteJsonAtlasProvider> CreateAsepriteProvider(
        const Shared::Data::EntityDef& entityDef);

    Texture2D LoadTextureCached(const std::string& path);
    nlohmann::json LoadJsonFile(const std::string& path);

private:
    // キャッシュ
    std::unordered_map<std::string, std::unique_ptr<GridSheetProvider>> gridProviders_;
    std::unordered_map<std::string, std::unique_ptr<AsepriteJsonAtlasProvider>> asepriteProviders_;
    std::unordered_map<std::string, Texture2D> textureCache_;

    // 依存
    Shared::Core::GameContext& context_;
    Shared::Data::DefinitionRegistry& definitions_;
};

} // namespace Shared::Simulation

