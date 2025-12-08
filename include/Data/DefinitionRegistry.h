#pragma once

#include <string>
#include <unordered_map>

#include "Data/Definitions/AbilityDef.h"
#include "Data/Definitions/DeckDef.h"
#include "Data/Definitions/EntityDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/WaveDef.h"

namespace New::Data {

class DefinitionRegistry {
public:
  DefinitionRegistry() = default;
  ~DefinitionRegistry() = default;

  // Entity
  bool RegisterEntity(const EntityDef &def);
  const EntityDef *GetEntity(const std::string &id) const;
  bool HasEntity(const std::string &id) const;

  // Wave
  bool RegisterWave(const WaveDef &def);
  const WaveDef *GetWave(const std::string &id) const;
  bool HasWave(const std::string &id) const;

  // Ability
  bool RegisterAbility(const AbilityDef &def);
  const AbilityDef *GetAbility(const std::string &id) const;
  bool HasAbility(const std::string &id) const;

  // Stage
  bool RegisterStage(const StageDef &def);
  const StageDef *GetStage(const std::string &id) const;
  bool HasStage(const std::string &id) const;

  // Deck
  bool RegisterDeck(const DeckDef &def);
  const DeckDef *GetDeck(const std::string &id) const;
  bool HasDeck(const std::string &id) const;

  void Clear();

private:
  std::unordered_map<std::string, EntityDef> entities_;
  std::unordered_map<std::string, WaveDef> waves_;
  std::unordered_map<std::string, AbilityDef> abilities_;
  std::unordered_map<std::string, StageDef> stages_;
  std::unordered_map<std::string, DeckDef> decks_;
};

} // namespace New::Data
