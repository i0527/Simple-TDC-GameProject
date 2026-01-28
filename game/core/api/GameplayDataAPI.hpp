#pragma once

// 標準ライブラリ
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクト内
#include "../config/SharedContext.hpp"
#include "../ecs/entities/CharacterManager.hpp"
#include "../ecs/entities/ItemPassiveManager.hpp"
#include "../ecs/entities/StageManager.hpp"
#include "../ecs/entities/TowerAttachmentManager.hpp"
#include "../system/PlayerDataManager.hpp"
#include "../api/BattleProgressAPI.hpp"

namespace game {
namespace core {

/// @brief ステージクリア報酬レポート
struct StageClearReport {
    std::vector<std::string> newCharacters;  // 新規解放されたキャラクターIDリスト
    int ticketsRewarded;                     // 付与されたチケット数
    int rewardGold;                          // 付与されたゴールド数
    float survivalTime;                      // 無限ステージの生存時間（秒）
    
    StageClearReport() : ticketsRewarded(0), rewardGold(0), survivalTime(0.0f) {}
};

/// @brief ゲームプレイのデータアクセス統合API
///
/// Character/ItemPassive/Stage/PlayerData の取得・検索・整合性チェックを集約し、
/// 限定的な書き込み（セーブ更新など）をラップして提供します。
class GameplayDataAPI {
public:
    GameplayDataAPI() = default;
    ~GameplayDataAPI() = default;

    bool Initialize(const std::string& characterJsonPath = "data/characters.json",
                    const std::string& itemPassiveJsonPath = "data/item_passive.json",
                    const std::string& stageJsonPath = "data/stages.json",
                    const std::string& playerSavePath = "data/saves/player_save.json",
                    const std::string& towerAttachmentJsonPath = "data/tower_attachments.json");
    void Shutdown();

    // ===== Character =====
    std::shared_ptr<entities::Character> GetCharacterTemplate(const std::string& characterId);
    std::vector<std::string> GetAllCharacterIds() const;
    bool HasCharacter(const std::string& characterId) const;
    size_t GetCharacterCount() const;
    const std::unordered_map<std::string, entities::Character>& GetAllCharacterMasters() const;
    bool SaveCharacterMasters(const std::unordered_map<std::string, entities::Character>& masters);
    // ===== Item/Passive =====
    const entities::PassiveSkill* GetPassiveSkill(const std::string& id) const;
    std::vector<const entities::PassiveSkill*> GetAllPassiveSkills() const;
    const entities::Equipment* GetEquipment(const std::string& id) const;
    std::vector<const entities::Equipment*> GetAllEquipment() const;
    const entities::ItemPassiveManager* GetItemPassiveManager() const;
    const std::unordered_map<std::string, entities::PassiveSkill>& GetAllPassiveMasters() const;
    const std::unordered_map<std::string, entities::Equipment>& GetAllEquipmentMasters() const;
    bool SaveItemPassiveMasters(const std::unordered_map<std::string, entities::PassiveSkill>& passives,
                                const std::unordered_map<std::string, entities::Equipment>& equipment);
    // ===== Tower Attachments =====
    const entities::TowerAttachment* GetTowerAttachment(const std::string& id) const;
    std::vector<const entities::TowerAttachment*> GetAllTowerAttachments() const;
    const entities::TowerAttachmentManager* GetTowerAttachmentManager() const;
    const std::unordered_map<std::string, entities::TowerAttachment>& GetAllTowerAttachmentMasters() const;

    // ===== Stage =====
    std::shared_ptr<entities::StageData> GetStageDataById(const std::string& stageId);
    std::shared_ptr<entities::StageData> GetStageData(int stageNumber);
    std::vector<entities::StageData> GetAllStageData() const;
    std::vector<std::string> GetAllStageIds() const;
    bool HasStage(const std::string& stageId) const;
    size_t GetStageCount() const;
    const std::unordered_map<std::string, entities::StageData>& GetAllStages() const;
    bool SaveStageMasters(const std::unordered_map<std::string, entities::StageData>& stages);
    PlayerDataManager::PlayerSaveData::StageState GetStageState(const std::string& stageId) const;
    void SetStageState(const std::string& stageId,
                       const PlayerDataManager::PlayerSaveData::StageState& state);
    void MarkStageCleared(const std::string& stageId, int starsEarned = 3, 
                         const BattleProgressAPI::BattleStats* battleStats = nullptr);
    std::string GetPreferredNextStageId(const std::string& stageId) const;
    const StageClearReport& GetLastStageClearReport() const;

    // ===== PlayerData (limited write) =====
    bool Save() const;
    void ApplyToSharedContext(SharedContext& ctx) const;
    void SetFormationFromSharedContext(const FormationData& formation);
    PlayerDataManager::CharacterState GetCharacterState(const std::string& characterId) const;
    void SetCharacterState(const std::string& characterId,
                           const PlayerDataManager::CharacterState& state);
    int GetOwnedEquipmentCount(const std::string& equipmentId) const;
    int GetOwnedPassiveCount(const std::string& passiveId) const;
    int GetOwnedTowerAttachmentCount(const std::string& attachmentId) const;
    void SetOwnedEquipmentCount(const std::string& equipmentId, int count);
    void SetOwnedPassiveCount(const std::string& passiveId, int count);
    void SetOwnedTowerAttachmentCount(const std::string& attachmentId, int count);
    int GetGold() const;
    void SetGold(int gold);
    void AddGold(int delta);
    int GetGems() const;
    void SetGems(int gems);
    void AddGems(int delta);
    int GetTickets() const;
    void SetTickets(int tickets);
    void AddTickets(int delta);
    int GetMaxTickets() const;
    void SetMaxTickets(int maxTickets);
    int GetGachaDust() const;
    void SetGachaDust(int value);
    void AddGachaDust(int delta);
    int GetGachaPityCounter() const;
    void SetGachaPityCounter(int value);
    void AddGachaPityCounter(int delta);
    int GetGachaRollSequence() const;
    int NextGachaRollSequence();
    const std::vector<PlayerDataManager::PlayerSaveData::GachaHistoryEntry>& GetGachaHistory() const;
    void AddGachaHistoryEntry(const PlayerDataManager::PlayerSaveData::GachaHistoryEntry& entry);
    const PlayerDataManager::PlayerSaveData& GetSaveData() const;
    PlayerDataManager::PlayerSaveData::TowerEnhancementState GetTowerEnhancements() const;
    void SetTowerEnhancements(const PlayerDataManager::PlayerSaveData::TowerEnhancementState& st);
    std::array<PlayerDataManager::PlayerSaveData::TowerAttachmentSlot, 3> GetTowerAttachments() const;
    void SetTowerAttachments(
        const std::array<PlayerDataManager::PlayerSaveData::TowerAttachmentSlot, 3>& slots);

    // ===== Consistency =====
    bool ValidateFormation(const FormationData& formation,
                           std::vector<std::string>* invalidCharacterIds = nullptr) const;

private:
    std::unique_ptr<entities::CharacterManager> characterManager_;
    std::unique_ptr<entities::ItemPassiveManager> itemPassiveManager_;
    std::unique_ptr<entities::StageManager> stageManager_;
    std::unique_ptr<entities::TowerAttachmentManager> towerAttachmentManager_;
    std::unique_ptr<PlayerDataManager> playerDataManager_;
    std::string characterJsonPath_;
    std::string itemPassiveJsonPath_;
    std::string stageJsonPath_;
    std::string playerSavePath_;
    std::string towerAttachmentJsonPath_;
    bool isInitialized_ = false;
    
    // 最後のクリア報酬レポート
    StageClearReport lastClearReport_;
};

} // namespace core
} // namespace game
