#include "Data/Validators/SchemaValidator.h"

namespace New::Data::Validators {

bool SchemaValidator::ValidateEntityDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "entities", nlohmann::json::value_t::array,
                   "entities"))
    return false;
  return errors_.empty();
}

bool SchemaValidator::ValidateWaveDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "waves", nlohmann::json::value_t::array, "waves"))
    return false;
  return errors_.empty();
}

bool SchemaValidator::ValidateAbilityDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "abilities", nlohmann::json::value_t::array,
                   "abilities"))
    return false;
  return errors_.empty();
}

bool SchemaValidator::ValidateUILayoutDef(const nlohmann::json &json) {
  Clear();
  if (!RequireType(json, "layouts", nlohmann::json::value_t::array, "layouts"))
    return false;
  return errors_.empty();
}

bool SchemaValidator::RequireKeys(const nlohmann::json &obj,
                                  const std::vector<std::string> &keys,
                                  const std::string &path) {
  for (const auto &key : keys) {
    if (!obj.contains(key)) {
      errors_.push_back(path + ": missing key '" + key + "'");
    }
  }
  return errors_.empty();
}

bool SchemaValidator::RequireType(const nlohmann::json &obj,
                                  const std::string &key,
                                  nlohmann::json::value_t type,
                                  const std::string &path) {
  if (!obj.contains(key)) {
    errors_.push_back(path + ": missing key '" + key + "'");
    return false;
  }
  if (obj.at(key).type() != type) {
    errors_.push_back(path + ": key '" + key + "' has unexpected type");
    return false;
  }
  return true;
}

} // namespace New::Data::Validators
