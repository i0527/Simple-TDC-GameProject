#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Shared::Data {

struct SpriteFrame {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int sourceX = 0;
    int sourceY = 0;
    int sourceW = 0;
    int sourceH = 0;
    int durationMs = 0;
    bool rotated = false;
    bool trimmed = false;
};

struct SpriteAnimationClip {
    std::vector<int> frameIndices;
    bool loop = true;
    std::string direction = "forward"; // forward / reverse / pingpong
};

struct SpriteSheetAtlas {
    std::string imagePath;
    std::vector<SpriteFrame> frames;
    std::unordered_map<std::string, SpriteAnimationClip> tags;
};

class SpriteSheetLoader {
public:
    bool LoadFromJson(const std::string& jsonPath, SpriteSheetAtlas& outAtlas);
};

class SpriteAtlasCache {
public:
    const SpriteSheetAtlas* GetOrLoad(const std::string& jsonPath);
    void Clear();

private:
    SpriteSheetLoader loader_;
    std::unordered_map<std::string, SpriteSheetAtlas> cache_;
};

} // namespace Shared::Data

