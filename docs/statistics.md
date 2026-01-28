# Cat Tower Defense - 実装統計情報

このドキュメントは、プロジェクトの現在の実装状況を統計的にまとめたものです。

**最終更新**: 2026-01-23

---

## 📊 ファイル統計

### ファイル数

| 種類 | ファイル数 | 説明 |
|------|-----------|------|
| **C++ソースファイル (.cpp)** | 89 | 実装ファイル |
| **C++ヘッダーファイル (.hpp)** | 107 | ヘッダーファイル |
| **Cヘッダーファイル (.h)** | 1 | ユーティリティヘッダー |
| **合計** | **197** | 全ソースコードファイル |

### コード行数

| 種類 | 行数 | 説明 |
|------|------|------|
| **C++ソース (.cpp)** | 25,293行 | 実装コード |
| **C++ヘッダー (.hpp)** | 7,538行 | ヘッダーコード |
| **合計** | **32,831行** | 全コード行数 |

---

## 🏗️ アーキテクチャ統計

### ディレクトリ別ファイル数

| ディレクトリ | ファイル数 | 主な内容 |
|------------|-----------|---------|
| `core/api/` | 42ファイル (30 .cpp, 12 .hpp) | API層 |
| `core/states/` | 約50ファイル | シーン・オーバーレイ |
| `core/ecs/` | 約30ファイル | ECSコンポーネント・エンティティ |
| `core/ui/` | 約20ファイル | UIコンポーネント |
| `core/system/` | 約10ファイル | コアシステム |
| `core/game/` | 8ファイル | ゲームロジック |
| `core/config/` | 6ファイル | 設定・型定義 |
| `core/init/` | 2ファイル | 初期化関連 |
| `core/entities/` | 約10ファイル | エンティティ関連 |
| `main/` | 1ファイル | エントリーポイント |
| `utils/` | 2ファイル | ユーティリティ |

---

## 🧩 コンポーネント統計

### ECSコンポーネント

| コンポーネント | ファイル | 説明 |
|--------------|---------|------|
| `Position` | Position.hpp | 位置情報 |
| `Health` | Health.hpp | HP情報 |
| `Stats` | Stats.hpp | ステータス |
| `Movement` | Movement.hpp | 移動情報 |
| `Combat` | Combat.hpp | 戦闘情報 |
| `Sprite` | Sprite.hpp | スプライト情報 |
| `Animation` | Animation.hpp | アニメーション情報 |
| `CharacterId` | CharacterId.hpp | キャラクターID |
| `Team` | Team.hpp | 陣営情報 |
| `Equipment` | Equipment.hpp | 装備データ |
| `PassiveSkills` | PassiveSkills.hpp | パッシブスキルデータ |
| **合計** | **11コンポーネント** | POD型コンポーネント |

---

## 🔌 API統計

### APIクラス数

| APIカテゴリ | API数 | 主要API |
|-----------|------|---------|
| **基盤API** | 1 | BaseSystemAPI |
| **サブシステムAPI** | 6 | RenderSystemAPI, ResourceSystemAPI, AudioSystemAPI, WindowSystemAPI, TimingSystemAPI, CollisionSystemAPI |
| **ECS API** | 1 | ECSystemAPI |
| **入力API** | 1 | InputSystemAPI |
| **UI API** | 1 | UISystemAPI |
| **オーディオ制御API** | 1 | AudioControlAPI |
| **シーン/オーバーレイ制御API** | 1 | SceneOverlayControlAPI |
| **ゲームプレイデータAPI** | 1 | GameplayDataAPI |
| **セットアップAPI** | 1 | SetupAPI |
| **戦闘セットアップAPI** | 1 | BattleSetupAPI |
| **戦闘進行API** | 1 | BattleProgressAPI |
| **デバッグUI API** | 1 | DebugUIAPI |
| **合計** | **17 API** | 全APIクラス |

---

## 🎬 シーン・オーバーレイ統計

### シーン数

| シーン | ファイル | 説明 |
|-------|---------|------|
| `InitScene` | InitScene.cpp/hpp | リソース初期化シーン |
| `TitleScreen` | TitleScreen.cpp/hpp | タイトル画面 |
| `HomeScreen` | HomeScreen.cpp/hpp | ホーム画面 |
| `GameScene` | GameScene.cpp/hpp | ゲーム画面（バトル） |
| `EditorScene` | EditorScene.cpp/hpp | エディタ画面 |
| **合計** | **5シーン** | 全シーン |

### オーバーレイ数

| オーバーレイ | ファイル | 説明 |
|------------|---------|------|
| `StageSelectOverlay` | StageSelectOverlay.cpp/hpp | ステージ選択 |
| `FormationOverlay` | FormationOverlay.cpp/hpp | 編成 |
| `EnhancementOverlay` | EnhancementOverlay.cpp/hpp | 強化 |
| `CharacterEnhancementOverlay` | CharacterEnhancementOverlay.cpp/hpp | キャラクター強化 |
| `CodexOverlay` | CodexOverlay.cpp/hpp | 図鑑 |
| `SettingsOverlay` | SettingsOverlay.cpp/hpp, SettingsOverlay.Core.cpp, SettingsOverlay.Logic.cpp, SettingsOverlay.Render.cpp | 設定 |
| `GachaOverlay` | GachaOverlay.cpp/hpp, GachaOverlay.Core.cpp, GachaOverlay.Render.cpp, GachaOverlay.State.cpp, GachaOverlay.Update.cpp | ガチャ |
| `LicenseOverlay` | LicenseOverlay.cpp/hpp | ライセンス情報 |
| `BattleResultOverlay` | BattleResultOverlay.cpp/hpp | 戦闘結果 |
| `PauseOverlay` | PauseOverlay.cpp/hpp | 一時停止 |
| **合計** | **10オーバーレイ** | 全オーバーレイ |

### ホーム画面専用コンポーネント

| コンポーネント | ファイル | 説明 |
|--------------|---------|------|
| `ResourceHeader` | ResourceHeader.cpp/hpp | リソースヘッダー |
| `TabBarManager` | TabBarManager.cpp/hpp | タブバー管理 |
| `ContentContainer` | ContentContainer.cpp/hpp | コンテンツコンテナ |
| `TabContent` | TabContent.cpp/hpp | タブコンテンツ |
| `ITabContent` | ITabContent.hpp | タブコンテンツインターフェース |
| **合計** | **5コンポーネント** | ホーム画面専用 |

---

## 🎨 UIコンポーネント統計

### UIコンポーネント数

| コンポーネント | ファイル | 説明 |
|--------------|---------|------|
| `Button` | Button.cpp/hpp | ボタンコンポーネント |
| `Card` | Card.cpp/hpp | カードコンポーネント |
| `List` | List.cpp/hpp | リストコンポーネント |
| `Tile` | Tile.cpp/hpp | タイルコンポーネント |
| `Panel` | Panel.cpp/hpp | パネルコンポーネント |
| **合計** | **5 UIコンポーネント** | 全UIコンポーネント |

### UI関連ファイル

| ファイル | 説明 |
|---------|------|
| `IUIComponent.hpp` | UIコンポーネント基底インターフェース |
| `UIEvent.hpp` | UIイベント定義 |
| `UIEffects.hpp` | UIエフェクト関数 |
| `OverlayColors.hpp` | オーバーレイカラー定義 |
| `UiAssetKeys.hpp` | UIアセットキー定義 |
| `HUDRenderer.cpp/hpp` | HUDレンダラー |
| `BattleHUDRenderer.cpp/hpp` | 戦闘HUDレンダラー |
| `ImGuiSoundHelpers.hpp` | ImGuiサウンドヘルパー |

---

## 🎮 ゲームロジック統計

### ゲームロジッククラス

| クラス | ファイル | 説明 |
|-------|---------|------|
| `BattleRenderer` | BattleRenderer.cpp/hpp | バトル描画 |
| `FieldManager` | FieldManager.cpp/hpp | フィールド管理 |
| `InputHandler` | InputHandler.cpp/hpp | 入力処理 |
| `WaveLoader` | WaveLoader.cpp/hpp | Wave読み込み |
| **合計** | **4クラス** | ゲームロジック |

---

## 📦 エンティティ・マネージャー統計

### エンティティ関連クラス

| クラス | ファイル | 説明 |
|-------|---------|------|
| `Character` | Character.cpp/hpp | キャラクターエンティティ |
| `CharacterManager` | CharacterManager.cpp/hpp | キャラクターマネージャー |
| `CharacterLoader` | CharacterLoader.cpp/hpp | キャラクターローダー |
| `CharacterStatCalculator` | CharacterStatCalculator.hpp | キャラクターステータス計算 |
| `CharacterToECS` | CharacterToECS.hpp | キャラクター→ECS変換 |
| `ItemPassiveManager` | ItemPassiveManager.cpp/hpp | アイテム/パッシブマネージャー |
| `ItemPassiveLoader` | ItemPassiveLoader.cpp/hpp | アイテム/パッシブローダー |
| `StageManager` | StageManager.cpp/hpp | ステージマネージャー |
| `StageLoader` | StageLoader.cpp/hpp | ステージローダー |
| `TowerAttachmentManager` | TowerAttachmentManager.cpp/hpp | タワーアタッチメントマネージャー |
| `TowerAttachmentLoader` | TowerAttachmentLoader.cpp/hpp | タワーアタッチメントローダー |
| `TowerAttachment` | TowerAttachment.hpp | タワーアタッチメント |
| `EntityCreationData` | EntityCreationData.hpp | エンティティ生成データ |
| `AnimationData` | AnimationData.hpp | アニメーションデータ |
| **合計** | **14クラス** | エンティティ関連 |

---

## 🔧 システム統計

### コアシステムクラス

| クラス | ファイル | 説明 |
|-------|---------|------|
| `GameSystem` | GameSystem.cpp/hpp | ゲームシステム統合クラス |
| `ModuleSystem` | ModuleSystem.cpp/hpp | モジュール管理システム |
| `OverlayManager` | OverlayManager.cpp/hpp | オーバーレイ管理システム |
| `PlayerDataManager` | PlayerDataManager.cpp/hpp | プレイヤーデータ管理 |
| `TowerEnhancementEffects` | TowerEnhancementEffects.hpp | タワー強化効果 |
| `ResourceInitializer` | ResourceInitializer.cpp/hpp | リソース初期化 |
| **合計** | **6システム** | コアシステム |

---

## 📝 コード品質統計

### 名前空間使用

| 名前空間 | 使用箇所 | 説明 |
|---------|---------|------|
| `namespace game` | 全ファイル | メイン名前空間 |
| `namespace core` | 全ファイル | コア名前空間 |
| `namespace ecs` | ECS関連 | ECS名前空間 |
| `namespace ui` | UI関連 | UI名前空間 |
| `namespace states` | シーン関連 | ステート名前空間 |
| **合計** | **896箇所** | 名前空間定義 |

### 関数・メソッド統計

| カテゴリ | 関数数 | 説明 |
|---------|-------|------|
| **API層** | 約381関数 | APIメソッド |
| **システム層** | 約52関数 | システムメソッド |
| **シーン・オーバーレイ** | 約308関数 | シーン・オーバーレイメソッド |
| **合計** | **約741関数** | 全関数・メソッド |

### クラス・構造体定義

| 種類 | 数 | 説明 |
|------|---|------|
| **クラス定義** | 約175 | 全クラス定義 |
| **構造体定義** | 約174 | 全構造体定義 |
| **enum定義** | 22 | 全enum定義 |
| **合計** | **約371** | 型定義総数 |

### テンプレート使用

| 使用箇所 | 説明 |
|---------|------|
| **58箇所** | テンプレート定義・使用 |
| 主な使用: `ECSystemAPI` (28箇所), `GameModuleAPI` (28箇所) | テンプレートメタプログラミング |

### スマートポインタ使用

| 種類 | 使用箇所 | 説明 |
|------|---------|------|
| `std::unique_ptr` | 118箇所 | 所有権を持つポインタ |
| `std::shared_ptr` | 多数 | 共有ポインタ |
| `std::weak_ptr` | 少数 | 弱参照ポインタ |

### ログ出力

| ログレベル | 使用箇所 | 説明 |
|----------|---------|------|
| `LOG_INFO` | 多数 | 情報ログ |
| `LOG_WARN` | 多数 | 警告ログ |
| `LOG_ERROR` | 多数 | エラーログ |
| `LOG_DEBUG` | 多数 | デバッグログ |
| **合計** | **435箇所** | ログ出力箇所 |

### Include文

| 種類 | 数 | 説明 |
|------|---|------|
| **Include文** | **632箇所** | 全include文 |

---

## 📂 ディレクトリ構造統計

### 主要ディレクトリ

```
game/
├── main/                   1ファイル
├── core/
│   ├── api/                42ファイル (30 .cpp, 12 .hpp)
│   ├── config/             6ファイル
│   ├── ecs/
│   │   ├── Components/     11ファイル (コンポーネント)
│   │   └── entities/       約20ファイル (エンティティ・マネージャー)
│   ├── entities/           約10ファイル
│   ├── game/               8ファイル
│   ├── init/               2ファイル
│   ├── states/
│   │   ├── overlays/       約40ファイル (オーバーレイ)
│   │   └── (シーン)         約10ファイル
│   ├── system/             約10ファイル
│   └── ui/
│       ├── components/     10ファイル (UIコンポーネント)
│       └── (その他)        約10ファイル
└── utils/                  2ファイル
```

---

## 🎯 機能別統計

### 実装済み機能

| 機能カテゴリ | 実装数 | 説明 |
|------------|-------|------|
| **API層** | 17 API | 全APIクラス |
| **ECSコンポーネント** | 11コンポーネント | POD型コンポーネント |
| **シーン** | 5シーン | 全シーン |
| **オーバーレイ** | 10オーバーレイ | 全オーバーレイ |
| **UIコンポーネント** | 5コンポーネント | 再利用可能UIコンポーネント |
| **ゲームロジック** | 4クラス | バトル・フィールド管理 |
| **エンティティ管理** | 14クラス | キャラクター・アイテム・ステージ管理 |
| **コアシステム** | 6システム | ゲームシステム・モジュール管理 |

---

## 📈 コード規模

### 規模の目安

| 指標 | 値 | 評価 |
|------|---|------|
| **総コード行数** | 32,831行 | 中規模プロジェクト |
| **総ファイル数** | 197ファイル | 適切な規模 |
| **平均ファイルサイズ** | 約167行/ファイル | 適切なサイズ |
| **最大ファイル** | CodexOverlay.cpp (約2,000行以上) | 大型ファイル |
| **最小ファイル** | コンポーネントヘッダー (約20-30行) | 適切なサイズ |

---

## 🔍 コードパターン統計

### 設計パターン使用

| パターン | 使用箇所 | 説明 |
|---------|---------|------|
| **ファサードパターン** | GameSystem | 複雑なサブシステムの統合 |
| **依存性注入** | SharedContext | APIの共有 |
| **ステートパターン** | GameState | シーン管理 |
| **オブザーバーパターン** | シーン/オーバーレイ | 遷移リクエスト |
| **RAII** | unique_ptr | 自動リソース管理 |
| **テンプレート** | ECSystemAPI, GameModuleAPI | 型安全なAPI |

### メモリ管理

| 方式 | 使用箇所 | 説明 |
|------|---------|------|
| **スマートポインタ** | 118箇所以上 | 自動メモリ管理 |
| **RAII** | 全リソース | 自動クリーンアップ |
| **キャッシュ** | テクスチャ・輝度・テキスト色 | パフォーマンス最適化 |

---

## 📊 まとめ

### プロジェクト規模

- **総コード行数**: 32,831行
- **総ファイル数**: 197ファイル
- **平均ファイルサイズ**: 約167行/ファイル

### 主要コンポーネント

- **API層**: 17 API
- **ECSコンポーネント**: 11コンポーネント
- **シーン**: 5シーン
- **オーバーレイ**: 10オーバーレイ
- **UIコンポーネント**: 5コンポーネント
- **エンティティ管理**: 14クラス
- **コアシステム**: 6システム

### コード品質

- **名前空間**: 適切に使用（896箇所）
- **スマートポインタ**: 積極的に使用（118箇所以上）
- **ログ出力**: 適切に実装（435箇所）
- **テンプレート**: 型安全性の向上（58箇所）

### 特徴

1. **モジュール化**: 明確なディレクトリ構造と責務分離
2. **型安全性**: テンプレートとスマートポインタの積極的使用
3. **保守性**: 統一された命名規則とコーディング規約
4. **拡張性**: インターフェースベースの設計
5. **パフォーマンス**: データ志向設計とキャッシュ戦略

---

## 📋 詳細統計

### ファイルサイズ分布

| サイズ範囲 | ファイル数 | 説明 |
|-----------|-----------|------|
| **0-100行** | 多数 | 小規模ファイル（コンポーネントヘッダーなど） |
| **100-500行** | 多数 | 中規模ファイル（一般的な実装） |
| **500-1000行** | 少数 | 大規模ファイル（複雑な実装） |
| **1000行以上** | 少数 | 超大規模ファイル（CodexOverlay.cppなど） |

### コード密度

| 指標 | 値 | 説明 |
|------|---|------|
| **平均ファイルサイズ** | 約167行/ファイル | 適切なサイズ |
| **最大ファイル** | CodexOverlay.cpp (約2,000行以上) | 大型ファイル（リファクタリング候補） |
| **最小ファイル** | コンポーネントヘッダー (約20-30行) | 適切なサイズ |

### 依存関係

| 種類 | 数 | 説明 |
|------|---|------|
| **Include文** | 632箇所 | 全include文 |
| **外部ライブラリ** | Raylib, EnTT, ImGui, nlohmann/json, spdlog | 主要依存ライブラリ |
| **内部依存** | 明確な階層構造 | API層 → シーン層 → オーバーレイ層 |

---

## 📋 詳細統計

### ファイルサイズ分布

| サイズ範囲 | ファイル数 | 説明 |
|-----------|-----------|------|
| **0-100行** | 多数 | 小規模ファイル（コンポーネントヘッダーなど） |
| **100-500行** | 多数 | 中規模ファイル（一般的な実装） |
| **500-1000行** | 少数 | 大規模ファイル（複雑な実装） |
| **1000行以上** | 少数 | 超大規模ファイル（CodexOverlay.cppなど） |

### コード密度

| 指標 | 値 | 説明 |
|------|---|------|
| **平均ファイルサイズ** | 約167行/ファイル | 適切なサイズ |
| **最大ファイル** | CodexOverlay.cpp (約2,000行以上) | 大型ファイル（リファクタリング候補） |
| **最小ファイル** | コンポーネントヘッダー (約20-30行) | 適切なサイズ |

### 依存関係

| 種類 | 数 | 説明 |
|------|---|------|
| **Include文** | 632箇所 | 全include文 |
| **外部ライブラリ** | Raylib, EnTT, ImGui, nlohmann/json, spdlog | 主要依存ライブラリ |
| **内部依存** | 明確な階層構造 | API層 → シーン層 → オーバーレイ層 |

---

**この統計情報は、プロジェクトの現在の実装状況を反映しています。**
