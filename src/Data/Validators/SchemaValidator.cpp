#include "Data/Validators/SchemaValidator.h"
#include <filesystem>
#include <unordered_set>

namespace New::Data::Validators {

ValidationReport
SchemaValidator::ValidateEntityDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "entities", nlohmann::json::value_t::array,
                   "entities")) {
    return MakeReport();
  }

  const auto &arr = json.at("entities");
  std::unordered_set<std::string> ids;
  for (size_t i = 0; i < arr.size(); ++i) {
    const auto &e = arr.at(i);
    const std::string base = "entities[" + std::to_string(i) + "]";
    if (!e.is_object()) {
      AddIssue(Severity::Error, base, "entity must be object");
      continue;
    }
    if (RequireType(e, "id", nlohmann::json::value_t::string, base)) {
      const auto id = e.at("id").get<std::string>();
      if (ids.count(id) > 0) {
        AddIssue(Severity::Warning, base, "duplicate id '" + id + "'");
      }
      ids.insert(id);
    }
    RequireType(e, "name", nlohmann::json::value_t::string, base,
                Severity::Warning);
    if (RequireType(e, "health", nlohmann::json::value_t::number_integer, base,
                    Severity::Warning)) {
      const int hp = e.at("health").get<int>();
      if (hp <= 0) {
        AddIssue(Severity::Warning, base, "health should be > 0");
      }
    }
  }
  return MakeReport();
}

ValidationReport SchemaValidator::ValidateWaveDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "waves", nlohmann::json::value_t::array, "waves")) {
    return MakeReport();
  }

  const auto &arr = json.at("waves");
  std::unordered_set<std::string> ids;
  for (size_t i = 0; i < arr.size(); ++i) {
    const auto &w = arr.at(i);
    const std::string base = "waves[" + std::to_string(i) + "]";
    if (!w.is_object()) {
      AddIssue(Severity::Error, base, "wave must be object");
      continue;
    }
    if (RequireType(w, "id", nlohmann::json::value_t::string, base)) {
      const auto id = w.at("id").get<std::string>();
      if (ids.count(id) > 0) {
        AddIssue(Severity::Warning, base, "duplicate id '" + id + "'");
      }
      ids.insert(id);
    }
    if (w.contains("entries")) {
      RequireType(w, "entries", nlohmann::json::value_t::array, base);
      if (w.at("entries").is_array()) {
        const auto &entries = w.at("entries");
        for (size_t j = 0; j < entries.size(); ++j) {
          const auto &entry = entries.at(j);
          const std::string eb = base + ".entries[" + std::to_string(j) + "]";
          if (!entry.is_object()) {
            AddIssue(Severity::Error, eb, "entry must be object");
            continue;
          }
          RequireType(entry, "enemyId", nlohmann::json::value_t::string, eb);
          if (entry.contains("delay")) {
            RequireType(entry, "delay", nlohmann::json::value_t::number_float,
                        eb);
            if (entry.at("delay").is_number()) {
              const double d = entry.at("delay").get<double>();
              if (d < 0.0) {
                AddIssue(Severity::Warning, eb, "delay should be >= 0");
              }
            }
          }
        }
      }
    } else {
      AddIssue(Severity::Error, base, "missing entries array");
    }
  }
  return MakeReport();
}

ValidationReport
SchemaValidator::ValidateAbilityDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "abilities", nlohmann::json::value_t::array,
                   "abilities")) {
    return MakeReport();
  }

  const auto &arr = json.at("abilities");
  std::unordered_set<std::string> ids;
  for (size_t i = 0; i < arr.size(); ++i) {
    const auto &a = arr.at(i);
    const std::string base = "abilities[" + std::to_string(i) + "]";
    if (!a.is_object()) {
      AddIssue(Severity::Error, base, "ability must be object");
      continue;
    }
    if (RequireType(a, "id", nlohmann::json::value_t::string, base)) {
      const auto id = a.at("id").get<std::string>();
      if (ids.count(id) > 0) {
        AddIssue(Severity::Warning, base, "duplicate id '" + id + "'");
      }
      ids.insert(id);
    }
    RequireType(a, "name", nlohmann::json::value_t::string, base,
                Severity::Warning);
    if (a.contains("description")) {
      RequireType(a, "description", nlohmann::json::value_t::string, base,
                  Severity::Warning);
    }
    if (a.contains("cost")) {
      RequireType(a, "cost", nlohmann::json::value_t::number_integer, base,
                  Severity::Warning);
      if (a.at("cost").is_number_integer() && a.at("cost").get<int>() < 0) {
        AddIssue(Severity::Warning, base, "cost should be >= 0");
      }
    }
    if (a.contains("cooldown")) {
      RequireType(a, "cooldown", nlohmann::json::value_t::number_float, base,
                  Severity::Warning);
      if (a.at("cooldown").is_number()) {
        const double cd = a.at("cooldown").get<double>();
        if (cd < 0.0) {
          AddIssue(Severity::Warning, base, "cooldown should be >= 0");
        }
      }
    }
    if (a.contains("power")) {
      if (a.at("power").is_number()) {
        const double p = a.at("power").get<double>();
        if (p < 0.0) {
          AddIssue(Severity::Warning, base, "power should be >= 0");
        }
      } else {
        AddIssue(Severity::Warning, base, "power has unexpected type");
      }
    }
    if (a.contains("type") && !a.at("type").is_string()) {
      AddIssue(Severity::Warning, base, "type should be string");
    }
    if (a.contains("element") && !a.at("element").is_string()) {
      AddIssue(Severity::Warning, base, "element should be string");
    }
    if (a.contains("target") && !a.at("target").is_string()) {
      AddIssue(Severity::Warning, base, "target should be string");
    }
    if (a.contains("critMultiplier")) {
      if (a.at("critMultiplier").is_number()) {
        const double cm = a.at("critMultiplier").get<double>();
        if (cm < 0.0) {
          AddIssue(Severity::Warning, base, "critMultiplier should be >= 0");
        }
      } else {
        AddIssue(Severity::Warning, base, "critMultiplier has unexpected type");
      }
    }
    if (a.contains("duration")) {
      if (a.at("duration").is_number()) {
        const double dur = a.at("duration").get<double>();
        if (dur < 0.0) {
          AddIssue(Severity::Warning, base, "duration should be >= 0");
        }
      } else {
        AddIssue(Severity::Warning, base, "duration has unexpected type");
      }
    }
    if (a.contains("isDot") && !a.at("isDot").is_boolean()) {
      AddIssue(Severity::Warning, base, "isDot should be boolean");
    }
    if (a.contains("isHot") && !a.at("isHot").is_boolean()) {
      AddIssue(Severity::Warning, base, "isHot should be boolean");
    }
    if (a.contains("modifiers")) {
      if (!a.at("modifiers").is_array()) {
        AddIssue(Severity::Warning, base, "modifiers should be an array");
      } else {
        const auto &mods = a.at("modifiers");
        for (size_t j = 0; j < mods.size(); ++j) {
          const auto &m = mods.at(j);
          const std::string mb = base + ".modifiers[" + std::to_string(j) + "]";
          if (!m.is_object()) {
            AddIssue(Severity::Warning, mb, "modifier must be object");
            continue;
          }
          RequireType(m, "stat", nlohmann::json::value_t::string, mb,
                      Severity::Warning);
          if (m.contains("amount")) {
            if (!m.at("amount").is_number_integer()) {
              AddIssue(Severity::Warning, mb, "amount must be integer");
            }
          } else {
            AddIssue(Severity::Warning, mb, "missing key 'amount'");
          }
        }
      }
    }
  }
  return MakeReport();
}

ValidationReport
SchemaValidator::ValidateUILayoutDef(const nlohmann::json &json) {
  Clear();
  RequireType(json, "layouts", nlohmann::json::value_t::array, "layouts");
  return MakeReport();
}

ValidationReport SchemaValidator::ValidateStates(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "states", nlohmann::json::value_t::array, "states")) {
    return MakeReport();
  }

  const auto &states = json.at("states");
  for (size_t i = 0; i < states.size(); ++i) {
    const auto &state = states.at(i);
    const std::string base = "states[" + std::to_string(i) + "]";
    RequireType(state, "id", nlohmann::json::value_t::string, base);
    RequireType(state, "source", nlohmann::json::value_t::string, base,
                Severity::Warning);
    if (state.contains("timeline")) {
      RequireType(state, "timeline", nlohmann::json::value_t::array,
                  base + ".timeline");
    }
  }

  return MakeReport();
}

ValidationReport SchemaValidator::ValidateTimeline(const nlohmann::json &json) {
  Clear();
  if (!json.is_array()) {
    AddIssue(Severity::Error, "timeline",
             "timeline should be an array of events");
    return MakeReport();
  }

  for (size_t i = 0; i < json.size(); ++i) {
    const auto &ev = json.at(i);
    const std::string base = "timeline[" + std::to_string(i) + "]";
    if (!ev.contains("time") || !ev.at("time").is_number()) {
      AddIssue(Severity::Error, base, "time must be a number");
    }
    RequireType(ev, "type", nlohmann::json::value_t::string, base);
    if (ev.contains("payload")) {
      RequireType(ev, "payload", nlohmann::json::value_t::object, base);
    }
  }

  return MakeReport();
}

ValidationReport SchemaValidator::ValidateMetadata(const nlohmann::json &json) {
  Clear();
  if (!json.is_object()) {
    AddIssue(Severity::Error, "metadata", "metadata must be an object");
    return MakeReport();
  }

  // Minimal required keys for encyclopedia/formation use-cases
  if (json.contains("encyclopedia")) {
    const auto &enc = json.at("encyclopedia");
    const std::string base = "metadata.encyclopedia";
    RequireType(enc, "title", nlohmann::json::value_t::string, base);
    RequireType(enc, "description", nlohmann::json::value_t::string, base,
                Severity::Warning);
  }

  if (json.contains("formation")) {
    const auto &formation = json.at("formation");
    const std::string base = "metadata.formation";
    RequireType(formation, "role", nlohmann::json::value_t::string, base);
    RequireType(formation, "required", nlohmann::json::value_t::boolean, base);
  }

  return MakeReport();
}

ValidationReport
SchemaValidator::ValidateImagePaths(const nlohmann::json &json,
                                    const std::filesystem::path &basePath) {
  Clear();
  if (!json.is_object()) {
    AddIssue(Severity::Error, "metadata", "metadata must be an object");
    return MakeReport();
  }

  const auto validateImageField = [&](const std::string &field, bool required) {
    if (!json.contains(field)) {
      if (required) {
        AddIssue(Severity::Error, "metadata." + field,
                 "required image path is missing");
      } else {
        AddIssue(Severity::Warning, "metadata." + field,
                 "optional image path is missing");
      }
      return;
    }
    if (!json.at(field).is_string()) {
      AddIssue(Severity::Error, "metadata." + field,
               "image path must be a string");
      return;
    }
    ValidatePathExists(json.at(field).get<std::string>(), basePath,
                       "metadata." + field, required);
  };

  validateImageField("icon", true);
  validateImageField("portrait", false);

  return MakeReport();
}

void SchemaValidator::Clear() { issues_.clear(); }

void SchemaValidator::AddIssue(Severity severity, const std::string &path,
                               const std::string &message) {
  issues_.push_back({severity, path, message});
}

bool SchemaValidator::RequireKeys(const nlohmann::json &obj,
                                  const std::vector<std::string> &keys,
                                  const std::string &path) {
  for (const auto &key : keys) {
    if (!obj.contains(key)) {
      AddIssue(Severity::Error, path, "missing key '" + key + "'");
    }
  }
  return issues_.empty();
}

bool SchemaValidator::RequireType(const nlohmann::json &obj,
                                  const std::string &key,
                                  nlohmann::json::value_t type,
                                  const std::string &path, Severity severity) {
  if (!obj.contains(key)) {
    AddIssue(severity, path, "missing key '" + key + "'");
    return false;
  }
  if (obj.at(key).type() != type) {
    AddIssue(severity, path, "key '" + key + "' has unexpected type");
    return false;
  }
  return true;
}

bool SchemaValidator::ValidatePathExists(const std::string &relativePath,
                                         const std::filesystem::path &basePath,
                                         const std::string &context,
                                         bool required) {
  if (relativePath.empty()) {
    AddIssue(required ? Severity::Error : Severity::Warning, context,
             "path is empty");
    return false;
  }

  auto it = pathCache_.find(relativePath);
  bool exists = false;
  if (it != pathCache_.end()) {
    exists = it->second;
  } else {
    const auto fullPath = basePath / relativePath;
    exists = std::filesystem::exists(fullPath);
    pathCache_[relativePath] = exists;
  }

  if (!exists) {
    AddIssue(required ? Severity::Error : Severity::Warning, context,
             "file not found: " + relativePath);
  }

  return exists;
}

ValidationReport SchemaValidator::MakeReport() const { return {issues_}; }

} // namespace New::Data::Validators
