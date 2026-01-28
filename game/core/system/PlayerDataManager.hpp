#pragma once

// 標準ライブラリ
#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクト内
#include "../config/SharedContext.hpp"

namespace game {
namespace core {

namespace entities {
class CharacterManager;
class ItemPassiveManager;
class StageManager;
} // namespace entities

/// @brief プレイヤー永続データの管理（単一JSON）
///
/// 保存先: data/saves/player_save.json
/// 例外安全: JSONパースは必ず try-catch し、失敗時はデフォルト値で継続します。
class PlayerDataManager {
public:
    struct PassiveSlot {
        std::string id;
        int level = 1;
    };

    struct CharacterState {
        bool unlocked = false;
        int level = 1;
        std::array<PassiveSlot, 3> passives{};
        std::array<std::string, 3> equipment{};
    };

    struct PlayerSaveData {
        int version = 5;
        int gold = 10000; // 強化用の初期所持Gold（保存データに存在しない場合のデフォルト）
        int gems = 0; // プレミアム通貨
        int tickets = 30; // チケット（現状値）
        int maxTickets = 1000; // チケット最大値
        int gachaDust = 0;
        int gachaPityCounter = 0;
        int gachaRollSequence = 0;
        FormationData formation;
        std::unordered_map<std::string, CharacterState> characters;
        std::unordered_map<std::string, int> ownedEquipment;
        std::unordered_map<std::string, int> ownedPassives;
        std::unordered_map<std::string, int> ownedTowerAttachments;
        struct GachaHistoryEntry {
            int seq = 0;
            std::string equipmentId;
            std::string rarity;
            int countAfter = 0;
        };
        std::vector<GachaHistoryEntry> gachaHistory;

        struct StageState {
            bool isCleared = false;
            bool isLocked = true;
            int starsEarned = 0;
        };
        std::unordered_map<std::string, StageState> stages;

        // タワー強化（ホーム: タワー強化タブ）
        struct TowerAttachmentSlot {
            std::string id;
            int level = 1;
        };
        struct TowerEnhancementState {
            int towerHpLevel = 0;       // 城HP最大値
            int walletGrowthLevel = 0;  // お財布成長（最大値増加/秒）
            int costRegenLevel = 0;     // コスト回復（/秒）
            int allyAttackLevel = 0;    // 味方攻撃力（%）
            int allyHpLevel = 0;        // 味方HP（%）
        };
        TowerEnhancementState towerEnhancements{};
        std::array<TowerAttachmentSlot, 3> towerAttachments{};
    };

    PlayerDataManager() = default;
    ~PlayerDataManager() = default;

    bool LoadOrCreate(const std::string& filePath,
                      const entities::CharacterManager& characterManager,
                      const entities::ItemPassiveManager& itemPassiveManager,
                      const entities::StageManager& stageManager);

    bool Save() const;

    /// @brief 現在の保存データを SharedContext に反映（主に formation）
    void ApplyToSharedContext(SharedContext& ctx) const;

    /// @brief SharedContextのformationを保存データへ反映
    void SetFormationFromSharedContext(const FormationData& formation);

    /// @brief キャラ状態を取得（存在しない場合はデフォルトを返す）
    CharacterState GetCharacterState(const std::string& characterId) const;

    /// @brief キャラ状態を上書き（存在しない場合は作成）
    void SetCharacterState(const std::string& characterId, const CharacterState& state);

    /// @brief ステージ状態を取得（存在しない場合はデフォルトを返す）
    PlayerSaveData::StageState GetStageState(const std::string& stageId) const;

    /// @brief ステージ状態を上書き（存在しない場合は作成）
    void SetStageState(const std::string& stageId, const PlayerSaveData::StageState& state);

    int GetOwnedEquipmentCount(const std::string& equipmentId) const;
    int GetOwnedPassiveCount(const std::string& passiveId) const;
    int GetOwnedTowerAttachmentCount(const std::string& attachmentId) const;
    void SetOwnedEquipmentCount(const std::string& equipmentId, int count);
    void SetOwnedPassiveCount(const std::string& passiveId, int count);
    void SetOwnedTowerAttachmentCount(const std::string& attachmentId, int count);

    int GetGold() const { return data_.gold; }
    void SetGold(int gold) { data_.gold = std::max(0, gold); }
    void AddGold(int delta) { data_.gold = std::max(0, data_.gold + delta); }

    int GetGems() const { return data_.gems; }
    void SetGems(int gems) { data_.gems = std::max(0, gems); }
    void AddGems(int delta) { data_.gems = std::max(0, data_.gems + delta); }

    int GetTickets() const { return data_.tickets; }
    void SetTickets(int tickets) { data_.tickets = std::max(0, tickets); }
    void AddTickets(int delta) { data_.tickets = std::max(0, data_.tickets + delta); }

    int GetMaxTickets() const { return data_.maxTickets; }
    void SetMaxTickets(int maxTickets) { data_.maxTickets = std::max(0, maxTickets); }

    int GetGachaDust() const { return data_.gachaDust; }
    void SetGachaDust(int value) { data_.gachaDust = std::max(0, value); }
    void AddGachaDust(int delta) { data_.gachaDust = std::max(0, data_.gachaDust + delta); }

    int GetGachaPityCounter() const { return data_.gachaPityCounter; }
    void SetGachaPityCounter(int value) { data_.gachaPityCounter = std::max(0, value); }
    void AddGachaPityCounter(int delta) { data_.gachaPityCounter = std::max(0, data_.gachaPityCounter + delta); }

    int GetGachaRollSequence() const { return data_.gachaRollSequence; }
    int NextGachaRollSequence() { return ++data_.gachaRollSequence; }

    const std::vector<PlayerSaveData::GachaHistoryEntry>& GetGachaHistory() const { return data_.gachaHistory; }
    void AddGachaHistoryEntry(const PlayerSaveData::GachaHistoryEntry& entry);

    const PlayerSaveData& GetSaveData() const { return data_; }

    /// @brief タワー強化状態を取得
    PlayerSaveData::TowerEnhancementState GetTowerEnhancements() const { return data_.towerEnhancements; }

    /// @brief タワー強化状態を上書き
    void SetTowerEnhancements(const PlayerSaveData::TowerEnhancementState& st) { data_.towerEnhancements = st; }

    /// @brief タワーアタッチメント状態を取得
    std::array<PlayerSaveData::TowerAttachmentSlot, 3> GetTowerAttachments() const { return data_.towerAttachments; }

    /// @brief タワーアタッチメント状態を上書き
    void SetTowerAttachments(const std::array<PlayerSaveData::TowerAttachmentSlot, 3>& slots) {
        data_.towerAttachments = slots;
    }

private:
    std::string filePath_ = "data/saves/player_save.json";
    PlayerSaveData data_{};

    void EnsureDefaultsFromMasters(const entities::CharacterManager& characterManager,
                                  const entities::ItemPassiveManager& itemPassiveManager);
    void EnsureStageStatesFromMasters(const entities::StageManager& stageManager);
};

} // namespace core
} // namespace game

