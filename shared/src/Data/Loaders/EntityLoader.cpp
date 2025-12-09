#include "Data/Loaders/EntityLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Shared::Data {

bool EntityLoader::LoadFromJson(const std::string &json_path,
                                DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open entity file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    // Accept either top-level array or { "entities": [...] }
    if (j.is_array()) {
      // ok
    } else if (j.contains("entities") && j["entities"].is_array()) {
      j = j["entities"];
    } else {
      std::cerr
          << "Invalid entity JSON format: expected array or {\"entities\":[]}"
          << std::endl;
      return false;
    }

    for (const auto &entity_json : j) {
      if (!entity_json.is_object()) {
        std::cerr << "[EntityLoader] Skip non-object entry in " << json_path
                  << std::endl;
        continue;
      }
      try {
        EntityDef def;
        def.id = entity_json.value("id", "");
        if (def.id.empty()) {
          std::cerr << "[EntityLoader] Skip entry with empty id in "
                    << json_path << std::endl;
          continue;
        }
        def.name = entity_json.value("name", "");
        def.description = entity_json.value("description", "");
        def.rarity = entity_json.value("rarity", 1);
        def.type = entity_json.value("type", "main");
        def.is_enemy = entity_json.value("is_enemy", false);

        def.cost = entity_json.value("cost", 0);
        def.cooldown = entity_json.value("cooldown", 0.0f);

        if (entity_json.contains("stats") && entity_json["stats"].is_object()) {
          auto stats = entity_json["stats"];
          def.stats.hp = stats.value("hp", 100);
          def.stats.attack = stats.value("attack", 10);
          def.stats.attack_speed = stats.value("attack_speed", 1.0f);
          def.stats.range = stats.value("range", 100);
          def.stats.move_speed = stats.value("move_speed", 50.0f);
          def.stats.knockback = stats.value("knockback", 0);
        }

        def.draw_type = entity_json.value("draw_type", "sprite");

        if (entity_json.contains("display") &&
            entity_json["display"].is_object()) {
          auto display = entity_json["display"];
          def.display.sprite_sheet = display.value("sprite_sheet", "");
          def.display.idle_animation = display.value("idle_animation", "");
          def.display.walk_animation = display.value("walk_animation", "");
          def.display.attack_animation = display.value("attack_animation", "");
          def.display.death_animation = display.value("death_animation", "");
        }

        if (entity_json.contains("skill_ids") &&
            entity_json["skill_ids"].is_array()) {
          for (const auto &skill_id : entity_json["skill_ids"]) {
            def.skill_ids.push_back(skill_id.get<std::string>());
          }
        }

        if (entity_json.contains("ability_ids") &&
            entity_json["ability_ids"].is_array()) {
          for (const auto &ability_id : entity_json["ability_ids"]) {
            def.ability_ids.push_back(ability_id.get<std::string>());
          }
        }

        if (entity_json.contains("tags") && entity_json["tags"].is_array()) {
          for (const auto &tag : entity_json["tags"]) {
            def.tags.push_back(tag.get<std::string>());
          }
        }

        registry.RegisterEntity(def);
      } catch (const std::exception &e) {
        std::cerr << "[EntityLoader] Skip invalid entity entry: " << e.what()
                  << std::endl;
        continue;
      }
    }

    std::cout << "Loaded entities from: " << json_path << std::endl;
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in entity file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading entities: " << e.what() << std::endl;
    return false;
  }
}

bool EntityLoader::SaveToJson(const std::string &json_path,
                              const DefinitionRegistry &registry) {
  try {
    nlohmann::json j;
    nlohmann::json entities_array = nlohmann::json::array();

    for (const auto *entity : registry.GetAllEntities()) {
      nlohmann::json entity_json;
      entity_json["id"] = entity->id;
      entity_json["name"] = entity->name;
      entity_json["description"] = entity->description;
      entity_json["rarity"] = entity->rarity;
      entity_json["type"] = entity->type;
      entity_json["is_enemy"] = entity->is_enemy;

      entity_json["cost"] = entity->cost;
      entity_json["cooldown"] = entity->cooldown;

      entity_json["stats"]["hp"] = entity->stats.hp;
      entity_json["stats"]["attack"] = entity->stats.attack;
      entity_json["stats"]["attack_speed"] = entity->stats.attack_speed;
      entity_json["stats"]["range"] = entity->stats.range;
      entity_json["stats"]["move_speed"] = entity->stats.move_speed;
      entity_json["stats"]["knockback"] = entity->stats.knockback;

      entity_json["draw_type"] = entity->draw_type;

      entity_json["display"]["sprite_sheet"] = entity->display.sprite_sheet;
      entity_json["display"]["idle_animation"] = entity->display.idle_animation;
      entity_json["display"]["walk_animation"] = entity->display.walk_animation;
      entity_json["display"]["attack_animation"] =
          entity->display.attack_animation;
      entity_json["display"]["death_animation"] =
          entity->display.death_animation;

      entity_json["skill_ids"] = entity->skill_ids;
      entity_json["ability_ids"] = entity->ability_ids;
      entity_json["tags"] = entity->tags;

      entities_array.push_back(entity_json);
    }

    j["entities"] = entities_array;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path
                << std::endl;
      return false;
    }

    file << j.dump(2);
    std::cout << "Saved entities to: " << json_path << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving entities: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
