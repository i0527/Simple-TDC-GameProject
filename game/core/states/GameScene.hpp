#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../game/BattleRenderer.hpp"
#include "../api/BattleProgressAPI.hpp"
#include "../ui/BattleHUDRenderer.hpp"
#include <memory>
#include <string>

namespace game {
namespace core {
class InputSystemAPI;
class BattleProgressAPI;

namespace states {

/// @brief ゲームシーンクラス
///
/// 責勁E
/// - 1レーン横スクロール戦闘（にめE��こ型�E��E管琁E
/// - 入力とUIの統合（段階的に実裁E��E
/// - ゲームロジチE��の進行制御
class GameScene : public IScene {
public:
    GameScene();
    ~GameScene() override;

    // ========== ISceneインターフェース実裁E==========

    /// @brief シーンの初期匁E
    bool Initialize(BaseSystemAPI* systemAPI) override;

    /// @brief シーンの更新処琁E
    void Update(float deltaTime) override;

    /// @brief シーンの描画処琁E
    void Render() override;

    /// @brief HUD描画
    void RenderHUD() override;

    /// @brief シーンのクリーンアチE�E
    void Shutdown() override;

    /// @brief 遷移リクエストを取征E
    bool RequestTransition(GameState& nextState) override;

    /// @brief 終亁E��クエストを取征E
    bool RequestQuit() override;

    // ========== SharedContext設宁E==========

    /// @brief SharedContextを設定！EameSystemから呼ばれる�E�E
    void SetSharedContext(SharedContext* ctx) override {
        sharedContext_ = ctx;
        inputAPI_ = ctx ? ctx->inputAPI : nullptr;
        battleProgressAPI_ = ctx ? ctx->battleProgressAPI : nullptr;
        if (battleRenderer_) {
            battleRenderer_->SetEcsAPI(ctx ? ctx->ecsAPI : nullptr);
        }
    }

private:
    // コアシスチE��
    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_;

    // 入劁E
    InputSystemAPI* inputAPI_;
    std::unique_ptr<::game::core::ui::BattleHUDRenderer> battleHud_;

    // ECS管琁E��戦闘ユニット用�E�E
    std::unique_ptr<::game::core::game::BattleRenderer> battleRenderer_;

    // 戦闘進行API
    BattleProgressAPI* battleProgressAPI_;

    // 遷移リクエスチE
    mutable bool requestTransition_;
    mutable GameState nextState_;
    mutable bool requestQuit_;

    // ========== ダメージホップアップ ==========
    struct DamagePopup {
        Vec2 position;      // 画面上の位置
        int damage;         // ダメージ値
        float lifetime;     // 残り表示時間（秒）
        float maxLifetime;  // 最大表示時間（1.0秒推奨）
        ColorRGBA color;    // 色（デフォルト：赤）
    };
    std::vector<DamagePopup> damagePopups_;
    size_t lastAttackLogSize_ = 0;  // 前回フレームの攻撃ログサイズ

    // ========== 冁E��処琁E==========

    /// @brief 入力�E琁E
    void ProcessInput();

    /// @brief ボタンクリチE��処琁E
    void HandleButtonClick(const std::string& buttonId);

    // 描画�E�にめE��こ型�E�E
    void RenderBattle();
    void HandleHUDAction(const ::game::core::ui::BattleHUDAction& action);
    
    // ========== ヘルパー関数 ==========
    
    /// @brief ステージIDから背景画像パスを生成
    std::string GetStageBackgroundPath(const std::string& stageId) const;
    
    /// @brief クエスト表示を描画
    void RenderQuestPanel();
    
    /// @brief ダメージホップアップを更新
    void UpdateDamagePopups(float deltaTime);
    
    /// @brief ダメージホップアップを描画
    void RenderDamagePopups();
};

} // namespace states
} // namespace core
} // namespace game
