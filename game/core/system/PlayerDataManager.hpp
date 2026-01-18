#pragma once

// 標準ライブラリ
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
        int version = 1;
        FormationData formation;
        std::unordered_map<std::string, CharacterState> characters;
        std::unordered_map<std::string, int> ownedEquipment;
        std::unordered_map<std::string, int> ownedPassives;
    };

    PlayerDataManager() = default;
    ~PlayerDataManager() = default;

    bool LoadOrCreate(const std::string& filePath,
                      const entities::CharacterManager& characterManager,
                      const entities::ItemPassiveManager& itemPassiveManager);

    bool Save() const;

    /// @brief 現在の保存データを SharedContext に反映（主に formation）
    void ApplyToSharedContext(SharedContext& ctx) const;

    /// @brief SharedContextのformationを保存データへ反映
    void SetFormationFromSharedContext(const FormationData& formation);

    /// @brief キャラ状態を取得（存在しない場合はデフォルトを返す）
    CharacterState GetCharacterState(const std::string& characterId) const;

    /// @brief キャラ状態を上書き（存在しない場合は作成）
    void SetCharacterState(const std::string& characterId, const CharacterState& state);

    int GetOwnedEquipmentCount(const std::string& equipmentId) const;
    int GetOwnedPassiveCount(const std::string& passiveId) const;
    void SetOwnedEquipmentCount(const std::string& equipmentId, int count);
    void SetOwnedPassiveCount(const std::string& passiveId, int count);

    const PlayerSaveData& GetSaveData() const { return data_; }

private:
    std::string filePath_ = "data/saves/player_save.json";
    PlayerSaveData data_{};

    void EnsureDefaultsFromMasters(const entities::CharacterManager& characterManager,
                                  const entities::ItemPassiveManager& itemPassiveManager);
};

} // namespace core
} // namespace game

