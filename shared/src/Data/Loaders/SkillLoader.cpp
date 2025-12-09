#include "Data/Loaders/SkillLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Shared::Data {

bool SkillLoader::LoadFromJson(const std::string &json_path,
                               DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open skill file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    // Accept either top-level array or { "skills": [...] }
    if (j.is_array()) {
      // ok
    } else if (j.contains("skills") && j["skills"].is_array()) {
      j = j["skills"];
    } else {
      std::cerr
          << "Invalid skill JSON format: expected array or {\"skills\":[]}"
          << std::endl;
      return false;
    }

    for (const auto &skill_json : j) {
      if (!skill_json.is_object()) {
        std::cerr << "[SkillLoader] Skip non-object entry in " << json_path
                  << std::endl;
        continue;
      }
      try {
        SkillDef def;
        def.id = skill_json.value("id", "");
        if (def.id.empty()) {
          std::cerr << "[SkillLoader] Skip entry with empty id in " << json_path
                    << std::endl;
          continue;
        }
        def.name = skill_json.value("name", "");
        def.description = skill_json.value("description", "");
        def.type = skill_json.value("type", "passive");

        if (skill_json.contains("event_trigger") &&
            skill_json["event_trigger"].is_object()) {
          auto trigger = skill_json["event_trigger"];
          def.event_trigger.event_type = trigger.value("event_type", "");
          def.event_trigger.interval = trigger.value("interval", 0.0f);
        }

        if (skill_json.contains("effects") &&
            skill_json["effects"].is_array()) {
          for (const auto &effect_json : skill_json["effects"]) {
            SkillDef::Effect effect;
            effect.effect_type = effect_json.value("effect_type", "");
            if (effect_json.contains("parameters")) {
              effect.parameters = effect_json["parameters"];
            }
            def.effects.push_back(effect);
          }
        }

        if (skill_json.contains("target") && skill_json["target"].is_object()) {
          auto target = skill_json["target"];
          def.target.target_type = target.value("target_type", "self");
          def.target.range = target.value("range", 0);
          def.target.max_targets = target.value("max_targets", 1);
        }

        def.cooldown = skill_json.value("cooldown", 0.0f);
        def.activation_chance = skill_json.value("activation_chance", 1.0f);

        if (skill_json.contains("tags") && skill_json["tags"].is_array()) {
          for (const auto &tag : skill_json["tags"]) {
            def.tags.push_back(tag.get<std::string>());
          }
        }

        registry.RegisterSkill(def);
      } catch (const std::exception &e) {
        std::cerr << "[SkillLoader] Skip invalid skill entry: " << e.what()
                  << std::endl;
        continue;
      }
    }

    std::cout << "Loaded skills from: " << json_path << std::endl;
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in skill file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading skills: " << e.what() << std::endl;
    return false;
  }
}

bool SkillLoader::SaveToJson(const std::string &json_path,
                             const DefinitionRegistry &registry) {
  try {
    nlohmann::json j;
    nlohmann::json skills_array = nlohmann::json::array();

    for (const auto *skill : registry.GetAllSkills()) {
      nlohmann::json skill_json;
      skill_json["id"] = skill->id;
      skill_json["name"] = skill->name;
      skill_json["description"] = skill->description;
      skill_json["type"] = skill->type;

      skill_json["event_trigger"]["event_type"] =
          skill->event_trigger.event_type;
      skill_json["event_trigger"]["interval"] = skill->event_trigger.interval;

      nlohmann::json effects_array = nlohmann::json::array();
      for (const auto &effect : skill->effects) {
        nlohmann::json effect_json;
        effect_json["effect_type"] = effect.effect_type;
        effect_json["parameters"] = effect.parameters;
        effects_array.push_back(effect_json);
      }
      skill_json["effects"] = effects_array;

      skill_json["target"]["target_type"] = skill->target.target_type;
      skill_json["target"]["range"] = skill->target.range;
      skill_json["target"]["max_targets"] = skill->target.max_targets;

      skill_json["cooldown"] = skill->cooldown;
      skill_json["activation_chance"] = skill->activation_chance;
      skill_json["tags"] = skill->tags;

      skills_array.push_back(skill_json);
    }

    j["skills"] = skills_array;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path
                << std::endl;
      return false;
    }

    file << j.dump(2);
    std::cout << "Saved skills to: " << json_path << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving skills: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
