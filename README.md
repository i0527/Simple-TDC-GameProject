# Simple-TDC-GameProject

[![Windows Build](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml/badge.svg)](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml)

## 概要

Simple-TDC-GameProjectは、ECS（Entity Component System）アーキテクチャを採用したC++統合ゲームプラットフォームです。タワーディフェンスとローグライクの2つのゲームモードを1つのプラットフォームで実現し、データ駆動設計とモダンなC++開発手法を実践します。

### プロジェクトの特徴

- **統合プラットフォーム**: 3つのゲームプロジェクト（旧TD、新TD、Roguelike）を共通基盤で統合
- **データ駆動設計**: JSON定義ファイルによる柔軟なゲーム設定
- **ImGui内蔵エディター**: F1キーで開くゲーム内エディターでキャラクター、ステージ、UI、スキル、エフェクト、サウンドを編集
- **レイヤー分離アーキテクチャ**: Core/Game/TD/Roguelike層による明確な責務分離

## 技術スタック

- **言語**: C++17以上
- **ビルドシステム**: CMake 3.15以上
- **ECS**: [EnTT](https://github.com/skypjack/entt) - 高速で軽量なEntity Component Systemライブラリ
- **JSON**: [nlohmann/json](https://github.com/nlohmann/json) - モダンなC++ JSON ライブラリ
- **レンダリング/入力**: [Raylib](https://www.raylib.com/) - シンプルで使いやすいゲーム開発ライブラリ
- **UI (HUD/操作パネル)**: [raygui](https://github.com/raysan5/raygui) - Raylib用の即時モードGUIライブラリ
- **UI (内蔵エディター)**: [Dear ImGui](https://github.com/ocornut/imgui) + [rlImGui](https://github.com/raylib-extras/rlImGui) - Raylib統合されたImGui
- **CI/CD**: GitHub Actions（Windows自動ビルド、actions/checkout@v4、actions/upload-artifact@v4）

## プロジェクト構成

### ゲームプロジェクト

本プロジェクトは3つのゲームプロジェクトを含む統合プラットフォームです：

| プロジェクト | ビルドターゲット | エントリポイント | 状態 | 説明 |
|-------------|----------------|----------------|------|------|
| **統合プラットフォーム** | SimpleTDCGame | main_unified.cpp | 🎯 推奨 | ホーム画面から各ゲームモードを選択 |
| **新TDゲーム** | SimpleTDCGame_NewArch | main_new.cpp | 🟢 アクティブ | データ駆動TD、ImGui内蔵エディター対応 |
| **Roguelike** | NetHackGame | main_roguelike.cpp | 🔴 開発中 | ターン制ローグライク（設計済、実装未着手） |

**推奨**: 初めての方は **SimpleTDCGame**（統合プラットフォーム）を使用してください。

> **Note**: 旧TDゲームは完全に廃止されました。統合プラットフォーム（main_unified.cpp）のみを使用してください。

### ディレクトリ構造

```
Simple-TDC-GameProject/
├── CMakeLists.txt          # CMakeビルド設定
├── src/                    # ソースコード
│   ├── main_unified.cpp   # 統合プラットフォームエントリポイント（推奨）
│   ├── main_new.cpp       # 新TDゲームエントリポイント
│   ├── main_roguelike.cpp # Roguelikeエントリポイント
│   ├── Application/       # アプリケーション層
│   │   ├── UnifiedGame.cpp
│   │   └── HomeScene.cpp
│   ├── Core/              # 共通基盤層
│   ├── Game/              # ゲーム汎用層
│   │   ├── Editor/        # ImGui内蔵エディター
│   │   └── Systems/
│   ├── Data/              # データ層
│   │   └── Serializers/
│   └── ...
├── include/                # ヘッダーファイル
│   ├── Application/       # アプリケーション層
│   ├── Core/              # Core層ヘッダー
│   ├── Game/              # Game層ヘッダー
│   ├── TD/                # TD固有ヘッダー
│   ├── Roguelike/         # Roguelike固有ヘッダー（設計済）
│   ├── ComponentsNew.h    # 統合コンポーネント定義
│   └── ...
├── external/               # 外部ライブラリ（FetchContentで管理）
├── assets/                 # ゲームアセット
│   ├── definitions/       # JSON定義ファイル
│   │   ├── characters/    # キャラクター定義
│   │   ├── stages/        # ステージ定義
│   │   ├── skills/        # スキル定義（Phase 6）
│   │   ├── effects/       # エフェクト定義（Phase 6）
│   │   └── sounds/        # サウンド定義（Phase 6）
│   ├── config.json        # 基本設定ファイル
│   └── fonts/             # フォントファイル（日本語フォント等）
├── docs/                   # ドキュメント
│   ├── README.md          # ドキュメント索引
│   ├── INTEGRATION_STATUS.md  # 統合状況レポート
│   ├── HANDOVER.md        # 新アーキテクチャ詳細
│   ├── ROGUELIKE_SYSTEM_DESIGN.md  # Roguelike設計
│   ├── CHARACTER_SYSTEM_DESIGN.md  # キャラクターシステム設計
│   └── ...
├── .cursor/                # Cursor IDE開発ガイド
│   ├── CURSOR_DEVELOPMENT_GUIDE.md
│   ├── AI_AGENT_QUICK_REFERENCE.md
│   └── ARCHITECTURE_DIAGRAMS.md
└── .github/
    ├── workflows/         # GitHub Actions設定
    ├── ISSUE_TEMPLATE/    # Issueテンプレート
    └── copilot-instructions.md  # コーディング規約
```

## ビルド手順

### 最初のセットアップ（推奨）

初回クローン後、このコマンドを実行すればすべてが自動化されます：

#### Windows (PowerShell)

```powershell
# リポジトリをクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# 自動セットアップ実行
.\setup.ps1

# または Python を使用
python setup.py
```

#### Linux / macOS

```bash
# リポジトリをクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# 自動セットアップ実行
python3 setup.py
```

**セットアップスクリプトが自動的に以下を実行します：**

- ✅ Ninja ビルドツールのダウンロード
- ✅ ゲームの完全ビルド
- ✅ 環境検証

**所要時間**: 初回 3-5 分（インターネット速度に依存）

### 必要な環境

- **CMake 3.15 以上**（CMake Presets を使用する場合は 3.19 以上）
- **C++17 対応コンパイラ**
  - Windows: Visual Studio 2019 以上、MinGW、または Clang
  - Linux: GCC 7 以上、または Clang 5 以上
  - macOS: Xcode 10 以上、または Clang 5 以上

### 高速ビルド（Ninja 使用・推奨）

Ninja を使用するとVisual Studioよりも高速にビルドできます。

**🎉 Ninja は自動的にダウンロードされます！** CMake実行時にNinjaが見つからない場合、自動的に`.tools/bin/`にダウンロードされます。

#### 方法1: 自動セットアップスクリプト（最も簡単）

```powershell
# Ninjaのダウンロード、MSVC環境セットアップ、ビルドを自動実行
.\scripts\build-with-ninja.ps1

# Releaseビルドの場合
.\scripts\build-with-ninja.ps1 -Config Release
```

#### 方法2: CMake Presetsを使用（Ninja自動ダウンロード対応）

```powershell
# CMakeがNinjaを自動的にダウンロードします（初回のみ）
cmake --preset ninja-debug
cmake --build build --config Debug
```

#### 方法3: 手動セットアップ

```powershell
# 1. Ninja を手動でダウンロード（初回のみ）
.\scripts\bootstrap-ninja.ps1

# 2. MSVC環境とビルドを実行
.\scripts\setup-ninja-env.ps1 -Config Debug
```

詳細は [docs/BUILD_WITH_NINJA.md](docs/BUILD_WITH_NINJA.md) を参照してください。

### Windows (Visual Studio) - 手動ビルド

```bash
# リポジトリをクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# ビルドディレクトリを作成
mkdir build
cd build

# CMake設定
cmake ..

# ビルド
cmake --build . --config Release

# 実行
.\bin\Release\SimpleTDCGame.exe
```

### Linux / macOS - 手動ビルド

```bash
# リポジトリをクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# ビルドディレクトリを作成
mkdir build
cd build

# CMake設定とビルド
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# 実行
./bin/SimpleTDCGame
```

## 外部ライブラリ管理

### CMake FetchContent（推奨）

本プロジェクトでは CMake の FetchContent を使用して外部ライブラリを自動取得します。
ビルド時に以下のライブラリが自動的にダウンロードされます：

- EnTT (v3.12.2)
- nlohmann/json (v3.11.2)
- Raylib (5.0)
- raygui (4.0)
- Dear ImGui (v1.90.1)
- rlImGui (57efef0)

### Git Submodule（代替方法）

手動で git submodule を使用してライブラリを管理する場合は、以下のコマンドを実行します：

```bash
# raygui を external/raygui に追加
git submodule add https://github.com/raysan5/raygui.git external/raygui
cd external/raygui && git checkout 4.0 && cd ../..

# Dear ImGui を external/imgui に追加
git submodule add https://github.com/ocornut/imgui.git external/imgui
cd external/imgui && git checkout v1.90.1 && cd ../..

# rlImGui を external/rlImGui に追加
git submodule add https://github.com/raylib-extras/rlImGui.git external/rlImGui
cd external/rlImGui && git checkout 57efef0 && cd ../..

# サブモジュールを初期化・更新
git submodule update --init --recursive
```

注意: サブモジュールを使用する場合は、CMakeLists.txt の FetchContent 部分を適宜修正してください。

## 開発者モード（予定）

### 概要

**🚧 実装予定**: WebUIを廃止し、ゲーム内に統合された開発者モードを実装します。F1キーで起動し、Asepriteライクなウィンドウ配置で、エンティティ、ステージ、イベント、UIをリアルタイムで編集できます。

### 主な機能（予定）

- **エンティティエディター**: TD専用のエンティティ動作定義
  - スプライトアニメーション設定
  - ステートマシンのビジュアル編集
  - エフェクト・サウンドのタイミング調整
  - プレビュー機能

- **ステージエディター**: ステージ制作
  - Wave構成の定義
  - エンティティ配置・タイミング設定
  - プレビュー再生

- **イベントエディター**: イベント制作
  - トリガー条件設定
  - アクションシーケンス
  - カットシーン制作

- **UIエディター**: UI定義
  - レイアウト調整
  - インタラクション設定

- **ホットリロード**: JSON定義ファイルの変更を自動検知し、ゲーム実行中にリアルタイム反映

### 詳細ドキュメント

- [開発者モード設計書](docs/DEVELOPER_MODE_DESIGN.md) - アーキテクチャと設計方針
- [TD専用エンティティ動作システム](docs/TD_ENTITY_BEHAVIOR_SYSTEM.md) - にゃんこ大戦争風のシンプルな動作定義
- [WebUI移行ガイド](docs/WEBUI_TO_INGAME_EDITOR_MIGRATION.md) - HTTPServer廃止と移行計画
- [開発者モード クイックリファレンス](docs/DEVELOPER_MODE_QUICK_REFERENCE.md) - 操作方法と機能一覧
- [実装ロードマップ](docs/DEVELOPER_MODE_IMPLEMENTATION_ROADMAP.md) - 8週間の実装計画

## ImGui内蔵エディター（現行）

### 概要

F1キーで開くImGuiベースの内蔵エディターで、ゲームの定義ファイル（キャラクター、ステージ、UI、スキル、エフェクト、サウンド）を閲覧できます。

> **Note**: 現在は閲覧機能のみ実装されています。上記の「開発者モード」で完全な編集機能を提供する予定です。

### 現在の機能

- **キャラクターエディター**: キャラクター定義の閲覧
  - キャラクターリスト表示
  - 詳細情報ビュー（ID、名前、説明、ステータス）

- **コンソール**: ログメッセージの表示
  - タイムスタンプ付きログ
  - 自動スクロール

- **インスペクター**: エンティティ情報の表示（予定）

### 使い方

1. ゲームを起動
2. F1キーを押してエディターを表示/非表示
3. メニューバーから各ウィンドウを開く
4. キャラクターリストから編集したいキャラクターを選択

## 開発方針

### ECSアーキテクチャ

このプロジェクトではEnTTライブラリを使用したECSアーキテクチャを採用しています。

- **Entity（エンティティ）**: ゲーム内のオブジェクトを表す一意のID
- **Component（コンポーネント）**: データのみを持つシンプルな構造体
- **System（システム）**: コンポーネントに対するロジックを実装

#### レイヤー分離アーキテクチャ

プロジェクトは明確な責務分離により、保守性と拡張性を確保しています：

```
┌─────────────────────────────────────────────┐
│         Application層（アプリケーション）      │
│  UnifiedGame, HomeScene, モード管理          │
└─────────────────────┬───────────────────────┘
                      │
        ┌─────────────┴─────────────┐
        │                           │
┌───────▼────────┐         ┌────────▼────────┐
│    TD層        │         │  Roguelike層    │
│  TD固有の      │         │  Roguelike固有  │
│  コンポーネント │         │  のコンポーネント│
│  システム      │         │  システム        │
└───────┬────────┘         └────────┬────────┘
        │                           │
        └─────────────┬─────────────┘
                      │
        ┌─────────────▼─────────────┐
        │       Game層（汎用）       │
        │  Sprite, Animation等      │
        │  ゲーム共通要素            │
        └─────────────┬─────────────┘
                      │
        ┌─────────────▼─────────────┐
        │       Core層（基盤）       │
        │  World, GameContext,      │
        │  SystemRunner, Renderer   │
        └─────────────┬─────────────┘
                      │
        ┌─────────────▼─────────────┐
        │    External Libraries     │
        │  EnTT, Raylib, ImGui等    │
        └───────────────────────────┘
```

**Core層** (共通基盤):

- `World`: entt::registry ラッパー
- `GameContext`: 依存性注入コンテナ
- `SystemRunner`: システム実行管理
- `EntityFactory`: エンティティ生成
- `DefinitionRegistry`: JSON定義管理
- `GameRenderer`: FHD固定レンダリング

**Game層** (ゲーム汎用):

- `Sprite`, `Animation`: 描画関連コンポーネント
- 複数ゲームで共有可能な汎用要素

**TD層** (タワーディフェンス固有):

- `Lane`, `Stats`: TD固有コンポーネント
- 10個のTDシステム（移動、戦闘、Wave管理等）

**Roguelike層** (ローグライク固有):

- `GridPosition`, `Tile`: グリッドベース要素
- ターン制システム、ダンジョン生成（設計済）

#### モジュラーシステムアーキテクチャ

ゲームの各処理（入力、更新、レンダリング）は独立したシステムとして実装され、疎結合な設計になっています。

- **Core::ISystem**: すべてのシステムが実装する抽象インターフェース
  - `ProcessInput()`: 入力処理
  - `Update()`: ゲームロジック更新
  - `Render()`: レンダリング処理

- **SystemRunner**: システムのライフサイクルを管理
  - システムの登録・実行を一元管理
  - `unique_ptr`でメモリ管理を自動化
  - システムの追加・削除が容易

- **実装済みシステム例**:
  - `InputSystem`: プレイヤー入力処理
  - `LaneMovementSystem`: レーン移動
  - `CombatSystem`: 戦闘処理
  - `AbilitySystem`: スキル・能力実行

この設計により、システムの差し替えや単体テストが容易になり、将来的な拡張性が向上しています。

### データ駆動設計

ゲームの設定やデータはJSON形式で外部ファイルとして管理し、コードの変更なしにゲームの挙動を調整できるようにします。

**JSON定義ファイルの種類:**

- `assets/definitions/characters/*.char.json` - キャラクター定義
- `assets/definitions/stages/*.stage.json` - ステージ定義
- `assets/definitions/ui/*.ui.json` - UIレイアウト定義
- `assets/definitions/skills/*.skill.json` - スキル定義（Phase 6）
- `assets/definitions/effects/*.effect.json` - エフェクト定義（Phase 6）
- `assets/definitions/sounds/*.sound.json` - サウンド定義（Phase 6）

**開発者モード（予定）:**
これらのJSON定義ファイルは、ゲーム内蔵の開発者モードで視覚的に編集でき、ホットリロード機能によりゲーム実行中にリアルタイムで反映されます。詳細は[開発者モード設計書](docs/DEVELOPER_MODE_DESIGN.md)を参照してください。

### UIシステム (raygui + Dear ImGui)

本プロジェクトでは2つのUIライブラリを共存させて使用しています。

#### raygui

- **用途**: ゲーム内HUD、操作パネル、メニュー
- **特徴**: Raylibネイティブ、軽量、即時モードGUI

#### Dear ImGui (rlImGui経由)

- **用途**: 開発者モード、デバッグウィンドウ、開発ツール
- **特徴**: 豊富なウィジェット、Docking対応、業界標準
- **将来**: ゲーム内蔵エディタのメインUIとして使用予定

#### 描画順序

メインループでは以下の順序で描画を行います：

```cpp
BeginDrawing();
ClearBackground(RAYWHITE);

// 1. ゲーム世界・3D描画
// ... (ゲームシーン描画)

// 2. raygui描画 (HUD/操作パネル)
GuiButton((Rectangle){...}, "日本語ボタン"); 

// 3. rlImGui描画 (デバッグ/ツール)
rlImGuiBegin();
ImGui::Begin("Debug Info");
ImGui::Text("FPS: %d", GetFPS());
ImGui::Text("日本語デバッグ表示テスト");
ImGui::End();
rlImGuiEnd();

EndDrawing();
```

#### 日本語フォント対応

日本語フォントを使用する場合は、`assets/fonts/`ディレクトリにフォントファイル（例: `NotoSansCJKjp-Regular.otf`）を配置してください。UIManagerが自動的にrayguiとImGuiの両方に適用します。

### コーディング規約

詳細なコーディング規約については、[.github/copilot-instructions.md](.github/copilot-instructions.md)を参照してください。

主な規約:

- EnTTコンポーネントはシンプルなデータ構造として定義
- JSON解析は必ずtry-catchで囲む
- 1コミット = 1機能単位
- ブランチ戦略: GitFlow

### Cursor IDE 開発者向け

Cursor IDE でマルチエージェント開発を行う場合は、以下のドキュメントを参照してください：

- **[.cursor/CURSOR_DEVELOPMENT_GUIDE.md](.cursor/CURSOR_DEVELOPMENT_GUIDE.md)** - 包括的な開発ガイド（必読）
- **[.cursor/AI_AGENT_QUICK_REFERENCE.md](.cursor/AI_AGENT_QUICK_REFERENCE.md)** - クイックリファレンス
- **[.cursor/ARCHITECTURE_DIAGRAMS.md](.cursor/ARCHITECTURE_DIAGRAMS.md)** - アーキテクチャ図解
- **[.cursorrules](.cursorrules)** - Cursor プロジェクトルール

### Cursor でのビルド

Cursor は VS Code の拡張としても動作するため、`.vscode/tasks.json` に用意したビルドタスクを `Ctrl+Shift+B`（または Command⌘+Shift+B）で呼び出すことで、`scripts/build-with-ninja.ps1` を使った Debug ビルドを実行できます。Release ビルドはコマンドパレットの「Tasks: Run Build Task」で `Build SimpleTDCGame_NewArch (Release)` を選ぶか、ショートカットで直接起動してください。タスクは Ninja と VS 環境を自動セットアップした後、`SimpleTDCGame_NewArch` をビルドします。

これらのドキュメントには以下が含まれます：

- マルチエージェント開発戦略
- 設計改善の要点
- 推奨される設計パターン
- AIエージェント向けタスク分割ガイド

## ドキュメント

プロジェクトの詳細なドキュメントは `docs/` ディレクトリにあります：

### 概要・設計ドキュメント

- **[docs/README.md](docs/README.md)** - ドキュメント索引（全ドキュメント一覧）
- **[docs/INTEGRATION_STATUS.md](docs/INTEGRATION_STATUS.md)** - 3プロジェクト統合状況レポート
- **[docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md)** - プロジェクト概要と重要な推奨事項
- **[docs/HANDOVER.md](docs/HANDOVER.md)** - 新アーキテクチャ詳細（Phase 5実装仕様）

### システム設計ドキュメント

- **[docs/CHARACTER_SYSTEM_DESIGN.md](docs/CHARACTER_SYSTEM_DESIGN.md)** - 統一キャラクターシステム設計
- **[docs/ROGUELIKE_SYSTEM_DESIGN.md](docs/ROGUELIKE_SYSTEM_DESIGN.md)** - Roguelikeシステム設計
- **[docs/DEVELOPER_MODE_DESIGN.md](docs/DEVELOPER_MODE_DESIGN.md)** - 開発者モード設計書（新規）
- **[docs/TD_ENTITY_BEHAVIOR_SYSTEM.md](docs/TD_ENTITY_BEHAVIOR_SYSTEM.md)** - TD専用エンティティ動作システム（新規）
- **[docs/CODE_ANALYSIS.md](docs/CODE_ANALYSIS.md)** - 詳細なコード分析レポート
- **[docs/REFACTORING_PLAN.md](docs/REFACTORING_PLAN.md)** - リファクタリング計画

### 開発者向けドキュメント

- **[docs/DEVELOPER_MANUAL.md](docs/DEVELOPER_MANUAL.md)** - 開発者マニュアル
- **[docs/MIGRATION_GUIDE.md](docs/MIGRATION_GUIDE.md)** - 旧アーキテクチャからの移行ガイド
- **[docs/WEBUI_TO_INGAME_EDITOR_MIGRATION.md](docs/WEBUI_TO_INGAME_EDITOR_MIGRATION.md)** - WebUI移行ガイド（新規）
- **[docs/DEVELOPER_MODE_QUICK_REFERENCE.md](docs/DEVELOPER_MODE_QUICK_REFERENCE.md)** - 開発者モード クイックリファレンス（新規）
- **[docs/DEVELOPER_MODE_IMPLEMENTATION_ROADMAP.md](docs/DEVELOPER_MODE_IMPLEMENTATION_ROADMAP.md)** - 実装ロードマップ（新規）
- **[docs/BUILD_WITH_NINJA.md](docs/BUILD_WITH_NINJA.md)** - Ninjaビルドガイド
- **[docs/FONT_SETUP.md](docs/FONT_SETUP.md)** - 日本語フォント設定
- **[docs/QUICK_REFERENCE.md](docs/QUICK_REFERENCE.md)** - クイックリファレンス
- **[docs/OPTIMIZATION_SUMMARY.md](docs/OPTIMIZATION_SUMMARY.md)** - 最適化サマリー

### WebUIエディター（廃止予定）

> **Note**: WebUIエディターは将来的に廃止され、ゲーム内蔵の開発者モードに置き換えられます。

- **[tools/webui-editor/README.md](tools/webui-editor/README.md)** - WebUIエディター使用ガイド（参考用）

### Phase完了レポート

- **[PHASE5_PR_DESCRIPTION.md](PHASE5_PR_DESCRIPTION.md)** - Phase 5完了レポート（WebUIエディター基盤）
- **[PHASE6_REPORT.md](PHASE6_REPORT.md)** - Phase 6完了レポート（スキル・エフェクト・サウンド）

## プロジェクト進捗状況

```
Phase 1:   旧TD廃止                      ✅ 完了
Phase 2:   ゲームシーン実装              ✅ 完了
  2.1:     TDゲームシーン               ✅ 完了
  2.2:     Roguelikeゲームシーン        ✅ 完了
  2.3:     シーン遷移実装               ✅ 完了
Phase 3:   Roguelike基本機能            ⏳ 次フェーズ
Phase 4:   アイテム共有システム         ⏳ 計画中
Phase 5:   HTTPサーバー完成             ✅ 完了
  5A:      REST API基盤                ✅ 完了
  5B:      エンティティエディター       ✅ 完了
  5C:      ステージエディター          ✅ 完了
  5D:      UIエディター                ✅ 完了
Phase 6:   ドキュメント整理             ✅ 完了
  6A:      開発者モード設計書          ✅ 完了
  6B:      TD専用エンティティ設計       ✅ 完了
  6C:      WebUI移行計画               ✅ 完了
Phase 7:   開発者モード実装             📋 設計完了
  7A:      基盤構築 (2週間)            ⏳ 計画中
  7B:      ホットリロード (1週間)       ⏳ 計画中
  7C:      エンティティエディタ (2週間) ⏳ 計画中
  7D:      ステージエディタ (1週間)     ⏳ 計画中
  7E:      統合・最適化 (2週間)         ⏳ 計画中
```

## 今後の展望

### 短期目標（Phase 7）

- **開発者モード実装**: WebUIを廃止し、ゲーム内蔵エディタを実装
  - DockSpaceベースのウィンドウ管理
  - エンティティ・ステージ・イベント・UIエディタ
  - ホットリロードシステム
  - ビジュアルステートマシンエディタ
  - リアルタイムプレビュー機能

### 中期目標（Phase 8）

- TD専用ゲーム開発環境の完成
  - にゃんこ大戦争風のシンプルなTDゲーム制作
  - レベルデザイナー向けワークフロー最適化
- Roguelikeゲームの実装開始（基本移動、ダンジョン生成、戦闘システム）
- パフォーマンス最適化
- テストカバレッジ拡充

### 長期目標

- HTTPServer完全廃止とWebUI削除
- 開発環境の完全なゲーム内統合
- プラグイン化による統合プラットフォーム完成
- クロスプラットフォーム対応の検討

## 貢献方法

プロジェクトへの貢献を歓迎します！詳細は[CONTRIBUTING.md](CONTRIBUTING.md)をご覧ください。

## ライセンス

このプロジェクトはMITライセンスの下で公開されています。

## 参考リンク

### ライブラリ公式ドキュメント

- [EnTT Documentation](https://github.com/skypjack/entt/wiki)
- [Raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [nlohmann/json Documentation](https://json.nlohmann.me/)
- [raygui Repository](https://github.com/raysan5/raygui)
- [Dear ImGui Documentation](https://github.com/ocornut/imgui)
- [rlImGui Repository](https://github.com/raylib-extras/rlImGui)

### プロジェクトドキュメント

- [ドキュメント索引](docs/README.md) - 全ドキュメント一覧
- [Cursor IDE 開発ガイド](.cursor/CURSOR_DEVELOPMENT_GUIDE.md) - マルチエージェント開発
