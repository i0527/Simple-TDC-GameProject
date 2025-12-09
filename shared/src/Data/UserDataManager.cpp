#include "Data/UserDataManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace Shared::Data {

namespace {

template <typename T>
T ValueOrDefault(const nlohmann::json &j, const std::string &key,
                 const T &def) {
  if (j.contains(key)) {
    try {
      return j.at(key).get<T>();
    } catch (...) {
      return def;
    }
  }
  return def;
}

} // namespace

bool UserDataManager::Initialize(const std::string &base_directory) {
  base_dir_ = base_directory;
  try {
    if (!std::filesystem::exists(base_dir_)) {
      std::filesystem::create_directories(base_dir_);
    }
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[UserDataManager] Failed to init save dir '" << base_dir_
              << "': " << e.what() << std::endl;
    return false;
  }
}

bool UserDataManager::IsValidSlot(int slot_id) {
  return slot_id >= 0 && slot_id <= 99;
}

std::string UserDataManager::SlotPath(int slot_id) const {
  return base_dir_ + "/slot_" + std::to_string(slot_id) + ".json";
}

bool UserDataManager::SaveSlot(const SaveData &data) const {
  if (!IsValidSlot(data.slot_id)) {
    std::cerr << "[UserDataManager] Invalid slot id: " << data.slot_id
              << std::endl;
    return false;
  }

  nlohmann::json j;
  j["version"] = data.version;
  j["slot_id"] = data.slot_id;
  j["saved_at"] = data.saved_at;
  j["stage_progress"]["cleared_stage_ids"] =
      data.stage_progress.cleared_stage_ids;

  nlohmann::json chars = nlohmann::json::array();
  for (const auto &ch : data.characters) {
    nlohmann::json cj;
    cj["id"] = ch.id;
    cj["level"] = ch.level;
    cj["exp"] = ch.exp;
    cj["upgrades"] = ch.upgrades;
    chars.push_back(cj);
  }
  j["characters"] = chars;

  j["gold"] = data.gold;
  j["tower"]["hp_level"] = data.tower.hp_level;
  j["meta"]["save_manager_version"] = data.meta.save_manager_version;

  try {
    const std::string path = SlotPath(data.slot_id);
    std::ofstream ofs(path);
    if (!ofs.is_open()) {
      std::cerr << "[UserDataManager] Failed to open file for save: " << path
                << std::endl;
      return false;
    }
    ofs << j.dump(2);
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[UserDataManager] Save error: " << e.what() << std::endl;
    return false;
  }
}

bool UserDataManager::LoadSlot(int slot_id, SaveData &out_data) const {
  if (!IsValidSlot(slot_id)) {
    std::cerr << "[UserDataManager] Invalid slot id: " << slot_id << std::endl;
    return false;
  }
  const std::string path = SlotPath(slot_id);
  if (!std::filesystem::exists(path)) {
    return false;
  }

  try {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
      std::cerr << "[UserDataManager] Failed to open save file: " << path
                << std::endl;
      return false;
    }
    nlohmann::json j = nlohmann::json::parse(ifs);

    SaveData d;
    d.version = ValueOrDefault<int>(j, "version", 1);
    d.slot_id = ValueOrDefault<int>(j, "slot_id", slot_id);
    d.saved_at = ValueOrDefault<std::string>(j, "saved_at", "");

    if (j.contains("stage_progress")) {
      const auto &sp = j.at("stage_progress");
      d.stage_progress.cleared_stage_ids =
          ValueOrDefault<std::vector<std::string>>(sp, "cleared_stage_ids", {});
    }

    d.characters.clear();
    if (j.contains("characters") && j.at("characters").is_array()) {
      for (const auto &cj : j.at("characters")) {
        CharacterSave ch;
        ch.id = ValueOrDefault<std::string>(cj, "id", "");
        ch.level = ValueOrDefault<int>(cj, "level", 1);
        ch.exp = ValueOrDefault<int>(cj, "exp", 0);
        ch.upgrades =
            ValueOrDefault<std::vector<std::string>>(cj, "upgrades", {});
        d.characters.push_back(ch);
      }
    }

    d.gold = ValueOrDefault<int>(j, "gold", 0);

    if (j.contains("tower")) {
      const auto &tw = j.at("tower");
      d.tower.hp_level = ValueOrDefault<int>(tw, "hp_level", 1);
    }

    if (j.contains("meta")) {
      const auto &mt = j.at("meta");
      d.meta.save_manager_version =
          ValueOrDefault<int>(mt, "save_manager_version", 1);
    }

    out_data = d;
    return true;
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "[UserDataManager] Parse error: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "[UserDataManager] Load error: " << e.what() << std::endl;
    return false;
  }
}

bool UserDataManager::SlotExists(int slot_id) const {
  if (!IsValidSlot(slot_id)) {
    return false;
  }
  return std::filesystem::exists(SlotPath(slot_id));
}

std::vector<int> UserDataManager::ListExistingSlots() const {
  std::vector<int> slots;
  if (!std::filesystem::exists(base_dir_)) {
    return slots;
  }
  for (const auto &entry : std::filesystem::directory_iterator(base_dir_)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const auto name = entry.path().filename().string();
    if (name.rfind("slot_", 0) == 0 && name.size() > 5) {
      try {
        int id = std::stoi(name.substr(5, name.find_last_of('.') - 5));
        if (IsValidSlot(id)) {
          slots.push_back(id);
        }
      } catch (...) {
        // ignore parse errors
      }
    }
  }
  return slots;
}

} // namespace Shared::Data
