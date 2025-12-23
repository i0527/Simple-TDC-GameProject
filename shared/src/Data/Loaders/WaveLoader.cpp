#include "Data/Loaders/WaveLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Shared::Data {

bool WaveLoader::LoadFromJson(const std::string &json_path,
                              DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open wave file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    // Accept either top-level array or { "waves": [...] }
    if (j.is_array()) {
      // ok
    } else if (j.contains("waves") && j["waves"].is_array()) {
      j = j["waves"];
    } else {
      std::cerr << "Invalid wave JSON format: expected array or {\"waves\":[]}"
                << std::endl;
      return false;
    }

    for (const auto &wave_json : j) {
      WaveDef def;
      def.id = wave_json.value("id", "");
      def.stage_id = wave_json.value("stage_id", "");
      def.wave_number = wave_json.value("wave_number", 1);

      if (wave_json.contains("spawn_groups") &&
          wave_json["spawn_groups"].is_array()) {
        for (const auto &group_json : wave_json["spawn_groups"]) {
          WaveDef::SpawnGroup group;
          group.entity_id = group_json.value("entity_id", "");
          group.count = group_json.value("count", 1);
          group.spawn_interval = group_json.value("spawn_interval", 1.0f);
          group.delay_from_wave_start =
              group_json.value("delay_from_wave_start", 0.0f);
          def.spawn_groups.push_back(group);
        }
      }

      def.duration = wave_json.value("duration", 0.0f);

      if (wave_json.contains("tags") && wave_json["tags"].is_array()) {
        for (const auto &tag : wave_json["tags"]) {
          def.tags.push_back(tag.get<std::string>());
        }
      }

      registry.RegisterWave(def);
    }

    std::cout << "Loaded waves from: " << json_path << std::endl;
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in wave file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading waves: " << e.what() << std::endl;
    return false;
  }
}

bool WaveLoader::SaveToJson(const std::string &json_path,
                            const DefinitionRegistry &registry) {
  try {
    nlohmann::json j;
    nlohmann::json waves_array = nlohmann::json::array();

    for (const auto *wave : registry.GetAllWaves()) {
      nlohmann::json wave_json;
      wave_json["id"] = wave->id;
      wave_json["stage_id"] = wave->stage_id;
      wave_json["wave_number"] = wave->wave_number;

      nlohmann::json groups_array = nlohmann::json::array();
      for (const auto &group : wave->spawn_groups) {
        nlohmann::json group_json;
        group_json["entity_id"] = group.entity_id;
        group_json["count"] = group.count;
        group_json["spawn_interval"] = group.spawn_interval;
        group_json["delay_from_wave_start"] = group.delay_from_wave_start;
        groups_array.push_back(group_json);
      }
      wave_json["spawn_groups"] = groups_array;

      wave_json["duration"] = wave->duration;
      wave_json["tags"] = wave->tags;

      waves_array.push_back(wave_json);
    }

    j["waves"] = waves_array;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path
                << std::endl;
      return false;
    }

    file << j.dump(2);
    std::cout << "Saved waves to: " << json_path << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving waves: " << e.what() << std::endl;
    return false;
  }
}

bool WaveLoader::SaveSingleWave(const std::string &json_path,
                                const WaveDef &def) {
  try {
    nlohmann::json j;
    j["id"] = def.id;
    j["stage_id"] = def.stage_id;
    j["wave_number"] = def.wave_number;

    nlohmann::json groups_array = nlohmann::json::array();
    for (const auto &group : def.spawn_groups) {
      nlohmann::json group_json;
      group_json["entity_id"] = group.entity_id;
      group_json["count"] = group.count;
      group_json["spawn_interval"] = group.spawn_interval;
      group_json["delay_from_wave_start"] = group.delay_from_wave_start;
      groups_array.push_back(group_json);
    }
    j["spawn_groups"] = groups_array;

    j["duration"] = def.duration;
    j["tags"] = def.tags;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path << std::endl;
      return false;
    }

    file << j.dump(2);
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving wave: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
