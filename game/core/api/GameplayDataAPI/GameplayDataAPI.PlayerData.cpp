#include "../GameplayDataAPI.hpp"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

namespace game {
namespace core {

namespace {
const std::vector<PlayerDataManager::PlayerSaveData::GachaHistoryEntry> kEmptyGachaHistory{};
} // namespace

bool GameplayDataAPI::Save() const {
    if (!playerDataManager_) {
        return false;
    }
    const bool ok = playerDataManager_->Save();
#if defined(__EMSCRIPTEN__)
    if (ok) {
        EM_ASM(Module.syncSaveToPersistent && Module.syncSaveToPersistent(););
    }
#endif
    return ok;
}

void GameplayDataAPI::ApplyToSharedContext(SharedContext& ctx) const {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->ApplyToSharedContext(ctx);
}

void GameplayDataAPI::SetFormationFromSharedContext(const FormationData& formation) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetFormationFromSharedContext(formation);
}

PlayerDataManager::CharacterState GameplayDataAPI::GetCharacterState(
    const std::string& characterId) const {
    if (!playerDataManager_) {
        return {};
    }
    return playerDataManager_->GetCharacterState(characterId);
}

void GameplayDataAPI::SetCharacterState(const std::string& characterId,
                                        const PlayerDataManager::CharacterState& state) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetCharacterState(characterId, state);
}

int GameplayDataAPI::GetOwnedEquipmentCount(const std::string& equipmentId) const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetOwnedEquipmentCount(equipmentId);
}

int GameplayDataAPI::GetOwnedPassiveCount(const std::string& passiveId) const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetOwnedPassiveCount(passiveId);
}

void GameplayDataAPI::SetOwnedEquipmentCount(const std::string& equipmentId, int count) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetOwnedEquipmentCount(equipmentId, count);
}

void GameplayDataAPI::SetOwnedPassiveCount(const std::string& passiveId, int count) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetOwnedPassiveCount(passiveId, count);
}

int GameplayDataAPI::GetOwnedTowerAttachmentCount(const std::string& attachmentId) const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetOwnedTowerAttachmentCount(attachmentId);
}

void GameplayDataAPI::SetOwnedTowerAttachmentCount(const std::string& attachmentId, int count) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetOwnedTowerAttachmentCount(attachmentId, count);
}

int GameplayDataAPI::GetGold() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetGold();
}

void GameplayDataAPI::SetGold(int gold) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetGold(gold);
}

void GameplayDataAPI::AddGold(int delta) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddGold(delta);
}

int GameplayDataAPI::GetGems() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetGems();
}

void GameplayDataAPI::SetGems(int gems) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetGems(gems);
}

void GameplayDataAPI::AddGems(int delta) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddGems(delta);
}

int GameplayDataAPI::GetTickets() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetTickets();
}

void GameplayDataAPI::SetTickets(int tickets) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetTickets(tickets);
}

void GameplayDataAPI::AddTickets(int delta) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddTickets(delta);
}

int GameplayDataAPI::GetMaxTickets() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetMaxTickets();
}

void GameplayDataAPI::SetMaxTickets(int maxTickets) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetMaxTickets(maxTickets);
}

int GameplayDataAPI::GetGachaDust() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetGachaDust();
}

void GameplayDataAPI::SetGachaDust(int value) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetGachaDust(value);
}

void GameplayDataAPI::AddGachaDust(int delta) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddGachaDust(delta);
}

int GameplayDataAPI::GetGachaPityCounter() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetGachaPityCounter();
}

void GameplayDataAPI::SetGachaPityCounter(int value) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetGachaPityCounter(value);
}

void GameplayDataAPI::AddGachaPityCounter(int delta) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddGachaPityCounter(delta);
}

int GameplayDataAPI::GetGachaRollSequence() const {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->GetGachaRollSequence();
}

int GameplayDataAPI::NextGachaRollSequence() {
    if (!playerDataManager_) {
        return 0;
    }
    return playerDataManager_->NextGachaRollSequence();
}

const std::vector<PlayerDataManager::PlayerSaveData::GachaHistoryEntry>&
GameplayDataAPI::GetGachaHistory() const {
    if (!playerDataManager_) {
        return kEmptyGachaHistory;
    }
    return playerDataManager_->GetGachaHistory();
}

void GameplayDataAPI::AddGachaHistoryEntry(
    const PlayerDataManager::PlayerSaveData::GachaHistoryEntry& entry) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->AddGachaHistoryEntry(entry);
}

const PlayerDataManager::PlayerSaveData& GameplayDataAPI::GetSaveData() const {
    if (!playerDataManager_) {
        static const PlayerDataManager::PlayerSaveData kEmptySave{};
        return kEmptySave;
    }
    return playerDataManager_->GetSaveData();
}

PlayerDataManager::PlayerSaveData::TowerEnhancementState
GameplayDataAPI::GetTowerEnhancements() const {
    if (!playerDataManager_) {
        return {};
    }
    return playerDataManager_->GetTowerEnhancements();
}

void GameplayDataAPI::SetTowerEnhancements(
    const PlayerDataManager::PlayerSaveData::TowerEnhancementState& st) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetTowerEnhancements(st);
}

std::array<PlayerDataManager::PlayerSaveData::TowerAttachmentSlot, 3>
GameplayDataAPI::GetTowerAttachments() const {
    if (!playerDataManager_) {
        return {};
    }
    return playerDataManager_->GetTowerAttachments();
}

void GameplayDataAPI::SetTowerAttachments(
    const std::array<PlayerDataManager::PlayerSaveData::TowerAttachmentSlot, 3>& slots) {
    if (!playerDataManager_) {
        return;
    }
    playerDataManager_->SetTowerAttachments(slots);
}

} // namespace core
} // namespace game
