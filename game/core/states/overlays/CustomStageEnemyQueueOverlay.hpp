#pragma once

#include "IOverlay.hpp"
#include "../../config/BattleSetupData.hpp"
#include "../../ecs/entities/Character.hpp"

// 標準ライブラリ
#include <string>
#include <vector>

namespace game {
namespace core {

/// @brief カスタムステージの敵キュー設定オーバーレイ
class CustomStageEnemyQueueOverlay : public IOverlay {
public:
    CustomStageEnemyQueueOverlay();
    ~CustomStageEnemyQueueOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::CustomStageEnemyQueue; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

    /// @brief 設定対象のステージIDを設定
    void SetTargetStageId(const std::string& stageId);

private:
    BaseSystemAPI* systemAPI_ = nullptr;
    bool isInitialized_ = false;

    mutable bool requestClose_ = false;
    mutable bool hasTransitionRequest_ = false;
    mutable GameState requestedNextState_ = GameState::Home;

    // ステージ情報
    std::string targetStageId_;

    // キュー情報
    std::vector<CustomEnemyEntry> queue_;

    // 保有キャラクター一覧
    std::vector<const entities::Character*> availableCharacters_;

    // UI状態
    int selectedCharacterIndex_ = -1;
    int selectedLevel_ = 1;
    float selectedSpawnDelay_ = 1.0f;
    int selectedQueueIndex_ = -1;
    float characterListScrollOffset_ = 0.0f;
    float queueListScrollOffset_ = 0.0f;

    // ヘルパー関数
    void LoadAvailableCharacters(SharedContext& ctx);
    void LoadQueueFromStageData(SharedContext& ctx);
    void SaveQueueToStageData(SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
    void HandleKeyboardInput(SharedContext& ctx);
    void RenderCharacterList(SharedContext& ctx);
    void RenderSelectionPanel(SharedContext& ctx);
    void RenderQueueList(SharedContext& ctx);
    int GetCharacterMaxLevel(SharedContext& ctx, const std::string& characterId) const;
};

} // namespace core
} // namespace game
