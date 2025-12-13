#include "Game/Managers/FormationManager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace Game::Managers {

namespace {
constexpr const char *DEFAULT_FORMATION_PATH = "formation_debug.json";
constexpr int DEFAULT_MAX_SLOTS = 10; // 2 x 5 固定
}

FormationManager::FormationManager(Shared::Core::GameContext &context,
                                   Shared::Data::DefinitionRegistry &definitions)
    : context_(context), definitions_(definitions) {
  max_slots_ = DEFAULT_MAX_SLOTS;
  ResetToDefaults();
  LoadFromFile(DEFAULT_FORMATION_PATH);
}

bool FormationManager::LoadFromFile(const std::string &relative_path) {
  const std::string path = context_.GetDataPath(relative_path);
  try {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "[FormationManager] Failed to open file: " << path
                << std::endl;
      BuildDefaultCandidates();
      EnsureSlotSize();
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    max_slots_ = DEFAULT_MAX_SLOTS;

    if (j.contains("candidates") && j["candidates"].is_array()) {
      std::vector<std::string> ids;
      for (const auto &id : j["candidates"]) {
        ids.push_back(id.get<std::string>());
      }
      unlocked_ids_ = FilterValid(ids);
    }

    if (j.contains("slots") && j["slots"].is_array()) {
      std::vector<std::string> s;
      for (const auto &id : j["slots"]) {
        s.push_back(id.get<std::string>());
      }
      slots_ = FilterValid(s);
    }

    BuildDefaultCandidates();
    EnsureSlotSize();
    return true;
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "[FormationManager] JSON parse error: " << e.what()
              << " in " << path << std::endl;
    BuildDefaultCandidates();
    EnsureSlotSize();
    return false;
  } catch (const std::exception &e) {
    std::cerr << "[FormationManager] Error loading formation: " << e.what()
              << std::endl;
    BuildDefaultCandidates();
    EnsureSlotSize();
    return false;
  }
}

void FormationManager::ResetToDefaults() {
  unlocked_ids_.clear();
  slots_.clear();
  max_slots_ = DEFAULT_MAX_SLOTS;
  BuildDefaultCandidates();
  EnsureSlotSize();
}

bool FormationManager::AssignToSlot(int index, const std::string &entity_id) {
  if (!IsValidFriendly(entity_id)) {
    return false;
  }
  if (index < 0 || index >= max_slots_) {
    return false;
  }
  EnsureSlotSize();
  slots_[index] = entity_id;
  return true;
}

void FormationManager::ClearSlot(int index) {
  if (index < 0 || index >= max_slots_) {
    return;
  }
  EnsureSlotSize();
  slots_[index].clear();
}

void FormationManager::SwapSlots(int a, int b) {
  if (a < 0 || b < 0 || a >= max_slots_ || b >= max_slots_) {
    return;
  }
  EnsureSlotSize();
  std::swap(slots_[a], slots_[b]);
}

void FormationManager::SetSlots(const std::vector<std::string> &slots) {
  slots_ = FilterValid(slots);
  EnsureSlotSize();
}

void FormationManager::SetUnlocked(const std::vector<std::string> &ids) {
  unlocked_ids_ = FilterValid(ids);
  BuildDefaultCandidates();
}

const std::vector<std::string> &FormationManager::GetSlots() const {
  return slots_;
}

const std::vector<std::string> &FormationManager::GetCandidates() const {
  return unlocked_ids_;
}

int FormationManager::GetMaxSlots() const { return max_slots_; }

void FormationManager::ExportForSave(std::vector<std::string> &out_slots,
                                     std::vector<std::string> &out_unlocked) const {
  out_slots = slots_;
  out_unlocked = unlocked_ids_;
}

void FormationManager::EnsureSlotSize() {
  if (max_slots_ <= 0) {
    max_slots_ = 1;
  }
  if (static_cast<int>(slots_.size()) < max_slots_) {
    slots_.resize(static_cast<size_t>(max_slots_));
  } else if (static_cast<int>(slots_.size()) > max_slots_) {
    slots_.resize(static_cast<size_t>(max_slots_));
  }
}

void FormationManager::BuildDefaultCandidates() {
  if (!unlocked_ids_.empty()) {
    std::vector<std::string> filtered = FilterValid(unlocked_ids_);
    std::unordered_set<std::string> seen;
    std::vector<std::string> unique_ids;
    unique_ids.reserve(filtered.size());
    for (const auto &id : filtered) {
      if (seen.insert(id).second) {
        unique_ids.push_back(id);
      }
    }
    unlocked_ids_.swap(unique_ids);
    if (!unlocked_ids_.empty()) {
      return;
    }
  }

  unlocked_ids_.clear();
  for (const auto *def : definitions_.GetAllEntities()) {
    if (def && !def->is_enemy) {
      unlocked_ids_.push_back(def->id);
    }
  }
}

bool FormationManager::IsValidFriendly(const std::string &entity_id) const {
  if (entity_id.empty()) {
    return false;
  }
  const auto *def = definitions_.GetEntity(entity_id);
  if (def == nullptr) {
    return false;
  }
  if (def->is_enemy) {
    return false;
  }
  return true;
}

std::vector<std::string>
FormationManager::FilterValid(const std::vector<std::string> &ids) const {
  std::vector<std::string> out;
  out.reserve(ids.size());
  for (const auto &id : ids) {
    if (IsValidFriendly(id)) {
      out.push_back(id);
    }
  }
  return out;
}

} // namespace Game::Managers

