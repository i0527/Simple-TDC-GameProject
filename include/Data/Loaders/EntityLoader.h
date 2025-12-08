#pragma once

#include <vector>

#include "Data/Definitions/EntityDef.h"
#include "Data/Loaders/DataLoaderBase.h"

namespace New::Data::Loaders {

class EntityLoader : public DataLoaderBase {
public:
  bool ParseFromJson(const nlohmann::json &json) override;
  bool RegisterTo(DefinitionRegistry &registry) override;

private:
  std::vector<EntityDef> defs_;
  bool GenerateFallback() override;
};

} // namespace New::Data::Loaders
