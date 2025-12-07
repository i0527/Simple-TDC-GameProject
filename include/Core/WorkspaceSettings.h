#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>

namespace New::Core {

struct WorkspaceSettings {
  // Which state to use for preview per entity/clip
  std::unordered_map<std::string, std::string> statePreviewByEntity;
  std::optional<std::string> lastPreviewState;

  static WorkspaceSettings Load(const std::filesystem::path &path);
  bool Save(const std::filesystem::path &path) const;

  // Merge user-local settings with project defaults; user settings win.
  void MergeFrom(const WorkspaceSettings &projectDefaults);

  nlohmann::json ToJson() const;
  static WorkspaceSettings FromJson(const nlohmann::json &json);
};

} // namespace New::Core
