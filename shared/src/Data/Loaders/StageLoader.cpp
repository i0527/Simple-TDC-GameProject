#include "Data/Loaders/StageLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Shared::Data {

bool StageLoader::LoadFromJson(const std::string &json_path,
                               DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open stage file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);
    bool used_wrapper_object = false;

    // Accept either top-level array or { "stages": [...] }
    if (j.is_array()) {
      // ok
    } else if (j.contains("stages") && j["stages"].is_array()) {
      j = j["stages"];
      used_wrapper_object = true;
    } else {
      std::cerr
          << "Invalid stage JSON format: expected array or {\"stages\":[]}"
          << std::endl;
      return false;
    }

    size_t loaded_count = 0;
    for (const auto &stage_json : j) {
      if (!stage_json.is_object()) {
        std::cerr << "[StageLoader] Skip non-object entry in " << json_path
                  << std::endl;
        continue;
      }
      StageDef def;
      def.id = stage_json.value("id", "");
      if (def.id.empty()) {
        std::cerr << "[StageLoader] Skip entry with empty id in " << json_path
                  << std::endl;
        continue;
      }
      def.name = stage_json.value("name", "");
      def.description = stage_json.value("description", "");
      def.difficulty = stage_json.value("difficulty", 1);
      def.domain = stage_json.value("domain", "");
      def.subdomain = stage_json.value("subdomain", 0);
      def.base_arrival_damage = stage_json.value("baseArrivalDamage", 1);

      if (stage_json.contains("environment")) {
        auto env = stage_json["environment"];
        def.environment.background_image = env.value("background_image", "");
        def.environment.bgm_id = env.value("bgm_id", "");
        def.environment.scroll_speed = env.value("scroll_speed", 0.0f);
      }

      if (stage_json.contains("castle_hp")) {
        auto castle = stage_json["castle_hp"];
        def.castle_hp.player_castle_hp =
            castle.value("player_castle_hp", 10000);
        def.castle_hp.enemy_castle_hp = castle.value("enemy_castle_hp", 10000);
      }

      if (stage_json.contains("wave_ids") &&
          stage_json["wave_ids"].is_array()) {
        for (const auto &wave_id : stage_json["wave_ids"]) {
          def.wave_ids.push_back(wave_id.get<std::string>());
        }
      } else if (stage_json.contains("waves") &&
                 stage_json["waves"].is_array()) {
        for (const auto &wave_id : stage_json["waves"]) {
          def.wave_ids.push_back(wave_id.get<std::string>());
        }
      }

      if (stage_json.contains("rewards")) {
        auto rewards = stage_json["rewards"];
        def.rewards.gold = rewards.value("gold", 0);
        def.rewards.exp = rewards.value("exp", 0);
        if (rewards.contains("items") && rewards["items"].is_array()) {
          for (const auto &item : rewards["items"]) {
            def.rewards.items.push_back(item.get<std::string>());
          }
        }
      }

      if (stage_json.contains("tags") && stage_json["tags"].is_array()) {
        for (const auto &tag : stage_json["tags"]) {
          def.tags.push_back(tag.get<std::string>());
        }
      }

      registry.RegisterStage(def);
      ++loaded_count;
      if (def.id.empty()) {
        std::cerr << "[StageLoader] Warning: stage with empty id in "
                  << json_path << std::endl;
      }
      if (def.domain.empty()) {
        std::cerr << "[StageLoader] Warning: stage \"" << def.id
                  << "\" missing domain; default \"\" may affect sorting"
                  << std::endl;
      }
    }

    std::cout << "Loaded " << loaded_count << " stages from " << json_path
              << " (" << (used_wrapper_object ? "object.stages" : "array")
              << ")" << std::endl;
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in stage file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading stages: " << e.what() << std::endl;
    return false;
  }
}

bool StageLoader::SaveToJson(const std::string &json_path,
                             const DefinitionRegistry &registry) {
  try {
    nlohmann::json j;
    nlohmann::json stages_array = nlohmann::json::array();

    for (const auto *stage : registry.GetAllStages()) {
      nlohmann::json stage_json;
      stage_json["id"] = stage->id;
      stage_json["name"] = stage->name;
      stage_json["description"] = stage->description;
      stage_json["difficulty"] = stage->difficulty;
      stage_json["domain"] = stage->domain;
      stage_json["subdomain"] = stage->subdomain;
      stage_json["baseArrivalDamage"] = stage->base_arrival_damage;

      stage_json["environment"]["background_image"] =
          stage->environment.background_image;
      stage_json["environment"]["bgm_id"] = stage->environment.bgm_id;
      stage_json["environment"]["scroll_speed"] =
          stage->environment.scroll_speed;

      stage_json["castle_hp"]["player_castle_hp"] =
          stage->castle_hp.player_castle_hp;
      stage_json["castle_hp"]["enemy_castle_hp"] =
          stage->castle_hp.enemy_castle_hp;

      stage_json["wave_ids"] = stage->wave_ids;

      stage_json["rewards"]["gold"] = stage->rewards.gold;
      stage_json["rewards"]["exp"] = stage->rewards.exp;
      stage_json["rewards"]["items"] = stage->rewards.items;

      stage_json["tags"] = stage->tags;

      stages_array.push_back(stage_json);
    }

    j["stages"] = stages_array;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path
                << std::endl;
      return false;
    }

    file << j.dump(2);
    std::cout << "Saved stages to: " << json_path << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving stages: " << e.what() << std::endl;
    return false;
  }
}

bool StageLoader::SaveSingleStage(const std::string &json_path,
                                  const StageDef &def) {
  try {
    nlohmann::json j;
    j["id"] = def.id;
    j["name"] = def.name;
    j["description"] = def.description;
    j["difficulty"] = def.difficulty;
    j["domain"] = def.domain;
    j["subdomain"] = def.subdomain;
    j["baseArrivalDamage"] = def.base_arrival_damage;

    j["environment"]["background_image"] = def.environment.background_image;
    j["environment"]["bgm_id"] = def.environment.bgm_id;
    j["environment"]["scroll_speed"] = def.environment.scroll_speed;

    j["castle_hp"]["player_castle_hp"] = def.castle_hp.player_castle_hp;
    j["castle_hp"]["enemy_castle_hp"] = def.castle_hp.enemy_castle_hp;

    j["wave_ids"] = def.wave_ids;

    j["rewards"]["gold"] = def.rewards.gold;
    j["rewards"]["exp"] = def.rewards.exp;
    j["rewards"]["items"] = def.rewards.items;

    j["tags"] = def.tags;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path << std::endl;
      return false;
    }

    file << j.dump(2);
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving stage: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
