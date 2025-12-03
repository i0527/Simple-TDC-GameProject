# Simple-TDC-GameProject

[![Windows Build](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml/badge.svg)](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml)

## 概要

Simple-TDC-GameProjectは、ECS（Entity Component System）アーキテクチャを採用したC++ゲームプロジェクトです。データ駆動設計とモダンなC++開発手法を学ぶためのシンプルなゲームフレームワークを提供します。

## 技術スタック

- **言語**: C++17以上
- **ビルドシステム**: CMake 3.15以上
- **ECS**: [EnTT](https://github.com/skypjack/entt) - 高速で軽量なEntity Component Systemライブラリ
- **JSON**: [nlohmann/json](https://github.com/nlohmann/json) - モダンなC++ JSON ライブラリ
- **レンダリング/入力**: [Raylib](https://www.raylib.com/) - シンプルで使いやすいゲーム開発ライブラリ
- **UI (HUD/操作パネル)**: [raygui](https://github.com/raysan5/raygui) - Raylib用の即時モードGUIライブラリ
- **UI (デバッグ/ツール)**: [Dear ImGui](https://github.com/ocornut/imgui) + [rlImGui](https://github.com/raylib-extras/rlImGui) - Raylib統合されたImGui
- **CI/CD**: GitHub Actions（Windows自動ビルド、actions/checkout@v4、actions/upload-artifact@v4）

## プロジェクト構成

```
Simple-TDC-GameProject/
├── CMakeLists.txt          # CMakeビルド設定
├── src/                    # ソースコード
│   ├── main.cpp           # エントリーポイント
│   ├── Game.cpp           # ゲームメインループ
│   ├── UIManager.cpp      # UIマネージャー (raygui + ImGui)
│   ├── Systems.cpp        # ECSシステム実装
│   └── SystemManager.cpp  # システム管理
├── include/                # ヘッダーファイル
│   ├── Game.h
│   ├── UIManager.h        # UIマネージャー定義
│   ├── Components.h       # ECSコンポーネント定義
│   ├── Systems.h          # ECSシステム定義
│   ├── System.h           # システムインターフェース
│   └── SystemManager.h    # システム管理
├── external/               # 外部ライブラリ（FetchContentで管理）
├── assets/                 # ゲームアセット
│   ├── config.json        # 設定ファイル
│   └── fonts/             # フォントファイル（日本語フォント等）
└── .github/
    ├── workflows/         # GitHub Actions設定
    ├── ISSUE_TEMPLATE/    # Issueテンプレート
    └── copilot-instructions.md  # コーディング規約
```

## ビルド手順

### 必要な環境

- CMake 3.15以上（CMake Presets を使用する場合は 3.19以上）
- C++17対応コンパイラ
  - Windows: Visual Studio 2019以上、MinGW、または Clang
  - Linux: GCC 7以上、または Clang 5以上
  - macOS: Xcode 10以上、または Clang 5以上

### 高速ビルド（Ninja 使用・推奨）

Ninja を使用するとVisual Studioよりも高速にビルドできます。

**🎉 Ninjaは自動的にダウンロードされます！** CMake実行時にNinjaが見つからない場合、自動的に`.tools/bin/`にダウンロードされます。

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

### Windows (Visual Studio)

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

### Linux / macOS

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

## 開発方針

### ECSアーキテクチャ

このプロジェクトではEnTTライブラリを使用したECSアーキテクチャを採用しています。

- **Entity（エンティティ）**: ゲーム内のオブジェクトを表す一意のID
- **Component（コンポーネント）**: データのみを持つシンプルな構造体
- **System（システム）**: コンポーネントに対するロジックを実装

#### モジュラーシステムアーキテクチャ

ゲームの各処理（入力、更新、レンダリング）は独立したシステムとして実装され、疎結合な設計になっています。

- **Core::ISystem**: すべてのシステムが実装する抽象インターフェース
  - `ProcessInput()`: 入力処理
  - `Update()`: ゲームロジック更新
  - `Render()`: レンダリング処理

- **SystemManager**: システムのライフサイクルを管理
  - システムの登録・実行を一元管理
  - `unique_ptr`でメモリ管理を自動化
  - システムの追加・削除が容易

- **実装済みシステム**:
  - `InputSystem`: プレイヤー入力処理
  - `MovementSystem`: 移動ロジック
  - `RenderSystem`: エンティティ描画

この設計により、システムの差し替えや単体テストが容易になり、将来的な拡張性が向上しています。

### データ駆動設計

ゲームの設定やデータはJSON形式で外部ファイルとして管理し、コードの変更なしにゲームの挙動を調整できるようにします。

### UIシステム (raygui + Dear ImGui)

本プロジェクトでは2つのUIライブラリを共存させて使用しています。

#### raygui

- **用途**: ゲーム内HUD、操作パネル、メニュー
- **特徴**: Raylibネイティブ、軽量、即時モードGUI

#### Dear ImGui (rlImGui経由)

- **用途**: デバッグウィンドウ、開発ツール、プロファイラ
- **特徴**: 豊富なウィジェット、Docking対応、業界標準

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

これらのドキュメントには以下が含まれます：

- マルチエージェント開発戦略
- 設計改善の要点
- 推奨される設計パターン
- AIエージェント向けタスク分割ガイド

## ドキュメント

プロジェクトの詳細なドキュメントは `docs/` ディレクトリにあります：

- **[docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md)** - プロジェクト概要と重要な推奨事項
- **[docs/CODE_ANALYSIS.md](docs/CODE_ANALYSIS.md)** - 詳細な分析レポート
- **[docs/REFACTORING_PLAN.md](docs/REFACTORING_PLAN.md)** - リファクタリング計画
- **[docs/README.md](docs/README.md)** - ドキュメント索引

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
