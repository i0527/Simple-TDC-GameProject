#pragma once

namespace game {
namespace core {

/// @brief ゲームステート定義
///
/// ゲームの現在の状態を表すenum。
/// GameSystemがステート管理に使用します。
enum class GameState {
  Initializing, // リソース初期化中
  Title,        // タイトル画面
  Home,         // ホーム画面
  Game,         // ゲーム画面
};

/// @brief オーバーレイステート定義
///
/// オーバーレイの種類を表すenum。
/// OverlayManagerがオーバーレイ管理に使用します。
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
};

} // namespace core
} // namespace game
