#include "Shared/Simulation/Factories/CharacterFactory.h"

#include "Core/GameContext.h"
#include "Game/Graphics/AsepriteJsonAtlasProvider.h"
#include "Game/Graphics/GridSheetProvider.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

using Game::Components::Animation;
using Game::Components::AttackCooldown;
using Game::Components::EntityDefId;
using Game::Components::Sprite;
using Game::Components::Stats;
using Game::Components::Team;
using Game::Components::Velocity;
using Game::Graphics::GridSheetProvider;
using Game::Graphics::AsepriteJsonAtlasProvider;

namespace {

// 複数のAsepriteアトラスをアクション名で束ねる簡易プロバイダ
class MultiAsepriteProvider : public Shared::Data::Graphics::IFrameProvider {
public:
    struct ClipEntry {
        std::unique_ptr<AsepriteJsonAtlasProvider> provider;
        std::string providerClipName; // 実際のアトラス内クリップ名
        std::optional<bool> loopOverride;
    };

    void AddClip(const std::string& exposedName, ClipEntry entry) {
        clips_[exposedName] = std::move(entry);
    }

    size_t ClipCount() const { return clips_.size(); }

    bool HasClip(const std::string& clipName) const override {
        return clips_.find(clipName) != clips_.end();
    }

    int GetFrameCount(const std::string& clipName) const override {
        auto it = clips_.find(clipName);
        if (it == clips_.end()) return 0;
        return it->second.provider->GetFrameCount(it->second.providerClipName);
    }

    Shared::Data::Graphics::FrameRef GetFrame(const std::string& clipName, int frameIndex) const override {
        auto it = clips_.find(clipName);
        if (it == clips_.end()) return {};
        return it->second.provider->GetFrame(it->second.providerClipName, frameIndex);
    }

    float GetClipFps(const std::string& clipName) const override {
        auto it = clips_.find(clipName);
        if (it == clips_.end()) return 0.0f;
        return it->second.provider->GetClipFps(it->second.providerClipName);
    }

    bool IsLooping(const std::string& clipName) const override {
        auto it = clips_.find(clipName);
        if (it == clips_.end()) return false;
        if (it->second.loopOverride.has_value()) return *it->second.loopOverride;
        return it->second.provider->IsLooping(it->second.providerClipName);
    }

private:
    std::unordered_map<std::string, ClipEntry> clips_;
};

} // namespace

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

    registry.emplace<GameComponents::Transform>(entity, position.x, position.y, 1.0f, 1.0f, 0.0f, entityDef->display.mirror_h, entityDef->display.mirror_v);
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
        // Providerの所有権は別管理想定（TODO）
        // アクションJSONとミラー既定をAnimationへ反映
        auto &anim = registry.get<Animation>(entity);
        anim.useAtlas = true;
        anim.currentAction = "idle";
        if (!entityDef->display.animations.empty()) {
            for (const auto &kv : entityDef->display.animations) {
                const std::string &action = kv.first;
                const auto &clip = kv.second;
                anim.actionToJson[action] = clip.json;
                anim.mirrorHByAction[action] = clip.mirror_h;
                anim.mirrorVByAction[action] = clip.mirror_v;
            }
            if (entityDef->display.animations.find("idle") == entityDef->display.animations.end()) {
                anim.currentAction = entityDef->display.animations.begin()->first;
            }
        } else {
            anim.actionToJson = entityDef->display.sprite_actions;
            for (const auto &kv : entityDef->display.sprite_actions) {
                const std::string &action = kv.first;
                const std::string &jsonPath = kv.second;
                try {
                    nlohmann::json j = LoadJsonFile(jsonPath);
                    if (j.contains("meta") && j["meta"].is_object()) {
                        const auto &m = j["meta"];
                        if (m.contains("mirror") && m["mirror"].is_object()) {
                            const auto &mir = m["mirror"];
                            bool mh = false, mv = false;
                            try { mh = mir.value("horizontal", false); } catch (...) {}
                            try { mv = mir.value("vertical", false); } catch (...) {}
                            anim.mirrorHByAction[action] = mh;
                            anim.mirrorVByAction[action] = mv;
                        }
                    }
                } catch (const std::exception &e) {
                    std::cerr << "[CharacterFactory] JSON meta read error for action '" << action
                              << "': " << e.what() << std::endl;
                }
            }
        }
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
        case ProviderType::AsepriteMulti: {
            auto provider = CreateAsepriteMultiProvider(entityDef);
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

    if (!entityDef.display.animations.empty()) {
        return ProviderType::AsepriteMulti;
    }

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

std::unique_ptr<Shared::Data::Graphics::IFrameProvider> CharacterFactory::CreateAsepriteMultiProvider(
    const Shared::Data::EntityDef& entityDef) {

    auto multiProvider = std::make_unique<MultiAsepriteProvider>();

    for (const auto& kv : entityDef.display.animations) {
        const std::string& action = kv.first;
        const auto& clip = kv.second;

        if (clip.atlas.empty() || clip.json.empty()) {
            std::cerr << "[CharacterFactory] animation entry missing atlas/json for action: " << action << std::endl;
            continue;
        }

        Texture2D texture = LoadTextureCached(clip.atlas);
        if (texture.id == 0) {
            std::cerr << "[CharacterFactory] Failed to load texture for action: " << action << " path=" << clip.atlas << std::endl;
            continue;
        }

        try {
            nlohmann::json atlasJson = LoadJsonFile(clip.json);
            auto provider = std::make_unique<AsepriteJsonAtlasProvider>(texture, atlasJson);

            std::string providerClipName = action;
            if (!provider->HasClip(providerClipName)) {
                // fallback: first clip defined in this atlas
                if (!atlasJson.contains("meta") || !atlasJson["meta"].contains("frameTags") || atlasJson["meta"]["frameTags"].empty()) {
                    std::cerr << "[CharacterFactory] No clip tags found for action: " << action << std::endl;
                    continue;
                }
                try {
                    providerClipName = atlasJson["meta"]["frameTags"].at(0).value("name", providerClipName);
                } catch (...) {
                    std::cerr << "[CharacterFactory] Failed to pick fallback clip for action: " << action << std::endl;
                    continue;
                }
                if (!provider->HasClip(providerClipName)) {
                    std::cerr << "[CharacterFactory] Provider still missing clip '" << providerClipName
                              << "' for action: " << action << std::endl;
                    continue;
                }
            }

            MultiAsepriteProvider::ClipEntry entry;
            entry.provider = std::move(provider);
            entry.providerClipName = providerClipName;
            entry.loopOverride = clip.loop;

            multiProvider->AddClip(action, std::move(entry));

        } catch (const std::exception& e) {
            std::cerr << "[CharacterFactory] Failed to create Aseprite provider for action '" << action
                      << "': " << e.what() << std::endl;
        }
    }

    if (multiProvider->ClipCount() == 0) {
        return nullptr;
    }

    return multiProvider;
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

