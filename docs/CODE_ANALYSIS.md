# プロジェクト全体コード分析レポート

**作成日**: 2025-12-02  
**分析対象**: Simple-TDC-GameProject  
**目的**: プロジェクト全体の現状把握、アーキテクチャ評価、改善点の特定

---

## 目次

1. [プロジェクト概要](#1-プロジェクト概要)
2. [コードベース統計](#2-コードベース統計)
3. [アーキテクチャ分析](#3-アーキテクチャ分析)
4. [コンポーネント設計評価](#4-コンポーネント設計評価)
5. [システム間依存関係](#5-システム間依存関係)
6. [重複コードと問題点](#6-重複コードと問題点)
7. [品質評価](#7-品質評価)
8. [推奨事項](#8-推奨事項)

---

## 1. プロジェクト概要

### 1.1 プロジェクトの目的

Simple-TDC-GameProjectは、ECS（Entity Component System）アーキテクチャを採用した教育用C++ゲームプロジェクトです。以下の3つのゲームプロジェクトが統合されています：

1. **オリジナルTDゲーム** (`SimpleTDCGame`) - シーンベースの従来型アーキテクチャ
2. **新アーキテクチャTD** (`SimpleTDCGame_NewArch`) - データ駆動設計の改良版
3. **NetHack風ローグライク** (`NetHackGame`) - ターン制ローグライクゲーム

### 1.2 技術スタック

| カテゴリ | 技術 | バージョン |
|---------|------|-----------|
| 言語 | C++ | 17以上 |
| ビルドシステム | CMake | 3.19以上 |
| ECSライブラリ | EnTT | v3.12.2 |
| JSONライブラリ | nlohmann/json | v3.11.2 |
| レンダリング | Raylib | 5.0 |
| GUI（ゲーム内） | raygui | 4.0 |
| GUI（デバッグ） | Dear ImGui | v1.90.1 |
| プラットフォーム | Windows | 専用 |

---

## 2. コードベース統計

### 2.1 コード量

| ファイル種別 | ファイル数 | 総行数 |
|-------------|-----------|--------|
| ヘッダーファイル (.h, .hpp) | 90 | 17,586 |
| ソースファイル (.cpp) | 19 | 3,792 |
| **合計** | **109** | **21,378** |

### 2.2 ディレクトリ構造

```
Simple-TDC-GameProject/
├── include/                        # ヘッダーファイル (17,586行)
│   ├── Core/                       # Core層: 基盤ECSシステム
│   │   ├── Components/             # Core層コンポーネント
│   │   ├── AnimationPlayer.h
│   │   ├── DefinitionLoader.h
│   │   ├── DefinitionRegistry.h
│   │   ├── Definitions.h
│   │   ├── EffectManager.h
│   │   ├── EntityFactory.h
│   │   ├── Events.h
│   │   ├── FallbackRenderer.h
│   │   ├── FileUtils.h
│   │   ├── GameContext.h
│   │   ├── GameRenderer.h
│   │   ├── Platform.h
│   │   ├── SoundManager.h
│   │   ├── SystemRunner.h
│   │   ├── UIDefinitions.h
│   │   ├── UILoader.h
│   │   ├── UIRenderer.h
│   │   └── World.h
│   ├── Game/                       # Game層: 汎用ゲーム機能
│   │   └── Components/
│   │       └── GameComponents.h
│   ├── TD/                         # TD層: タワーディフェンス固有
│   │   ├── Components/
│   │   │   └── TDComponents.h
│   │   ├── Managers/
│   │   │   ├── GameStateManager.h
│   │   │   ├── SpawnManager.h
│   │   │   └── WaveManager.h
│   │   ├── Systems/
│   │   │   └── TDSystems.h
│   │   └── UI/
│   │       └── GameUI.h
│   ├── Roguelike/                  # Roguelike層: ローグライク固有
│   │   ├── Components/
│   │   │   ├── CombatComponents.h
│   │   │   ├── GridComponents.h
│   │   │   ├── HungerComponents.h
│   │   │   ├── ItemComponents.h
│   │   │   ├── RoguelikeComponents.h
│   │   │   └── TurnComponents.h
│   │   ├── Generators/
│   │   │   ├── DungeonGenerator.h
│   │   │   ├── ItemSpawner.h
│   │   │   └── MonsterSpawner.h
│   │   ├── Managers/
│   │   │   └── TurnManager.h
│   │   ├── Rendering/
│   │   │   └── TileRenderer.h
│   │   ├── Systems/
│   │   │   ├── AISystem.h
│   │   │   ├── ActionSystem.h
│   │   │   ├── CombatSystem.h
│   │   │   ├── FOVSystem.h
│   │   │   ├── HungerSystem.h
│   │   │   ├── InputSystem.h
│   │   │   ├── ItemSystem.h
│   │   │   └── LevelUpSystem.h
│   │   └── RoguelikeGame.h
│   ├── Data/                       # データローダー
│   │   ├── AnimationDef.h
│   │   ├── AnimationLoader.h
│   │   ├── AsepriteLoader.h
│   │   ├── EffectDef.h
│   │   ├── EffectLoader.h
│   │   ├── SoundDef.h
│   │   └── SoundLoader.h
│   ├── Scenes/                     # シーン実装（旧アーキテクチャ）
│   │   ├── HomeScene.h
│   │   ├── NethackGameScene.h
│   │   ├── TDGameScene.h
│   │   ├── TDTestGameScene.h
│   │   └── TitleScene.h
│   ├── AnimationSystem.h           # 旧アニメーションシステム
│   ├── Components.h                # 旧コンポーネント定義
│   ├── ComponentsNew.h             # 統合コンポーネントヘッダー
│   ├── ConfigManager.h             # 設定管理（Singleton）
│   ├── Game.h                      # 旧ゲームクラス
│   ├── GameNew.h                   # 新ゲームクラス
│   ├── InputManager.h              # 入力管理（Singleton）
│   ├── ResourceManager.h           # リソース管理（Singleton）
│   ├── SceneManager.h              # シーン管理（Singleton）
│   ├── System.h                    # 旧システムインターフェース
│   ├── SystemManager.h             # 旧システム管理
│   ├── Systems.h                   # 旧システム実装
│   └── UIManager.h                 # UI管理（Singleton）
├── src/                            # ソースファイル (3,792行)
│   ├── Scenes/
│   │   ├── HomeScene.cpp
│   │   ├── NethackGameScene.cpp
│   │   ├── TDGameScene.cpp
│   │   ├── TDTestGameScene.cpp
│   │   └── TitleScene.cpp
│   ├── ConfigManager.cpp
│   ├── FileUtils.cpp
│   ├── Game.cpp
│   ├── GameNew.cpp                 # 新アーキテクチャ実装
│   ├── InputManager.cpp
│   ├── ResourceManager.cpp
│   ├── SceneManager.cpp
│   ├── SystemManager.cpp
│   ├── Systems.cpp
│   ├── TestScene.cpp
│   ├── UIManager.cpp
│   ├── main.cpp                    # 旧アーキテクチャエントリポイント
│   ├── main_new.cpp                # 新アーキテクチャエントリポイント
│   └── main_roguelike.cpp          # ローグライクエントリポイント
├── assets/                         # アセット
│   ├── config.json
│   └── fonts/
└── docs/                           # ドキュメント
    ├── BUILD_WITH_NINJA.md
    ├── CHARACTER_SYSTEM_DESIGN.md
    ├── FONT_SETUP.md
    ├── HANDOVER.md
    ├── MIGRATION_GUIDE.md
    ├── OPTIMIZATION_SUMMARY.md
    └── ROGUELIKE_SYSTEM_DESIGN.md
```

---

## 3. アーキテクチャ分析

### 3.1 アーキテクチャパターン

プロジェクトは**3層アーキテクチャ**を採用しており、レイヤーごとに責務が分離されています：

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Game.cpp     │  │ GameNew.cpp  │  │ Roguelike    │      │
│  │ (旧)         │  │ (新)         │  │ Game         │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
         │                   │                   │
         ▼                   ▼                   ▼
┌──────────────┐  ┌──────────────────────┐  ┌────────────────┐
│  旧シーン層   │  │    新アーキテクチャ   │  │  Roguelike層  │
│              │  │                      │  │                │
│ Singleton    │  │ GameContext (DI)    │  │ 固有システム    │
│ 依存         │  │ World (ECS)         │  │ ターン制       │
│              │  │ SystemRunner        │  │ グリッド       │
└──────────────┘  └──────────────────────┘  └────────────────┘
         │                   │                   │
         └───────────────────┴───────────────────┘
                             │
                             ▼
         ┌───────────────────────────────────────────┐
         │           Core Layer (共通基盤)            │
         │                                           │
         │ • World (entt::registry wrapper)         │
         │ • GameContext (DI Container)             │
         │ • SystemRunner (システム実行管理)          │
         │ • EntityFactory (エンティティ生成)         │
         │ • DefinitionRegistry (データ管理)         │
         │ • GameRenderer (FHD固定レンダリング)       │
         └───────────────────────────────────────────┘
```

### 3.2 各アーキテクチャの評価

#### 3.2.1 旧アーキテクチャ（SimpleTDCGame）

**特徴:**
- Singletonパターンの多用（5つ以上）
- シーンベースの構造
- 直接的なシステム呼び出し

**利点:**
- ✅ シンプルで理解しやすい
- ✅ 小規模プロジェクトに適している
- ✅ グローバルアクセスが容易

**問題点:**
- ❌ Singleton依存によるテスト困難性
- ❌ シーン間でのコード重複
- ❌ 暗黙的な依存関係
- ❌ マルチスレッド対応困難
- ❌ 拡張性が低い

#### 3.2.2 新アーキテクチャ（SimpleTDCGame_NewArch）

**特徴:**
- 依存性注入（DI）パターン
- データ駆動設計
- FHD固定レンダリング（1920x1080）
- システム実行順序の明示的管理

**利点:**
- ✅ テスタビリティが高い
- ✅ 依存関係が明示的
- ✅ 拡張性が高い
- ✅ JSON定義による柔軟性
- ✅ 解像度非依存の座標系

**問題点:**
- ⚠️ 学習コストが高い
- ⚠️ ボイラープレートコードが増える
- ⚠️ 旧コードとの統合が未完了

#### 3.2.3 Roguelikeアーキテクチャ（NetHackGame）

**特徴:**
- ターン制システム
- グリッドベース移動
- ヘッダーオンリー実装（多くが未実装）

**利点:**
- ✅ 明確なドメイン分離
- ✅ NetHackの設計思想を踏襲
- ✅ 拡張可能な設計

**問題点:**
- ❌ 実装が大部分未完成
- ❌ Core層との統合方針が不明確
- ❌ ドキュメントのみで実装が追いついていない

### 3.3 レイヤー間の依存関係

```
Roguelike層 ──────┐
                  │
TD層 ─────────────┼──→ Core層 ← Game層
                  │
旧Scenes層 ───────┘
```

**依存の方向性:**
- すべての上位層がCore層に依存（正しい）
- Roguelike層とTD層は独立（良い設計）
- 旧Scenes層が独自のSingletonに依存（改善必要）

---

## 4. コンポーネント設計評価

### 4.1 コンポーネントの種類と数

| レイヤー | ファイル | 主なコンポーネント数 |
|---------|---------|-------------------|
| Core | CoreComponents.h | 5-10 |
| Game | GameComponents.h | 10-15 |
| TD | TDComponents.h | 20+ |
| Roguelike | 6ファイル | 30+ |

### 4.2 コンポーネント設計の評価

#### ✅ 良い点

1. **データ指向設計の遵守**
   ```cpp
   // Core層コンポーネント例
   struct Transform {
       Vector2 position;
       float rotation;
       Vector2 scale;
   };
   ```
   - POD型（Plain Old Data）として定義
   - ロジックを含まない純粋なデータ構造

2. **適切な粒度**
   ```cpp
   // TD層コンポーネント例
   struct LanePosition { float x; int lane; };
   struct LaneMovement { float speed; int direction; };
   ```
   - 単一責任原則を守っている
   - 組み合わせで機能を実現

3. **フォールバック対応**
   ```cpp
   struct FallbackVisual {
       Shape shape;
       Color primaryColor;
       Color secondaryColor;
       float size;
   };
   ```
   - アセット欠損時の対応が組み込まれている

#### ❌ 問題点

1. **コンポーネントの重複**
   - `Components.h`（旧）と`ComponentsNew.h`（新）が共存
   - `Position`が複数の場所で定義されている可能性

2. **過度に大きなコンポーネント**
   ```cpp
   // 改善が必要な例
   struct Stats {
       float maxHealth;
       float currentHealth;
       float attackPower;
       float attackSpeed;
       float attackRange;
       float knockbackResist;
       float cost;
       float cooldown;
   };
   ```
   - 単一責任原則違反の可能性
   - 分割を検討すべき

### 4.3 コンポーネント命名規則

**一貫性のある命名:**
- ✅ Core層: `Transform`, `Velocity`
- ✅ Game層: `Sprite`, `Animation`
- ✅ TD層: `LanePosition`, `LaneMovement`
- ✅ Roguelike層: `GridPosition`, `TurnActor`

**命名パターン:**
- データを表す名詞形
- 機能を表す動詞+名詞形（`LaneMovement`）
- タグコンポーネントは単純な名詞（`Dead`, `Player`）

---

## 5. システム間依存関係

### 5.1 システムの分類

| 分類 | 説明 | 例 |
|-----|------|---|
| **入力システム** | ユーザー入力を処理 | InputSystem |
| **更新システム** | ゲームロジックを実行 | MovementSystem, CombatSystem |
| **描画システム** | レンダリングを担当 | RenderSystem, TileRenderer |
| **管理システム** | 高レベル制御 | WaveManager, TurnManager |

### 5.2 システム実装パターン

#### パターン1: 静的関数（推奨）

```cpp
// EnTT推奨パターン
namespace Systems {
    void MovementSystem(Core::World& world, float dt) {
        auto view = world.Registry().view<Position, Velocity>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& vel = view.get<Velocity>(entity);
            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
        }
    }
}
```

**利点:**
- ステートレス
- テストしやすい
- 並列化が容易

#### パターン2: クラスベース（旧）

```cpp
class System {
public:
    virtual void ProcessInput() = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
};
```

**問題点:**
- 余分なボイラープレート
- 状態を持ちがち
- 抽象クラスによるオーバーヘッド

### 5.3 システム実行順序

新アーキテクチャでは`SystemRunner`が明示的に順序を管理：

```
1. 入力処理
   └─ InputSystem
2. ゲームロジック更新
   ├─ MovementSystem
   ├─ CombatDetectionSystem
   ├─ CombatSystem
   ├─ HealthSystem
   ├─ DeathSystem
   └─ AbilitySystem
3. 描画
   ├─ SpriteRenderSystem
   └─ UIRenderSystem
```

---

## 6. 重複コードと問題点

### 6.1 重複コード

#### 1. コンポーネント定義の重複

**場所:**
- `include/Components.h` (旧)
- `include/ComponentsNew.h` (新・統合)
- `include/Core/Components/CoreComponents.h` (Core層)

**問題:**
- `Position`, `Velocity`などが複数箇所で定義
- メンテナンス性の低下
- 混乱の原因

**推奨対策:**
- 旧`Components.h`を段階的に廃止
- `ComponentsNew.h`を使用するよう統一

#### 2. Manager クラスの重複

**Singleton実装:**
- `ConfigManager`
- `SceneManager`
- `InputManager`
- `ResourceManager`
- `UIManager`

**新アーキテクチャの対応クラス:**
- `GameContext`（DIコンテナ）
- `WaveManager`（GameContextに登録）
- `SpawnManager`（GameContextに登録）

**問題:**
- 2つのパターンが混在
- 移行が中途半端

#### 3. ゲームクラスの重複

**3つのエントリポイント:**
1. `main.cpp` → `Game`
2. `main_new.cpp` → `GameNew`
3. `main_roguelike.cpp` → `RoguelikeGame`

**問題:**
- 同じ機能を3回実装
- ビルドターゲットが3つに分散

### 6.2 アーキテクチャ上の問題

#### 1. Singletonパターンの過剰使用

**影響:**
- グローバル状態による予測不可能性
- ユニットテストが困難
- マルチスレッド対応が難しい

**例:**
```cpp
// 問題のあるコード
auto& config = ConfigManager::GetInstance();
auto& input = InputManager::GetInstance();
```

**改善案:**
```cpp
// DIパターン
class MySystem {
    ConfigManager& config_;
    InputManager& input_;
public:
    MySystem(ConfigManager& config, InputManager& input)
        : config_(config), input_(input) {}
};
```

#### 2. シーン内のロジック重複

**問題:**
各シーン（`TDGameScene`, `TDTestGameScene`, `NethackGameScene`）で類似コードが重複

**例:**
- 入力処理
- エンティティ生成
- システム呼び出し

**改善案:**
- 共通機能を抽象基底クラスに移動
- または新アーキテクチャに完全移行

#### 3. 不明瞭な責務境界

**Core層の肥大化:**
- `Core::EntityFactory`が具体的なゲームロジックを含む
- `Core::GameRenderer`がFHD固定などゲーム固有の決定を含む

**推奨:**
- Coreは真に汎用的な機能のみ
- ゲーム固有機能はGame/TD層へ

### 6.3 未実装・未完成部分

#### 1. Roguelikeプロジェクト

**状態:** 設計ドキュメントは充実しているが、実装が大部分未完成

**未実装機能:**
- ダンジョン生成（`DungeonGenerator.h`はヘッダーのみ）
- アイテムシステム（`ItemSystem.h`はヘッダーのみ）
- AIシステム（`AISystem.h`はヘッダーのみ）
- 戦闘システム（`CombatSystem.h`はヘッダーのみ）

#### 2. WebUIエディタ

**状態:** 設計ドキュメント存在、実装未着手

**計画内容:**
- React + TypeScript
- React Flow (ノードエディタ)
- REST API
- WebSocket通信

**問題:**
- `tools/entity-editor/`ディレクトリが存在しない
- C++側のHTTPサーバー未統合

#### 3. データ定義ファイル

**状態:** 定義形式は決定、実際のアセットが不足

**不足しているもの:**
- `assets/definitions/characters/*.char.json`
- `assets/definitions/stages/*.stage.json`
- `assets/definitions/monsters/*.monster.json`
- 画像アセット（スプライトシート）

---

## 7. 品質評価

### 7.1 コード品質指標

| 指標 | 評価 | 説明 |
|-----|------|------|
| **モジュール性** | 🟡 中 | レイヤー分離は良いが、重複が多い |
| **テスタビリティ** | 🟡 中 | 新アーキは高、旧アーキは低 |
| **保守性** | 🟡 中 | ドキュメント充実だが、コード重複 |
| **拡張性** | 🟢 良 | ECS採用により拡張が容易 |
| **可読性** | 🟢 良 | コメント・命名が適切 |
| **一貫性** | 🔴 低 | 3つのアーキが混在 |

### 7.2 ドキュメント品質

**✅ 充実している点:**
- 各システムの設計ドキュメント完備
- ハンドオーバー文書が詳細
- 移行ガイドが存在
- BUILD手順が明確

**⚠️ 改善が必要な点:**
- ドキュメント間の重複（特にTD/Roguelike）
- 実装とドキュメントの乖離（特にRoguelike）
- 全体構造を示す統合ドキュメントが不足

### 7.3 ビルドシステム評価

**✅ 良い点:**
- CMakeの適切な使用
- FetchContentによる依存管理
- マルチターゲット対応

**⚠️ 問題点:**
- Windows専用（クロスプラットフォーム対応なし）
- 3つのターゲットが独立（統合されていない）

---

## 8. 推奨事項

### 8.1 短期的改善（1-2週間）

#### 優先度: 高 🔴

1. **重複コンポーネントの整理**
   - `Components.h`を非推奨化
   - `ComponentsNew.h`に統一
   - 移行ガイドの作成

2. **ドキュメント統合**
   - `CODE_ANALYSIS.md`（本ドキュメント）
   - `INTEGRATION_STATUS.md`（統合状況）
   - `REFACTORING_PLAN.md`（改善計画）

3. **ビルドターゲットの明確化**
   - デフォルトターゲットの決定
   - 各ターゲットのREADME更新

#### 優先度: 中 🟡

4. **Singleton削減の開始**
   - まず`ConfigManager`をDI化
   - 旧シーンから新アーキへの移行計画

5. **アセット整備**
   - フォールバック用のJSON定義追加
   - サンプル画像の作成

### 8.2 中期的改善（1-2ヶ月）

#### 優先度: 高 🔴

6. **アーキテクチャ統一**
   - 新アーキテクチャへの完全移行
   - 旧コードの段階的廃止

7. **Roguelike実装の完成**
   - Phase 1: 基本移動（優先）
   - Phase 2: ダンジョン生成
   - Phase 3: 戦闘システム

#### 優先度: 中 🟡

8. **テストインフラ整備**
   - Google Test導入
   - コンポーネント単体テスト
   - システム統合テスト

9. **CI/CD強化**
   - 自動テスト実行
   - コードカバレッジ測定

### 8.3 長期的改善（3ヶ月以上）

10. **WebUIエディタ実装**
    - Phase 5: HTTPサーバー統合
    - React + TypeScriptフロントエンド
    - ノードベースエディタ

11. **クロスプラットフォーム対応**
    - Linux/macOSサポート
    - CMakeの改善

12. **パフォーマンス最適化**
    - プロファイリング
    - 空間分割（Quadtree）
    - オブジェクトプーリング

---

## 付録A: ファイル別詳細分析

### 旧アーキテクチャファイル（要整理）

| ファイル | 行数 | 状態 | 推奨アクション |
|---------|------|------|---------------|
| Components.h | ? | 旧定義 | 非推奨化 |
| Game.h/cpp | ? | 旧クラス | 段階的廃止 |
| SceneManager.h/cpp | ? | Singleton | DI移行検討 |
| ConfigManager.h/cpp | ? | Singleton | DI移行検討 |
| InputManager.h/cpp | ? | Singleton | DI移行検討 |
| ResourceManager.h/cpp | ? | Singleton | DI移行検討 |
| UIManager.h/cpp | ? | Singleton | 一部維持可 |
| Systems.h/cpp | ? | 混在 | 静的関数に統一 |

### 新アーキテクチャファイル（推奨）

| ファイル | 行数 | 状態 | 推奨アクション |
|---------|------|------|---------------|
| GameNew.h/cpp | ? | 実装済み | 継続開発 |
| Core/World.h | ? | 実装済み | 継続使用 |
| Core/GameContext.h | ? | 実装済み | 継続使用 |
| Core/SystemRunner.h | ? | 実装済み | 継続使用 |
| ComponentsNew.h | ? | 統合済み | 継続使用 |

---

## まとめ

**プロジェクトの現状:**
- ✅ 技術的基盤は堅牢（ECS, データ駆動）
- ✅ ドキュメントが充実
- ⚠️ アーキテクチャが3つ混在
- ⚠️ 重複コードが多い
- ⚠️ 未実装部分が多い（特にRoguelike）

**次のステップ:**
1. 重複の整理とドキュメント統合（本レポート含む）
2. 新アーキテクチャへの統一方針決定
3. 優先度の高い機能の実装完了（Roguelike Phase 1）
4. テストとCI/CD強化

**長期的ビジョン:**
- データ駆動で拡張しやすいゲームエンジン
- WebUIエディタによる非プログラマー対応
- 複数ゲームジャンルの統合プラットフォーム

---

**関連ドキュメント:**
- [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) - 具体的なリファクタリング計画
- [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) - 3プロジェクトの統合状況
- [HANDOVER.md](./HANDOVER.md) - 新アーキテクチャの詳細
- [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike設計
- [CHARACTER_SYSTEM_DESIGN.md](./CHARACTER_SYSTEM_DESIGN.md) - キャラクターシステム設計

---

**作成者メモ:**  
本ドキュメントは2025-12-02時点のコードベースを分析した結果です。  
今後のコード変更に応じて定期的な更新が推奨されます。
