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

/// @brief 繧ｲ繝ｼ繝繧ｷ繧ｹ繝・Β邨ｱ蜷医け繝ｩ繧ｹ
///
/// 雋ｬ蜍・
/// - 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ蜈ｨ菴薙・蛻晄悄蛹悶・邨ゆｺ・ｮ｡逅・
/// - 繝｡繧､繝ｳ繝ｫ繝ｼ繝励・邂｡逅・ｼ医ヵ繝ｬ繝ｼ繝蛻ｶ蠕｡・・
/// - BaseSystemAPI縺ｨECSystemAPI縺ｮ謇譛峨・邂｡逅・
/// - SharedContext縺ｮ謇譛峨・邂｡逅・
/// - 繧ｹ繝・・繝育ｮ｡逅・ｼ磯・遘ｻ蛻ｶ蠕｡縺ｮ縺ｿ・・
/// - 蜷・せ繝・・繝医け繝ｩ繧ｹ縺ｮ謇譛峨・邂｡逅・
/// - 繧ｹ繝・・繝亥・譛溷喧/邨ゆｺ・・SceneOverlayControlAPI縺ｫ髮・ｴ・
/// - 螳牙・縺ｪ繧ｹ繝・・繝磯・遘ｻ・井ｺ碁㍾蛻晄悄蛹・隗｣謾ｾ髦ｲ豁｢・・
/// - 繧ｪ繝ｼ繝舌・繝ｬ繧､邂｡逅・ｼ・verlayManager邨ｱ蜷茨ｼ・
///
/// OverlayManager縺ｨ縺ｮ髢｢菫・
/// - GameSystem縺薫verlayManager繧呈園譛・
/// - 繝｡繧､繝ｳ繝ｫ繝ｼ繝励〒OverlayManager縺ｮUpdate/Render繧貞他縺ｳ蜃ｺ縺・
/// - 繧ｪ繝ｼ繝舌・繝ｬ繧､縺九ｉ縺ｮ驕ｷ遘ｻ繝ｪ繧ｯ繧ｨ繧ｹ繝医ｒ蜃ｦ逅・
class GameSystem {
public:
  GameSystem();
  ~GameSystem() = default;

  /// @brief 繧ｲ繝ｼ繝縺ｮ蛻晄悄蛹・
  /// @return 謌仙粥譎・縲∝､ｱ謨玲凾髱・
  int Initialize();

  /// @brief 繝｡繧､繝ｳ繝ｫ繝ｼ繝怜ｮ溯｡・
  /// @return 邨ゆｺ・さ繝ｼ繝会ｼ磯壼ｸｸ0・・
  int Run();

  /// @brief 繧ｲ繝ｼ繝縺ｮ繧ｷ繝｣繝・ヨ繝繧ｦ繝ｳ
  void Shutdown();

  /// @brief 繧ｹ繝・・繝磯・遘ｻ繝ｪ繧ｯ繧ｨ繧ｹ繝茨ｼ医が繝ｼ繝舌・繝ｬ繧､縺九ｉ蜻ｼ縺ｳ蜃ｺ縺礼畑・・
  /// @param newState 驕ｷ遘ｻ蜈医・繧ｹ繝・・繝・
  /// @note 繧ｪ繝ｼ繝舌・繝ｬ繧､縺九ｉ縺ｮ驕ｷ遘ｻ繝ｪ繧ｯ繧ｨ繧ｹ繝医ｒ蜃ｦ逅・☆繧九◆繧√↓蜈ｬ髢・
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

  // 繧ｹ繝・・繝磯・遘ｻ邂｡逅・
  void transitionTo(GameState newState);

  // 蛻晄悄蛹・邨ゆｺ・・蛻・牡
  bool InitializeBaseSystem();
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
