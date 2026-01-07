#pragma once

#include <string>

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief スプライトコンポーネント
struct Sprite {
    std::string sheet_path = "";  // スプライトシートパス
    int frame_width = 0;          // 1フレームの幅
    int frame_height = 0;         // 1フレームの高さ

    Sprite() = default;
    Sprite(const std::string& path, int width, int height)
        : sheet_path(path), frame_width(width), frame_height(height) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
