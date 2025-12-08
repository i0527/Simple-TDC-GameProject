#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "Core/TraceCompat.h"

namespace New::Data {
class DefinitionRegistry;
} // namespace New::Data

namespace New::Data::Loaders {

class DataLoaderBase {
public:
  virtual ~DataLoaderBase() = default;

  bool LoadFromFile(const std::string &path);
  virtual bool ParseFromJson(const nlohmann::json &json) = 0;
  virtual bool RegisterTo(DefinitionRegistry &registry) = 0;

  // テスト時などにフォールバック生成を抑止したい場合に使用
  static void SetFallbackEnabled(bool enabled) { fallbackEnabled_ = enabled; }
  static bool IsFallbackEnabled() { return fallbackEnabled_; }

protected:
  bool ReadJsonFile(const std::string &path, nlohmann::json &outJson);
  // 読み込みに失敗した際のフォールバックデータ生成。未実装ならfalseを返す。
  virtual bool GenerateFallback() { return false; }

private:
  static bool fallbackEnabled_;
};

} // namespace New::Data::Loaders
