#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../game/InputHandler.hpp"
#include "../game/BattleRenderer.hpp"
#include "../game/WaveLoader.hpp"
#include "../ui/BattleHUDRenderer.hpp"
#include <memory>
#include <entt/entt.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace game {
namespace core {
namespace states {

/// @brief ゲームシーンクラス
///
/// 責務:
/// - 1レーン横スクロール戦闘（にゃんこ型）の管理
/// - 入力とUIの統合（段階的に実装）
/// - ゲームロジックの進行制御
class GameScene : public IScene {
public:
    GameScene();
    ~GameScene() override;

    // ========== ISceneインターフェース実装 ==========

    /// @brief シーンの初期化
    bool Initialize(BaseSystemAPI* systemAPI) override;

    /// @brief シーンの更新処理
    void Update(float deltaTime) override;

    /// @brief シーンの描画処理
    void Render() override;

    /// @brief シーンのクリーンアップ
    void Shutdown() override;

    /// @brief 遷移リクエストを取得
    bool RequestTransition(GameState& nextState) override;

    /// @brief 終了リクエストを取得
    bool RequestQuit() override;

    // ========== SharedContext設定 ==========

    /// @brief SharedContextを設定（GameSystemから呼ばれる）
    void SetSharedContext(SharedContext* ctx) { sharedContext_ = ctx; }

private:
    // コアシステム
    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_;

    // 入力
    std::unique_ptr<gamescene::InputHandler> inputHandler_;
    std::unique_ptr<::game::core::ui::BattleHUDRenderer> battleHud_;

    // ECSレジストリ（戦闘ユニット用）
    entt::registry registry_;
    std::unique_ptr<::game::core::game::BattleRenderer> battleRenderer_;

    // ========== バトル設定 / 状態 ==========
    struct LaneConfig {
        float y = 360.0f;
        float startX = 120.0f;
        float endX = 1800.0f;
        float minGap = 72.0f;
    };

    struct TowerState {
        int currentHp = 1000;
        int maxHp = 1000;
        float x = 0.0f;       // タワー中心X
        float y = 0.0f;       // タワー底辺Y
        float width = 140.0f;
        float height = 260.0f;
    };

    enum class BattleResult {
        InProgress,
        Victory,
        Defeat
    };

    LaneConfig lane_;
    TowerState playerTower_;
    TowerState enemyTower_;
    BattleResult battleResult_ = BattleResult::InProgress;

    float battleTime_ = 0.0f;

    // Wave進行（後続TODOでWaveLoader導入）
    int currentWave_ = 1;
    int totalWaves_ = 1;
    ::game::core::game::WaveLoader waveLoader_;
    std::vector<::game::core::game::SpawnEvent> spawnSchedule_;
    size_t spawnCursor_ = 0;

    // 経済/速度/ポーズ
    int gold_;
    // にゃんこのお財布方式: 上限が時間で増加し、その上限までゴールドが回復する
    int goldMaxCap_ = 9999;                // 上限の最終到達値
    float goldMaxCurrent_ = 9999.0f;       // 上限の現在値（時間で増える）
    float goldMaxGrowthPerSecond_ = 30.0f; // 上限の増加速度（毎秒）

    float goldRegenPerSecond_ = 10.0f;     // ゴールド回復（毎秒）
    float goldRegenAccumulator_ = 0.0f;
    float gameSpeed_;
    bool isPaused_;
    std::string gameStateText_;

    // 出撃クールダウン（後続TODOでHUD/編成と接続）
    std::unordered_map<std::string, float> unitCooldownUntil_;

    // 敵ID -> キャラID の簡易マッピング（見た目のフォールバック）
    std::unordered_map<std::string, std::string> enemyToCharacterId_;

    // 遷移リクエスト
    mutable bool requestTransition_;
    mutable GameState nextState_;
    mutable bool requestQuit_;

    // ========== 内部処理 ==========

    /// @brief ゲームロジック更新
    void UpdateGameLogic(float deltaTime);

    /// @brief 入力処理
    void ProcessInput();

    /// @brief ボタンクリック処理
    void HandleButtonClick(const std::string& buttonId);

    // 初期化/更新/描画（にゃんこ型）
    void InitializeBattleFromStage();
    void UpdateBattle(float deltaTime);
    void RenderBattle();
    void HandleHUDAction(const ::game::core::ui::BattleHUDAction& action);

    // 勝敗
    void CheckBattleEnd();
};

} // namespace states
} // namespace core
} // namespace game
