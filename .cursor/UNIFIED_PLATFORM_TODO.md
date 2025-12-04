# 統合プラットフォーム実装計画

> **Cursor開発者向けタスク管理ドキュメント**  
> このドキュメントは、統合ゲームプラットフォーム完成に向けた実装計画を定義します。  
> AIエージェントとの協働開発に最適化されています。

## 📋 クイックナビゲーション

- [Phase 1: 旧TD完全廃止](#phase-1-旧td完全廃止-緊急) 🔴 最優先
- [Phase 2: ゲームシーン実装](#phase-2-ゲームシーン実装-重要) 🟡 重要
- [Phase 3: Roguelike基本機能](#phase-3-roguelike基本機能実装-中期) 🟢 中期
- [Phase 4: アイテム共有システム](#phase-4-アイテム共有システム-長期) 🔵 長期
- [Phase 5: HTTPサーバー開発者モード](#phase-5-httpサーバー開発者モード完成-重要) 🟡 重要
- [Phase 6: ドキュメント整理](#phase-6-ドキュメント整理-重要) 📚 重要
- [マイルストーン](#マイルストーン)
- [Cursor開発ワークフロー](#cursor開発ワークフロー)

---

## 🎯 最終目標

統合ゲームプラットフォームの完成により、以下を実現します：

1. ✅ **ホームシーン経由のゲーム選択** - 完了
2. ⏳ **旧TD完全廃止** - Phase 1
3. ⏳ **新TD・Roguelike統合** - Phase 2-3
4. ⏳ **アイテム共有システム** - Phase 4
5. ⏳ **HTTPサーバー開発者モード完成** - Phase 5
6. ⏳ **ドキュメント整理** - Phase 6

---

## Phase 1: 旧TD完全廃止 🔴 緊急

**優先度**: 🔴 最優先  
**予想工数**: 2時間  
**担当**: AIエージェント推奨

### 📌 目的

レガシーコードを削除し、統合プラットフォーム（`main_unified.cpp`）のみに集中する。

### ✅ タスクリスト

#### 1.1 旧TDファイルの削除

```bash
# Cursorターミナルで実行
git rm src/main.cpp
git rm src/Game.cpp
git rm include/Game.h
git rm src/SceneManager.cpp
git rm include/SceneManager.h
git rm src/ConfigManager.cpp
git rm include/ConfigManager.h
git rm src/InputManager.cpp
git rm include/InputManager.h
git rm src/ResourceManager.cpp
git rm include/ResourceManager.h
git rm src/UIManager.cpp
git rm include/UIManager.h
git rm include/Components.h
git rm src/TestScene.cpp
```

**削除対象ファイル一覧**:

- [ ] `src/main.cpp` - 旧TDエントリポイント
- [ ] `src/Game.cpp` - 旧Gameクラス
- [ ] `include/Game.h`
- [ ] `src/SceneManager.cpp` - 旧シーン管理
- [ ] `include/SceneManager.h`
- [ ] `src/ConfigManager.cpp` - 旧設定管理
- [ ] `include/ConfigManager.h`
- [ ] `src/InputManager.cpp` - 旧入力管理
- [ ] `include/InputManager.h`
- [ ] `src/ResourceManager.cpp` - 旧リソース管理
- [ ] `include/ResourceManager.h`
- [ ] `src/UIManager.cpp` - 旧UI管理
- [ ] `include/UIManager.h`
- [ ] `include/Components.h` - 旧コンポーネント定義
- [ ] `src/TestScene.cpp` - テストシーン

#### 1.2 CMakeLists.txt確認

- [x] `SimpleTDCGame`ターゲットが`main_unified.cpp`を使用していることを確認済み
- [ ] 削除したファイルがCMakeLists.txtに含まれていないことを確認

#### 1.3 README.md更新

**編集箇所**: `README.md`

```markdown
# 変更前
| **旧TDゲーム** | SimpleTDCGame (旧) | main.cpp | 🟡 保守 | レガシーコード、将来廃止予定 |

# 変更後（行削除）
```

- [ ] "旧TDゲーム"の行を削除
- [ ] ゲームプロジェクト表を3行に簡略化
- [ ] 統合プラットフォームを最優先として強調

#### 1.4 コミット

```bash
git commit -m "chore: 旧TDゲーム完全廃止 - 統合プラットフォームへの移行完了

- 旧TDのエントリポイント（main.cpp）削除
- 旧アーキテクチャクラス（Game, SceneManager等）削除
- 統合プラットフォーム（main_unified.cpp）のみに集約
- README.mdを更新し、旧TD項目を削除"
```

### 🎯 完了条件

- ✅ 旧TDファイルが全て削除されている
- ✅ `cmake --build build --config Release` が成功する
- ✅ `SimpleTDCGame.exe`が正常に起動する
- ✅ README.mdが更新されている

### 💡 Cursorでの作業手順

1. **Cursor Composer**で一括削除を依頼

   ```
   旧TDファイル（main.cpp, Game.cpp等）を削除してください。
   削除対象は Phase 1.1 のリストを参照。
   ```

2. **ビルドテスト**（Ctrl+Shift+B）

   ```
   Build SimpleTDCGame (Release)
   ```

3. **README.md更新**（Cursor Chat）

   ```
   README.mdのゲームプロジェクト表から旧TDの行を削除
   ```

4. **コミット**（Source Control）

---

## Phase 2: ゲームシーン実装 🟡 重要

**優先度**: 🟡 高  
**予想工数**: 8時間  
**担当**: 段階的実装推奨

### 📌 目的

HomeSceneから選択した後の各ゲームモード専用シーンを実装し、実際に遊べるようにする。

### 📊 現状分析

- ✅ `HomeScene`完全実装済み（`src/Application/HomeScene.cpp`）
- ✅ `UnifiedGame`でホームシーン経由のフロー実装済み
- ❌ TD専用ゲームシーンクラスが未実装
- ❌ Roguelike専用ゲームシーンクラスが未実装
- ❌ シーン遷移後のゲームループが未完成

### ✅ タスクリスト

#### 2.1 TDゲームシーン実装

**新規ファイル**:

- `include/Application/TDGameScene.h`
- `src/Application/TDGameScene.cpp`

**実装内容**:

```cpp
// include/Application/TDGameScene.h
#pragma once
#include "Application/IScene.h"
#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/SystemRunner.h"

namespace Application {

class TDGameScene : public IScene {
public:
    TDGameScene(Core::World& world, Core::GameContext& context);
    ~TDGameScene() override = default;
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    
    std::string GetName() const override { return "TDGame"; }
    
private:
    Core::World& world_;
    Core::GameContext& context_;
    Core::SystemRunner systemRunner_;
    
    void RegisterTDSystems();
    void LoadStage(const std::string& stageId);
};

} // namespace Application
```

**チェックリスト**:

- [ ] `TDGameScene`クラス作成
- [ ] `IScene`インターフェース実装
- [ ] TD固有システム登録（LaneMovement, Combat, Wave等）
- [ ] `Update()`でシステム実行
- [ ] `Render()`でゲーム描画
- [ ] ESCキーでホームに戻る機能
- [ ] ステージ選択UI（オプション）

#### 2.2 Roguelikeゲームシーン実装

**新規ファイル**:

- `include/Application/RoguelikeGameScene.h`
- `src/Application/RoguelikeGameScene.cpp`

**実装内容**:

```cpp
// include/Application/RoguelikeGameScene.h
#pragma once
#include "Application/IScene.h"
#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/SystemRunner.h"

namespace Application {

class RoguelikeGameScene : public IScene {
public:
    RoguelikeGameScene(Core::World& world, Core::GameContext& context);
    ~RoguelikeGameScene() override = default;
    
    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;
    void Render() override;
    
    std::string GetName() const override { return "Roguelike"; }
    
private:
    Core::World& world_;
    Core::GameContext& context_;
    Core::SystemRunner systemRunner_;
    
    void RegisterRoguelikeSystems();
    void GenerateDungeon();
};

} // namespace Application
```

**チェックリスト**:

- [ ] `RoguelikeGameScene`クラス作成
- [ ] `IScene`インターフェース実装
- [ ] Roguelike固有システム登録（Movement, Turn, AI等）
- [ ] `Update()`でターン制ゲームループ
- [ ] `Render()`でグリッド描画
- [ ] ESCキーでホームに戻る機能
- [ ] ダンジョン生成実装（基本版）

#### 2.3 UnifiedGameのシーン遷移強化

**編集ファイル**: `src/Application/UnifiedGame.cpp`

```cpp
// SetGameMode()の実装を強化
void UnifiedGame::SetGameMode(Application::GameMode mode) {
    if (currentMode_ == mode) return;
    
    // 現在のシーンを終了
    if (currentScene_) {
        currentScene_->OnExit();
    }
    
    currentMode_ = mode;
    
    // 新しいシーンに切り替え
    switch (mode) {
        case GameMode::Menu:
            currentSceneName_ = "Home";
            break;
        case GameMode::TD:
            currentSceneName_ = "TDGame";
            break;
        case GameMode::Roguelike:
            currentSceneName_ = "Roguelike";
            break;
    }
    
    if (scenes_.find(currentSceneName_) != scenes_.end()) {
        currentScene_ = scenes_[currentSceneName_].get();
        currentScene_->OnEnter();
    }
}
```

**チェックリスト**:

- [ ] `SetGameMode()`でシーン切り替え実装
- [ ] HomeScene ⇄ TDGameScene 遷移
- [ ] HomeScene ⇄ RoguelikeGameScene 遷移
- [ ] シーン遷移時のリソース管理
- [ ] `OnEnter()`/`OnExit()`の適切な呼び出し

#### 2.4 CMakeLists.txt更新

**編集ファイル**: `CMakeLists.txt`

```cmake
# SimpleTDCGameターゲットにファイル追加
set(UNIFIED_SOURCES
    # ... 既存のファイル
    src/Application/HomeScene.cpp
    src/Application/TDGameScene.cpp        # 追加
    src/Application/RoguelikeGameScene.cpp # 追加
    src/Application/UnifiedGame.cpp
    # ...
)
```

**チェックリスト**:

- [ ] `TDGameScene.cpp`を追加
- [ ] `RoguelikeGameScene.cpp`を追加
- [ ] ビルドが成功することを確認

### 🎯 完了条件

- ✅ HomeSceneからTDゲームを選択して遊べる
- ✅ HomeSceneからRoguelikeを選択して遊べる
- ✅ ESCキーで各ゲームからホームに戻れる
- ✅ シーン遷移時のメモリリークがない
- ✅ 60FPS維持

### 💡 Cursorでの作業手順

1. **TDGameScene実装**（Cursor Composer）

   ```
   include/Application/TDGameScene.hとsrc/Application/TDGameScene.cppを作成。
   ISceneを実装し、TD固有のシステムを登録してください。
   Phase 2.1の仕様に従ってください。
   ```

2. **RoguelikeGameScene実装**（Cursor Composer）

   ```
   include/Application/RoguelikeGameScene.hとsrc/Application/RoguelikeGameScene.cppを作成。
   Phase 2.2の仕様に従ってください。
   ```

3. **UnifiedGame強化**（Cursor Chat）

   ```
   UnifiedGame::SetGameMode()を強化し、シーン遷移を実装してください。
   Phase 2.3の仕様を参照。
   ```

4. **CMakeLists.txt更新**（手動編集）

5. **ビルド＆テスト**

   ```bash
   cmake --build build --config Release
   ./build/bin/Release/SimpleTDCGame.exe
   ```

---

## Phase 3: 既存実装統合（TD/Roguelike） 🟢 中期

**優先度**: 🟢 中  
**予想工数**: 16時間  
**担当**: 段階的実装推奨

### 📌 目的

過去に実装されたRoguelike/TD関連コンポーネント・システムを、
現在の統合プラットフォームに段階的に統合し、実際に遊べる状態にする。

### 📊 現状分析

- ✅ Roguelike コンポーネント定義済み（`include/Domain/Roguelike/Components/`）
  - GridComponents.h（グリッド座標、タイル、マップ）
  - RoguelikeComponents.h（プレイヤー、敵、ステータス）
  - TurnComponents.h（ターン管理）
  - CombatComponents.h（戦闘）
  - ItemComponents.h（アイテム）
  - HungerComponents.h（満腹度）

- ✅ Roguelike システム実装済み（`include/Domain/Roguelike/Systems/`）
  - ActionSystem.h（行動実行）
  - AISystem.h（敵AI）
  - CombatSystem.h（戦闘）
  - FOVSystem.h（視野）
  - HungerSystem.h（満腹度）
  - InputSystem.h（入力処理）
  - ItemSystem.h（アイテム）
  - LevelUpSystem.h（レベルアップ）

- ✅ Roguelike マネージャー実装済み（`include/Domain/Roguelike/Managers/`）
  - TurnManager.h（ターン管理）

- ✅ TD コンポーネント定義済み（`include/Domain/TD/Components/`）
  - TDComponents.h

- ✅ TD システム実装済み（`include/Domain/TD/Systems/`）
  - TDSystems.h

- ✅ TD マネージャー実装済み（`include/Domain/TD/Managers/`）
  - WaveManager, SpawnManager, GameStateManager

- ❌ GameSceneへのシステム統合が未完了
- ❌ ダンジョン生成システムが未完全
- ❌ 両ゲームモードの動作確認が未完了

### ✅ タスクリスト

#### 3.1 Roguelike システム統合

**編集ファイル**: `src/Application/RoguelikeGameScene.cpp`

**実装内容**:

- [ ] ActionSystem の登録と実行
- [ ] AISystem の登録と実行
- [ ] CombatSystem の登録と実行
- [ ] FOVSystem の登録と実行
- [ ] InputSystem の登録と実行
- [ ] ItemSystem の登録と実行
- [ ] TurnManager の統合
- [ ] グリッド表示システムの実装
- [ ] ダンジョン生成システムの統合

**参考ファイル**:

```
include/Domain/Roguelike/Systems/*.h
include/Domain/Roguelike/Components/*.h
include/Domain/Roguelike/Managers/*.h
```

#### 3.2 ダンジョン生成システムの統合

**参照**: Roguelike設計ドキュメント

**実装内容**:

- [ ] DungeonGenerator インターフェース定義（存在確認）
- [ ] BSPアルゴリズムの統合
- [ ] 部屋生成の統合
- [ ] 通路生成の統合
- [ ] エンティティ配置（プレイヤー、敵、アイテム）

#### 3.3 Roguelike 描画システム実装

**新規ファイル**: `src/Application/Renderers/RoguelikeRenderer.cpp`

**実装内容**:

- [ ] グリッドレイアウト描画
- [ ] タイル描画（床、壁、階段）
- [ ] エンティティ描画（プレイヤー、敵）
- [ ] UI描画（HP、ターン数、ステータス）
- [ ] FOV による視野表示

#### 3.4 TD システム統合

**編集ファイル**: `src/Application/TDGameScene.cpp`

**実装内容**:

- [ ] TDSystems の登録と実行
- [ ] WaveManager の統合
- [ ] SpawnManager の統合
- [ ] GameStateManager の統合
- [ ] ステージ管理システムの実装
- [ ] ゲーム終了条件（クリア/失敗）の実装

**参考ファイル**:

```
include/Domain/TD/Systems/TDSystems.h
include/Domain/TD/Managers/*.h
include/Domain/TD/Components/TDComponents.h
```

#### 3.5 TD 描画システム実装

**新規ファイル**: `src/Application/Renderers/TDRenderer.cpp`

**実装内容**:

- [ ] ゲームボード描画
- [ ] レーン描画
- [ ] ユニット描画（味方タワー、敵）
- [ ] UI描画（HP、Wave、資源）
- [ ] エフェクト描画

#### 3.6 ステージ/ダンジョン データ読み込み

**編集ファイル**: `RoguelikeGameScene.cpp`, `TDGameScene.cpp`

**実装内容**:

- [ ] JSON ステージ定義の読み込み
- [ ] ダンジョン生成またはステージ読み込み
- [ ] 初期エンティティ配置
- [ ] ゲーム状態の初期化

#### 3.7 統合テスト

**テスト対象**:

- [ ] Roguelike ゲームプレイ（移動、攻撃、アイテム）
- [ ] TD ゲームプレイ（Wave、敵出現、タワー配置）
- [ ] メモリリーク確認（Valgrind/Dr.Memory）
- [ ] FPS 計測（60FPS 維持確認）

### 🎯 完了条件

- ✅ Roguelike がダンジョン生成で遊べる状態
- ✅ TD がステージ読み込みで遊べる状態
- ✅ 両ゲーム間のシーン切り替えが円滑
- ✅ メモリリークがない
- ✅ 60FPS 維持確認

### 💡 Cursorでの作業手順

#### 段階的実装推奨（複数セッション）

**セッション1: Roguelike システム統合**

```
@RoguelikeGameScene.cpp, @ActionSystem.h, @AISystem.h
RoguelikeGameScene に ActionSystem, AISystem を統合してください。
Phase 3.1 の仕様に従ってください。
```

**セッション2: ダンジョン生成統合**

```
ダンジョン生成システムを RoguelikeGameScene に統合してください。
DungeonGenerator と MapData を使用した初期化を実装。
```

**セッション3: Roguelike 描画システム**

```
@RoguelikeRenderer.cpp を作成し、グリッド表示、タイル描画、エンティティ描画を実装。
```

**セッション4: TD システム統合**

```
@TDGameScene.cpp
TD システムを統合し、ステージ読み込み、敵出現、ゲーム終了条件を実装。
```

**セッション5: 統合テスト**

```
ビルドテスト、実行テスト、パフォーマンステスト。
```

---

## Phase 4: アイテム共有システム 🔵 長期

**優先度**: 🔵 低（Phase 3完了後）  
**予想工数**: 16時間

### 📌 目的

TD/Roguelike間でアイテムを共有し、統合プラットフォームとしての価値を高める。

### 🎨 設計方針

#### 共通アイテム定義スキーマ

```json
{
  "id": "sword_basic",
  "name": "ショートソード",
  "description": "基本的な剣",
  "type": "weapon",
  "rarity": "common",
  "gameMode": "both",
  
  "stats": {
    "attack": 5,
    "defense": 0,
    "weight": 3,
    "value": 50
  },
  
  "tdSpecific": {
    "deployable": false,
    "costModifier": 0
  },
  
  "roguelikeSpecific": {
    "equipSlot": "weapon",
    "identified": false,
    "stackable": false
  },
  
  "effects": [
    {
      "type": "damageBonus",
      "value": 5
    }
  ]
}
```

### ✅ タスクリスト

#### 4.1 共通アイテム定義

**新規ファイル**: `include/Data/Definitions/ItemDef.h`

```cpp
namespace Data {

enum class ItemType {
    Weapon,
    Armor,
    Consumable,
    Special
};

enum class GameModeSupport {
    TD,
    Roguelike,
    Both
};

struct ItemStats {
    int attack = 0;
    int defense = 0;
    int weight = 0;
    int value = 0;
};

struct TDSpecific {
    bool deployable = false;
    int costModifier = 0;
};

struct RoguelikeSpecific {
    std::string equipSlot;
    bool identified = true;
    bool stackable = false;
};

struct ItemEffect {
    std::string type;
    int value;
};

struct ItemDef {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    std::string rarity;
    GameModeSupport gameMode;
    
    ItemStats stats;
    TDSpecific tdData;
    RoguelikeSpecific roguelikeData;
    std::vector<ItemEffect> effects;
};

} // namespace Data
```

**チェックリスト**:

- [ ] `ItemDef`構造体定義
- [ ] enum定義（ItemType, GameModeSupport）
- [ ] TDSpecific/RoguelikeSpecific構造体

#### 4.2 ItemLoader実装

**新規ファイル**:

- `include/Data/Loaders/ItemLoader.h`
- `src/Data/Loaders/ItemLoader.cpp`

**チェックリスト**:

- [ ] JSON → ItemDef 変換
- [ ] バリデーション（必須フィールドチェック）
- [ ] エラーハンドリング

#### 4.3 ItemSerializer実装

**新規ファイル**:

- `include/Data/Serializers/ItemSerializer.h`
- `src/Data/Serializers/ItemSerializer.cpp`

**チェックリスト**:

- [ ] ItemDef → JSON 変換
- [ ] 整形出力（インデント付き）

#### 4.4 DefinitionRegistryにItems統合

**編集ファイル**: `include/Data/Registry.h`

```cpp
class DefinitionRegistry {
public:
    // ... 既存メソッド
    
    void RegisterItem(const ItemDef& item);
    ItemDef* GetItem(const std::string& id);
    std::vector<ItemDef*> GetItemsByGameMode(GameModeSupport mode);
    
private:
    std::unordered_map<std::string, ItemDef> items_;
};
```

**チェックリスト**:

- [ ] `RegisterItem()`追加
- [ ] `GetItem()`追加
- [ ] `GetItemsByGameMode()`追加（gameModeフィルタ）

#### 4.5 サンプルアイテム作成

**ディレクトリ構成**:

```
assets/definitions/items/
├── weapons/
│   ├── sword_basic.item.json
│   ├── bow_basic.item.json
│   └── staff_basic.item.json
├── armor/
│   ├── leather_armor.item.json
│   └── plate_armor.item.json
├── consumables/
│   ├── health_potion.item.json
│   └── mana_potion.item.json
└── special/
    └── upgrade_token.item.json
```

**チェックリスト**:

- [ ] 武器10種（剣、弓、杖等）
- [ ] 防具10種（軽装、重装、盾等）
- [ ] 消耗品10種（ポーション、巻物等）
- [ ] 特殊アイテム5種（アップグレード、鍵等）

### 🎯 完了条件

- ✅ TD/Roguelike両方でアイテムが使用できる
- ✅ アイテム定義がJSON化されている
- ✅ 共通アイテムが両ゲームモードで動作する
- ✅ サンプルアイテムが35種以上ある

---

## Phase 5: HTTPサーバー開発者モード完成 🟡 重要

**優先度**: 🟡 高  
**予想工数**: 20時間

### 📌 目的

WebUIエディター経由で全てのゲーム要素を編集・作成できるようにする。

### 📊 現状分析

- ✅ Characters, Stages, UI Layouts API実装済み
- ✅ WebUIエディター（Entity, Stage, UI）実装済み
- ❌ Skills, Effects, Sounds API未実装
- ❌ Maps（Roguelike）API未実装
- ❌ Items API未実装

### ✅ タスクリスト

#### 5.1 Skills API実装

**編集ファイル**: `src/Core/HTTPServer.cpp`

**エンドポイント**:

```
GET    /api/skills           # スキル一覧取得
POST   /api/skills           # 新規スキル作成
GET    /api/skills/{id}      # スキル詳細取得
PUT    /api/skills/{id}      # スキル更新
DELETE /api/skills/{id}      # スキル削除
POST   /api/skills/validate  # スキル定義検証
```

**チェックリスト**:

- [ ] エンドポイント実装
- [ ] SkillSerializer統合
- [ ] ホットリロード対応
- [ ] バリデーション実装

#### 5.2 Effects API実装

**エンドポイント**:

```
GET/POST/PUT/DELETE /api/effects
POST /api/effects/validate
```

**チェックリスト**:

- [ ] エンドポイント実装
- [ ] EffectSerializer統合
- [ ] ホットリロード対応

#### 5.3 Sounds API実装

**エンドポイント**:

```
GET/POST/PUT/DELETE /api/sounds
POST /api/sounds/validate
```

**チェックリスト**:

- [ ] エンドポイント実装
- [ ] SoundSerializer統合
- [ ] ホットリロード対応

#### 5.4 Maps API実装（Roguelike用）

**エンドポイント**:

```
GET/POST/PUT/DELETE /api/maps
POST /api/maps/validate
```

**チェックリスト**:

- [ ] エンドポイント実装
- [ ] MapSerializer作成・実装
- [ ] ホットリロード対応

#### 5.5 Items API実装

**エンドポイント**:

```
GET    /api/items
POST   /api/items
GET    /api/items/{id}
PUT    /api/items/{id}
DELETE /api/items/{id}
POST   /api/items/validate
GET    /api/items?gameMode={td|roguelike|both}
```

**チェックリスト**:

- [ ] エンドポイント実装
- [ ] ItemSerializer統合
- [ ] gameModeフィルタ機能
- [ ] ホットリロード対応

#### 5.6 WebUIエディター拡張

**新規ファイル**:

- `tools/webui-editor/src/components/Editors/MapEditor.tsx`
- `tools/webui-editor/src/components/Editors/ItemEditor.tsx`
- `tools/webui-editor/src/types/map.ts`
- `tools/webui-editor/src/types/item.ts`
- `tools/webui-editor/src/api/maps.ts`
- `tools/webui-editor/src/api/items.ts`

**チェックリスト**:

- [ ] MapEditor実装
- [ ] ItemEditor実装
- [ ] 型定義追加
- [ ] API統合
- [ ] ナビゲーション更新

### 🎯 完了条件

- ✅ 全ての定義ファイルをWebUIで編集できる
- ✅ ホットリロードが全定義タイプで動作する
- ✅ バリデーションエラーが適切に表示される
- ✅ プレビュー機能が動作する

---

## Phase 6: ドキュメント整理 📚 重要

**優先度**: 📚 中  
**予想工数**: 4時間  
**担当**: AIエージェント推奨（並行作業可能）

### 📌 目的

プロジェクトドキュメントを整理し、新規開発者の参入障壁を下げる。

### ✅ タスクリスト

#### 6.1 不要ドキュメントの削除

```bash
git rm docs/README_REORGANIZED.md
```

**チェックリスト**:

- [ ] `docs/README_REORGANIZED.md`削除（README.mdに統合済み）

#### 6.2 カテゴリ別フォルダ構成

**新構成**:

```
docs/
├── README.md (索引)
├── 00_EXECUTIVE_SUMMARY.md
├── Architecture/
│   ├── CODE_ANALYSIS.md
│   ├── INTEGRATION_STATUS.md
│   └── REFACTORING_PLAN.md
├── Design/
│   ├── CHARACTER_SYSTEM_DESIGN.md
│   └── ROGUELIKE_SYSTEM_DESIGN.md
├── Development/
│   ├── DEVELOPER_GUIDE.md
│   ├── BUILD_WITH_NINJA.md
│   └── MIGRATION_GUIDE.md
├── Technical/
│   ├── FONT_SETUP.md
│   ├── OPTIMIZATION_SUMMARY.md
│   └── HANDOVER.md
└── Archive/
    ├── PHASE5_REPORT.md
    ├── PHASE6_REPORT.md
    └── PHASE6_PLAN.md
```

**チェックリスト**:

- [ ] `docs/Architecture/`フォルダ作成
- [ ] `docs/Design/`フォルダ作成
- [ ] `docs/Development/`フォルダ作成
- [ ] `docs/Technical/`フォルダ作成
- [ ] `docs/Archive/`フォルダ作成
- [ ] 各ドキュメントを移動
- [ ] README.md索引更新

#### 6.3 統合ドキュメント作成

**新規ファイル**: `docs/Development/DEVELOPER_GUIDE.md`

**内容**:

- DEVELOPER_MANUALの内容を統合
- QUICK_REFERENCEの内容を統合
- このTODO.mdへのリンクを追加

**チェックリスト**:

- [ ] DEVELOPER_GUIDE.md作成
- [ ] DEVELOPER_MANUAL.md削除
- [ ] QUICK_REFERENCE.md削除

#### 6.4 README.md更新

**編集箇所**:

```markdown
## 開発ロードマップ

詳細な実装計画は [.cursor/UNIFIED_PLATFORM_TODO.md](.cursor/UNIFIED_PLATFORM_TODO.md) を参照してください。

## ドキュメント

### 概要・設計ドキュメント
- [エグゼクティブサマリー](docs/00_EXECUTIVE_SUMMARY.md)
- [アーキテクチャ分析](docs/Architecture/)
- [システム設計](docs/Design/)

### 開発者向け
- [開発者ガイド](docs/Development/DEVELOPER_GUIDE.md)
- [ビルド手順](docs/Development/BUILD_WITH_NINJA.md)

### 技術情報
- [最適化](docs/Technical/)
```

**チェックリスト**:

- [ ] ゲームプロジェクト表を更新
- [ ] ドキュメント索引を新構成に更新
- [ ] TODO.mdへのリンクを追加
- [ ] Phase 7の進捗状況を更新

### 🎯 完了条件

- ✅ docs/がカテゴリ別に整理されている
- ✅ 重複ドキュメントが削除されている
- ✅ README.mdが最新の構成を反映している
- ✅ 新規開発者がドキュメントを見つけやすい

---

## マイルストーン

### 🎯 マイルストーン 1: 統合プラットフォーム基盤完成

**期限**: 2週間以内  
**含まれるPhase**: 1, 2, 6

**完了条件**:

- ✅ 旧TDコード完全削除
- ✅ HomeSceneからTD/Roguelikeを選択して遊べる
- ✅ ドキュメントが整理されている

**状態**: ホームシーンから両ゲームモードを選択可能

---

### 🎯 マイルストーン 2: 開発者モード完成

**期限**: 4週間以内  
**含まれるPhase**: 5

**完了条件**:

- ✅ WebUIで全ゲーム要素を編集可能
- ✅ ホットリロード動作
- ✅ バリデーション実装

**状態**: 開発効率が大幅に向上

---

### 🎯 マイルストーン 3: Roguelike実装

**期限**: 8週間以内  
**含まれるPhase**: 3

**完了条件**:

- ✅ Roguelikeが基本機能込みで遊べる
- ✅ ダンジョン生成動作
- ✅ ターン制戦闘実装

**状態**: 2つのゲームが完全に遊べる

---

### 🎯 マイルストーン 4: 統合ゲームプラットフォーム完成

**期限**: 12週間以内  
**含まれるPhase**: 4, 7, 8

**完了条件**:

- ✅ TD/Roguelike間でアイテム共有
- ✅ 豊富なサンプルコンテンツ
- ✅ パフォーマンス最適化

**状態**: プロダクション品質

---

## 実装優先順位

### 🔴 最優先（完了）

1. ✅ **Phase 1: 旧TD完全廃止** - 完了
2. ✅ **Phase 2: ゲームシーン実装** - 完了

### 🟡 高優先（Phase 3が次）

3. **Phase 3: 既存実装統合（TD/Roguelike）** - 次のフェーズ
   - Roguelike システム統合（ActionSystem, AISystem等）
   - TD システム統合（TDSystems, Wave管理等）
   - 両ゲーム動作確認

### 🟢 中優先（Phase 3完了後）

4. **Phase 6: ドキュメント整理** - 並行作業可能
5. **Phase 5: HTTPサーバー開発者モード完成** - WebUI完成

### 🔵 低優先（Phase 5完了後）

6. **Phase 4: アイテム共有システム** - 統合プラットフォームの価値向上
7. **Phase 7: サンプルデータ拡充** - ゲーム体験向上（12時間）
8. **Phase 8: パフォーマンス最適化** - 品質向上（8時間）

---

## Cursor開発ワークフロー

### 🚀 推奨開発フロー

#### 1. タスク開始前

```
Cursor Composer: @UNIFIED_PLATFORM_TODO.md の Phase X を開始します。
関連ファイルを確認してください。
```

#### 2. 実装中

- **Cursor Chat**で質問・確認
- **Cursor Composer**で複数ファイル一括編集
- **Ctrl+Shift+B**で頻繁にビルドテスト

#### 3. タスク完了後

- [ ] ビルドテスト（Release）
- [ ] 実行テスト（実際に動作確認）
- [ ] コミット（適切なメッセージ）
- [ ] このTODOのチェックボックスを更新

### 🛠️ Cursor便利機能

#### Composer活用

```
# 複数ファイル一括生成
@UNIFIED_PLATFORM_TODO.md Phase 2.1のTDGameSceneを実装してください。
ヘッダーと実装ファイルの両方を生成。

# リファクタリング
@UnifiedGame.cpp のSetGameMode()を Phase 2.3の仕様に従って強化してください。
```

#### Chat活用

```
# 設計相談
Phase 3のダンジョン生成アルゴリズムについて、BSP法の実装方針を教えてください。

# バグ調査
TDGameSceneでメモリリークが発生しています。原因を特定してください。
```

#### ショートカット

- `Ctrl+K` - Cursor Chat起動
- `Ctrl+Shift+I` - Cursor Composer起動
- `Ctrl+Shift+B` - ビルドタスク実行
- `Ctrl+Shift+P` - コマンドパレット

---

## 進捗管理

### ✅ 完了済み

- ✅ Phase 1: 旧TD廃止
- ✅ Phase 2: ゲームシーン実装（HomeScene, TDGameScene, RoguelikeGameScene）
- ✅ Phase 3: 既存実装統合（基本実装完了、詳細確認待ち）
  - ✅ 3.1: Roguelike システム統合
  - ✅ 3.2: ダンジョン生成統合
  - ✅ 3.3: Roguelike 描画
  - ✅ 3.4: TD システム統合
  - ✅ 3.5: TD 描画
  - ✅ 3.6-7: 統合テスト（ビルド・実行確認済み）
- ✅ HomeScene完全実装
- ✅ UnifiedGame統合
- ✅ HTTPサーバー基盤（Characters, Stages, UI）
- ✅ WebUIエディター基盤（Entity, Stage, UI）

### ⏳ 進行中

- ⏳ Phase 3修正: InputSystem/StageDef シグネチャ確認

### 📋 未着手

- 📋 Phase 4-8

---

## 注意事項

### コーディング規約

詳細は `.github/copilot-instructions.md` を参照

**主要規約**:

- EnTTコンポーネントはPOD型として定義
- JSON解析は必ずtry-catchで囲む
- 1コミット = 1機能単位
- コミットメッセージプレフィックス: `feat:`, `fix:`, `refactor:`, `docs:`, `chore:`

### テスト

各Phase完了時:

- [ ] `cmake --build build --config Release` が成功
- [ ] `SimpleTDCGame.exe` が起動
- [ ] メモリリークチェック（Valgrind or Dr. Memory）
- [ ] 60FPS維持確認

### ドキュメント

- 新機能実装時は該当ドキュメントを更新
- Phase完了時にREADME.mdの進捗状況を更新
- コード内コメントは日本語または英語

---

## 更新履歴

- **2025-12-04**: TODO作成（Phase 1-8定義）
- **2025-12-04**: Cursor開発者向けに再構成、`.cursor/`に配置

---

## 関連ドキュメント

- [Cursor開発ガイド](.cursor/CURSOR_DEVELOPMENT_GUIDE.md)
- [AIエージェントクイックリファレンス](.cursor/AI_AGENT_QUICK_REFERENCE.md)
- [アーキテクチャ図解](.cursor/ARCHITECTURE_DIAGRAMS.md)
- [コーディング規約](../.github/copilot-instructions.md)
- [プロジェクトREADME](../README.md)
