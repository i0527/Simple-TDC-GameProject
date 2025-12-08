#include "Data/Loaders/EntityLoader.h"
#include "Core/TraceCompat.h"

#include "Data/DefinitionRegistry.h"

namespace {
constexpr const char *kFieldId = "id";
constexpr const char *kFieldName = "name";
constexpr const char *kFieldHealth = "health";
} // namespace

namespace New::Data::Loaders {

bool EntityLoader::ParseFromJson(const nlohmann::json &json) {
  defs_.clear();

  if (!json.is_array()) {
    TRACELOG(LOG_ERROR, "EntityLoader: root must be an array");
    return false;
  }

  for (const auto &elem : json) {
    if (!elem.is_object()) {
      TRACELOG(LOG_WARNING, "EntityLoader: skipped non-object element");
      continue;
    }
    EntityDef def{};
    if (!elem.contains(kFieldId) || !elem[kFieldId].is_string()) {
      TRACELOG(LOG_ERROR, "EntityLoader: 'id' is required and must be string");
      return false;
    }
    def.id = elem[kFieldId].get<std::string>();

    if (elem.contains(kFieldName) && elem[kFieldName].is_string()) {
      def.name = elem[kFieldName].get<std::string>();
    }
    if (elem.contains(kFieldHealth) && elem[kFieldHealth].is_number_integer()) {
      def.health = elem[kFieldHealth].get<int>();
    }

    defs_.push_back(std::move(def));
  }

  if (defs_.empty()) {
    TRACELOG(LOG_WARNING,
             "EntityLoader: no valid entities parsed (empty or skipped)");
    return false; // triggers fallback in LoadFromFile
  }

  return true;
}

bool EntityLoader::RegisterTo(DefinitionRegistry &registry) {
  bool ok = true;
  int insertedCount = 0;
  for (const auto &def : defs_) {
    const bool inserted = registry.RegisterEntity(def);
    if (!inserted) {
      TRACELOG(LOG_WARNING, "EntityLoader: duplicate or invalid id: %s",
               def.id.c_str());
      ok = false;
    } else {
      ++insertedCount;
    }
  }

  if (insertedCount == 0) {
    TRACELOG(LOG_WARNING,
             "EntityLoader: no entities registered, generating fallback");
    defs_.clear();
    if (GenerateFallback()) {
      for (const auto &def : defs_) {
        if (registry.RegisterEntity(def)) {
          ++insertedCount;
        }
      }
    }
    ok = ok && insertedCount > 0;
  }

  return ok;
}

bool EntityLoader::GenerateFallback() {
  defs_.clear();
  EntityDef def{};
  def.id = "fallback_entity";
  def.name = "Fallback Entity";
  def.health = 100;
  defs_.push_back(def);
  TRACELOG(LOG_INFO, "EntityLoader: generated procedural fallback entity");
  return true;
}

} // namespace New::Data::Loaders
