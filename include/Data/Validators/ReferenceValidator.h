#pragma once

#include "Data/Validators/ValidationResult.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace New::Data::Validators {

class ReferenceValidator {
public:
  ValidationReport ValidateStateClipConsistency(const nlohmann::json &states,
                                                const nlohmann::json &clips);

  ValidationReport ValidateEventPayloads(
      const nlohmann::json &timeline,
      const std::unordered_map<std::string, std::vector<std::string>>
          &requiredFieldsByEvent);

  ValidationReport ValidateTimelineScopes(const nlohmann::json &timeline);

  ValidationReport
  ValidateHudBindings(const nlohmann::json &layout,
                      const std::vector<std::string> &required);
};

} // namespace New::Data::Validators
