#ifndef __GAME_CORE_GAMESYSTEM_HPP__
#define __GAME_CORE_GAMESYSTEM_HPP__

#include "../api/BaseSystemAPI.hpp"
#include "../api/AudioControlAPI.hpp"
#include "../api/ECSystemAPI.hpp"
#include "../api/InputSystemAPI.hpp"
#include "../api/UISystemAPI.hpp"
#include "../api/DebugUIAPI.hpp"
#include "../api/SceneOverlayControlAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../config/GameConfig.hpp"
#include "../api/SetupAPI.hpp"
#include "../api/BattleProgressAPI.hpp"
#include "../api/BattleSetupAPI.hpp"
#include "../states/InitScene.hpp"
#include "../states/TitleScreen.hpp"
#include "../states/HomeScreen.hpp"
#include "../states/GameScene.hpp"
#include "../states/EditorScene.hpp"
#include "../api/GameplayDataAPI.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief ??????E???????
///
/// ??E
/// - ???????????E????E??E???E
/// - ???????E??E?????????E?E
/// - BaseSystemAPI?ECSystemAPI????E??E
/// - SharedContext????E??E
/// - ??E?E???E???E??????E?E
/// - ?E???E?E????????E??E
/// - ??E?E??E??/??E?ESceneOverlayControlAPI??E??E
/// - ??E???E?E??E??E??????E?????E?E
/// - ????E????E??EverlayManager???E
///
/// OverlayManager????E
/// - GameSystem?OverlayManager???E
/// - ???????OverlayManager?Update/Render?????E
/// - ????E???????????????E
class GameSystem {
public:
  GameSystem();
  ~GameSystem() = default;

  /// @brief ???????E
  /// @return ???E?????E
  int Initialize();

  /// @brief ????????E
  /// @return ??E???????0?E?E
  int Run();

  /// @brief ???????E?????
  void Shutdown();

  /// @brief ??E?E??E???????????E??????????E?E
  /// @param newState ????E??E?E?E
  /// @note ????E???????????????E????????E
  void RequestTransition(GameState newState);

private:
  std::unique_ptr<BaseSystemAPI> systemAPI_;
  std::unique_ptr<AudioControlAPI> audioAPI_;
  std::unique_ptr<ECSystemAPI> ecsAPI_;
  std::unique_ptr<InputSystemAPI> inputAPI_;
  std::unique_ptr<UISystemAPI> uiAPI_;
  std::unique_ptr<DebugUIAPI> debugUIAPI_;
  std::unique_ptr<SceneOverlayControlAPI> sceneOverlayAPI_;
  std::unique_ptr<SetupAPI> setupAPI_;
  std::unique_ptr<BattleProgressAPI> battleProgressAPI_;
  std::unique_ptr<BattleSetupAPI> battleSetupAPI_;
  std::unique_ptr<states::InitScene> initScene_;
  std::unique_ptr<TitleScreen> titleScreen_;
  std::unique_ptr<states::HomeScreen> homeScreen_;
  std::unique_ptr<states::GameScene> gameScene_;
  std::unique_ptr<states::EditorScene> editorScene_;
  std::unique_ptr<GameplayDataAPI> gameplayDataAPI_;
  SharedContext sharedContext_;
  GameState currentState_;
  bool requestShutdown_;

  // ??E?E??E???E
  void transitionTo(GameState newState);

  // ????????
  struct StartupSettings {
    Resolution resolution = Resolution::FHD;
    WindowMode windowMode = WindowMode::Windowed;
    bool showCursor = false;
  };
  StartupSettings LoadStartupSettings();
  bool InitializeBaseSystem(StartupSettings settings);
  bool InitializeAudio();
  bool InitializeInput();
  bool InitializeUI();
  void InitializeGameplayData();
  void SetupSharedContext();
  bool InitializeDebugUI();
  bool InitializeSetup();
  bool InitializeBattleSetup();
  bool InitializeSceneOverlay();
  bool InitializeBattleProgress();
  bool InitializeScenes();

  void ShutdownScenes();
  void ShutdownBattleProgress();
  void ShutdownDebugUI();
  void ShutdownBattleSetup();
  void ShutdownSetup();
  void ShutdownGameplayData();
  void ShutdownSceneOverlay();
  void ShutdownUI();
  void ShutdownInput();
  void ShutdownAudio();
  void ShutdownBaseSystem();

};
} // namespace core
} // namespace game

#endif // __GAME_CORE_GAMESYSTEM_HPP__
