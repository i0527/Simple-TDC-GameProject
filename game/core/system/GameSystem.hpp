#ifndef __GAME_CORE_GAMESYSTEM_HPP__
#define __GAME_CORE_GAMESYSTEM_HPP__

#include "../api/BaseSystemAPI.hpp"
#include "../api/GameModuleAPI.hpp"
#include "ModuleSystem.hpp"
#include "OverlayManager.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../init/ResourceInitializer.hpp"
#include "../states/TitleScreen.hpp"
#include "../states/HomeScreen.hpp"
#include "../states/GameScene.hpp"
#include "../entities/CharacterManager.hpp"
#include "../entities/ItemPassiveManager.hpp"
#include "../entities/StageManager.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief ゲームシステム統合クラス
///
/// 責務:
/// - アプリケーション全体の初期化・終了管理
/// - メインループの管理（フレーム制御）
/// - BaseSystemAPIとGameModuleAPIの所有・管理
/// - SharedContextの所有・管理
/// - ステート管理（enum + 遷移制御）
/// - 各ステートクラスの所有・管理
/// - 安全なステート遷移（二重初期化/解放防止）
/// - モジュール登録の呼び出し（RegisterModules）
/// - オーバーレイ管理（OverlayManager統合）
///
/// ModuleSystemとの関係:
/// - GameSystemがModuleSystemを所有
/// - GameSystemがSharedContextを所有し、ModuleSystemに参照として渡す
/// - メインループでModuleSystemのUpdate/Renderを呼び出す
///
/// OverlayManagerとの関係:
/// - GameSystemがOverlayManagerを所有
/// - メインループでOverlayManagerのUpdate/Renderを呼び出す
/// - オーバーレイからの遷移リクエストを処理
class GameSystem {
public:
  GameSystem();
  ~GameSystem() = default;

  /// @brief ゲームの初期化
  /// @return 成功時0、失敗時非0
  int Initialize();

  /// @brief メインループ実行
  /// @return 終了コード（通常0）
  int Run();

  /// @brief ゲームのシャットダウン
  void Shutdown();

  /// @brief ステート遷移リクエスト（オーバーレイから呼び出し用）
  /// @param newState 遷移先のステート
  /// @note オーバーレイからの遷移リクエストを処理するために公開
  void RequestTransition(GameState newState);

private:
  std::unique_ptr<BaseSystemAPI> systemAPI_;
  std::unique_ptr<GameModuleAPI> gameAPI_;
  std::unique_ptr<ModuleSystem> moduleSystem_;
  std::unique_ptr<OverlayManager> overlayManager_;
  std::unique_ptr<ResourceInitializer> resourceInitializer_;
  std::unique_ptr<TitleScreen> titleScreen_;
  std::unique_ptr<states::HomeScreen> homeScreen_;
  std::unique_ptr<states::GameScene> gameScene_;
  std::unique_ptr<entities::CharacterManager> characterManager_;
  std::unique_ptr<entities::ItemPassiveManager> itemPassiveManager_;
  std::unique_ptr<entities::StageManager> stageManager_;
  SharedContext sharedContext_;
  GameState currentState_;
  bool requestShutdown_;

  // ステート管理
  void transitionTo(GameState newState);
  void cleanupCurrentState();
  bool initializeState(GameState state);

  // モジュール管理
  void RegisterModules();
};
} // namespace core
} // namespace game

#endif // __GAME_CORE_GAMESYSTEM_HPP__
