#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"

namespace Game::Managers {

class FormationManager {
public:
  FormationManager(Shared::Core::GameContext &context,
                   Shared::Data::DefinitionRegistry &definitions);

  bool LoadFromFile(const std::string &relative_path);

  void ResetToDefaults();

  bool AssignToSlot(int index, const std::string &entity_id);
  void ClearSlot(int index);
  void SwapSlots(int a, int b);
  void SetSlots(const std::vector<std::string> &slots);
  void SetUnlocked(const std::vector<std::string> &ids);

  const std::vector<std::string> &GetSlots() const;
  const std::vector<std::string> &GetCandidates() const;
  int GetMaxSlots() const;

  void ExportForSave(std::vector<std::string> &out_slots,
                     std::vector<std::string> &out_unlocked) const;

private:
  Shared::Core::GameContext &context_;
  Shared::Data::DefinitionRegistry &definitions_;

  int max_slots_ = 10;
  std::vector<std::string> slots_;
  std::vector<std::string> unlocked_ids_;

  void EnsureSlotSize();
  void BuildDefaultCandidates();
  bool IsValidFriendly(const std::string &entity_id) const;
  std::vector<std::string> FilterValid(const std::vector<std::string> &ids) const;
};

} // namespace Game::Managers

