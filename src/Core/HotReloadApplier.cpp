#include "Core/HotReloadApplier.h"

namespace New::Core {

HotReloadApplier::HotReloadApplier(Validator validator, ApplyFunc apply,
                                   RollbackFunc rollback)
    : validator_(std::move(validator)), apply_(std::move(apply)),
      rollback_(std::move(rollback)) {}

bool HotReloadApplier::Apply(
    const nlohmann::json &next,
    New::Data::Validators::ValidationReport &outReport) {
  outReport = validator_(next);
  if (outReport.HasErrors()) {
    return false;
  }

  if (!apply_(next)) {
    if (hasLastGood_) {
      rollback_(lastGood_);
    }
    return false;
  }

  lastGood_ = next;
  hasLastGood_ = true;
  return true;
}

} // namespace New::Core
