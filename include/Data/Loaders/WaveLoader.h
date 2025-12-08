#pragma once

#include <vector>

#include "Data/Definitions/WaveDef.h"
#include "Data/Loaders/DataLoaderBase.h"

namespace New::Data::Loaders {

class WaveLoader : public DataLoaderBase {
public:
  bool ParseFromJson(const nlohmann::json &json) override;
  bool RegisterTo(DefinitionRegistry &registry) override;

private:
  std::vector<WaveDef> defs_;
  bool GenerateFallback() override;
};

} // namespace New::Data::Loaders
