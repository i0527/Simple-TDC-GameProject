#include "Data/Loaders/AbilityLoader.h"
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

bool AbilityLoader::LoadFromJson(const std::string &json_path,
                                 DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open ability file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    // Accept either top-level array or { "abilities": [...] }
    if (j.is_array()) {
      // ok
    } else if (j.contains("abilities") && j["abilities"].is_array()) {
      j = j["abilities"];
    } else {
      std::cerr
          << "Invalid ability JSON format: expected array or {\"abilities\":[]}"
          << std::endl;
      return false;
    }

    for (const auto &ability_json : j) {
      if (!ability_json.is_object()) {
        std::cerr << "[AbilityLoader] Skip non-object entry in " << json_path
                  << std::endl;
        continue;
      }
      try {
        AbilityDef def;
        def.id = ValueOrDefault<std::string>(ability_json, "id", "");
        if (def.id.empty()) {
          std::cerr << "[AbilityLoader] Skip entry with empty id in "
                    << json_path << std::endl;
          continue;
        }
        def.name = ValueOrDefault<std::string>(ability_json, "name", "");
        def.description =
            ValueOrDefault<std::string>(ability_json, "description", "");
        def.type =
            ValueOrDefault<std::string>(ability_json, "type", "stat_boost");

        if (ability_json.contains("effect") &&
            ability_json["effect"].is_object()) {
          auto effect = ability_json["effect"];
          def.effect.stat_type =
              ValueOrDefault<std::string>(effect, "stat_type", "");
          def.effect.value = ValueOrDefault<float>(effect, "value", 0.0f);
          def.effect.is_percentage =
              ValueOrDefault<bool>(effect, "is_percentage", false);
        }

        if (ability_json.contains("tags") && ability_json["tags"].is_array()) {
          for (const auto &tag : ability_json["tags"]) {
            try {
              def.tags.push_back(tag.get<std::string>());
            } catch (...) {
              continue;
            }
          }
        }

        registry.RegisterAbility(def);
      } catch (const std::exception &e) {
        std::cerr << "[AbilityLoader] Skip invalid ability entry: " << e.what()
                  << std::endl;
        continue;
      }
    }

    std::cout << "Loaded abilities from: " << json_path << std::endl;
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in ability file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading abilities: " << e.what() << std::endl;
    return false;
  }
}

bool AbilityLoader::SaveToJson(const std::string &json_path,
                               const DefinitionRegistry &registry) {
  try {
    nlohmann::json j;
    nlohmann::json abilities_array = nlohmann::json::array();

    for (const auto *ability : registry.GetAllAbilities()) {
      nlohmann::json ability_json;
      ability_json["id"] = ability->id;
      ability_json["name"] = ability->name;
      ability_json["description"] = ability->description;
      ability_json["type"] = ability->type;

      ability_json["effect"]["stat_type"] = ability->effect.stat_type;
      ability_json["effect"]["value"] = ability->effect.value;
      ability_json["effect"]["is_percentage"] = ability->effect.is_percentage;

      ability_json["tags"] = ability->tags;

      abilities_array.push_back(ability_json);
    }

    j["abilities"] = abilities_array;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path
                << std::endl;
      return false;
    }

    file << j.dump(2);
    std::cout << "Saved abilities to: " << json_path << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving abilities: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
