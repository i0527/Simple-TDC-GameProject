#include "Data/Validators/MapValidator.h"

namespace New::Data::Validators {

namespace {
void AddIssue(std::vector<ValidationIssue> &issues, Severity severity,
              const std::string &path, const std::string &message) {
  issues.push_back({severity, path, message});
}

bool RequireArray(const nlohmann::json &obj, const std::string &key,
                  const std::string &path,
                  std::vector<ValidationIssue> &issues) {
  if (!obj.contains(key)) {
    AddIssue(issues, Severity::Error, path, "missing key '" + key + "'");
    return false;
  }
  if (!obj.at(key).is_array()) {
    AddIssue(issues, Severity::Error, path,
             "key '" + key + "' must be an array");
    return false;
  }
  return true;
}
} // namespace

ValidationReport MapValidator::ValidateMapTD(const nlohmann::json &json) {
  std::vector<ValidationIssue> issues;

  if (!json.is_object()) {
    AddIssue(issues, Severity::Error, "map", "map must be an object");
    return {issues};
  }

  RequireArray(json, "paths", "map.paths", issues);
  RequireArray(json, "tiles", "map.tiles", issues);
  RequireArray(json, "spawns", "map.spawns", issues);
  RequireArray(json, "waves", "map.waves", issues);

  if (json.contains("buildable") && !json.at("buildable").is_array()) {
    AddIssue(issues, Severity::Error, "map.buildable",
             "buildable must be an array");
  }

  // Minimal wave schedule check
  if (json.contains("waves") && json.at("waves").is_array()) {
    const auto &waves = json.at("waves");
    for (size_t i = 0; i < waves.size(); ++i) {
      const auto &wave = waves.at(i);
      const std::string path = "map.waves[" + std::to_string(i) + "]";
      if (!wave.is_object()) {
        AddIssue(issues, Severity::Error, path, "wave must be an object");
        continue;
      }
      if (!wave.contains("schedule")) {
        AddIssue(issues, Severity::Warning, path, "schedule missing");
      } else if (!wave.at("schedule").is_array()) {
        AddIssue(issues, Severity::Error, path,
                 "schedule must be an array of entries");
      }
    }
  }

  return {issues};
}

} // namespace New::Data::Validators
