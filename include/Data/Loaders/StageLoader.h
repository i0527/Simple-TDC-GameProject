#pragma once

#include <vector>

#include "Data/Definitions/StageDef.h"
#include "Data/Loaders/DataLoaderBase.h"

namespace New::Data::Loaders {

class StageLoader : public DataLoaderBase {
public:
  bool ParseFromJson(const nlohmann::json &json) override;
  bool RegisterTo(DefinitionRegistry &registry) override;

private:
  std::vector<StageDef> defs_;
  bool GenerateFallback() override;
};

} // namespace New::Data::Loaders
