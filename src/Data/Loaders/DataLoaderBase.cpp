#include "Data/Loaders/DataLoaderBase.h"
#include "Core/TraceCompat.h"

#include <fstream>
#include <sstream>

namespace New::Data::Loaders {

bool DataLoaderBase::fallbackEnabled_ = true;

bool DataLoaderBase::LoadFromFile(const std::string &path) {
  nlohmann::json json;
  if (!ReadJsonFile(path, json)) {
    TRACELOG(LOG_WARNING,
             "DataLoaderBase: load failed, trying procedural fallback: %s",
             path.c_str());
    if (fallbackEnabled_) {
      return GenerateFallback();
    }
    TRACELOG(LOG_WARNING,
             "DataLoaderBase: fallback disabled, returning failure: %s",
             path.c_str());
    return false;
  }
  if (!ParseFromJson(json)) {
    TRACELOG(LOG_WARNING,
             "DataLoaderBase: parse failed, trying procedural fallback: %s",
             path.c_str());
    if (fallbackEnabled_) {
      return GenerateFallback();
    }
    TRACELOG(LOG_WARNING,
             "DataLoaderBase: fallback disabled, returning failure: %s",
             path.c_str());
    return false;
  }
  return true;
}

bool DataLoaderBase::ReadJsonFile(const std::string &path,
                                  nlohmann::json &outJson) {
  try {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
      TRACELOG(LOG_ERROR, "DataLoaderBase: failed to open file: %s",
               path.c_str());
      return false;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    ifs.close();

    outJson = nlohmann::json::parse(buffer.str());
    return true;
  } catch (const nlohmann::json::parse_error &e) {
    TRACELOG(LOG_ERROR, "DataLoaderBase: json parse error in %s: %s",
             path.c_str(), e.what());
  } catch (const std::exception &e) {
    TRACELOG(LOG_ERROR, "DataLoaderBase: exception reading %s: %s",
             path.c_str(), e.what());
  }
  return false;
}

} // namespace New::Data::Loaders
