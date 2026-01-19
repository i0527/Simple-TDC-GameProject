#pragma once

#include <string>
#include "../../config/RenderTypes.hpp"

namespace game {
namespace core {
namespace entities {

// エンティティ生成時のデータ
struct EntityCreationData {
    std::string character_id;  // キャラクターID
    Vector2 position;          // 生成位置
    int level;                 // レベル（編成時のレベル）
    
    // 将来的に追加可能なパラメータ
    // float rotation = 0.0f;
    // int team_id = 0;
    // std::string formation_id = "";
};

} // namespace entities
} // namespace core
} // namespace game
