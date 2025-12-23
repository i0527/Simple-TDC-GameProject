#include "Game/Graphics/SeparatedSpriteProvider.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace Game::Graphics {

SeparatedSpriteProvider::SeparatedSpriteProvider(const std::string& devAnimationJsonPath) {
    LoadDevAnimationConfig(devAnimationJsonPath);
}

SeparatedSpriteProvider::~SeparatedSpriteProvider() {
    // テクスチャのアンロード
    for (auto& [clipName, clip] : clips_) {
        for (auto& frame : clip.frames) {
            if (frame.loaded && frame.texture.id != 0) {
                UnloadTexture(frame.texture);
                frame.texture = Texture2D{};
                frame.loaded = false;
            }
        }
    }
}

void SeparatedSpriteProvider::LoadDevAnimationConfig(const std::string& jsonPath) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            std::cerr << "[SeparatedSpriteProvider] Failed to open: " << jsonPath << std::endl;
            return;
        }

        nlohmann::json config;
        file >> config;

        spriteBasePath_ = config.value("sprite_base_path", "");
        defaultFootOffsetY_ = config.value("default_foot_offset_y", 0.0f);

        // 各クリップをロード
        if (config.contains("clips") && config["clips"].is_object()) {
            for (const auto& [clipName, clipConfig] : config["clips"].items()) {
                if (clipConfig.is_object() && clipConfig.contains("config_path") && clipConfig["config_path"].is_string()) {
                    std::string configPath = clipConfig["config_path"];
                    LoadClipConfig(clipName, configPath);
                }
            }
        }

        std::cout << "[SeparatedSpriteProvider] Loaded " << clips_.size() << " clips from: " << jsonPath << std::endl;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[SeparatedSpriteProvider] JSON parse error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[SeparatedSpriteProvider] Error: " << e.what() << std::endl;
    }
}

void SeparatedSpriteProvider::LoadClipConfig(const std::string& clipName, const std::string& configPath) {
    try {
        fs::path fullPath = fs::path(spriteBasePath_) / configPath;
        if (!fs::exists(fullPath)) {
            std::cerr << "[SeparatedSpriteProvider] Clip config not found: " << fullPath << std::endl;
            return;
        }

        std::ifstream file(fullPath);
        if (!file.is_open()) {
            std::cerr << "[SeparatedSpriteProvider] Failed to open clip config: " << fullPath << std::endl;
            return;
        }

        nlohmann::json clipJson;
        file >> clipJson;

        ClipData clip;
        clip.name = clipName;
        clip.defaultFps = clipJson.value("fps", 12.0f);
        clip.loop = clipJson.value("loop", true);
        clip.footOffsetY = clipJson.value("foot_offset_y", defaultFootOffsetY_);

        // フレームをロード
        if (clipJson.contains("frames") && clipJson["frames"].is_array()) {
            fs::path clipDir = fullPath.parent_path();
            for (const auto& frameJson : clipJson["frames"]) {
                FrameData frame;
                frame.durationSec = frameJson.value("duration_ms", 83) / 1000.0f;
                
                std::string fileName = frameJson.value("file", "");
                if (!fileName.empty()) {
                    fs::path framePath = clipDir / fileName;
                    LoadFrameTexture(frame, framePath.string());
                }
                
                clip.frames.push_back(frame);
            }
        }

        if (!clip.frames.empty()) {
            clips_[clipName] = std::move(clip);
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[SeparatedSpriteProvider] Clip JSON parse error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[SeparatedSpriteProvider] Clip load error: " << e.what() << std::endl;
    }
}

bool SeparatedSpriteProvider::LoadFrameTexture(FrameData& frameData, const std::string& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "[SeparatedSpriteProvider] Texture not found: " << filePath << std::endl;
        return false;
    }

    Texture2D tex = LoadTexture(filePath.c_str());
    if (tex.id == 0) {
        std::cerr << "[SeparatedSpriteProvider] Failed to load texture: " << filePath << std::endl;
        return false;
    }

    frameData.texture = tex;
    frameData.loaded = true;
    return true;
}

bool SeparatedSpriteProvider::HasClip(const std::string& clipName) const {
    return clips_.find(clipName) != clips_.end();
}

int SeparatedSpriteProvider::GetFrameCount(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    if (it == clips_.end()) {
        return 0;
    }
    return static_cast<int>(it->second.frames.size());
}

Shared::Data::Graphics::FrameRef SeparatedSpriteProvider::GetFrame(
    const std::string& clipName,
    int frameIndex
) const {
    auto it = clips_.find(clipName);
    if (it == clips_.end() || frameIndex < 0 || frameIndex >= static_cast<int>(it->second.frames.size())) {
        return Shared::Data::Graphics::FrameRef{}; // 無効なFrameRef
    }

    const auto& clip = it->second;
    
    if (frameIndex >= static_cast<int>(clip.frames.size())) {
        return Shared::Data::Graphics::FrameRef{}; // 範囲外
    }
    
    const auto& frame = clip.frames[frameIndex];
    
    if (!frame.loaded) {
        return Shared::Data::Graphics::FrameRef{}; // 未ロード
    }

    return CreateFrameRef(clip, frameIndex);
}

Shared::Data::Graphics::FrameRef SeparatedSpriteProvider::CreateFrameRef(
    const ClipData& clip,
    int frameIndex
) const {
    const auto& frame = clip.frames[frameIndex];
    
    Shared::Data::Graphics::FrameRef ref;
    ref.texture = &frame.texture; // 非constからconstへの変換は自動
    ref.src = Rectangle{0.0f, 0.0f, static_cast<float>(frame.texture.width), static_cast<float>(frame.texture.height)};
    ref.origin = GetFootOrigin(frame.texture, clip.footOffsetY);
    ref.offset = Vector2{0.0f, 0.0f}; // 分割ファイルではoffset不要
    ref.durationSec = frame.durationSec;
    ref.valid = true;
    
    return ref;
}

Vector2 SeparatedSpriteProvider::GetFootOrigin(const Texture2D& texture, float footOffsetY) const {
    // 足元を基準点にする（画像の下中央）
    return Vector2{
        static_cast<float>(texture.width) * 0.5f,
        static_cast<float>(texture.height) - footOffsetY
    };
}

float SeparatedSpriteProvider::GetClipFps(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    if (it == clips_.end()) {
        return 12.0f; // デフォルト
    }
    return it->second.defaultFps;
}

bool SeparatedSpriteProvider::IsLooping(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    if (it == clips_.end()) {
        return true; // デフォルト
    }
    return it->second.loop;
}

} // namespace Game::Graphics

