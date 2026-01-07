#pragma once

#include <string>

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief キャラクターIDコンポーネント（参照用）
struct CharacterId {
    std::string id = "";  // キャラクターID（CharacterManagerのマスターデータ参照用）

    CharacterId() = default;
    CharacterId(const std::string& character_id) : id(character_id) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
