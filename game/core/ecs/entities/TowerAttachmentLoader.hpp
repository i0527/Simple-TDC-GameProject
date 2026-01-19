#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>

// プロジェクト内
#include "TowerAttachment.hpp"

namespace game {
namespace core {
namespace entities {

/// @brief タワーアタッチメントマスターのロード/保存ユーティリティ
class TowerAttachmentLoader {
public:
    static bool LoadFromJSON(const std::string& json_path,
                             std::unordered_map<std::string, TowerAttachment>& outMasters);
    static void LoadHardcoded(std::unordered_map<std::string, TowerAttachment>& outMasters);
};

} // namespace entities
} // namespace core
} // namespace game
