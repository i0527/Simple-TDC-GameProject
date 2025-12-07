#include "Data/Validators/SchemaValidator.h"
#include <filesystem>

namespace New::Data::Validators {

ValidationReport
SchemaValidator::ValidateEntityDef(const nlohmann::json &json) {
  Clear();
  RequireType(json, "entities", nlohmann::json::value_t::array, "entities");
  return MakeReport();
}

ValidationReport SchemaValidator::ValidateWaveDef(const nlohmann::json &json) {
  Clear();
  RequireType(json, "waves", nlohmann::json::value_t::array, "waves");
  return MakeReport();
}

ValidationReport
SchemaValidator::ValidateAbilityDef(const nlohmann::json &json) {
  Clear();
  RequireType(json, "abilities", nlohmann::json::value_t::array, "abilities");
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
