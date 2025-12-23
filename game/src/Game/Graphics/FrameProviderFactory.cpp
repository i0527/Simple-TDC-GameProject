#include "Game/Graphics/FrameProviderFactory.h"
#include "Game/Graphics/SeparatedSpriteProvider.h"
#include "Game/Graphics/AsepriteJsonAtlasProvider.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace Game::Graphics {

std::unique_ptr<Shared::Data::Graphics::IFrameProvider>
FrameProviderFactory::Create(const Shared::Data::EntityDef& def) {
    // 開発モードの場合
    if (def.display.use_dev_mode && !def.display.dev_animation_config_path.empty()) {
        if (fs::exists(def.display.dev_animation_config_path)) {
            try {
                return std::make_unique<SeparatedSpriteProvider>(def.display.dev_animation_config_path);
            } catch (const std::exception& e) {
                std::cerr << "[FrameProviderFactory] Failed to create SeparatedSpriteProvider: " << e.what() << std::endl;
                // フォールバックとして本番モードを試す
            }
        } else {
            std::cerr << "[FrameProviderFactory] Dev animation config not found: " << def.display.dev_animation_config_path << std::endl;
            // フォールバックとして本番モードを試す
        }
    }
    
    // 本番モード：AsepriteJsonAtlasProvider
    if (!def.display.atlas_texture.empty()) {
        std::string pngPath = def.display.atlas_texture;
        if (!fs::exists(pngPath)) {
            std::cerr << "[FrameProviderFactory] Atlas texture not found: " << pngPath << std::endl;
            return nullptr;
        }

        Texture2D tex = LoadTexture(pngPath.c_str());
        if (tex.id == 0) {
            std::cerr << "[FrameProviderFactory] Failed to load texture: " << pngPath << std::endl;
            return nullptr;
        }

        // sprite_actions から最初のJSONをロード
        nlohmann::json atlasJson;
        bool loaded = false;
        for (const auto& [action, jsonPath] : def.display.sprite_actions) {
            fs::path fullJsonPath = fs::path(pngPath).parent_path() / jsonPath;
            if (fs::exists(fullJsonPath)) {
                std::ifstream file(fullJsonPath.string());
                if (file) {
                    try {
                        file >> atlasJson;
                        loaded = true;
                        break;
                    } catch (const nlohmann::json::parse_error& e) {
                        std::cerr << "[FrameProviderFactory] JSON parse error in " << fullJsonPath << ": " << e.what() << std::endl;
                    }
                }
            }
        }
        
        if (!loaded) {
            std::cerr << "[FrameProviderFactory] Failed to load atlas JSON for: " << def.id << std::endl;
            UnloadTexture(tex);
            return nullptr;
        }

        try {
            return std::make_unique<AsepriteJsonAtlasProvider>(tex, atlasJson);
        } catch (const std::exception& e) {
            std::cerr << "[FrameProviderFactory] Failed to create AsepriteJsonAtlasProvider: " << e.what() << std::endl;
            UnloadTexture(tex);
            return nullptr;
        }
    }
    
    std::cerr << "[FrameProviderFactory] No valid provider configuration for entity: " << def.id << std::endl;
    return nullptr;
}

} // namespace Game::Graphics

