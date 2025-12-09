# Shared Layer 詳細仕様 - 最適化設計（ハイブリッドアプローチ）

**プロジェクト**: SimpleTDCGame_NewArch  
**目的**: 既存設計 + AI提案を統合した最適Shared Layer仕様  
**バージョン**: 1.0.0（最終版）  
**更新日**: 2025-12-08

---

## 📑 目次

1. [設計方針](#設計方針)
2. [設計選択の最終結論](#設計選択の最終結論)
3. [GameContext（ハイブリッド型）](#gamecontextハイブリッド型)
4. [EventSystem（型安全文字列型）](#eventsystem型安全文字列型)
5. [Validator（チェーン型）](#validatorチェーン型)
6. [FileWatcher（EventSystem統合型）](#filewatchereventsystem統合型)
7. [Expected型（エラーハンドリング）](#expected型エラーハンドリング)
8. [Loader（各独立型）](#loaderの各独立型実装)
9. [DefinitionRegistry](#definitionregistry)
10. [実装優先度](#実装優先度)

---

## 設計方針

### 基本原則

```yaml
設計哲学:
  "既存実装の良さを保ちながら、モダンC++とハイブリッドアプローチで拡張性を実現"

3つの柱:
  1. 互換性: デフォルトコンストラクタで既存コード動作
  2. 拡張性: 設定・チェーン・イベントで柔軟対応
  3. 安全性: 型安全・Expected型・バリデーションチェーン

優先順位:
  ✅ テスト容易性
  ✅ シンプルさ
  ✅ パフォーマンス
  ✅ 拡張性
```

### Shared Layerの責務（確認）

```
含まれるもの:
  ✅ ファイル I/O（JSON ロード・保存）
  ✅ データ定義クラス（EntityDef など 7種類）
  ✅ JSON ↔ C++ マッピング（Loader）
  ✅ バリデーション（チェーン型）
  ✅ イベントシステム（型安全）
  ✅ ファイル監視
  ✅ エラーハンドリング（Expected型）

含まれないもの:
  ❌ ゲーム固有ロジック（ECS など）
  ❌ エディタ UI（ImGui など）
  ❌ Raylib 依存
  ❌ ゲーム状態管理
```

---

## 設計選択の最終結論

| 項目 | 選択 | 理由 |
|------|------|------|
| **GameContext** | C: ハイブリッド型 | シンプル + 拡張性 |
| **EventSystem** | C: 型安全文字列型 | 型安全 + 柔軟性 |
| **Loader** | A: 各独立 | テスト容易性・責務分離 |
| **Validator** | C: チェーン型 | 柔軟性 + 詳細エラー |
| **FileWatcher** | B: EventSystem統合型 | 一貫性 + デバッグ性 |
| **エラー処理** | C: Expected型 | モダンC++ + 安全性 |

**推奨C++標準**: C++17以上  
**依存ライブラリ**: nlohmann/json のみ

---

## GameContext（ハイブリッド型）

### 設計概要

既存のシンプル型の良さを保ちながら、設定で拡張可能に。

```cpp
// shared/include/Core/GameContext.h
#pragma once

#include <memory>
#include <string>
#include <filesystem>

namespace Shared::Core {

// Forward declarations
class EventSystem;
class FileWatcher;

// 軽量な設定構造体（オプション）
struct GameContextConfig {
  std::string data_path = "assets/definitions";
  std::string assets_path = "assets";
  std::string config_file = "config.json";
  
  bool enable_file_watch = true;
  bool debug_mode = false;
};

class GameContext {
private:
  GameContextConfig config_;
  
  std::unique_ptr<EventSystem> event_system_;
  std::unique_ptr<FileWatcher> file_watcher_;
  
  bool initialized_ = false;

public:
  // デフォルトコンストラクタ（既存互換）
  GameContext() = default;
  
  // 設定付きコンストラクタ（拡張性）
  explicit GameContext(const GameContextConfig& config)
    : config_(config) {}
  
  ~GameContext() = default;
  
  // 初期化
  bool Initialize();
  bool Initialize(const std::string& config_path);
  void Shutdown();
  
  bool IsInitialized() const { return initialized_; }
  
  // パス管理（std::filesystem使用）
  std::filesystem::path GetDataPath(const std::string& relative = "") const;
  std::filesystem::path GetAssetsPath(const std::string& relative = "") const;
  std::string GetConfigPath() const { return config_.config_file; }
  
  // マネージャーアクセス
  EventSystem& GetEventSystem();
  const EventSystem& GetEventSystem() const;
  
  FileWatcher& GetFileWatcher();
  const FileWatcher& GetFileWatcher() const;
  
  // デバッグモード
  bool IsDebugMode() const { return config_.debug_mode; }
  void SetDebugMode(bool enable) { config_.debug_mode = enable; }

private:
  bool LoadConfigFile(const std::string& path);
  bool SetupPaths();
};

} // namespace Shared::Core
```

### 実装例

```cpp
// shared/src/Core/GameContext.cpp
#include "Core/GameContext.h"
#include "Core/EventSystem.h"
#include "Core/FileWatcher.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Shared::Core {

bool GameContext::Initialize() {
  if (initialized_) return true;
  
  event_system_ = std::make_unique<EventSystem>();
  file_watcher_ = std::make_unique<FileWatcher>(event_system_.get());
  
  initialized_ = true;
  return true;
}

bool GameContext::Initialize(const std::string& config_path) {
  if (!Initialize()) return false;
  
  return LoadConfigFile(config_path);
}

void GameContext::Shutdown() {
  if (file_watcher_) {
    file_watcher_->Clear();
  }
  if (event_system_) {
    event_system_->Clear();
  }
  initialized_ = false;
}

std::filesystem::path GameContext::GetDataPath(const std::string& relative) const {
  auto path = std::filesystem::path(config_.data_path);
  if (!relative.empty()) {
    path /= relative;
  }
  return path;
}

std::filesystem::path GameContext::GetAssetsPath(const std::string& relative) const {
  auto path = std::filesystem::path(config_.assets_path);
  if (!relative.empty()) {
    path /= relative;
  }
  return path;
}

EventSystem& GameContext::GetEventSystem() {
  if (!event_system_) {
    Initialize();
  }
  return *event_system_;
}

const EventSystem& GameContext::GetEventSystem() const {
  return *event_system_;
}

FileWatcher& GameContext::GetFileWatcher() {
  if (!file_watcher_) {
    const_cast<GameContext*>(this)->Initialize();
  }
  return *file_watcher_;
}

const FileWatcher& GameContext::GetFileWatcher() const {
  return *file_watcher_;
}

bool GameContext::LoadConfigFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return false;
  }
  
  try {
    nlohmann::json json;
    file >> json;
    
    config_.data_path = json.value("data_path", config_.data_path);
    config_.assets_path = json.value("assets_path", config_.assets_path);
    config_.debug_mode = json.value("debug_mode", config_.debug_mode);
    
    return true;
  } catch (...) {
    return false;
  }
}

} // namespace Shared::Core
```

### メリット

- ✅ デフォルトコンストラクタで既存コード互換
- ✅ 設定構造体で柔軟な初期化
- ✅ `std::filesystem::path`でクロスプラットフォーム対応
- ✅ シンプルさと拡張性の両立

---

## EventSystem（型安全文字列型）

### 設計概要

文字列ベースだが、定数管理で型安全性を実現。購読解除IDで厳密な管理。

```cpp
// shared/include/Core/Events.h
#pragma once

namespace Shared::Core::Events {

// イベント ID 定義（コンパイル時定数）
constexpr const char* EntityLoaded = "EntityLoaded";
constexpr const char* SkillLoaded = "SkillLoaded";
constexpr const char* StageLoaded = "StageLoaded";
constexpr const char* EffectLoaded = "EffectLoaded";
constexpr const char* AbilityLoaded = "AbilityLoaded";

constexpr const char* FileChanged = "FileChanged";
constexpr const char* ValidationFailed = "ValidationFailed";
constexpr const char* DataReloaded = "DataReloaded";

} // namespace Shared::Core::Events
```

```cpp
// shared/include/Core/EventSystem.h
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace Shared::Core {

using EventData = nlohmann::json;
using EventCallback = std::function<void(const EventData&)>;

class EventSystem {
private:
  struct Subscription {
    size_t id;
    EventCallback callback;
  };
  
  std::unordered_map<std::string, std::vector<Subscription>> subscribers_;
  size_t next_id_ = 0;

public:
  EventSystem() = default;
  ~EventSystem() = default;
  
  // イベント購読（戻り値は解除用ID）
  size_t Subscribe(const std::string& event_type, EventCallback callback) {
    size_t id = next_id_++;
    subscribers_[event_type].push_back({id, std::move(callback)});
    return id;
  }
  
  // イベント発行
  void Emit(const std::string& event_type, const EventData& data = EventData()) {
    auto it = subscribers_.find(event_type);
    if (it == subscribers_.end()) return;
    
    for (const auto& sub : it->second) {
      if (sub.callback) {
        sub.callback(data);
      }
    }
  }
  
  // 購読解除（ID指定）
  bool Unsubscribe(const std::string& event_type, size_t subscription_id) {
    auto it = subscribers_.find(event_type);
    if (it == subscribers_.end()) return false;
    
    auto& subs = it->second;
    auto sub_it = std::remove_if(subs.begin(), subs.end(),
      [subscription_id](const Subscription& s) { return s.id == subscription_id; });
    
    if (sub_it != subs.end()) {
      subs.erase(sub_it, subs.end());
      return true;
    }
    return false;
  }
  
  // 全購読者削除
  void Clear() { subscribers_.clear(); }
  
  // デバッグ：購読者数取得
  size_t GetSubscriberCount(const std::string& event_type) const {
    auto it = subscribers_.find(event_type);
    return it != subscribers_.end() ? it->second.size() : 0;
  }
};

} // namespace Shared::Core
```

### 使用例

```cpp
// 購読
auto sub_id = context.GetEventSystem().Subscribe(
  Events::EntityLoaded,
  [](const EventData& data) {
    std::cout << "Entity loaded: " << data["id"] << std::endl;
  }
);

// 発行
context.GetEventSystem().Emit(Events::EntityLoaded, {
  {"id", "char_001"},
  {"name", "炎猫"}
});

// 解除
context.GetEventSystem().Unsubscribe(Events::EntityLoaded, sub_id);
```

### メリット

- ✅ `Events::`名前空間で文字列定数を型安全に管理
- ✅ 購読解除IDで厳密な管理
- ✅ JSONデータで柔軟性維持
- ✅ デバッグ機能（購読者数取得）

---

## Validator（チェーン型）

### 設計概要

複数のバリデータをチェーンして柔軟に検証。エラーと警告を分離。

```cpp
// shared/include/Data/Validators/IValidator.h
#pragma once

#include <vector>
#include <string>

namespace Shared::Data {

struct ValidationResult {
  bool success = true;
  std::vector<std::string> errors;
  std::vector<std::string> warnings;
  
  // 結果を結合
  ValidationResult& operator+=(const ValidationResult& other) {
    success = success && other.success;
    errors.insert(errors.end(), other.errors.begin(), other.errors.end());
    warnings.insert(warnings.end(), other.warnings.begin(), other.warnings.end());
    return *this;
  }
  
  bool HasErrors() const { return !errors.empty(); }
  bool HasWarnings() const { return !warnings.empty(); }
};

// テンプレートバリデータインターフェース
template<typename T>
class IValidator {
public:
  virtual ~IValidator() = default;
  virtual ValidationResult Validate(const T& data) = 0;
};

} // namespace Shared::Data
```

```cpp
// shared/include/Data/Validators/ValidatorChain.h
#pragma once

#include "Data/Validators/IValidator.h"
#include <vector>
#include <memory>

namespace Shared::Data {

template<typename T>
class ValidatorChain {
private:
  std::vector<std::unique_ptr<IValidator<T>>> validators_;

public:
  // バリデータを追加（チェーンパターン）
  ValidatorChain& Add(std::unique_ptr<IValidator<T>> validator) {
    validators_.push_back(std::move(validator));
    return *this;
  }
  
  // 全バリデータで検証
  ValidationResult ValidateAll(const T& data) {
    ValidationResult result;
    
    for (const auto& validator : validators_) {
      result += validator->Validate(data);
    }
    
    return result;
  }
};

} // namespace Shared::Data
```

```cpp
// shared/include/Data/Validators/EntityValidators.h
#pragma once

#include "Data/Validators/IValidator.h"
#include "Data/Definitions/EntityDef.h"
#include "Data/DefinitionRegistry.h"

namespace Shared::Data {

// スキーマ検証
class EntitySchemaValidator : public IValidator<EntityDef> {
public:
  ValidationResult Validate(const EntityDef& entity) override {
    ValidationResult result;
    
    if (entity.id.empty()) {
      result.errors.push_back("Entity ID is empty");
      result.success = false;
    }
    
    if (entity.name.empty()) {
      result.errors.push_back("Entity name is empty");
      result.success = false;
    }
    
    if (entity.rarity < 1 || entity.rarity > 5) {
      result.errors.push_back("Entity rarity must be 1-5");
      result.success = false;
    }
    
    if (entity.type != "main" && entity.type != "sub") {
      result.errors.push_back("Entity type must be 'main' or 'sub'");
      result.success = false;
    }
    
    return result;
  }
};

// 参照検証
class EntityReferenceValidator : public IValidator<EntityDef> {
private:
  const DefinitionRegistry& registry_;

public:
  explicit EntityReferenceValidator(const DefinitionRegistry& registry)
    : registry_(registry) {}
  
  ValidationResult Validate(const EntityDef& entity) override {
    ValidationResult result;
    
    for (const auto& skill_id : entity.skill_ids) {
      if (!registry_.HasSkill(skill_id)) {
        result.errors.push_back("Skill ID not found: " + skill_id);
        result.success = false;
      }
    }
    
    for (const auto& ability_id : entity.ability_ids) {
      if (!registry_.HasAbility(ability_id)) {
        result.errors.push_back("Ability ID not found: " + ability_id);
        result.success = false;
      }
    }
    
    return result;
  }
};

} // namespace Shared::Data
```

### 使用例

```cpp
// バリデータチェーンの構築
ValidatorChain<EntityDef> chain;
chain.Add(std::make_unique<EntitySchemaValidator>())
     .Add(std::make_unique<EntityReferenceValidator>(registry));

// 検証実行
auto result = chain.ValidateAll(entity);

if (!result.success) {
  for (const auto& error : result.errors) {
    std::cerr << "Error: " << error << std::endl;
  }
}

for (const auto& warning : result.warnings) {
  std::cerr << "Warning: " << warning << std::endl;
}
```

### メリット

- ✅ 各バリデータが独立（単一責任）
- ✅ チェーン追加で柔軟に拡張
- ✅ エラーと警告を分離
- ✅ テスト容易

---

## FileWatcher（EventSystem統合型）

### 設計概要

FileWatcherとEventSystemを統合。変更検知時にイベント発行。

```cpp
// shared/include/Core/FileWatcher.h
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace Shared::Core {

// Forward declaration
class EventSystem;

class FileWatcher {
private:
  struct WatchedFile {
    std::string path;
    std::filesystem::file_time_type last_write_time;
  };
  
  std::vector<WatchedFile> watched_files_;
  EventSystem* event_system_ = nullptr;  // 参照保持

public:
  explicit FileWatcher(EventSystem* event_system = nullptr)
    : event_system_(event_system) {}
  
  // ファイル監視開始
  void Watch(const std::string& path) {
    if (!std::filesystem::exists(path)) {
      return;
    }
    
    for (const auto& file : watched_files_) {
      if (file.path == path) {
        return;  // 既に監視中
      }
    }
    
    WatchedFile watched;
    watched.path = path;
    watched.last_write_time = std::filesystem::last_write_time(path);
    watched_files_.push_back(watched);
  }
  
  // 変更チェック（毎フレーム呼び出し）
  void CheckChanges() {
    for (auto& file : watched_files_) {
      if (!std::filesystem::exists(file.path)) {
        continue;
      }
      
      try {
        auto current_time = std::filesystem::last_write_time(file.path);
        
        if (current_time != file.last_write_time) {
          file.last_write_time = current_time;
          
          // EventSystem 経由でイベント発行
          if (event_system_) {
            event_system_->Emit(Events::FileChanged, {
              {"path", file.path},
              {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
            });
          }
        }
      } catch (const std::filesystem::filesystem_error& e) {
        if (event_system_) {
          event_system_->Emit(Events::ValidationFailed, {
            {"message", "File watch error: " + std::string(e.what())},
            {"path", file.path}
          });
        }
      }
    }
  }
  
  // 監視解除
  void Unwatch(const std::string& path) {
    auto it = std::remove_if(watched_files_.begin(), watched_files_.end(),
      [&path](const WatchedFile& f) { return f.path == path; });
    
    if (it != watched_files_.end()) {
      watched_files_.erase(it, watched_files_.end());
    }
  }
  
  void Clear() { watched_files_.clear(); }
  
  size_t GetWatchedFileCount() const { return watched_files_.size(); }
};

} // namespace Shared::Core
```

### 使用例

```cpp
// GameContext 初期化時
auto& eventSystem = context.GetEventSystem();
auto& fileWatcher = context.GetFileWatcher();

// FileChanged イベント購読
eventSystem.Subscribe(Events::FileChanged, [&](const EventData& data) {
  std::string path = data["path"];
  std::cout << "File changed: " << path << std::endl;
  
  // 自動リロード
  if (path.find("entities") != std::string::npos) {
    LoadEntities(path);
  }
});

// ファイル監視開始
fileWatcher.Watch("assets/definitions/entities.json");
fileWatcher.Watch("assets/definitions/skills.json");

// 毎フレーム
fileWatcher.CheckChanges();  // 変更検知 → Events::FileChanged 発行
```

### メリット

- ✅ EventSystemと統合で一貫性
- ✅ コールバック地獄を回避
- ✅ エラーハンドリングも統一
- ✅ デバッグが容易

---

## Expected型（エラーハンドリング）

### 設計概要

C++23のstd::expected風の軽量実装。例外なしでエラーハンドリング。

```cpp
// shared/include/Core/Expected.h
#pragma once

#include <variant>
#include <vector>
#include <string>
#include <optional>

namespace Shared::Core {

template<typename T>
class Expected {
private:
  std::variant<T, std::vector<std::string>> data_;

public:
  // 成功時のコンストラクタ
  Expected(const T& value) : data_(value) {}
  Expected(T&& value) : data_(std::move(value)) {}
  
  // 失敗時のコンストラクタ
  Expected(const std::vector<std::string>& errors) : data_(errors) {}
  Expected(std::vector<std::string>&& errors) : data_(std::move(errors)) {}
  
  // 単一エラーメッセージから生成
  explicit Expected(const std::string& error)
    : data_(std::vector<std::string>{error}) {}
  
  // 成功・失敗判定
  bool HasValue() const { return std::holds_alternative<T>(data_); }
  bool HasError() const { return std::holds_alternative<std::vector<std::string>>(data_); }
  
  // bool 変換（HasValue()と同じ）
  operator bool() const { return HasValue(); }
  
  // 値取得
  const T& Value() const { return std::get<T>(data_); }
  T& Value() { return std::get<T>(data_); }
  T* operator->() { return &Value(); }
  const T* operator->() const { return &Value(); }
  T& operator*() { return Value(); }
  const T& operator*() const { return Value(); }
  
  // エラー取得
  const std::vector<std::string>& Errors() const {
    return std::get<std::vector<std::string>>(data_);
  }
  
  // デフォルト値取得
  T ValueOr(const T& default_value) const {
    return HasValue() ? Value() : default_value;
  }
  
  // エラー取得（空の場合はデフォルト）
  std::vector<std::string> ErrorsOr(const std::vector<std::string>& default_errors) const {
    return HasError() ? Errors() : default_errors;
  }
};

} // namespace Shared::Core
```

### 使用例

```cpp
// Loader の戻り値型
Expected<std::vector<EntityDef>> EntityLoader::LoadFromFile(
  const std::string& path) {
  
  std::ifstream file(path);
  if (!file.is_open()) {
    return Expected<std::vector<EntityDef>>(
      "Failed to open file: " + path);
  }
  
  nlohmann::json json_data;
  try {
    file >> json_data;
  } catch (const std::exception& e) {
    return Expected<std::vector<EntityDef>>(
      "JSON parse error: " + std::string(e.what()));
  }
  
  std::vector<EntityDef> entities;
  std::vector<std::string> errors;
  
  for (const auto& item : json_data) {
    EntityDef entity;
    entity.from_json(item);
    
    auto validation = EntitySchemaValidator().Validate(entity);
    if (!validation.success) {
      errors.insert(errors.end(), 
        validation.errors.begin(), validation.errors.end());
    } else {
      entities.push_back(entity);
    }
  }
  
  if (!errors.empty()) {
    return Expected<std::vector<EntityDef>>(errors);
  }
  
  return Expected<std::vector<EntityDef>>(entities);
}

// 使用側
auto result = EntityLoader::LoadFromFile("entities.json");

if (result) {
  for (const auto& entity : result.Value()) {
    registry.RegisterEntity(entity);
  }
} else {
  for (const auto& error : result.Errors()) {
    std::cerr << "Error: " << error << std::endl;
  }
}

// 簡潔な書き方
if (!result) {
  auto errors = result.Errors();
  // エラー処理
  return false;
}

auto& entities = result.Value();
// 成功処理
```

### メリット

- ✅ モダンC++パターン
- ✅ 例外を使わずエラーハンドリング
- ✅ 型安全
- ✅ チェーン可能

---

## Loaderの各独立型実装

### EntityLoader

```cpp
// shared/include/Data/Loaders/EntityLoader.h
#pragma once

#include <string>
#include "Core/Expected.h"
#include "Data/Definitions/EntityDef.h"

namespace Shared::Data {

class EntityLoader {
public:
  // ファイルから読み込み
  static Core::Expected<std::vector<EntityDef>> LoadFromFile(
    const std::string& path);
  
  // JSONオブジェクトから読み込み
  static Core::Expected<std::vector<EntityDef>> LoadFromJson(
    const nlohmann::json& json_data);
  
  // ファイルに保存
  static Core::Expected<void> SaveToFile(
    const std::string& path,
    const std::vector<EntityDef>& entities);

private:
  static bool ValidateEntityJson(const nlohmann::json& item);
};

} // namespace Shared::Data
```

### SkillLoader, StageLoader, etc.

各Loaderは同一のパターンで実装。

---

## DefinitionRegistry

### 設計概要

全Definitionを一元管理。参照検証に使用。

```cpp
// shared/include/Data/DefinitionRegistry.h
#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "Data/Definitions/EntityDef.h"
#include "Data/Definitions/SkillDef.h"
#include "Data/Definitions/StageDef.h"

namespace Shared::Data {

class DefinitionRegistry {
private:
  std::unordered_map<std::string, EntityDef> entities_;
  std::unordered_map<std::string, SkillDef> skills_;
  std::unordered_map<std::string, StageDef> stages_;
  // ... その他のDefinition

public:
  // Entity
  bool RegisterEntity(const EntityDef& entity);
  bool HasEntity(const std::string& id) const;
  const EntityDef* GetEntity(const std::string& id) const;
  std::vector<EntityDef> GetAllEntities() const;
  
  // Skill
  bool RegisterSkill(const SkillDef& skill);
  bool HasSkill(const std::string& id) const;
  const SkillDef* GetSkill(const std::string& id) const;
  
  // Stage
  bool RegisterStage(const StageDef& stage);
  bool HasStage(const std::string& id) const;
  const StageDef* GetStage(const std::string& id) const;
  
  // 一括クリア
  void Clear();
  
  // 統計
  size_t GetEntityCount() const { return entities_.size(); }
  size_t GetSkillCount() const { return skills_.size(); }
  size_t GetStageCount() const { return stages_.size(); }
};

} // namespace Shared::Data
```

---

## 実装優先度

### Phase 1: Core基盤（1週間）

```
Week 1 Day 1-2: GameContext（ハイブリッド型）
  ✓ GameContext クラス実装
  ✓ GameContextConfig 定義
  ✓ パス管理（std::filesystem）
  
Week 1 Day 3-4: EventSystem（型安全文字列型）
  ✓ Events 定数定義
  ✓ EventSystem クラス実装
  ✓ 購読・発行・解除ロジック
  
Week 1 Day 5: Expected型（エラーハンドリング）
  ✓ Expected<T> テンプレート実装
  ✓ 基本操作（HasValue, Value, Errors）
```

### Phase 2: Data層（1週間）

```
Week 2 Day 1-2: Definition クラス（全7種）
  ✓ EntityDef / SkillDef / StageDef / EffectDef
  ✓ AbilityDef / SoundDef / DeckDef
  ✓ from_json / to_json 実装
  
Week 2 Day 3-4: Loader（各独立型）
  ✓ EntityLoader / SkillLoader / StageLoader
  ✓ EffectLoader / AbilityLoader / SoundLoader
  ✓ Expected<T> 戻り値使用
  
Week 2 Day 5: Validator（チェーン型）
  ✓ IValidator<T> インターフェース
  ✓ ValidatorChain<T> チェーン実装
  ✓ EntityValidator / SkillValidator 実装
```

### Phase 3: 統合機能（3-4日）

```
Week 3 Day 1-2: FileWatcher（EventSystem統合型）
  ✓ FileWatcher クラス実装
  ✓ EventSystem 統合
  ✓ CheckChanges() メインループ
  
Week 3 Day 3-4: DefinitionRegistry
  ✓ 全Definition管理
  ✓ 参照検証対応
  ✓ CMakeLists.txt 最終化
```

---

## CMake統合設計

### shared/CMakeLists.txt（推奨）

```cmake
# shared/CMakeLists.txt

cmake_minimum_required(VERSION 3.19)
project(SimpleTDCShared CXX)

# ライブラリ定義
add_library(SimpleTDCShared STATIC
  # Core
  src/Core/GameContext.cpp
  src/Core/EventSystem.cpp
  src/Core/FileWatcher.cpp
  
  # Data - Definitions
  src/Data/Definitions/EntityDef.cpp
  src/Data/Definitions/SkillDef.cpp
  src/Data/Definitions/StageDef.cpp
  src/Data/Definitions/EffectDef.cpp
  src/Data/Definitions/AbilityDef.cpp
  src/Data/Definitions/SoundDef.cpp
  src/Data/Definitions/DeckDef.cpp
  
  # Data - Loaders
  src/Data/Loaders/EntityLoader.cpp
  src/Data/Loaders/SkillLoader.cpp
  src/Data/Loaders/StageLoader.cpp
  src/Data/Loaders/EffectLoader.cpp
  src/Data/Loaders/AbilityLoader.cpp
  src/Data/Loaders/SoundLoader.cpp
  
  # Data - Validators
  src/Data/Validators/EntityValidators.cpp
  src/Data/Validators/SkillValidators.cpp
  src/Data/Validators/StageValidators.cpp
  
  # Data - Registry
  src/Data/DefinitionRegistry.cpp
)

# インクルードディレクトリ
target_include_directories(SimpleTDCShared
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 依存ライブラリ
target_link_libraries(SimpleTDCShared
  PUBLIC
    nlohmann_json::nlohmann_json
)

# C++ 標準
target_compile_features(SimpleTDCShared PRIVATE cxx_std_17)

# コンパイラ警告
if(MSVC)
  target_compile_options(SimpleTDCShared PRIVATE /W4)
else()
  target_compile_options(SimpleTDCShared PRIVATE -Wall -Wextra -Wpedantic)
endif()

# インストール設定
install(DIRECTORY include/ DESTINATION include)
install(TARGETS SimpleTDCShared DESTINATION lib)
```

---

## まとめ：最適設計の利点

| 利点 | 実現方法 |
|------|---------|
| **既存互換性** | GameContext デフォルトコンストラクタ |
| **拡張性** | GameContextConfig + チェーン + イベント |
| **型安全性** | Events定数 + Expected型 |
| **テスト容易性** | 各モジュール独立 |
| **デバッグ性** | EventSystem統合 + 購読者数取得 |
| **モダンさ** | C++17 ベストプラクティス |
| **一貫性** | FileWatcher統合 + Expected戻り値 |

---

## 次のステップ

### 実装開始前の確認チェックリスト

- [ ] GameContext（ハイブリッド型）設計確認
- [ ] EventSystem（型安全文字列型）設計確認
- [ ] Validator（チェーン型）設計確認
- [ ] FileWatcher（統合型）設計確認
- [ ] Expected型（エラー処理）設計確認
- [ ] Phase 1: Core基盤（1週間）実装開始準備
- [ ] CMakeLists.txt の準備
- [ ] テスト環境の整備

### 質問・意見ありますか？

このハイブリッドアプローチで即実装開始可能です。

