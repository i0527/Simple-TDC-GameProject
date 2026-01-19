#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクト内
#include "IScene.hpp"
#include "../config/GameState.hpp"
#include "../ecs/entities/Character.hpp"
#include "../ecs/entities/StageManager.hpp"

namespace game {
namespace core {

class BaseSystemAPI;
class InputSystemAPI;
struct SharedContext;

namespace states {

class EditorScene : public IScene {
public:
    EditorScene();
    ~EditorScene() override;

    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(float deltaTime) override;
    void Render() override;
    void RenderImGui() override;
    void Shutdown() override;

    bool RequestTransition(GameState& nextState) override;
    bool RequestQuit() override;

    void SetSharedContext(SharedContext* ctx) override;

private:
    enum class EditorTab {
        Characters,
        Equipment,
        Passives,
        Stages,
        BattleDebug
    };

    void LoadDataFromAPI();
    void RenderCharacterTab();
    void RenderEquipmentTab();
    void RenderPassiveTab();
    void RenderStageTab();
    void RenderBattleDebugTab();
    void UpdateAttackPreview(float deltaTime);
    void UpdateMoveSimulation(float deltaTime);
    void ResetAttackPreview();
    void SetPreviewTime(float time);
    float GetAttackDuration() const;
    float GetPreviewDuration() const;
    const entities::Character* GetSelectedCharacter() const;
    entities::Character* GetSelectedCharacterMutable();
    bool IsCharacterModified(const std::string& id) const;
    bool IsEquipmentModified(const std::string& id) const;
    bool IsPassiveModified(const std::string& id) const;
    bool IsStageModified(const std::string& id) const;

    BaseSystemAPI* systemAPI_;
    InputSystemAPI* inputAPI_;
    SharedContext* sharedContext_;
    bool isInitialized_;

    bool requestTransition_;
    GameState nextState_;
    bool requestQuit_;

    EditorTab activeTab_;

    std::unordered_map<std::string, entities::Character> characterEdits_;
    std::unordered_map<std::string, entities::Character> characterOriginal_;
    std::vector<std::string> characterIds_;
    int selectedCharacterIndex_;
    bool lastCharacterSaveOk_;
    bool hasCharacterSaveResult_;

    std::unordered_map<std::string, entities::Equipment> equipmentEdits_;
    std::unordered_map<std::string, entities::Equipment> equipmentOriginal_;
    std::vector<std::string> equipmentIds_;
    int selectedEquipmentIndex_;
    bool lastItemSaveOk_;
    bool hasItemSaveResult_;

    std::unordered_map<std::string, entities::PassiveSkill> passiveEdits_;
    std::unordered_map<std::string, entities::PassiveSkill> passiveOriginal_;
    std::vector<std::string> passiveIds_;
    int selectedPassiveIndex_;

    std::unordered_map<std::string, entities::StageData> stageEdits_;
    std::unordered_map<std::string, entities::StageData> stageOriginal_;
    std::vector<std::string> stageIds_;
    int selectedStageIndex_;
    bool lastStageSaveOk_;
    bool hasStageSaveResult_;
    std::string stageJsonText_;
    std::string stageJsonError_;
    std::string stageUnlockText_;

    // 攻撃アニメーションのプレビュー
    float previewTime_;
    float previewSpeed_;
    bool previewPaused_;
    bool previewLoopEnabled_;
    bool previewUseMoveSprite_;
    float previewLoopStart_;
    float previewLoopEnd_;

    bool showHitMarker_;
    bool showTimeBar_;
    bool showRangeOverlay_;
    bool showStatusOverlay_;
    bool showAttackLog_;
    bool attackLogEnabled_;

    int spawnCharacterIndex_;
    bool spawnAsEnemy_;
    float spawnX_;
    float spawnY_;
    int spawnLevel_;

    bool moveSimEnabled_;
    float moveSimTime_;
    float moveSimSpeed_;
    float moveSimTargetOffset_;
    float moveSimStartOffset_;
    bool moveSimLoop_;
    bool moveSimShowPath_;
    bool moveSimShowStop_;
    bool moveSimShowDistance_;
    bool moveSimShowHitbox_;

    std::string characterFilter_;
    std::string equipmentFilter_;
    std::string passiveFilter_;
    std::string stageFilter_;
};

} // namespace states
} // namespace core
} // namespace game
