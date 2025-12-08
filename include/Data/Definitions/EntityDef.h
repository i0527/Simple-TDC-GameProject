#pragma once

#include <string>

namespace New::Data {

struct EntityDef {
  std::string id;
  std::string name;
  int health = 0;
};

} // namespace New::Data
