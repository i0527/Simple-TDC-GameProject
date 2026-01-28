#pragma once

namespace game {
namespace core {

/// @brief 繧ｲ繝ｼ繝繧ｹ繝・・繝亥ｮ夂ｾｩ
///
/// 繧ｲ繝ｼ繝縺ｮ迴ｾ蝨ｨ縺ｮ迥ｶ諷九ｒ陦ｨ縺册num縲・
/// GameSystem縺後せ繝・・繝育ｮ｡逅・↓菴ｿ逕ｨ縺励∪縺吶・
enum class GameState {
  Initializing, // 繝ｪ繧ｽ繝ｼ繧ｹ蛻晄悄蛹紋ｸｭ
  Title,        // 繧ｿ繧､繝医Ν逕ｻ髱｢
  Home,         // 繝帙・繝逕ｻ髱｢
  Game,         // 繧ｲ繝ｼ繝逕ｻ髱｢
  Editor,       // 繧ｨ繝・ぅ繧ｿ逕ｻ髱｢
  Count,
};

/// @brief 繧ｪ繝ｼ繝舌・繝ｬ繧､繧ｹ繝・・繝亥ｮ夂ｾｩ
///
/// 繧ｪ繝ｼ繝舌・繝ｬ繧､縺ｮ遞ｮ鬘槭ｒ陦ｨ縺册num縲・
/// OverlayManager縺後が繝ｼ繝舌・繝ｬ繧､邂｡逅・↓菴ｿ逕ｨ縺励∪縺吶・
enum class OverlayState {
  None = 0,
  StageSelect = 1,
  Formation = 2,
  Enhancement = 3,
  Codex = 4,
  Settings = 5,
  Gacha = 6,
  License = 7,
  BattleVictory = 8,
  BattleDefeat = 9,
  Pause = 10,
  CustomStageEnemyQueue = 11,
};

} // namespace core
} // namespace game
