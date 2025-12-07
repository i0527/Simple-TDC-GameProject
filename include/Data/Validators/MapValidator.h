#pragma once

#include "Data/Validators/ValidationResult.h"
#include <nlohmann/json.hpp>

namespace New::Data::Validators {

class MapValidator {
public:
  ValidationReport ValidateMapTD(const nlohmann::json &json);
};

} // namespace New::Data::Validators
