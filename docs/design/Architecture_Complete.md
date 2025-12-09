# アーキテクチャ設計 - 完全分離型（Game + Editor）確定版

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 2.0.0（完全分離型・確定版）  
**最終更新**: 2025-12-08

---

## 📑 目次

1. [全体設計概要](#全体設計概要)
2. [Shared Layer（共有層）](#shared-layer共有層)
3. [Game Executable（ゲーム本体）](#game-executableゲーム本体)
4. [Editor Executable（エディタ）](#editor-executableエディタ)
5. [層間通信・HotReload](#層間通信hotreload)
6. [ディレクトリ構成](#ディレクトリ構成)
7. [実装順序・フェーズ分け](#実装順序フェーズ分け)
8. [メリット・デメリット](#メリットデメリット)

---

## 全体設計概要

### アーキテクチャ図

```
┌──────────────────────────────────────────────────────────────┐
│                      Shared Layer                            │
│  ├─ Core Layer (GameContext, FileWatcher, EventSystem)      │
│  └─ Data Layer (Definitions, Loaders, Validators)           │
└──────────────┬──────────────────────────────────────────────┘
               │
        ┌──────┴──────┐
        ↓             ↓
┌─────────────────────┐  ┌──────────────────────────┐
│  Game Executable    │  │  Editor Executable       │
│  (SimpleTDCGame)    │  │  (SimpleTDCEditor)       │
│                     │  │                          │
│  ├─ Core Layer      │  │  ├─ Core Layer           │
│  ├─ Game Layer      │  │  ├─ Editor Layer         │
│  │  (Managers)      │  │  │  (Windows, Widgets)   │
│  ├─ TD Layer        │  │  ├─ ImGui Renderer       │
│  │  (ECS + Systems) │  │  └─ Services             │
│  └─ Raylib Renderer │  └──────────────────────────┘
└─────────────────────┘
```

### 設計方針

```yaml
基本戦略:
  分離度: 完全分離（2つの独立した実行ファイル）
  共有範囲: Shared Layer（Core + Data）のみ
  通信方法: JSON ファイル経由（HotReload）
  ビルド: CMake で2つのターゲット生成

メリット:
  ✅ リリース版はゲーム実行ファイルのみ（軽量）
  ✅ エディタ開発とゲーム開発が完全に独立
  ✅ ゲーム実行時にエディタのメモリ消費なし
  ✅ チーム開発時に並列開発可能
  ✅ テストが容易（各部が自己完結）

デメリット:
  ⚠️ 初期構築の手間（SharedLibrary作成）
  ⚠️ ビルド複雑度が増加
  ⚠️ エディタ・ゲーム間は JSON ファイルのみで通信
```

### 最適設計パターン組み合わせ

| 層/項目 | 選択 | 理由 |
|--------|------|------|
| **Core層** | A: シンプル型 | エディタ依存がなく、最小限で十分 |
| **Game層マネージャー** | A: 独立型 | 各マネージャーが独立し責務が明確 |
| **TD層Entity管理** | A: Component型 | EnTT標準、高性能、推奨パターン |
| **DI実装** | A: コンストラクタ | GameContext が一元管理 |
| **Registry管理** | A: GameEngine所有 | World で既に実装済み |
| **ホットリロード** | FileWatcher + Event | JSON変更をリアルタイム反映 |
| **エディタUI** | ImGui | 軽量、データドリブン向き |
| **エディタ設計** | プラグイン型ウィンドウ | 拡張性と柔軟性重視 |

---

## Shared Layer（共有層）

Shared層は、ゲーム本体とエディタの両方が依存する基盤層。**最小限に保つ**ことが重要。

### Shared層の責務

```yaml
Shared Layer:
  Core:
    - GameContext: 全体設定・パス管理
    - FileWatcher: ファイル変更監視
    - EventSystem: グローバルイベント
    - ResourceManager: リソース管理

  Data:
    - Definitions: 全Entity/Skill/Stage定義の C++ クラス
    - Loaders: JSON → Definitions への変換
    - Validators: スキーマ検証・参照チェック
```

### Shared Core Layer

```cpp
// shared/include/Core/GameContext.h
namespace Shared::Core {

class GameContext {
private:
  std::string data_path_;
  std::string assets_path_;
  std::unique_ptr<FileWatcher> file_watcher_;
  std::unique_ptr<EventSystem> event_system_;
  std::unique_ptr<ResourceManager> resource_manager_;

public:
  bool Initialize(const std::string& config_path);
  void Shutdown();
  
  // パス管理
  std::string GetDataPath(const std::string& relative_path) const;
  std::string GetAssetsPath(const std::string& relative_path) const;
  
  // マネージャーアクセス
  FileWatcher& GetFileWatcher();
  EventSystem& GetEventSystem();
  ResourceManager& GetResourceManager();
};

} // namespace Shared::Core
```

```cpp
// shared/include/Core/EventSystem.h
namespace Shared::Core {

class EventSystem {
private:
  std::unordered_map<std::string, 
    std::vector<std::function<void(const nlohmann::json&)>>> subscribers_;

public:
  void Subscribe(const std::string& event_type,
                std::function<void(const nlohmann::json&)> callback);
  void Emit(const std::string& event_type, const nlohmann::json& data);
};

} // namespace Shared::Core
```

```cpp
// shared/include/Core/FileWatcher.h
namespace Shared::Core {

class FileWatcher {
private:
  struct WatchedFile {
    std::string path;
    std::filesystem::file_time_type last_write_time;
    std::function<void()> on_changed;
  };
  std::vector<WatchedFile> watched_files_;

public:
  void Watch(const std::string& path, std::function<void()> callback);
  void CheckChanges();  // 毎フレーム呼び出し
};

} // namespace Shared::Core
```

### Shared Data Layer

```cpp
// shared/include/Data/Definitions/EntityDef.h
namespace Shared::Data {

struct EntityDef {
  std::string id;
  std::string name;
  std::string description;
  int rarity;
  std::string type;  // "main" / "sub"
  bool is_enemy;
  
  int cost;
  float cooldown;
  
  struct Stats {
    int hp;
    int attack;
    float attack_speed;
    int range;
  } stats;
  
  std::string draw_type;  // "parts_animation" / "sprite"
  
  struct Display {
    std::string sprite_sheet;
    // アニメーション定義は省略
  } display;
  
  std::vector<std::string> skill_ids;
  std::vector<std::string> ability_ids;
  
  std::vector<std::string> tags;
};

} // namespace Shared::Data
```

```cpp
// shared/include/Data/DefinitionRegistry.h
namespace Shared::Data {

class DefinitionRegistry {
private:
  std::unordered_map<std::string, EntityDef> entities_;
  std::unordered_map<std::string, SkillDef> skills_;
  std::unordered_map<std::string, StageDef> stages_;
  std::unordered_map<std::string, EffectDef> effects_;
  std::unordered_map<std::string, SoundDef> sounds_;
  
  std::vector<std::string> validation_errors_;

public:
  // Entity
  bool RegisterEntity(const EntityDef& def);
  const EntityDef* GetEntity(const std::string& id) const;
  std::vector<const EntityDef*> GetAllEntities() const;
  
  // Skill
  bool RegisterSkill(const SkillDef& def);
  const SkillDef* GetSkill(const std::string& id) const;
  
  // Stage
  bool RegisterStage(const StageDef& def);
  const StageDef* GetStage(const std::string& id) const;
  
  // Effect
  bool RegisterEffect(const EffectDef& def);
  const EffectDef* GetEffect(const std::string& id) const;
  
  // Sound
  bool RegisterSound(const SoundDef& def);
  const SoundDef* GetSound(const std::string& id) const;
  
  // バリデーション
  bool ValidateAll();
  const std::vector<std::string>& GetErrors() const;
};

} // namespace Shared::Data
```

```cpp
// shared/include/Data/Loaders/EntityLoader.h
namespace Shared::Data {

class EntityLoader {
public:
  static bool LoadFromJson(const std::string& json_path,
                          DefinitionRegistry& registry);
  static bool SaveToJson(const std::string& json_path,
                        const DefinitionRegistry& registry);
};

} // namespace Shared::Data
```

---

## Game Executable（ゲーム本体）

Game実行ファイルは、Shared層に依存し、Raylib を使用した TD ゲームの実装。

### Game層の設計

```yaml
Game Executable:
  Core Layer (Shared):
    - GameContext
    - FileWatcher
    - EventSystem

  Game Layer:
    - Managers (Character, Skill, Stage, Enemy, Effect)
    - 状態管理 (Player, Deck, Progress)

  TD Layer (ECS):
    - Components (Transform, Stats, Skill, Animation, etc)
    - Systems (Movement, Attack, Skill, Rendering, etc)
    - Factories (Character, Enemy, Effect)

  Application:
    - Game (メインアプリケーション)
    - SceneManager (画面遷移管理)
    - Scenes (Home, StageSelection, TDGame, Result)
```

### Game Application

```cpp
// game/include/Game/Application/Game.h
namespace Game::Application {

class Game {
private:
  std::unique_ptr<Shared::Core::GameContext> context_;
  std::unique_ptr<Game::World::GameEngine> game_engine_;
  std::unique_ptr<SceneManager> scene_manager_;
  
  float delta_time_;
  bool is_running_;

public:
  Game(std::unique_ptr<Shared::Core::GameContext> context);
  
  bool Initialize();
  void Run();
  void Shutdown();

private:
  void Update(float delta_time);
  void Render();
  
  void RegisterHotReloadCallbacks();
};

} // namespace Game::Application
```

```cpp
// game/src/main_game.cpp
#include "Game/Application/Game.h"

int main() {
  auto context = std::make_unique<Shared::Core::GameContext>();
  if (!context->Initialize("config.json")) {
    std::cerr << "Failed to initialize GameContext" << std::endl;
    return 1;
  }
  
  auto game = std::make_unique<Game::Application::Game>(std::move(context));
  if (!game->Initialize()) {
    std::cerr << "Failed to initialize Game" << std::endl;
    return 1;
  }
  
  game->Run();
  game->Shutdown();
  
  return 0;
}
```

### Game Manager（例：SkillManager）

```cpp
// game/include/Game/Managers/SkillManager.h
namespace Game::Managers {

class SkillManager {
private:
  Shared::Core::GameContext& context_;
  Shared::Data::DefinitionRegistry& definitions_;
  std::unordered_map<std::string, Shared::Data::SkillDef> skill_cache_;

public:
  SkillManager(Shared::Core::GameContext& context,
              Shared::Data::DefinitionRegistry& definitions);
  
  bool Initialize();
  
  const Shared::Data::SkillDef* GetSkill(const std::string& id) const;
  std::vector<const Shared::Data::SkillDef*> GetAllSkills() const;

private:
  void RegisterHotReloadCallback();
  void OnSkillsReloaded();
};

} // namespace Game::Managers
```

```cpp
// game/src/Game/Managers/SkillManager.cpp
namespace Game::Managers {

SkillManager::SkillManager(
  Shared::Core::GameContext& context,
  Shared::Data::DefinitionRegistry& definitions)
  : context_(context), definitions_(definitions) {}

bool SkillManager::Initialize() {
  RegisterHotReloadCallback();
  return true;
}

void SkillManager::RegisterHotReloadCallback() {
  context_.GetFileWatcher().Watch(
    context_.GetDataPath("definitions/skills/skills.json"),
    [this]() { this->OnSkillsReloaded(); }
  );
}

void SkillManager::OnSkillsReloaded() {
  // JSON 再ロード
  Shared::Data::SkillLoader::LoadFromJson(
    context_.GetDataPath("definitions/skills/skills.json"),
    definitions_);
  
  // キャッシュ更新
  skill_cache_.clear();
  
  // イベント発行
  context_.GetEventSystem().Emit("SkillsReloaded", {});
}

} // namespace Game::Managers
```

### Game ECS System（例：SkillSystem）

```cpp
// game/include/Game/Systems/SkillSystem.h
namespace Game::Systems {

class SkillSystem : public ISystem {
private:
  Shared::Core::GameContext& context_;
  Game::Managers::SkillManager& skill_manager_;

public:
  SkillSystem(Shared::Core::GameContext& context,
             Game::Managers::SkillManager& skill_manager);
  
  void Update(entt::registry& registry, float delta_time) override;
  void TriggerSkill(entt::registry& registry, entt::entity entity,
                   const std::string& skill_id);

private:
  void UpdateCooldowns(entt::registry& registry, float delta_time);
  void ApplySkillEffect(entt::registry& registry, entt::entity entity,
                       const Shared::Data::SkillDef& skill);
};

} // namespace Game::Systems
```

---

## Editor Executable（エディタ）

Editor実行ファイルは、Shared層に依存し、ImGui を使用したデータ編集ツール。

### Editor Application

```cpp
// editor/include/Editor/Application/EditorApp.h
namespace Editor::Application {

class EditorApp {
private:
  std::unique_ptr<Shared::Core::GameContext> context_;
  std::unique_ptr<Editor::Renderer::ImGuiRenderer> imgui_renderer_;
  
  std::vector<std::unique_ptr<Editor::Windows::IEditorWindow>> windows_;
  std::unique_ptr<Editor::Services::WorkspaceManager> workspace_manager_;
  std::unique_ptr<Editor::Services::DataBindingService> data_binding_;
  
  bool is_running_;

public:
  EditorApp(std::unique_ptr<Shared::Core::GameContext> context);
  
  bool Initialize();
  void Run();
  void Shutdown();

private:
  void InitializeWindows();
  void Update(float delta_time);
  void Render();
};

} // namespace Editor::Application
```

```cpp
// editor/src/main_editor.cpp
#include "Editor/Application/EditorApp.h"

int main() {
  auto context = std::make_unique<Shared::Core::GameContext>();
  if (!context->Initialize("config.json")) {
    std::cerr << "Failed to initialize GameContext" << std::endl;
    return 1;
  }
  
  auto editor = std::make_unique<Editor::Application::EditorApp>(
    std::move(context));
  
  if (!editor->Initialize()) {
    std::cerr << "Failed to initialize EditorApp" << std::endl;
    return 1;
  }
  
  editor->Run();
  editor->Shutdown();
  
  return 0;
}
```

### EditorWindow（プラグイン型）

```cpp
// editor/include/Editor/Windows/IEditorWindow.h
namespace Editor::Windows {

class IEditorWindow {
public:
  virtual ~IEditorWindow() = default;
  
  virtual void Initialize(Shared::Core::GameContext& context) = 0;
  virtual void OnDrawUI() = 0;
  virtual void OnUpdate(float delta_time) = 0;
  virtual std::string GetWindowTitle() const = 0;
  virtual bool IsOpen() const = 0;
  virtual void SetOpen(bool open) = 0;
};

} // namespace Editor::Windows
```

```cpp
// editor/include/Editor/Windows/EntityEditorWindow.h
namespace Editor::Windows {

class EntityEditorWindow : public IEditorWindow {
private:
  Shared::Core::GameContext* context_;
  Shared::Data::DefinitionRegistry& definitions_;
  
  std::string selected_entity_id_;
  bool is_open_;

public:
  EntityEditorWindow(Shared::Data::DefinitionRegistry& definitions);
  
  void Initialize(Shared::Core::GameContext& context) override;
  void OnDrawUI() override;
  void OnUpdate(float delta_time) override;
  
  std::string GetWindowTitle() const override { return "Entity Editor"; }
  bool IsOpen() const override { return is_open_; }
  void SetOpen(bool open) override { is_open_ = open; }

private:
  void DrawEntityList();
  void DrawEntityDetails();
  void DrawStatControls();
};

} // namespace Editor::Windows
```

### DataBindingService（双方向バインディング）

```cpp
// editor/include/Editor/Services/DataBindingService.h
namespace Editor::Services {

class DataBindingService {
private:
  Shared::Core::GameContext& context_;
  Shared::Data::DefinitionRegistry& definitions_;
  
  // ImGui とデータ定義の双方向バインディング
  std::unordered_map<std::string, nlohmann::json> bindings_;

public:
  DataBindingService(Shared::Core::GameContext& context,
                    Shared::Data::DefinitionRegistry& definitions);
  
  // ImGui → JSON
  void SaveDefinition(const std::string& type, const std::string& id);
  
  // JSON → ImGui
  void LoadDefinition(const std::string& type, const std::string& id);
  
  // ファイル保存
  bool WriteToFile(const std::string& type);
};

} // namespace Editor::Services
```

---

## 層間通信・HotReload

### HotReload メカニズム

```cpp
// shared/src/Core/FileWatcher.cpp
namespace Shared::Core {

void FileWatcher::CheckChanges() {
  for (auto& file : watched_files_) {
    auto current_time = std::filesystem::last_write_time(file.path);
    
    if (current_time != file.last_write_time) {
      file.last_write_time = current_time;
      
      // 変更コールバック実行
      if (file.on_changed) {
        file.on_changed();
      }
    }
  }
}

} // namespace Shared::Core
```

### ゲーム本体での HotReload

```cpp
// game/src/Game/Application/Game.cpp
namespace Game::Application {

bool Game::Initialize() {
  RegisterHotReloadCallbacks();
  return true;
}

void Game::RegisterHotReloadCallbacks() {
  auto& event_system = context_->GetEventSystem();
  
  // Skills再ロード時の処理
  event_system.Subscribe("SkillsReloaded", [this](const nlohmann::json& data) {
    // ゲーム実行中の Entity component を更新
    auto& registry = game_engine_->GetRegistry();
    auto view = registry.view<Game::Components::SkillComponent>();
    
    for (auto entity : view) {
      auto& skill_comp = registry.get<Game::Components::SkillComponent>(entity);
      // スキルキャッシュから再読み込み
    }
  });
  
  // Entities再ロード時の処理
  event_system.Subscribe("EntitiesReloaded", [this](const nlohmann::json& data) {
    // キャッシュ更新
  });
}

} // namespace Game::Application
```

### エディタとゲーム間の同期

```
Timeline:

1. エディタ起動
   ├─ SharedLayer初期化
   ├─ Definitions読み込み
   └─ FileWatcher開始

2. ユーザーが JSON 編集 (エディタUI)
   ├─ DataBindingService が変更をキャプチャ
   ├─ JSON ファイル保存
   └─ FileWatcher が変更検知

3. ゲーム実行中
   ├─ FileWatcher が JSON 変更を検知
   ├─ EventSystem が "SkillsReloaded" 発行
   ├─ Managers が定義を再ロード
   └─ Systems が実行中の Entity を更新

※ ゲームとエディタは同時起動可能
  エディタで編集 → ゲーム内リアルタイム反映
```

---

## ディレクトリ構成

### ルート構成

```
SimpleTDCGame/
├── CMakeLists.txt                    # ルート CMakeLists
├── CMakePresets.json
├── .gitignore
│
├── shared/                           # 🔑 共有ライブラリ
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Core/
│   │   │   ├── GameContext.h
│   │   │   ├── EventSystem.h
│   │   │   ├── FileWatcher.h
│   │   │   └── ResourceManager.h
│   │   │
│   │   └── Data/
│   │       ├── DefinitionRegistry.h
│   │       ├── Definitions/
│   │       │   ├── EntityDef.h
│   │       │   ├── SkillDef.h
│   │       │   ├── StageDef.h
│   │       │   ├── WaveDef.h
│   │       │   ├── EffectDef.h
│   │       │   ├── AbilityDef.h
│   │       │   ├── SoundDef.h
│   │       │   └── DeckDef.h
│   │       │
│   │       ├── Loaders/
│   │       │   ├── EntityLoader.h
│   │       │   ├── SkillLoader.h
│   │       │   ├── StageLoader.h
│   │       │   ├── EffectLoader.h
│   │       │   └── SoundLoader.h
│   │       │
│   │       └── Validators/
│   │           ├── SchemaValidator.h
│   │           ├── ReferenceValidator.h
│   │           └── MapValidator.h
│   │
│   └── src/
│       ├── Core/
│       ├── Data/
│       └── (実装ファイル)
│
├── game/                             # 🎮 ゲーム本体
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Game/
│   │   │   ├── Managers/
│   │   │   │   ├── CharacterManager.h
│   │   │   │   ├── SkillManager.h
│   │   │   │   ├── EnemyManager.h
│   │   │   │   ├── StageManager.h
│   │   │   │   ├── EffectManager.h
│   │   │   │   ├── DeckManager.h
│   │   │   │   └── AudioManager.h
│   │   │   │
│   │   │   ├── Components/
│   │   │   │   ├── TransformComponent.h
│   │   │   │   ├── StatsComponent.h
│   │   │   │   ├── SkillComponent.h
│   │   │   │   ├── AnimationComponent.h
│   │   │   │   ├── RenderComponent.h
│   │   │   │   └── AudioComponent.h
│   │   │   │
│   │   │   ├── Systems/
│   │   │   │   ├── ISystem.h
│   │   │   │   ├── MovementSystem.h
│   │   │   │   ├── SkillSystem.h
│   │   │   │   ├── AttackSystem.h
│   │   │   │   ├── RenderSystem.h
│   │   │   │   ├── AnimationSystem.h
│   │   │   │   ├── CollisionSystem.h
│   │   │   │   ├── EffectSystem.h
│   │   │   │   ├── AudioSystem.h
│   │   │   │   ├── WaveSystem.h
│   │   │   │   └── CostSystem.h
│   │   │   │
│   │   │   ├── Factories/
│   │   │   │   ├── CharacterFactory.h
│   │   │   │   ├── EnemyFactory.h
│   │   │   │   └── EffectFactory.h
│   │   │   │
│   │   │   ├── World/
│   │   │   │   ├── World.h
│   │   │   │   └── GameEngine.h
│   │   │   │
│   │   │   └── Application/
│   │   │       ├── Game.h
│   │   │       ├── SceneManager.h
│   │   │       ├── HomeScene.h
│   │   │       ├── StageSelectionScene.h
│   │   │       ├── TDGameScene.h
│   │   │       └── ResultScene.h
│   │   │
│   │   └── ComponentsGame.h (統合ヘッダー)
│   │
│   └── src/
│       ├── Game/
│       │   ├── Managers/
│       │   ├── Systems/
│       │   ├── Factories/
│       │   ├── World/
│       │   └── Application/
│       └── main_game.cpp
│
├── editor/                           # ✏️ エディタ本体
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Editor/
│   │   │   ├── Application/
│   │   │   │   ├── EditorApp.h
│   │   │   │   └── EditorState.h
│   │   │   │
│   │   │   ├── Windows/
│   │   │   │   ├── IEditorWindow.h
│   │   │   │   ├── EntityEditorWindow.h
│   │   │   │   ├── SkillEditorWindow.h
│   │   │   │   ├── StageEditorWindow.h
│   │   │   │   ├── EffectEditorWindow.h
│   │   │   │   ├── DeckEditorWindow.h
│   │   │   │   ├── HierarchyWindow.h
│   │   │   │   ├── InspectorWindow.h
│   │   │   │   ├── ConsoleWindow.h
│   │   │   │   └── AssetBrowserWindow.h
│   │   │   │
│   │   │   ├── Widgets/
│   │   │   │   ├── PropertyGrid.h
│   │   │   │   ├── JSONTreeView.h
│   │   │   │   ├── AnimationTimeline.h
│   │   │   │   └── PreviewPanel.h
│   │   │   │
│   │   │   ├── Services/
│   │   │   │   ├── WorkspaceManager.h
│   │   │   │   ├── DataBindingService.h
│   │   │   │   ├── ValidationService.h
│   │   │   │   └── ProjectManager.h
│   │   │   │
│   │   │   └── Renderer/
│   │   │       ├── ImGuiRenderer.h
│   │   │       ├── PreviewRenderer.h
│   │   │       └── ThemeManager.h
│   │   │
│   │   └── ComponentsEditor.h (統合ヘッダー)
│   │
│   └── src/
│       ├── Editor/
│       │   ├── Application/
│       │   ├── Windows/
│       │   ├── Widgets/
│       │   ├── Services/
│       │   └── Renderer/
│       └── main_editor.cpp
│
├── assets/                           # ゲームアセット
│   ├── definitions/
│   │   ├── entities/
│   │   │   └── entities.json
│   │   ├── skills/
│   │   │   └── skills.json
│   │   ├── stages/
│   │   │   └── stages.json
│   │   ├── waves/
│   │   │   └── waves.json
│   │   ├── effects/
│   │   │   └── effects.json
│   │   ├── abilities/
│   │   │   └── abilities.json
│   │   ├── sounds/
│   │   │   └── sounds.json
│   │   └── decks/
│   │       └── (プレイヤーデッキ)
│   │
│   ├── sprites/
│   │   ├── characters/
│   │   ├── effects/
│   │   └── ui/
│   │
│   ├── sounds/
│   │   ├── bgm/
│   │   ├── sfx/
│   │   └── voice/
│   │
│   ├── fonts/
│   │   └── default.ttf
│   │
│   └── config.json
│
├── docs/
│   ├── ARCHITECTURE.md           # 全体設計書（このドキュメント）
│   ├── API.md                    # API 仕様
│   ├── DATA_DEFINITIONS.md       # JSON スキーマ
│   ├── GAME_DESIGN.md            # ゲームデザイン
│   ├── EDITOR_GUIDE.md           # エディタ使い方
│   └── BUILD.md                  # ビルド手順
│
└── .github/
    ├── workflows/
    │   ├── build-game.yml        # ゲーム本体ビルド
    │   └── build-editor.yml      # エディタビルド
    └── copilot-instructions.md
```

---

## 実装順序・フェーズ分け

### Phase 1: Shared Layer 基盤（2-3週間）

```
目標: Game/Editor 両方が依存する基盤を完成

タスク:
  ✅ GameContext 実装
  ✅ FileWatcher 実装
  ✅ EventSystem 実装
  ✅ ResourceManager 実装
  ✅ 全 Definition クラス実装
  ✅ 全 Loader 実装
  ✅ Validator 実装
  ✅ shared ライブラリ CMake設定

成果物:
  - libSimpleTDCShared.a (Linux/Mac) / .lib (Windows)
```

### Phase 2: Game Executable（3-4週間）

```
目標: 独立したゲーム実行ファイル完成

タスク:
  ✅ Manager 実装（Character, Skill, Stage, Enemy, Effect）
  ✅ Component 実装（全 Component）
  ✅ System 実装（Movement, Skill, Attack, Render など）
  ✅ Factory 実装
  ✅ World / GameEngine 実装
  ✅ SceneManager 実装
  ✅ Game.cpp 実装
  ✅ main_game.cpp 実装
  ✅ HotReload コールバック登録

成果物:
  - SimpleTDCGame.exe / SimpleTDCGame (Linux/Mac)
  - 完全に動作する TD ゲーム
```

### Phase 3: Editor Executable 基盤（2-3週間）

```
目標: エディタの基本構造完成

タスク:
  ✅ ImGuiRenderer 実装
  ✅ EditorApp 実装
  ✅ IEditorWindow インターフェース定義
  ✅ WorkspaceManager 実装
  ✅ DataBindingService 基本実装
  ✅ ValidationService 実装
  ✅ main_editor.cpp 実装

成果物:
  - SimpleTDCEditor.exe / SimpleTDCEditor (Linux/Mac)
  - ImGui ウィンドウ表示可能
```

### Phase 4: Editor Windows（3-4週間）

```
目標: 各編集ウィンドウの実装

タスク:
  ✅ EntityEditorWindow
  ✅ SkillEditorWindow
  ✅ StageEditorWindow
  ✅ EffectEditorWindow
  ✅ AbilityEditorWindow
  ✅ HierarchyWindow
  ✅ InspectorWindow
  ✅ ConsoleWindow
  ✅ AssetBrowserWindow

成果物:
  - 全ウィンドウ動作確認
  - JSON 読み書き機能完全
```

### Phase 5: ポーランシング・最適化（1-2週間）

```
目標: 本番品質へ

タスク:
  ✅ HotReload 動作確認
  ✅ バリデーション強化
  ✅ エラーメッセージ改善
  ✅ パフォーマンス最適化
  ✅ ドキュメント完成
  ✅ CI/CD パイプライン構築
```

---

## メリット・デメリット

### 分離型のメリット ✨

| メリット | 詳細 |
|---------|------|
| **リリース軽量化** | ゲーム実行ファイルのみ配布（エディタ不要） |
| **メモリ効率** | ゲーム実行時にエディタメモリ消費ゼロ |
| **開発独立性** | ゲーム開発とエディタ開発が完全に並列可能 |
| **チーム開発** | エディタ担当と機能開発を分けて対応可能 |
| **テスト容易性** | 各部が独立し、単体テスト容易 |
| **保守性向上** | 各実行ファイルのコードが簡潔で理解容易 |
| **スケーラビリティ** | エディタ機能追加が容易 |
| **パフォーマンス** | ゲーム実行時に不要なコード（UI処理）が存在しない |

### 分離型のデメリット ⚠️

| デメリット | 対策 |
|----------|------|
| **初期構築手間** | Shared ライブラリ作成。段階的に進める |
| **ビルド複雑度** | CMake で2ターゲット管理。CI/CD で自動化 |
| **ゲーム・エディタ通信** | JSON ファイル＋ FileWatcher で十分。堅牢性確立 |
| **デバッグ難度** | エディタ・ゲーム同時起動で問題確認。ログ出力活用 |
| **版管理複雑性** | git でバージョン管理。定期的に統合テスト |

---

## 設計原則（重要）

```yaml
設計の心構え:
  1. "Shared は最小限"
     → Core + Data + Validators のみ
     → Game/Editor 固有ロジックは混ぜない

  2. "JSON は真実の源"
     → 全パラメータはファイルベース
     → エディタから編集 → ゲームが自動反映

  3. "イベント駆動を活用"
     → FileWatcher + EventSystem で疎結合
     → ゲーム・エディタが独立に動作

  4. "DI（依存性注入）で柔軟性確保"
     → GameContext を注入 → テスト容易
     → Mock オブジェクト挿入可能

  5. "エディタはプレイヤーに配布しない"
     → ゲーム実行ファイル（SimpleTDCGame.exe）のみ
     → エディタはプロジェクト内部ツール
```

---

## まとめ

### 推奨される全体設計 🏆

```
┌─ Shared Layer ─────────────────────┐
│ • Core (GameContext, FileWatcher)  │
│ • Data (Definitions, Loaders)      │
│ • Validators                       │
└────────────────────────────────────┘

┌─ Game Executable ───────────────┐
│ • Manager（A型：独立）           │
│ • ECS Components + Systems       │
│ • Raylib Renderer                │
│ • HotReload (FileWatcher)        │
│ • リリース対象 ✅                 │
└─────────────────────────────────┘

┌─ Editor Executable ──────────────┐
│ • ImGui Windows (Plugin型)       │
│ • DataBindingService             │
│ • WorkspaceManager               │
│ • ValidationService              │
│ • プロジェクト内部ツール ✅       │
└──────────────────────────────────┘
```

### ビルドコマンド例

```bash
# Shared ライブラリビルド
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target SimpleTDCShared

# ゲーム本体ビルド
cmake --build build --target SimpleTDCGame

# エディタビルド
cmake --build build --target SimpleTDCEditor

# 両方ビルド
cmake --build build
```

### 実行

```bash
# ゲーム実行
./build/SimpleTDCGame

# エディタ実行
./build/SimpleTDCEditor

# ゲーム＋エディタ同時起動（デバッグ用）
./build/SimpleTDCEditor &
./build/SimpleTDCGame
```

---

## 次のステップ

1. ✅ **Shared Layer 実装開始** → GameContext, FileWatcher, EventSystem
2. ✅ **Game Manager 実装** → CharacterManager, SkillManager など
3. ✅ **ECS Components/Systems 実装** → 基本 System から開始
4. ✅ **エディタ UI 構築** → ImGui ウィンドウプロトタイプ
5. ✅ **HotReload テスト** → JSON 編集がゲームに反映されることを確認

