#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>

// プロジェクト内
#include "Character.hpp"

namespace game {
namespace core {
namespace entities {

class ItemPassiveLoader {
public:
    static bool LoadFromJSON(
        const std::string& json_path,
        std::unordered_map<std::string, PassiveSkill>& outPassives,
        std::unordered_map<std::string, Equipment>& outEquipment);
    static bool SaveToJSON(
        const std::string& json_path,
        const std::unordered_map<std::string, PassiveSkill>& passives,
        const std::unordered_map<std::string, Equipment>& equipment);

    static void LoadHardcoded(
        std::unordered_map<std::string, PassiveSkill>& outPassives,
        std::unordered_map<std::string, Equipment>& outEquipment);
};

} // namespace entities
} // namespace core
} // namespace game
