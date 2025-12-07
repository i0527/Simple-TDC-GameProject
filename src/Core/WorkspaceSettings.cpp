#include "Core/WorkspaceSettings.h"
#include <fstream>

namespace New::Core {

WorkspaceSettings WorkspaceSettings::Load(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) {
    return {};
  }

  std::ifstream in(path);
  if (!in.is_open()) {
    return {};
  }

  nlohmann::json json;
  in >> json;
  return FromJson(json);
}

bool WorkspaceSettings::Save(const std::filesystem::path &path) const {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path);
  if (!out.is_open()) {
    return false;
  }
  out << ToJson().dump(2);
  return true;
}

void WorkspaceSettings::MergeFrom(const WorkspaceSettings &projectDefaults) {
  for (const auto &[entity, state] : projectDefaults.statePreviewByEntity) {
    if (!statePreviewByEntity.count(entity)) {
      statePreviewByEntity[entity] = state;
    }
  }
  if (!lastPreviewState && projectDefaults.lastPreviewState) {
    lastPreviewState = projectDefaults.lastPreviewState;
  }
}

nlohmann::json WorkspaceSettings::ToJson() const {
  nlohmann::json json;
  json["statePreviewByEntity"] = statePreviewByEntity;
  if (lastPreviewState) {
    json["lastPreviewState"] = *lastPreviewState;
  }
  return json;
}

WorkspaceSettings WorkspaceSettings::FromJson(const nlohmann::json &json) {
  WorkspaceSettings settings;
  if (json.contains("statePreviewByEntity") &&
      json.at("statePreviewByEntity").is_object()) {
    settings.statePreviewByEntity =
        json.at("statePreviewByEntity")
            .get<std::unordered_map<std::string, std::string>>();
  }
  if (json.contains("lastPreviewState") &&
      json.at("lastPreviewState").is_string()) {
    settings.lastPreviewState = json.at("lastPreviewState").get<std::string>();
  }
  return settings;
}

} // namespace New::Core
