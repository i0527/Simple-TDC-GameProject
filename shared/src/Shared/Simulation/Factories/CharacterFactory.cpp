#include "Shared/Simulation/Factories/CharacterFactory.h"

#include "Core/GameContext.h"
#include "Game/Graphics/AsepriteJsonAtlasProvider.h"
#include "Game/Graphics/GridSheetProvider.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using Game::Components::Animation;
using Game::Components::AttackCooldown;
using Game::Components::EntityDefId;
using Game::Components::Sprite;
using Game::Components::Stats;
using Game::Components::Team;
using Game::Components::Velocity;

// raylibのTransformとの衝突を避けるため、名前空間を明示
namespace GameComponents = Game::Components;

namespace Shared::Simulation {

CharacterFactory::CharacterFactory(Shared::Core::GameContext& context,
                                   Shared::Data::DefinitionRegistry& definitions)
    : context_(context), definitions_(definitions) {}

entt::entity CharacterFactory::CreateEntity(entt::registry& registry,
                                            const std::string& entityId,
                                            const Vector2& position,
                                            Team::Type team) {
    const auto* entityDef = definitions_.GetEntity(entityId);
    if (!entityDef) {
        std::cerr << "[CharacterFactory] Entity definition not found: " << entityId << std::endl;
        return entt::null;
    }

    auto entity = registry.create();

    registry.emplace<GameComponents::Transform>(entity, position.x, position.y, 1.0f, 1.0f, 0.0f, false, false);
    registry.emplace<Team>(entity, team);

    registry.emplace<Stats>(entity,
        entityDef->stats.hp,
        entityDef->stats.hp,
        entityDef->stats.attack,
        entityDef->stats.attack_speed,
        entityDef->stats.range,
        entityDef->stats.move_speed,
        entityDef->stats.knockback
    );

    registry.emplace<Velocity>(entity, 0.0f, 0.0f);
    registry.emplace<AttackCooldown>(entity);
    registry.emplace<Game::Components::SkillHolder>(entity);
    registry.emplace<Game::Components::SkillCooldown>(entity);
    registry.emplace<EntityDefId>(entity, entityId);

    auto provider = CreateProvider(*entityDef);
    if (provider) {
        registry.emplace<Animation>(entity);
        registry.emplace<Sprite>(entity, provider.get());
        // TODO: Providerの所有権を管理する仕組みを追加
    }

    return entity;
}

std::unique_ptr<Shared::Data::Graphics::IFrameProvider> CharacterFactory::CreateProvider(
    const Shared::Data::EntityDef& entityDef) {

    ProviderType type = DetectProviderType(entityDef);

    switch (type) {
        case ProviderType::GridSheet: {
            auto provider = CreateGridSheetProvider(entityDef);
            return std::move(provider);
        }
        case ProviderType::AsepriteJson: {
            auto provider = CreateAsepriteProvider(entityDef);
            return std::move(provider);
        }
        default:
            std::cerr << "[CharacterFactory] Unknown provider type for entity: "
                      << entityDef.id << std::endl;
            return nullptr;
    }
}

CharacterFactory::ProviderType CharacterFactory::DetectProviderType(
    const Shared::Data::EntityDef& entityDef) {

    if (!entityDef.display.atlas_texture.empty() &&
        !entityDef.display.sprite_actions.empty()) {
        return ProviderType::AsepriteJson;
    }

    if (!entityDef.display.sprite_sheet.empty()) {
        return ProviderType::GridSheet;
    }

    return ProviderType::Unknown;
}

std::unique_ptr<GridSheetProvider> CharacterFactory::CreateGridSheetProvider(
    const Shared::Data::EntityDef& entityDef) {

    const std::string& texturePath = entityDef.display.sprite_sheet;

    auto it = gridProviders_.find(texturePath);
    if (it != gridProviders_.end()) {
        return std::make_unique<GridSheetProvider>(*it->second);
    }

    Texture2D texture = LoadTextureCached(texturePath);
    if (texture.id == 0) {
        std::cerr << "[CharacterFactory] Failed to load texture: " << texturePath << std::endl;
        return nullptr;
    }

    // キャッシュから参照を取得（LoadTextureCachedで既に保存済み）
    Texture2D& cachedTexture = textureCache_.at(texturePath);

    std::filesystem::path textureDir = std::filesystem::path(texturePath).parent_path();
    std::string clipsPath = (textureDir / "clips.json").string();

    try {
        nlohmann::json clipsJson = LoadJsonFile(clipsPath);

        GridSheetProvider::Config config{256, 256, 16};

        if (clipsJson.contains("config")) {
            const auto& cfg = clipsJson["config"];
            if (cfg.contains("cellWidth")) config.cellWidth = cfg["cellWidth"];
            if (cfg.contains("cellHeight")) config.cellHeight = cfg["cellHeight"];
            if (cfg.contains("framesPerRow")) config.framesPerRow = cfg["framesPerRow"];
        }

        auto provider = std::make_unique<GridSheetProvider>(cachedTexture, config);

        if (clipsJson.contains("clips")) {
            for (const auto& clipJson : clipsJson["clips"]) {
                std::string name = clipJson["name"];
                int startIndex = clipJson["startIndex"];
                int length = clipJson["length"];
                bool loop = clipJson.value("loop", true);
                float fps = clipJson.value("fps", 12.0f);

                provider->RegisterClip(name, startIndex, length, loop, fps);
            }
        }

        gridProviders_[texturePath] = std::make_unique<GridSheetProvider>(*provider);

        return provider;

    } catch (const std::exception& e) {
        std::cerr << "[CharacterFactory] Failed to load clips.json: " << clipsPath
                  << " (" << e.what() << ")" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<AsepriteJsonAtlasProvider> CharacterFactory::CreateAsepriteProvider(
    const Shared::Data::EntityDef& entityDef) {

    const std::string& texturePath = entityDef.display.atlas_texture;

    auto it = asepriteProviders_.find(texturePath);
    if (it != asepriteProviders_.end()) {
        return std::make_unique<AsepriteJsonAtlasProvider>(*it->second);
    }

    Texture2D texture = LoadTextureCached(texturePath);
    if (texture.id == 0) {
        std::cerr << "[CharacterFactory] Failed to load texture: " << texturePath << std::endl;
        return nullptr;
    }

    if (entityDef.display.sprite_actions.empty()) {
        std::cerr << "[CharacterFactory] No sprite actions defined for Aseprite provider" << std::endl;
        return nullptr;
    }

    std::string jsonPath = entityDef.display.sprite_actions.begin()->second;

    try {
        nlohmann::json atlasJson = LoadJsonFile(jsonPath);
        auto provider = std::make_unique<AsepriteJsonAtlasProvider>(texture, atlasJson);

        asepriteProviders_[texturePath] = std::make_unique<AsepriteJsonAtlasProvider>(*provider);

        return provider;

    } catch (const std::exception& e) {
        std::cerr << "[CharacterFactory] Failed to create Aseprite provider: " << e.what() << std::endl;
        return nullptr;
    }
}

Texture2D CharacterFactory::LoadTextureCached(const std::string& path) {
    auto it = textureCache_.find(path);
    if (it != textureCache_.end()) {
        return it->second;
    }

    // ファイルパスの存在確認
    if (!std::filesystem::exists(path)) {
        std::cerr << "[CharacterFactory] Texture file not found: " << path << std::endl;
        return {};
    }

    // 絶対パスに変換（相対パスの場合）
    std::filesystem::path fsPath(path);
    std::string absolutePath = fsPath.is_absolute() ? path : std::filesystem::absolute(fsPath).string();

    Texture2D texture = LoadTexture(absolutePath.c_str());
    if (texture.id == 0) {
        std::cerr << "[CharacterFactory] Failed to load texture: " << absolutePath << std::endl;
        return {};
    }

    textureCache_[path] = texture;
    std::cout << "[CharacterFactory] Loaded texture: " << absolutePath << std::endl;
    return texture;
}

nlohmann::json CharacterFactory::LoadJsonFile(const std::string& path) {
    // ファイルパスの存在確認
    if (!std::filesystem::exists(path)) {
        std::cerr << "[CharacterFactory] JSON file not found: " << path << std::endl;
        throw std::runtime_error("File not found: " + path);
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[CharacterFactory] Failed to open JSON file: " << path << std::endl;
        throw std::runtime_error("Failed to open file: " + path);
    }

    try {
        nlohmann::json json;
        file >> json;
        return json;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[CharacterFactory] JSON parse error in " << path << ": " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "[CharacterFactory] Error loading JSON from " << path << ": " << e.what() << std::endl;
        throw;
    }
}

} // namespace Shared::Simulation

