# プロジェクト統合状況レポート

**作成日**: 2025-12-02  
**プロジェクト**: Simple-TDC-GameProject  
**目的**: 3つのゲームプロジェクトの統合状況の把握

---

## 目次

1. [概要](#1-概要)
2. [3つのプロジェクト詳細](#2-3つのプロジェクト詳細)
3. [共通基盤（Core層）](#3-共通基盤core層)
4. [統合状況マトリクス](#4-統合状況マトリクス)
5. [依存関係分析](#5-依存関係分析)
6. [統合における課題](#6-統合における課題)
7. [統合戦略](#7-統合戦略)

---

## 1. 概要

### 1.1 プロジェクト構成

Simple-TDC-GameProjectは、3つの独立したゲームプロジェクトを含んでいます：

| プロジェクト | ビルドターゲット | エントリポイント | 状態 |
|-------------|----------------|----------------|------|
| **旧TDゲーム** | SimpleTDCGame | main.cpp | 保守モード |
| **新TDゲーム** | SimpleTDCGame_NewArch | main_new.cpp | アクティブ開発 |
| **Roguelike** | NetHackGame | main_roguelike.cpp | 開発中 |

### 1.2 統合の目的

1. **コード再利用の最大化**
   - Core層の共有による重複削減
   - 共通システムの統合

2. **一貫したアーキテクチャ**
   - ECSパターンの統一
   - データ駆動設計の徹底

3. **開発効率の向上**
   - 1つの改善が全プロジェクトに波及
   - テストインフラの共有

---

## 2. 3つのプロジェクト詳細

### 2.1 プロジェクト1: 旧TDゲーム（SimpleTDCGame）

**概要:**
- オリジナルのタワーディフェンスゲーム
- シーンベースアーキテクチャ
- Singletonパターンの使用

**アーキテクチャ:**
```
Game (メインループ)
 ├─ SceneManager (Singleton)
 │   ├─ TitleScene
 │   ├─ HomeScene
 │   ├─ TDGameScene
 │   ├─ TDTestGameScene
 │   └─ NethackGameScene
 ├─ ConfigManager (Singleton)
 ├─ InputManager (Singleton)
 ├─ ResourceManager (Singleton)
 └─ UIManager (Singleton)
```

**主要ファイル:**
- `src/main.cpp` - エントリポイント
- `src/Game.cpp` - ゲームクラス
- `src/SceneManager.cpp` - シーン管理
- `src/Scenes/*.cpp` - 各シーン実装

**コンポーネント:**
- `include/Components.h` - 旧コンポーネント定義
- Position, Velocity, Renderable など

**システム:**
- `include/Systems.h` - クラスベース + 静的関数混在
- InputSystem, MovementSystem, RenderSystem など

**状態:** 🟡 保守モード
- 新規機能開発は行わない
- クリティカルバグのみ修正
- 将来的に廃止予定

**利点:**
- ✅ シンプルで理解しやすい
- ✅ 安定した動作

**問題点:**
- ❌ Singleton多用でテスト困難
- ❌ シーン間でコード重複
- ❌ 拡張性が低い

---

### 2.2 プロジェクト2: 新TDゲーム（SimpleTDCGame_NewArch）

**概要:**
- データ駆動設計のタワーディフェンス
- FHD固定レンダリング（1920x1080）
- 依存性注入パターン

**アーキテクチャ:**
```
GameNew
 ├─ Core::World (entt::registry wrapper)
 ├─ Core::GameContext (DI Container)
 │   ├─ DefinitionRegistry
 │   ├─ EntityFactory
 │   ├─ SoundManager
 │   ├─ EffectManager
 │   ├─ WaveManager
 │   ├─ SpawnManager
 │   └─ GameStateManager
 ├─ Core::SystemRunner
 │   ├─ LaneMovementSystem
 │   ├─ CombatDetectionSystem
 │   ├─ CombatSystem
 │   ├─ HealthSystem
 │   ├─ DeathSystem
 │   ├─ ProjectileSystem
 │   ├─ KnockbackSystem
 │   ├─ AbilitySystem
 │   ├─ BaseHealthSystem
 │   └─ ResourceSystem
 └─ Core::GameRenderer (FHD固定レンダリング)
```

**主要ファイル:**
- `src/main_new.cpp` - エントリポイント
- `src/GameNew.cpp` - ゲームクラス実装
- `src/FileUtils.cpp` - ファイル操作ユーティリティ

**レイヤー構造:**
```
Core層 (基盤)
 ├─ World, GameContext, SystemRunner
 ├─ EntityFactory, DefinitionRegistry
 ├─ GameRenderer, Platform
 └─ Events, FileUtils

Game層 (汎用)
 └─ GameComponents.h (Sprite, Animation等)

TD層 (TD固有)
 ├─ TDComponents.h (Lane系、Stats等)
 ├─ TDSystems.h (10システム)
 ├─ WaveManager, SpawnManager, GameStateManager
 └─ GameUI (UI統合クラス)
```

**コンポーネント:**
- `include/ComponentsNew.h` - 統合ヘッダー
- Core/Game/TD層のコンポーネントを統合

**データ定義:**
```
assets/definitions/
 ├─ characters/*.char.json
 └─ stages/*.stage.json
```

**状態:** 🟢 アクティブ開発
- メイン開発ターゲット
- 新規機能は全てここに実装
- テストカバレッジ拡充予定

**利点:**
- ✅ テスタビリティが高い
- ✅ 依存関係が明示的
- ✅ 拡張性が高い
- ✅ データ駆動で柔軟
- ✅ FHD固定で座標系統一

**課題:**
- ⚠️ 学習コスト高
- ⚠️ 旧コードとの統合未完了
- ⚠️ ドキュメント不足

---

### 2.3 プロジェクト3: Roguelike（NetHackGame）

**概要:**
- NetHack風ターン制ローグライク
- グリッドベース移動
- タイルレンダリング

**アーキテクチャ:**
```
RoguelikeGame
 ├─ Core::World (共通)
 ├─ Core::GameContext (共通)
 ├─ Roguelike::TurnManager
 ├─ Roguelike::DungeonManager
 ├─ Roguelike::TileRenderer
 └─ Roguelike Systems
     ├─ TurnSystem
     ├─ InputSystem
     ├─ GridMovementSystem
     ├─ FOVSystem
     ├─ AISystem
     ├─ CombatSystem
     ├─ ItemSystem
     ├─ HungerSystem
     └─ LevelUpSystem
```

**主要ファイル:**
- `src/main_roguelike.cpp` - エントリポイント
- `include/Roguelike/RoguelikeGame.h` - ゲームクラス

**コンポーネント（Roguelike層）:**
```
Roguelike/Components/
 ├─ GridComponents.h (GridPosition, Tile, MapData)
 ├─ TurnComponents.h (TurnActor, ActionCommand)
 ├─ CombatComponents.h (CombatStats, MonsterAI)
 ├─ ItemComponents.h (Item, Inventory, Equipment)
 ├─ HungerComponents.h (Hunger)
 └─ RoguelikeComponents.h (統合ヘッダー)
```

**システム（Roguelike層）:**
```
Roguelike/Systems/
 ├─ TurnSystem.h (ターン管理)
 ├─ InputSystem.h (キー入力)
 ├─ ActionSystem.h (行動実行)
 ├─ FOVSystem.h (視界計算)
 ├─ AISystem.h (モンスターAI)
 ├─ CombatSystem.h (戦闘)
 ├─ ItemSystem.h (アイテム)
 ├─ HungerSystem.h (空腹度)
 └─ LevelUpSystem.h (レベルアップ)
```

**ジェネレーター:**
```
Roguelike/Generators/
 ├─ DungeonGenerator.h (ダンジョン生成)
 ├─ MonsterSpawner.h (モンスター配置)
 └─ ItemSpawner.h (アイテム配置)
```

**レンダリング:**
```
Roguelike/Rendering/
 └─ TileRenderer.h (タイル描画)
```

**状態:** 🔴 開発中（大部分未実装）
- 設計ドキュメントは充実
- ヘッダーファイルのみ存在（多くが空）
- 実装はPhase 1から着手予定

**Phase計画:**
```
Phase 1: 基本移動とターン制 (未実装)
 └─ プレイヤー移動、ターン進行

Phase 2: ダンジョン生成と視界 (未実装)
 └─ ランダムマップ、FOV

Phase 3: モンスターと戦闘 (未実装)
 └─ 敵AI、戦闘システム

Phase 4: アイテムと成長 (未実装)
 └─ インベントリ、レベルアップ

Phase 5: 追加要素 (未実装)
 └─ 未識別、セーブ/ロード
```

**利点:**
- ✅ 明確なドメイン分離
- ✅ NetHackの設計思想を踏襲
- ✅ Core層を活用

**課題:**
- ❌ 実装がほぼ未着手
- ❌ ドキュメントと実装の乖離
- ❌ 統合テストなし

---

## 3. 共通基盤（Core層）

### 3.1 Core層の役割

3つのプロジェクト全てが依存する共通基盤：

**提供機能:**
```
Core/
 ├─ World.h - entt::registry ラッパー
 ├─ GameContext.h - DIコンテナ
 ├─ SystemRunner.h - システム実行管理
 ├─ EntityFactory.h - エンティティ生成
 ├─ DefinitionRegistry.h - データ定義管理
 ├─ DefinitionLoader.h - JSON読み込み
 ├─ GameRenderer.h - FHD固定レンダリング
 ├─ Platform.h - プラットフォーム抽象化
 ├─ Events.h - イベントシステム
 ├─ FileUtils.h - ファイル操作
 ├─ SoundManager.h - サウンド管理
 ├─ EffectManager.h - エフェクト管理
 ├─ AnimationPlayer.h - アニメーション再生
 ├─ FallbackRenderer.h - フォールバック描画
 ├─ UILoader.h - UI定義読み込み
 └─ UIRenderer.h - UI描画
```

### 3.2 Core層の使用状況

| 機能 | 旧TD | 新TD | Roguelike |
|-----|------|------|-----------|
| World | ❌ | ✅ | ✅ |
| GameContext | ❌ | ✅ | ✅ (予定) |
| SystemRunner | ❌ | ✅ | ✅ (予定) |
| EntityFactory | ❌ | ✅ | ✅ (予定) |
| DefinitionRegistry | ❌ | ✅ | 🟡 (部分) |
| GameRenderer | ❌ | ✅ | ❌ |
| Events | ❌ | ✅ | 🟡 (計画) |

**凡例:**
- ✅ 使用中
- 🟡 部分的/計画中
- ❌ 未使用

### 3.3 Core層の設計品質

**✅ 良い設計:**
1. **レイヤー分離**
   - Core/Game/TD/Roguelikeが明確に分離
   
2. **依存性注入**
   - GameContextによる疎結合
   
3. **データ駆動**
   - DefinitionRegistryでJSON管理

**⚠️ 改善点:**
1. **Core層の肥大化**
   - GameRendererは本当にCoreか？
   - FHD固定はゲーム固有の決定

2. **EntityFactoryの責務**
   - ゲーム固有のロジックを含む
   - より汎用的な設計が望ましい

---

## 4. 統合状況マトリクス

### 4.1 機能別統合状況

| 機能領域 | 旧TD | 新TD | Roguelike | 統合度 |
|---------|------|------|-----------|--------|
| **エンティティ管理** | registry直接 | World | World | 🟡 中 |
| **コンポーネント** | Components.h | ComponentsNew.h | Roguelike/* | 🟡 中 |
| **システム実行** | 手動呼び出し | SystemRunner | SystemRunner | 🟡 中 |
| **入力処理** | InputManager | GameContext | InputSystem | 🔴 低 |
| **設定管理** | ConfigManager | DefinitionRegistry | 未使用 | 🔴 低 |
| **リソース管理** | ResourceManager | DefinitionRegistry | 未使用 | 🔴 低 |
| **UI管理** | UIManager | GameUI | 未実装 | 🔴 低 |
| **レンダリング** | 直接描画 | GameRenderer | TileRenderer | 🔴 低 |
| **データ定義** | なし | JSON | JSON (計画) | 🟢 高 |
| **イベント** | なし | Events | 未使用 | 🟡 中 |

**統合度:**
- 🟢 高: 3プロジェクト全て統一
- 🟡 中: 2プロジェクト統一、または部分統一
- 🔴 低: 各プロジェクト独自実装

### 4.2 コード共有率

| レイヤー | 共有可能 | 実際の共有 | 共有率 |
|---------|---------|-----------|--------|
| Core層 | 100% | 60% | 🟡 60% |
| Game層 | 80% | 40% | 🔴 40% |
| TD層 | 0% | 0% | ✅ N/A |
| Roguelike層 | 0% | 0% | ✅ N/A |

**解釈:**
- Core層は設計済みだが、旧TDが未使用
- Game層の共有が不十分（SpriteなどRoguelikeでも使える）

---

## 5. 依存関係分析

### 5.1 依存グラフ

```
┌─────────────────────────────────────────────────────────┐
│                   External Libraries                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐  │
│  │  EnTT    │  │nlohmann/ │  │ Raylib   │  │ ImGui  │  │
│  │          │  │  json    │  │          │  │        │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────┘  │
└─────────────────────────────────────────────────────────┘
         │              │              │            │
         └──────────────┴──────────────┴────────────┘
                        │
         ┌──────────────┴──────────────┐
         │         Core Layer           │
         │  ┌────────────────────────┐  │
         │  │ World, GameContext,    │  │
         │  │ SystemRunner, etc.     │  │
         │  └────────────────────────┘  │
         └──────────────┬──────────────┘
                        │
         ┌──────────────┼──────────────┐
         │              │              │
         ▼              ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  Game Layer  │ │   TD Layer   │ │ Roguelike    │
│              │ │              │ │  Layer       │
│ Sprite,      │ │ Lane,        │ │ Grid,        │
│ Animation    │ │ Stats        │ │ Turn         │
└──────────────┘ └──────────────┘ └──────────────┘
         │              │              │
         ▼              ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  旧TD        │ │  新TD        │ │  Roguelike   │
│  (main.cpp)  │ │ (main_new)   │ │ (main_rogue) │
└──────────────┘ └──────────────┘ └──────────────┘
```

### 5.2 循環依存の有無

**✅ 循環依存なし**

依存の方向性は一方向：
```
Application → Game/TD/Roguelike → Core → External
```

これは健全な設計。

### 5.3 依存の密結合度

| プロジェクト | Core依存度 | 他層依存度 | 結合度 |
|-------------|-----------|-----------|--------|
| 旧TD | 低 (未使用) | 高 (Singleton) | 🔴 高 |
| 新TD | 高 (DIで注入) | 中 | 🟢 低 |
| Roguelike | 中 (計画) | 低 | 🟢 低 |

---

## 6. 統合における課題

### 6.1 技術的課題

#### 1. アーキテクチャの不統一

**問題:**
- 旧TD: Singleton中心
- 新TD: DI中心
- Roguelike: 部分的にDI

**影響:**
- コード共有が困難
- 移行コストが高い
- 新規開発者の混乱

**対策:**
- 新TDをメインアーキに決定
- 旧TDを段階的に廃止
- Roguelikeを新アーキに合わせる

#### 2. コンポーネント重複

**問題:**
- `Position`が複数箇所で定義
- `Components.h` vs `ComponentsNew.h`

**影響:**
- メンテナンス性低下
- コンパイル時間増加

**対策:**
- ComponentsNew.hに統一
- Components.hを非推奨化

#### 3. レンダリング方式の違い

**問題:**
- 旧TD: 直接描画
- 新TD: GameRenderer (FHD固定)
- Roguelike: TileRenderer (グリッド)

**影響:**
- 座標系の不統一
- レンダリングコードの重複

**対策:**
- GameRendererを汎用化
- TileRendererをGameRendererの上に構築

### 6.2 プロセス課題

#### 1. ドキュメント散在

**問題:**
- 設計ドキュメントが各所に散在
- 重複した情報
- 古い情報との区別困難

**対策:**
- docs/構造の再編成
- 索引の作成（README.md）
- カテゴリ別整理

#### 2. ビルドターゲットの混乱

**問題:**
- 3つのターゲットの役割が不明確
- どれを使えばいいか分からない

**対策:**
- CMakeLists.txtにコメント追加
- READMEで推奨ターゲット明記

#### 3. テスト不足

**問題:**
- 統合テストが存在しない
- 変更の影響範囲が不明

**対策:**
- テストインフラ整備
- 各プロジェクトで共通テストケース

---

## 7. 統合戦略

### 7.1 短期戦略（Phase 1: 1-2週間）

#### 目標: 現状の整理と明確化

**実施内容:**
1. **ドキュメント統合**
   - CODE_ANALYSIS.md
   - REFACTORING_PLAN.md
   - INTEGRATION_STATUS.md (本ドキュメント)
   - docs/README.md (索引)

2. **ビルドターゲット明確化**
   - 推奨ターゲット: SimpleTDCGame_NewArch
   - 保守モード: SimpleTDCGame
   - 開発中: NetHackGame

3. **コンポーネント統合開始**
   - Components.h非推奨化
   - ComponentsNew.h統一

**期待効果:**
- 開発者の混乱解消
- 情報へのアクセス性向上

### 7.2 中期戦略（Phase 2: 1-2ヶ月）

#### 目標: 新アーキへの統一

**実施内容:**
1. **旧TD廃止準備**
   - 新TDで全機能を実現
   - 移行ドキュメント作成
   - 非推奨警告

2. **Roguelike Phase 1-3実装**
   - Core層との統合確認
   - 共通コンポーネント抽出
   - システム統合

3. **Core層の改善**
   - ゲーム固有機能の分離
   - 汎用性の向上
   - ドキュメント充実

**期待効果:**
- アーキテクチャの統一
- コード共有率の向上
- 保守性の改善

### 7.3 長期戦略（Phase 3: 3ヶ月以降）

#### 目標: 完全統合プラットフォーム

**実施内容:**
1. **旧TD完全廃止**
   - SimpleTDCGame削除
   - SimpleTDCGame_NewArch → SimpleTDCGame名称変更

2. **プラグイン化**
   ```
   SimpleTDCGame (統合プラットフォーム)
    ├─ --mode=td (タワーディフェンス)
    ├─ --mode=roguelike (ローグライク)
    └─ --mode=menu (モード選択)
   ```

3. **WebUIエディタ統合**
   - 両ゲームで共通のエディタ
   - データ定義の統一フォーマット

**期待効果:**
- 1つのプラットフォームで複数ゲーム
- 開発効率の最大化
- ユーザー体験の向上

---

## 8. 統合のロードマップ

### 8.1 タイムライン

```
Week 1-2 (Phase 1)
├─ ドキュメント統合
├─ コンポーネント統合開始
└─ ビルドターゲット明確化

Week 3-4 (Phase 2.1)
├─ Singleton削減開始
└─ Roguelike Phase 1実装

Week 5-8 (Phase 2.2)
├─ Roguelike Phase 2-3実装
└─ Core層改善

Week 9-12 (Phase 2.3)
├─ テストインフラ整備
├─ CI/CD強化
└─ 旧TD非推奨化

Month 4+ (Phase 3)
├─ 旧TD廃止
├─ プラグイン化
└─ WebUIエディタ
```

### 8.2 マイルストーン

| マイルストーン | 完了条件 | 期限 |
|--------------|---------|------|
| M1: ドキュメント統合 | docs/再編成完了 | Week 2 |
| M2: コンポーネント統合 | ComponentsNew.h統一 | Week 2 |
| M3: Roguelike Phase 1 | 基本移動動作 | Week 4 |
| M4: Roguelike Phase 2-3 | 戦闘動作 | Week 8 |
| M5: テストカバレッジ30% | 自動テスト実行 | Week 12 |
| M6: 旧TD廃止 | SimpleTDCGame削除 | Month 4 |

---

## 9. 成功指標

### 9.1 統合度指標

| 指標 | 現状 | Phase 1 | Phase 2 | Phase 3 |
|-----|------|---------|---------|---------|
| Core層使用率 | 33% | 50% | 75% | 100% |
| コード共有率 | 40% | 50% | 65% | 80% |
| ビルドターゲット数 | 3 | 3 | 2 | 1 |
| ドキュメント重複率 | 高 | 中 | 低 | なし |

### 9.2 品質指標

| 指標 | 現状 | 目標 (Phase 3) |
|-----|------|---------------|
| テストカバレッジ | 0% | 50% |
| ビルド成功率 | 95% | 99% |
| 循環依存数 | 0 | 0 (維持) |

---

## 10. まとめ

### 10.1 現状評価

**強み:**
- ✅ Core層の設計が優秀
- ✅ 明確なレイヤー分離
- ✅ ドキュメントが充実

**弱み:**
- ❌ アーキテクチャ不統一
- ❌ コード重複多数
- ❌ 統合テスト不足

### 10.2 推奨アクション

**今すぐ:**
1. ドキュメント統合（本レポート含む）
2. 推奨ターゲット明記（SimpleTDCGame_NewArch）
3. Components.h非推奨化

**次の1ヶ月:**
4. Roguelike Phase 1実装
5. Core層改善
6. テスト整備開始

**長期的に:**
7. 旧TD廃止
8. プラットフォーム統合
9. WebUIエディタ

---

**関連ドキュメント:**
- [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) - コード分析レポート
- [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) - リファクタリング計画
- [HANDOVER.md](./HANDOVER.md) - 新アーキテクチャ詳細
- [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike設計

---

**改訂履歴:**
- 2025-12-02: 初版作成
