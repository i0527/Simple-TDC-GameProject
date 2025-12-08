#include "Data/DefinitionRegistry.h"

namespace New::Data {

bool DefinitionRegistry::RegisterEntity(const EntityDef &def) {
  if (def.id.empty()) {
    return false;
  }
  return entities_.emplace(def.id, def).second;
}

const EntityDef *DefinitionRegistry::GetEntity(const std::string &id) const {
  auto it = entities_.find(id);
  if (it != entities_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DefinitionRegistry::HasEntity(const std::string &id) const {
  return entities_.find(id) != entities_.end();
}

bool DefinitionRegistry::RegisterWave(const WaveDef &def) {
  if (def.id.empty()) {
    return false;
  }
  return waves_.emplace(def.id, def).second;
}

const WaveDef *DefinitionRegistry::GetWave(const std::string &id) const {
  auto it = waves_.find(id);
  if (it != waves_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DefinitionRegistry::HasWave(const std::string &id) const {
  return waves_.find(id) != waves_.end();
}

bool DefinitionRegistry::RegisterAbility(const AbilityDef &def) {
  if (def.id.empty()) {
    return false;
  }
  return abilities_.emplace(def.id, def).second;
}

const AbilityDef *DefinitionRegistry::GetAbility(const std::string &id) const {
  auto it = abilities_.find(id);
  if (it != abilities_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DefinitionRegistry::HasAbility(const std::string &id) const {
  return abilities_.find(id) != abilities_.end();
}

bool DefinitionRegistry::RegisterStage(const StageDef &def) {
  if (def.id.empty()) {
    return false;
  }
  return stages_.emplace(def.id, def).second;
}

const StageDef *DefinitionRegistry::GetStage(const std::string &id) const {
  auto it = stages_.find(id);
  if (it != stages_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DefinitionRegistry::HasStage(const std::string &id) const {
  return stages_.find(id) != stages_.end();
}

bool DefinitionRegistry::RegisterDeck(const DeckDef &def) {
  if (def.id.empty()) {
    return false;
  }
  return decks_.emplace(def.id, def).second;
}

const DeckDef *DefinitionRegistry::GetDeck(const std::string &id) const {
  auto it = decks_.find(id);
  if (it != decks_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DefinitionRegistry::HasDeck(const std::string &id) const {
  return decks_.find(id) != decks_.end();
}

void DefinitionRegistry::Clear() {
  entities_.clear();
  waves_.clear();
  abilities_.clear();
  stages_.clear();
  decks_.clear();
}

} // namespace New::Data
