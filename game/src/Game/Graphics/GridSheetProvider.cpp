#include "Game/Graphics/GridSheetProvider.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

using json = nlohmann::json;

namespace Game::Graphics {

GridSheetProvider::GridSheetProvider(Texture2D& texture, const Config& config)
    : texture_(&texture), config_(config) {
}

void GridSheetProvider::RegisterClip(const std::string& name,
                                     int startFrameIndex,
                                     int frameCount,
                                     bool loop,
                                     float fps) {
    clips_[name] = ClipDef{startFrameIndex, frameCount, loop, fps};
}

bool GridSheetProvider::LoadFromJson(const std::string& jsonPath) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            std::cerr << "GridSheetProvider::LoadFromJson: Cannot open " << jsonPath << std::endl;
            return false;
        }
        
        json data;
        file >> data;
        file.close();
        
        // JSON の形式は以下を想定:
        // {
        //   "clips": [
        //     { "name": "idle", "startIndex": 0, "length": 8, "loop": true, "fps": 12.0 }
        //   ]
        // }
        // または旧形式:
        // {
        //   "idle": {"start": 0, "count": 8, "loop": true, "fps": 12.0}
        // }
        
        if (data.contains("clips")) {
            // 新形式
            for (auto& clip : data["clips"]) {
                std::string name = clip.value("name", "");
                int start = clip.value("startIndex", 0);
                int count = clip.value("length", 1);
                bool loop = clip.value("loop", true);
                float fps = clip.value("fps", 12.0f);
                
                RegisterClip(name, start, count, loop, fps);
            }
        } else {
            // 旧形式
            for (auto& [name, def] : data.items()) {
                int start = def.value("start", 0);
                int count = def.value("count", 1);
                bool loop = def.value("loop", true);
                float fps = def.value("fps", 12.0f);
                
                RegisterClip(name, start, count, loop, fps);
            }
        }
        
        return true;
    } catch (const json::parse_error& e) {
        std::cerr << "GridSheetProvider::LoadFromJson: JSON parse error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "GridSheetProvider::LoadFromJson: Exception: " << e.what() << std::endl;
        return false;
    }
}

bool GridSheetProvider::HasClip(const std::string& clipName) const {
    return clips_.find(clipName) != clips_.end();
}

int GridSheetProvider::GetFrameCount(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    return (it != clips_.end()) ? it->second.frameCount : 0;
}

FrameRef GridSheetProvider::GetFrame(const std::string& clipName, int frameIndex) const {
    auto it = clips_.find(clipName);
    if (it == clips_.end()) {
        // クリップが見つからない
        return FrameRef{};
    }
    
    const auto& clip = it->second;
    if (frameIndex < 0 || frameIndex >= clip.frameCount) {
        // フレーム範囲外
        return FrameRef{};
    }
    
    // グリッド内の実フレームインデックス
    int gridFrameIndex = clip.startFrameIndex + frameIndex;
    auto [col, row] = FrameIndexToGridCoord(gridFrameIndex);
    
    auto ref = CreateFrameRef(col, row);
    ref.durationSec = 1.0f / clip.fps;
    ref.valid = true;
    
    return ref;
}

float GridSheetProvider::GetClipFps(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    return (it != clips_.end()) ? it->second.fps : 12.0f;
}

bool GridSheetProvider::IsLooping(const std::string& clipName) const {
    auto it = clips_.find(clipName);
    return (it != clips_.end()) ? it->second.loop : true;
}

std::pair<int, int> GridSheetProvider::FrameIndexToGridCoord(int frameIndex) const {
    int col = frameIndex % config_.framesPerRow;
    int row = frameIndex / config_.framesPerRow;
    return {col, row};
}

FrameRef GridSheetProvider::CreateFrameRef(int col, int row) const {
    FrameRef ref;
    ref.texture = texture_;
    
    // ソース矩形（テクスチャ内の切り出し位置）
    ref.src.x = col * config_.cellWidth;
    ref.src.y = row * config_.cellHeight;
    ref.src.width = config_.cellWidth;
    ref.src.height = config_.cellHeight;
    
    // 足元ベースの origin（中心X、下端Y）
    ref.origin = Vector2{
        config_.cellWidth * 0.5f,
        config_.cellHeight
    };
    
    // グリッド形式は trim がないので offset なし
    ref.offset = Vector2{0, 0};
    
    return ref;
}

} // namespace Game::Graphics
