#pragma once

#include <vector>

#include "Data/Definitions/DeckDef.h"
#include "Data/Loaders/DataLoaderBase.h"

namespace New::Data::Loaders {

class DeckLoader : public DataLoaderBase {
public:
  bool ParseFromJson(const nlohmann::json &json) override;
  bool RegisterTo(DefinitionRegistry &registry) override;

protected:
  bool GenerateFallback() override;

private:
  std::vector<DeckDef> defs_;
};

} // namespace New::Data::Loaders
