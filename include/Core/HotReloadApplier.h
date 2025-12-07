#pragma once

#include "Data/Validators/ValidationResult.h"
#include <functional>
#include <nlohmann/json.hpp>

namespace New::Core {

class HotReloadApplier {
public:
  using Validator = std::function<New::Data::Validators::ValidationReport(
      const nlohmann::json &)>;
  using ApplyFunc = std::function<bool(const nlohmann::json &)>;
  using RollbackFunc = std::function<void(const nlohmann::json &)>;

  HotReloadApplier(Validator validator, ApplyFunc apply, RollbackFunc rollback);

  bool Apply(const nlohmann::json &next,
             New::Data::Validators::ValidationReport &outReport);

  const nlohmann::json &LastGood() const { return lastGood_; }
  bool HasLastGood() const { return hasLastGood_; }

private:
  nlohmann::json lastGood_{};
  bool hasLastGood_ = false;
  Validator validator_;
  ApplyFunc apply_;
  RollbackFunc rollback_;
};

} // namespace New::Core
