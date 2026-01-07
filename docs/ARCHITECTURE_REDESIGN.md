# アーキテクチャ再設計ガイド

**最終更新**: 2025-01-XX

## 概要

本ドキュメントは、Cat Tower Defenseプロジェクトの現在のアーキテクチャを説明したものです。モジュールシステムとAPI層分離によるシンプルで拡張しやすい設計を採用しています。

## 設計方針

### 1. プロジェクト構成

#### ディレクトリ構造（実装済み）

```
Cat-TD-Game/
├── CMakeLists.txt          # メインビルド設定
├── CMakePresets.json       # CMakeプリセット
├── assets/                 # ゲームアセット（画像・SE・BGM・シェーダ）
├── data/                   # JSON定義（敵・ステージ・アニメーション・セーブ）
│   ├── characters.json     # キャラクター定義
│   ├── stages.json         # ステージ定義
│   └── assets/             # 素材データ（画像・フォント・音楽）
├── external/               # 外部ライブラリキャッシュ
├── docs/                   # 開発ドキュメント
├── tools/                  # ユーティリティスクリプト
└── game/                   # すべてのソースコード（統合）
    ├── main/               # エントリーポイント
    │   └── main.cpp
    ├── core/               # コアシステム
    │   ├── api/            # システムAPI層
    │   │   ├── BaseSystemAPI.hpp/cpp
    │   │   └── GameModuleAPI.hpp/cpp
    │   ├── config/         # 設定・型定義
    │   │   ├── GameConfig.hpp
    │   │   ├── GameState.hpp
    │   │   └── SharedContext.hpp
    │   ├── ecs/            # ECS関連（モジュール定義）
    │   │   ├── defineComponents.hpp
    │   │   ├── defineModules.hpp
    │   │   └── IModule.hpp
    │   ├── init/            # 初期化関連
    │   │   └── ResourceInitializer.hpp/cpp
    │   ├── states/         # ステート/シーン
    │   │   └── TitleScreen.hpp/cpp
    │   └── system/         # コアシステム
    │       ├── GameSystem.hpp/cpp
    │       └── ModuleSystem.hpp/cpp
    └── utils/               # ユーティリティ
        └── Log.h
```

### 2. アーキテクチャパターン

#### API層分離

**BaseSystemAPI（システムAPI層）**:
- Raylibウィンドウ・オーディオデバイスの初期化/終了
- リソース管理（テクスチャ、フォント、サウンド、ミュージック）
- 描画管理（RenderTexture、描画フレーム制御、描画プリミティブ）
- 入力管理（キーボード、マウス、ゲームパッド、タッチ）
- 解像度管理（FHD固定、スケーリング対応）
- オーディオ管理（ボリューム制御、再生制御）

**GameModuleAPI（ゲームモジュールAPI）**:
- EnTTレジストリのラッパー
- エンティティ操作（Create, Destroy, Valid, Clear）
- コンポーネント操作（Add, Get, Try, Remove, Has）
- ビュー作成（View, View with exclude）
- コンテキスト変数管理（Ctx）

#### モジュールシステム

**IModuleインターフェース**:
- すべてのゲームモジュールが実装する必要があるインターフェース
- ライフサイクル（初期化・更新・描画・終了）と優先順位管理を提供

**ModuleSystem**:
- モジュールの登録・管理（所有権を持つ）
- モジュールのライフサイクル管理（Initialize, Update, Render, Shutdown）
- 優先順位に基づいた実行順序の管理

**使用例**:

```cpp
// モジュール登録
moduleSystem_->RegisterModule<MovementModule>();
moduleSystem_->RegisterModule<CombatModule>();
moduleSystem_->RegisterModule<RenderModule>();

// モジュール実行
moduleSystem_->Update(sharedContext_, deltaTime);
moduleSystem_->Render(sharedContext_);
```

#### ステート管理

**GameState enum**:
- ゲームの現在の状態を表すenum
- GameSystemがステート管理に使用

**SharedContext**:
- GameSystemが所有し、すべてのモジュールに共有
- モジュールはこのコンテキストを通じてシステムAPIにアクセス
- 所有権は持たない（参照のみ）

**各ステートクラス**:
- TitleScreen（タイトル画面）: 実装済み
- 将来実装予定: HomeScreen, GameScreen, StageSelectScreen, ConfigScreen

### 3. コアシステム

#### GameSystem（ゲームシステム統合クラス）

**責務**:
- アプリケーション全体の初期化・終了管理
- メインループの管理（フレーム制御）
- BaseSystemAPIとGameModuleAPIの所有・管理
- SharedContextの所有・管理
- ステート管理（enum + 遷移制御）
- 各ステートクラスの所有・管理
- 安全なステート遷移（二重初期化/解放防止）
- モジュール登録の呼び出し（RegisterModules）

**実装例**:

```cpp
class GameSystem {
public:
    GameSystem();
    int Initialize();
    int Run();
    void Shutdown();

private:
    std::unique_ptr<BaseSystemAPI> systemAPI_;
    std::unique_ptr<GameModuleAPI> gameAPI_;
    std::unique_ptr<ModuleSystem> moduleSystem_;
    std::unique_ptr<ResourceInitializer> resourceInitializer_;
    std::unique_ptr<TitleScreen> titleScreen_;
    SharedContext sharedContext_;
    GameState currentState_;
    bool requestShutdown_;

    void transitionTo(GameState newState);
    void RegisterModules();
};
```

#### ModuleSystem（モジュール管理システム）

**責務**:
- モジュールの登録・管理（所有権を持つ）
- モジュールのライフサイクル管理（Initialize, Update, Render, Shutdown）
- 優先順位に基づいた実行順序の管理

**実装例**:

```cpp
class ModuleSystem {
public:
    explicit ModuleSystem(GameModuleAPI* gameAPI);
    
    template<typename ModuleType>
    void RegisterModule();
    
    bool Initialize(SharedContext& ctx);
    void Update(SharedContext& ctx, float dt);
    void Render(SharedContext& ctx);
    void Shutdown(SharedContext& ctx);

private:
    GameModuleAPI* gameAPI_;
    std::vector<std::unique_ptr<IModule>> modules_;
    void SortModulesByPriority();
};
```

### 4. データ駆動設計

#### JSON定義ファイル

全てのゲームデータはJSONで定義し、実行時にロード（将来実装予定）

**定義ファイルの配置**:

```
data/
├── characters.json      # キャラクター定義（統合JSON）
├── stages.json          # ステージ定義（統合JSON）
└── assets/              # 素材データ（画像・フォント・音楽）
    ├── characters/      # キャラクター画像
    ├── textures/        # 画像ファイル
    ├── sounds/          # 音楽ファイル
    ├── fonts/           # フォントファイル
    └── music/           # 音楽ファイル
```

定義ファイルは単一JSONファイル形式で、`data/`直下に配置されます。

#### エラーハンドリング

JSON解析は必ずエラーハンドリングを実装:

```cpp
try {
    json config = json::parse(file);
    // 処理
} catch (const json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    // デフォルト値で継続
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // デフォルト値で継続
}
```

### 5. リソース管理

#### アセット構成

```
assets/
├── fonts/              # フォントファイル
│   └── NotoSansJP-Medium.ttf
├── textures/          # テクスチャ画像
├── sounds/            # サウンドファイル
└── music/             # BGMファイル
```

#### フォント管理

- **標準フォント**: NotoSansJP-Medium.ttf
- **対応文字範囲**:
  - ひらがな（U+3040-U+309F）
  - カタカナ（U+30A0-U+30FF）
  - ASCII（U+0020-U+007E）
  - JIS第一水準（U+4E00-U+9FFF）
  - JIS第二水準（追加漢字）
- **解像度**: 1920x1080（FHD固定）
- **適用範囲**: Raylib、ImGui両方で統一

#### リソース初期化

**ResourceInitializer**:
- リソーススキャン・読み込み処理
- 進捗管理
- 初期化画面・エラー画面の描画
- 初期化完了/失敗の状態通知

**処理フロー**:
1. リソースファイルのスキャン（ScanResourceFiles）
2. 1つずつリソースを読み込み（LoadNextResource）
3. 進捗表示（プログレスバー）
4. 完了時にIsCompleted()がtrueを返す
5. エラー時はエラー画面を表示（5秒後に終了）

### 6. ビルドシステム

#### CMake構成

- FetchContentによる外部ライブラリ管理
- プリセットによるビルド設定の統一
- 実行ファイル（game）のビルド

#### 依存関係

```
CatTDGame.exe → raylib, EnTT, nlohmann/json, spdlog, rlImGui, imgui
```

### 7. コーディング規約

#### 命名規則

- **クラス名**: PascalCase（例: `GameSystem`）
- **関数名**: PascalCase（例: `Initialize`）
- **変数名**: camelCase、プライベートメンバーは末尾にアンダースコア（例: `deltaTime`, `systemAPI_`）
- **定数**: UPPER_CASE（例: `TARGET_FPS`）
- **名前空間**: PascalCase（例: `game::core`）

#### メモリ管理

- スマートポインタ（`std::unique_ptr`, `std::shared_ptr`）を優先
- RAIIパターンに従う
- リソースの生存期間を明確にする

#### エラーハンドリング

- ファイル操作は必ずエラーチェック
- リソース読み込み失敗時は適切にフォールバック
- ログ出力は`spdlog`を使用

## 実装状況

### 実装済み機能

- ✅ エントリーポイント（main.cpp）
- ✅ ゲームシステム（GameSystem）
- ✅ モジュールシステム（ModuleSystem）
- ✅ システムAPI（BaseSystemAPI - Raylib統合）
- ✅ ゲームモジュールAPI（GameModuleAPI - EnTTラッパー）
- ✅ タイトル画面（TitleScreen）
- ✅ リソース初期化（ResourceInitializer）
- ✅ 共有コンテキスト（SharedContext）
- ✅ ステート管理（GameState enum）

### 将来実装予定の機能

- ⏳ ECSコンポーネント（defineComponents.hppは空）
- ⏳ ECSシステム（MovementModule, CombatModule, RenderModuleなど）
- ⏳ ゲームシーン（GameScreen）
- ⏳ その他のシーン（HomeScreen, StageSelectScreen, ConfigScreen）
- ⏳ JSON定義ファイルの読み込み
- ⏳ エンティティ・スキル・ステージ定義のローダー

## アーキテクチャの利点

### ✅ シンプルさ

- API層とモジュールシステムで責務が明確
- 各モジュールが独立して動作
- ディレクトリ構造が浅く、理解しやすい

### ✅ 拡張性

- 新しいモジュールを追加するだけで機能拡張（IModuleを実装するだけ）
- モジュールの優先順位を設定可能
- 共有コンテキストによる情報共有

### ✅ 保守性

- 機能ごとにディレクトリが分かれている
- ファイルの役割が明確
- 型安全なECS操作（EnTT）

### ✅ テスト容易性

- 各モジュールを独立テスト可能
- API層のモック化が容易
- 共有コンテキストによる依存注入

## 参考資料

- ECSアーキテクチャ: EnTT公式ドキュメント
- データ駆動設計: Game Programming Patterns
- C++コーディング規約: C++ Core Guidelines
- Raylib: Raylib公式ドキュメント

## 更新履歴

- 2025-01-XX: 実装に合わせて全面更新（モジュールシステム・API層分離の説明を追加）
