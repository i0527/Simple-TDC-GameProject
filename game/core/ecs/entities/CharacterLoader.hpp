#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>

// プロジェクト内
#include "Character.hpp"

namespace game {
namespace core {
namespace entities {

class CharacterLoader {
public:
    static bool LoadFromJSON(const std::string& json_path,
                             std::unordered_map<std::string, Character>& outMasters);
    static bool SaveToJSON(
        const std::string& json_path,
        const std::unordered_map<std::string, Character>& masters);
    static void LoadHardcoded(
        std::unordered_map<std::string, Character>& outMasters);
};

} // namespace entities
} // namespace core
} // namespace game
