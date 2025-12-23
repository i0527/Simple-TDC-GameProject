#include "Data/Loaders/EntityLoader.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <unordered_set>

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
      // Accept either top-level array, {"entities": [...]}, or single object
    if (j.is_array()) {
      // ok
    } else if (j.contains("entities") && j["entities"].is_array()) {
      j = j["entities"];
        } else if (j.is_object() && j.contains("id")) {
          // Single entity object - wrap in array
          j = nlohmann::json::array({j});
    } else {
      std::cerr
          << "Invalid entity JSON format: expected array, {\"entities\":[]}, or single entity object"
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
        def.source_path = json_path;
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

        if (entity_json.contains("combat") && entity_json["combat"].is_object()) {
          auto combat = entity_json["combat"];
          def.combat.attack_point = combat.value("attack_point", 0.0f);
          def.combat.attack_frame = combat.value("attack_frame", -1);
          if (combat.contains("hitbox") && combat["hitbox"].is_object()) {
            auto hitbox = combat["hitbox"];
            def.combat.hitbox.width = hitbox.value("width", 32.0f);
            def.combat.hitbox.height = hitbox.value("height", 32.0f);
            def.combat.hitbox.offset_x = hitbox.value("offset_x", 0.0f);
            def.combat.hitbox.offset_y = hitbox.value("offset_y", 0.0f);
          }
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
          def.display.atlas_texture = display.value("atlas_texture", "");
          def.display.icon = display.value("icon", "");
          def.display.mirror_h = display.value("mirror_h", false);
          def.display.mirror_v = display.value("mirror_v", false);
          // 新形式 animations (固定4アニメ想定)
          if (display.contains("animations") && display["animations"].is_object()) {
            for (auto it = display["animations"].begin(); it != display["animations"].end(); ++it) {
              if (!it.value().is_object()) continue;
              Shared::Data::EntityDef::Display::AnimClip clip;
              const auto &obj = it.value();
              clip.atlas = obj.value("atlas", "");
              clip.json = obj.value("json", "");
              clip.loop = obj.value("loop", true);
              clip.mirror_h = obj.value("mirror_h", false);
              clip.mirror_v = obj.value("mirror_v", false);
              def.display.animations[it.key()] = clip;
            }
          }
          // アクション別ミラー既定
          if (display.contains("mirror_by_action") && display["mirror_by_action"].is_object()) {
            for (auto it = display["mirror_by_action"].begin(); it != display["mirror_by_action"].end(); ++it) {
              const std::string action = it.key();
              if (it.value().is_object()) {
                const auto &obj = it.value();
                def.display.action_mirror_h[action] = obj.value("h", false);
                def.display.action_mirror_v[action] = obj.value("v", false);
              }
            }
          }
          if (display.contains("sprite_actions") &&
              display["sprite_actions"].is_object()) {
            for (auto it = display["sprite_actions"].begin();
                 it != display["sprite_actions"].end(); ++it) {
              if (it.value().is_string()) {
                def.display.sprite_actions[it.key()] = it.value().get<std::string>();
              }
            }
          }
          // 新形式から旧形式へフォールバック連携（atlas/json指定をsprite_actionsへ反映）
          if (!def.display.animations.empty() && def.display.sprite_actions.empty()) {
            for (const auto &kv : def.display.animations) {
              if (!kv.second.json.empty()) {
                def.display.sprite_actions[kv.first] = kv.second.json;
              }
            }
          }
          // atlas_texture が空なら animations の先頭atlasを採用
          if (def.display.atlas_texture.empty() && !def.display.animations.empty()) {
            for (const auto &kv : def.display.animations) {
              if (!kv.second.atlas.empty()) { def.display.atlas_texture = kv.second.atlas; break; }
            }
          }
          // 開発用分割スプライト管理
          def.display.dev_animation_config_path = display.value("dev_animation_config_path", "");
          def.display.use_dev_mode = display.value("use_dev_mode", false);
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

bool EntityLoader::LoadFromDirectory(const std::string &dir_path,
                                     DefinitionRegistry &registry,
                                     bool allow_glob) {
  namespace fs = std::filesystem;
  try {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
      std::cerr << "[EntityLoader] Directory not found: " << dir_path
                << std::endl;
      return false;
    }

    bool loaded_any = false;
    for (const auto &entry : fs::directory_iterator(dir_path)) {
      if (!entry.is_directory()) {
        continue;
      }
      const auto character_dir = entry.path();
      std::vector<fs::path> candidates;
      candidates.push_back(character_dir / "character.json");
      if (allow_glob) {
        for (const auto &sub : fs::directory_iterator(character_dir)) {
          if (sub.is_regular_file() &&
              sub.path().filename().string().find(".character.json") !=
                  std::string::npos) {
            candidates.push_back(sub.path());
          }
        }
      }

      bool loaded = false;
      for (const auto &file_path : candidates) {
        if (!fs::exists(file_path)) {
          continue;
        }
        loaded = LoadSingleCharacter(file_path.string(), registry);
        if (loaded) {
          loaded_any = true;
          break; // 最初に見つけたファイルを採用
        }
      }
      if (!loaded) {
        std::cerr << "[EntityLoader] No character.json found in "
                  << character_dir.string() << std::endl;
      }
    }
    return loaded_any;
  } catch (const std::exception &e) {
    std::cerr << "[EntityLoader] Error reading directory " << dir_path << ": "
              << e.what() << std::endl;
    return false;
  }
}

bool EntityLoader::LoadSingleCharacter(const std::string &json_path,
                                       DefinitionRegistry &registry) {
  try {
    std::ifstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open entity file: " << json_path << std::endl;
      return false;
    }

    nlohmann::json entity_json = nlohmann::json::parse(file);
    if (!entity_json.is_object()) {
      std::cerr << "[EntityLoader] character.json must be object: " << json_path
                << std::endl;
      return false;
    }

    EntityDef def;
    def.id = entity_json.value("id", "");
    if (def.id.empty()) {
      std::cerr << "[EntityLoader] Skip entry with empty id in " << json_path
                << std::endl;
      return false;
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

    // ベースディレクトリを前置してAsepriteパスを解決
    const auto base_dir = std::filesystem::path(json_path).parent_path();

    if (entity_json.contains("display") && entity_json["display"].is_object()) {
      auto display = entity_json["display"];
      def.display.sprite_sheet = display.value("sprite_sheet", "");
      def.display.idle_animation = display.value("idle_animation", "");
      def.display.walk_animation = display.value("walk_animation", "");
      def.display.attack_animation = display.value("attack_animation", "");
      def.display.death_animation = display.value("death_animation", "");
      def.display.atlas_texture = display.value("atlas_texture", "");
      def.display.icon = display.value("icon", "");
      def.display.mirror_h = display.value("mirror_h", false);
      def.display.mirror_v = display.value("mirror_v", false);
      if (display.contains("mirror_by_action") && display["mirror_by_action"].is_object()) {
        for (auto it = display["mirror_by_action"].begin(); it != display["mirror_by_action"].end(); ++it) {
          const std::string action = it.key();
          if (it.value().is_object()) {
            const auto &obj = it.value();
            def.display.action_mirror_h[action] = obj.value("h", false);
            def.display.action_mirror_v[action] = obj.value("v", false);
          }
        }
      }
      if (display.contains("sprite_actions") &&
          display["sprite_actions"].is_object()) {
        for (auto it = display["sprite_actions"].begin();
             it != display["sprite_actions"].end(); ++it) {
          if (it.value().is_string()) {
            auto resolved =
                (base_dir / it.value().get<std::string>()).lexically_normal();
            def.display.sprite_actions[it.key()] = resolved.string();
          }
        }
      }
      if (!def.display.atlas_texture.empty() &&
          !std::filesystem::path(def.display.atlas_texture).is_absolute()) {
        def.display.atlas_texture =
            (base_dir / def.display.atlas_texture).lexically_normal().string();
      }
      if (def.display.icon.empty()) {
        std::vector<std::string> icon_candidates = {"icon.png", "Icon.png",
                                                    "icon.jpg", "Icon.jpg"};
        for (const auto &cand : icon_candidates) {
          auto p = base_dir / cand;
          if (std::filesystem::exists(p)) {
            def.display.icon = p.lexically_normal().string();
            break;
          }
        }
      }
      if (!def.display.icon.empty() &&
          !std::filesystem::path(def.display.icon).is_absolute()) {
        def.display.icon =
            (base_dir / def.display.icon).lexically_normal().string();
      }
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
    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in entity file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error loading entity: " << e.what() << std::endl;
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

      entity_json["combat"]["attack_point"] = entity->combat.attack_point;
      entity_json["combat"]["attack_frame"] = entity->combat.attack_frame;
      entity_json["combat"]["hitbox"]["width"] = entity->combat.hitbox.width;
      entity_json["combat"]["hitbox"]["height"] = entity->combat.hitbox.height;
      entity_json["combat"]["hitbox"]["offset_x"] = entity->combat.hitbox.offset_x;
      entity_json["combat"]["hitbox"]["offset_y"] = entity->combat.hitbox.offset_y;

      entity_json["draw_type"] = entity->draw_type;

      // display設定（相対パスに正規化）
      auto normalizePathForJson = [](const std::string& path) -> std::string {
        if (path.empty()) return "";
        // バックスラッシュをスラッシュに統一
        std::string normalized = path;
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        return normalized;
      };

      entity_json["display"]["sprite_sheet"] = normalizePathForJson(entity->display.sprite_sheet);
      entity_json["display"]["idle_animation"] = normalizePathForJson(entity->display.idle_animation);
      entity_json["display"]["walk_animation"] = normalizePathForJson(entity->display.walk_animation);
      entity_json["display"]["attack_animation"] = normalizePathForJson(entity->display.attack_animation);
      entity_json["display"]["death_animation"] = normalizePathForJson(entity->display.death_animation);
      entity_json["display"]["atlas_texture"] = normalizePathForJson(entity->display.atlas_texture);
      entity_json["display"]["icon"] = normalizePathForJson(entity->display.icon);
      entity_json["display"]["mirror_h"] = entity->display.mirror_h;
      entity_json["display"]["mirror_v"] = entity->display.mirror_v;
      // アクション別ミラー既定の書き出し
      {
        nlohmann::json mirror_by_action = nlohmann::json::object();
        // 可能なアクション集合（sprite_actions優先）
        std::unordered_set<std::string> actions;
        for (const auto &kv : entity->display.sprite_actions) actions.insert(kv.first);
        for (const auto &kv : entity->display.action_mirror_h) actions.insert(kv.first);
        for (const auto &kv : entity->display.action_mirror_v) actions.insert(kv.first);
        for (const auto &name : actions) {
          nlohmann::json obj;
          obj["h"] = entity->display.action_mirror_h.count(name) ? entity->display.action_mirror_h.at(name) : false;
          obj["v"] = entity->display.action_mirror_v.count(name) ? entity->display.action_mirror_v.at(name) : false;
          mirror_by_action[name] = obj;
        }
        entity_json["display"]["mirror_by_action"] = mirror_by_action;
      }
      
      // sprite_actionsも正規化
      nlohmann::json sprite_actions_json;
      for (const auto& [actionName, jsonPath] : entity->display.sprite_actions) {
        sprite_actions_json[actionName] = normalizePathForJson(jsonPath);
      }
      entity_json["display"]["sprite_actions"] = sprite_actions_json;

      // animations (新形式)
      if (!entity->display.animations.empty()) {
        nlohmann::json anims = nlohmann::json::object();
        for (const auto &kv : entity->display.animations) {
          nlohmann::json a;
          a["atlas"] = normalizePathForJson(kv.second.atlas);
          a["json"] = normalizePathForJson(kv.second.json);
          a["loop"] = kv.second.loop;
          a["mirror_h"] = kv.second.mirror_h;
          a["mirror_v"] = kv.second.mirror_v;
          anims[kv.first] = a;
        }
        entity_json["display"]["animations"] = anims;
      }

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

bool EntityLoader::SaveSingleEntity(const std::string &json_path,
                                    const EntityDef &def) {
  try {
    nlohmann::json j;
    j["id"] = def.id;
    j["name"] = def.name;
    j["description"] = def.description;
    j["rarity"] = def.rarity;
    j["type"] = def.type;
    j["is_enemy"] = def.is_enemy;
    j["cost"] = def.cost;
    j["cooldown"] = def.cooldown;
    j["draw_type"] = def.draw_type;

    j["stats"]["hp"] = def.stats.hp;
    j["stats"]["attack"] = def.stats.attack;
    j["stats"]["attack_speed"] = def.stats.attack_speed;
    j["stats"]["range"] = def.stats.range;
    j["stats"]["move_speed"] = def.stats.move_speed;
    j["stats"]["knockback"] = def.stats.knockback;

    j["combat"]["attack_point"] = def.combat.attack_point;
    j["combat"]["attack_frame"] = def.combat.attack_frame;
    j["combat"]["hitbox"]["width"] = def.combat.hitbox.width;
    j["combat"]["hitbox"]["height"] = def.combat.hitbox.height;
    j["combat"]["hitbox"]["offset_x"] = def.combat.hitbox.offset_x;
    j["combat"]["hitbox"]["offset_y"] = def.combat.hitbox.offset_y;

    j["display"]["sprite_sheet"] = def.display.sprite_sheet;
    j["display"]["idle_animation"] = def.display.idle_animation;
    j["display"]["walk_animation"] = def.display.walk_animation;
    j["display"]["attack_animation"] = def.display.attack_animation;
    j["display"]["death_animation"] = def.display.death_animation;
    j["display"]["atlas_texture"] = def.display.atlas_texture;
    j["display"]["icon"] = def.display.icon;
    j["display"]["sprite_actions"] = def.display.sprite_actions;
    j["display"]["mirror_h"] = def.display.mirror_h;
    j["display"]["mirror_v"] = def.display.mirror_v;
    if (!def.display.animations.empty()) {
      nlohmann::json anims = nlohmann::json::object();
      for (const auto &kv : def.display.animations) {
        nlohmann::json a;
        a["atlas"] = kv.second.atlas;
        a["json"] = kv.second.json;
        a["loop"] = kv.second.loop;
        a["mirror_h"] = kv.second.mirror_h;
        a["mirror_v"] = kv.second.mirror_v;
        anims[kv.first] = a;
      }
      j["display"]["animations"] = anims;
    }
    {
      nlohmann::json mirror_by_action = nlohmann::json::object();
      for (const auto &kv : def.display.action_mirror_h) {
        nlohmann::json obj;
        obj["h"] = kv.second;
        obj["v"] = def.display.action_mirror_v.count(kv.first) ? def.display.action_mirror_v.at(kv.first) : false;
        mirror_by_action[kv.first] = obj;
      }
      for (const auto &kv : def.display.action_mirror_v) {
        if (!mirror_by_action.contains(kv.first)) {
          nlohmann::json obj;
          obj["h"] = def.display.action_mirror_h.count(kv.first) ? def.display.action_mirror_h.at(kv.first) : false;
          obj["v"] = kv.second;
          mirror_by_action[kv.first] = obj;
        }
      }
      j["display"]["mirror_by_action"] = mirror_by_action;
    }

    j["skill_ids"] = def.skill_ids;
    j["ability_ids"] = def.ability_ids;
    j["tags"] = def.tags;

    std::ofstream file(json_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << json_path << std::endl;
      return false;
    }

    file << j.dump(2);
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error saving entity: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Data
