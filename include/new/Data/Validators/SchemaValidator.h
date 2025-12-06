#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace New::Data::Validators {

class SchemaValidator {
public:
  bool ValidateEntityDef(const nlohmann::json &json);
  bool ValidateWaveDef(const nlohmann::json &json);
  bool ValidateAbilityDef(const nlohmann::json &json);
  bool ValidateUILayoutDef(const nlohmann::json &json);

  const std::vector<std::string> &GetErrors() const { return errors_; }
  void Clear() { errors_.clear(); }

private:
  std::vector<std::string> errors_;

  bool RequireKeys(const nlohmann::json &obj,
                   const std::vector<std::string> &keys,
                   const std::string &path);
  bool RequireType(const nlohmann::json &obj, const std::string &key,
                   nlohmann::json::value_t type, const std::string &path);
};

} // namespace New::Data::Validators
