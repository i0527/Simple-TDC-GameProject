#include "Data/Loaders/SpriteSheetLoader.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Shared::Data {

namespace {
constexpr const char* FRAME_KEY = "frames";
constexpr const char* META_KEY = "meta";
constexpr const char* FRAME_TAGS_KEY = "frameTags";
constexpr const char* IMAGE_KEY = "image";

SpriteFrame ParseFrame(const nlohmann::json& frameJson) {
    SpriteFrame f{};
    if (frameJson.contains("frame") && frameJson["frame"].is_object()) {
        const auto& fr = frameJson["frame"];
        f.x = fr.value("x", 0);
        f.y = fr.value("y", 0);
        f.w = fr.value("w", 0);
        f.h = fr.value("h", 0);
    }

    f.rotated = frameJson.value("rotated", false);
    f.trimmed = frameJson.value("trimmed", false);

    if (frameJson.contains("spriteSourceSize") &&
        frameJson["spriteSourceSize"].is_object()) {
        const auto& ss = frameJson["spriteSourceSize"];
        f.sourceX = ss.value("x", 0);
        f.sourceY = ss.value("y", 0);
        f.sourceW = ss.value("w", f.w);
        f.sourceH = ss.value("h", f.h);
    }

    if (frameJson.contains("sourceSize") && frameJson["sourceSize"].is_object()) {
        const auto& s = frameJson["sourceSize"];
        // sourceSize は元画像のサイズ。offsetには使わないが持っておく。
        f.sourceW = s.value("w", f.sourceW);
        f.sourceH = s.value("h", f.sourceH);
    }

    f.durationMs = frameJson.value("duration", 0);
    return f;
}

SpriteAnimationClip BuildFullClip(int frameCount) {
    SpriteAnimationClip clip{};
    clip.loop = true;
    clip.direction = "forward";
    clip.frameIndices.reserve(frameCount);
    for (int i = 0; i < frameCount; ++i) {
        clip.frameIndices.push_back(i);
    }
    return clip;
}

} // namespace

bool SpriteSheetLoader::LoadFromJson(const std::string& jsonPath,
                                     SpriteSheetAtlas& outAtlas) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            std::cerr << "[SpriteSheetLoader] Failed to open: " << jsonPath
                      << std::endl;
            return false;
        }

        nlohmann::json j = nlohmann::json::parse(file);

        if (!j.contains(FRAME_KEY)) {
            std::cerr << "[SpriteSheetLoader] Missing frames in: " << jsonPath
                      << std::endl;
            return false;
        }

        outAtlas.frames.clear();
        const auto& framesNode = j[FRAME_KEY];
        if (framesNode.is_array()) {
            for (const auto& frameJson : framesNode) {
                if (!frameJson.is_object()) {
                    std::cerr << "[SpriteSheetLoader] Skip non-object frame in: "
                              << jsonPath << std::endl;
                    continue;
                }
                outAtlas.frames.push_back(ParseFrame(frameJson));
            }
        } else if (framesNode.is_object()) {
            // Aseprite が出力する "frames": { "name": {...}, ... } 形式
            for (const auto& item : framesNode.items()) {
                if (!item.value().is_object()) {
                    std::cerr << "[SpriteSheetLoader] Skip non-object frame in: "
                              << jsonPath << std::endl;
                    continue;
                }
                outAtlas.frames.push_back(ParseFrame(item.value()));
            }
        } else {
            std::cerr << "[SpriteSheetLoader] Invalid frames format in: "
                      << jsonPath << std::endl;
            return false;
        }

        // image path
        if (j.contains(META_KEY) && j[META_KEY].is_object()) {
            const auto& meta = j[META_KEY];
            outAtlas.imagePath = meta.value(IMAGE_KEY, "");
            if (!outAtlas.imagePath.empty() &&
                !std::filesystem::path(outAtlas.imagePath).is_absolute()) {
                outAtlas.imagePath = (std::filesystem::path(jsonPath)
                                          .parent_path() /
                                      outAtlas.imagePath)
                                         .lexically_normal()
                                         .string();
            }

            if (meta.contains(FRAME_TAGS_KEY) && meta[FRAME_TAGS_KEY].is_array()) {
                for (const auto& tagJson : meta[FRAME_TAGS_KEY]) {
                    if (!tagJson.is_object()) {
                        continue;
                    }
                    SpriteAnimationClip clip{};
                    clip.loop = true;
                    clip.direction = tagJson.value("direction", "forward");

                    int from = tagJson.value("from", 0);
                    int to = tagJson.value("to", from);
                    if (from > to) {
                        std::swap(from, to);
                    }
                    for (int i = from; i <= to; ++i) {
                        clip.frameIndices.push_back(i);
                    }
                    std::string name = tagJson.value("name", "");
                    if (!name.empty()) {
                        outAtlas.tags[name] = std::move(clip);
                    }
                }
            }
        }

        if (outAtlas.tags.empty()) {
            // frameTagsが無い場合は全フレームを一つのクリップとする
            outAtlas.tags["default"] = BuildFullClip(
                static_cast<int>(outAtlas.frames.size()));
            std::cerr << "[SpriteSheetLoader] frameTags missing, fallback to "
                         "default clip in: "
                      << jsonPath << std::endl;
        }

        return true;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[SpriteSheetLoader] JSON parse error: " << e.what()
                  << " in " << jsonPath << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[SpriteSheetLoader] Error: " << e.what()
                  << " in " << jsonPath << std::endl;
        return false;
    }
}

const SpriteSheetAtlas* SpriteAtlasCache::GetOrLoad(
    const std::string& jsonPath) {
    auto it = cache_.find(jsonPath);
    if (it != cache_.end()) {
        return &it->second;
    }

    SpriteSheetAtlas atlas{};
    if (!loader_.LoadFromJson(jsonPath, atlas)) {
        return nullptr;
    }

    auto inserted = cache_.emplace(jsonPath, std::move(atlas));
    return &inserted.first->second;
}

void SpriteAtlasCache::Clear() { cache_.clear(); }

} // namespace Shared::Data

