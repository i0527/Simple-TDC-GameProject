#include <iostream>
#include <cassert>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

#include "Data/Definitions/EntityDef.h"
#include "Data/DefinitionRegistry.h"
#include "Data/Loaders/EntityLoader.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

int main() {
    std::cout << "=== HatSlime Sprite Rendering Setup Test ===" << std::endl;
    std::cout << "" << std::endl;

    try {
        // 1. エンティティ定義を読み込む
        std::cout << "[1] Loading HatSlime entity definition..." << std::endl;
        std::string hatslime_entity_path = "assets/definitions/entities/characters/HatSlime/entity.json";
        
        if (!fs::exists(hatslime_entity_path)) {
            std::cerr << "ERROR: Entity file not found: " << hatslime_entity_path << std::endl;
            return 1;
        }

        // EntityDefinitionを手動でパース
        std::ifstream entity_file(hatslime_entity_path);
        json entity_json = json::parse(entity_file);
        entity_file.close();

        std::string entity_id = entity_json.value("id", "");
        std::string entity_name = entity_json.value("name", "");

        if (entity_id.empty()) {
            std::cerr << "ERROR: Entity ID is empty" << std::endl;
            return 1;
        }

        std::cout << "  ✓ Entity: " << entity_name << " (" << entity_id << ")" << std::endl;

        // 2. Display設定を確認
        std::cout << "[2] Checking display configuration..." << std::endl;
        if (!entity_json.contains("display")) {
            std::cerr << "ERROR: Display configuration not found" << std::endl;
            return 1;
        }

        auto display = entity_json["display"];
        std::cout << "  ✓ Display configuration found" << std::endl;

        // 3. 開発モードを確認
        std::cout << "[3] Checking dev mode setup..." << std::endl;
        bool use_dev_mode = display.value("use_dev_mode", false);
        std::string dev_animation_config_path = display.value("dev_animation_config_path", "");

        if (!use_dev_mode) {
            std::cerr << "ERROR: Dev mode is not enabled" << std::endl;
            return 1;
        }
        std::cout << "  ✓ Dev mode is enabled" << std::endl;

        if (dev_animation_config_path.empty()) {
            std::cerr << "ERROR: Dev animation config path not set" << std::endl;
            return 1;
        }
        std::cout << "  ✓ Dev animation config path: " << dev_animation_config_path << std::endl;

        // 4. アニメーション設定ファイルを確認
        std::cout << "[4] Verifying dev animation configuration..." << std::endl;
        if (!fs::exists(dev_animation_config_path)) {
            std::cerr << "ERROR: Dev animation config file not found: " << dev_animation_config_path << std::endl;
            return 1;
        }

        std::ifstream anim_file(dev_animation_config_path);
        json anim_json = json::parse(anim_file);
        anim_file.close();

        std::cout << "  ✓ Dev animation config file loaded" << std::endl;

        // 5. 各状態クリップを確認
        std::cout << "[5] Checking animation clips..." << std::endl;
        if (!anim_json.contains("clips")) {
            std::cerr << "ERROR: Clips not found in animation config" << std::endl;
            return 1;
        }

        auto clips = anim_json["clips"];
        std::vector<std::string> expectedClips = {"idle", "walk", "attack", "death"};
        
        int clipsFound = 0;
        for (const auto& clipName : expectedClips) {
            if (clips.contains(clipName)) {
                auto clip_config_path = clips[clipName].value("config_path", "");
                std::string full_path = "assets/characters/sub/HatSlime/dev/" + clip_config_path;
                
                if (fs::exists(full_path)) {
                    std::cout << "  ✓ Clip '" << clipName << "' - config: " << clip_config_path << std::endl;
                    clipsFound++;
                } else {
                    std::cerr << "  ✗ Clip config not found: " << full_path << std::endl;
                }
            } else {
                std::cerr << "  ✗ Clip '" << clipName << "' not defined" << std::endl;
            }
        }

        if (clipsFound < 4) {
            std::cerr << "ERROR: Not all clips were found or configured properly" << std::endl;
            return 1;
        }

        // 6. スプライトシートを確認
        std::cout << "[6] Verifying sprite sheets..." << std::endl;
        std::vector<std::string> spriteSheets = {"idle.png", "walk.png", "attack.png", "die.png"};
        std::string sprite_base_path = "assets/characters/sub/HatSlime/";

        int spritesFound = 0;
        for (const auto& sprite : spriteSheets) {
            std::string full_sprite_path = sprite_base_path + sprite;
            if (fs::exists(full_sprite_path)) {
                std::cout << "  ✓ " << sprite << std::endl;
                spritesFound++;
            } else {
                std::cerr << "  ✗ Sprite not found: " << full_sprite_path << std::endl;
            }
        }

        if (spritesFound < 4) {
            std::cerr << "ERROR: Not all sprite sheets were found" << std::endl;
            return 1;
        }

        // 7. 開発用分割スプライトディレクトリを確認
        std::cout << "[7] Verifying dev sprite directories..." << std::endl;
        std::vector<std::string> devDirs = {"idle", "walk", "attack", "death"};
        std::string dev_sprite_base = "assets/characters/sub/HatSlime/dev/";

        int devDirsFound = 0;
        for (const auto& dir : devDirs) {
            std::string full_dir_path = dev_sprite_base + dir;
            if (fs::exists(full_dir_path) && fs::is_directory(full_dir_path)) {
                std::cout << "  ✓ Dev directory: " << dir << "/" << std::endl;
                devDirsFound++;
            } else {
                std::cerr << "  ✗ Dev directory not found: " << full_dir_path << std::endl;
            }
        }

        if (devDirsFound < 4) {
            std::cerr << "ERROR: Not all dev directories were found" << std::endl;
            return 1;
        }

        std::cout << "" << std::endl;
        std::cout << "=== HatSlime Rendering Setup Complete ===" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "✓ All checks passed!" << std::endl;
        std::cout << "✓ Ready for rendering tests" << std::endl;
        std::cout << "" << std::endl;

        return 0;

    } catch (const json::exception& e) {
        std::cerr << "JSON Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
