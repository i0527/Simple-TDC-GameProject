#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Shared::Data {

struct CharacterSave {
  std::string id;
  int level = 1;
  int exp = 0;
  std::vector<std::string> upgrades;
};

struct StageProgress {
  std::vector<std::string> cleared_stage_ids;
};

struct TowerSave {
  int hp_level = 1;
};

struct SaveMeta {
  int save_manager_version = 1;
};

struct SaveData {
  int version = 1;
  int slot_id = 0;
  std::string saved_at; // ISO8601 string
  StageProgress stage_progress;
  std::vector<CharacterSave> characters;
  int gold = 0;
  TowerSave tower;
  SaveMeta meta;
};

/// @brief ユーザーデータの読み書きを担当（0〜99スロットのJSONセーブ）
class UserDataManager {
public:
  bool Initialize(const std::string &base_directory);

  bool SaveSlot(const SaveData &data) const;
  bool LoadSlot(int slot_id, SaveData &out_data) const;
  bool SlotExists(int slot_id) const;
  std::vector<int> ListExistingSlots() const;

  static bool IsValidSlot(int slot_id);

private:
  std::string base_dir_;
  std::string SlotPath(int slot_id) const;
};

} // namespace Shared::Data
