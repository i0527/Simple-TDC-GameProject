#pragma once

#include "Data/Validators/ValidationResult.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace New::Data::Validators {

class SchemaValidator {
public:
  ValidationReport ValidateEntityDef(const nlohmann::json &json);
  ValidationReport ValidateWaveDef(const nlohmann::json &json);
  ValidationReport ValidateAbilityDef(const nlohmann::json &json);
  ValidationReport ValidateUILayoutDef(const nlohmann::json &json);

  // Editor-specific
  ValidationReport ValidateStates(const nlohmann::json &json);
  ValidationReport ValidateTimeline(const nlohmann::json &json);
  ValidationReport ValidateMetadata(const nlohmann::json &json);
  ValidationReport ValidateImagePaths(const nlohmann::json &json,
                                      const std::filesystem::path &basePath);

  const std::vector<ValidationIssue> &GetIssues() const { return issues_; }
  void Clear();

private:
  std::vector<ValidationIssue> issues_;
  std::unordered_map<std::string, bool> pathCache_;

  void AddIssue(Severity severity, const std::string &path,
                const std::string &message);

  bool RequireKeys(const nlohmann::json &obj,
                   const std::vector<std::string> &keys,
                   const std::string &path);
  bool RequireType(const nlohmann::json &obj, const std::string &key,
                   nlohmann::json::value_t type, const std::string &path,
                   Severity severity = Severity::Error);

  bool ValidatePathExists(const std::string &relativePath,
                          const std::filesystem::path &basePath,
                          const std::string &context, bool required);

  ValidationReport MakeReport() const;
};

} // namespace New::Data::Validators
