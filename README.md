# Simple TDC Game - New Architecture

[![Windows Build](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml/badge.svg)](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml)

**バージョン**: 0.2.0  
**アーキテクチャ**: 完全分離型（Game + Editor）

## 概要

「にゃんこ大戦争」風の横スクロールタワーディフェンスを題材に、ECS（EnTT）とRaylibで構築したC++17プロジェクトです。ゲーム本体とデータエディタを分離し、データドリブン設計・ホットリロード・日本語UI対応を重視しています。

### 主要ポイント

- データドリブン：JSON定義でエンティティ/スキル/ステージを管理
- ホットリロード：FileWatcherで定義変更を即時反映
- ECS：EnTTによる高速なエンティティ管理
- FHD固定：1920x1080基準、Noto Sans JPによる日本語UI
- 依存関係は FetchContent で自動取得（Gitサブモジュール不要）

## アーキテクチャ

```
┌──────────────────────────────┐
│          Shared Layer         │ Core, Data (Loaders/Validators) │
└──────────────┬───────────────┘
               │
      ┌────────┴───────┐
      │                │
┌──────────────┐ ┌───────────────┐
│ Game          │ │ Editor        │
│ (SimpleTDCGame)│ │ (SimpleTDCEditor)│
└──────────────┘ └───────────────┘
```

- **Shared**: GameContext, EventSystem, FileWatcher, DefinitionRegistry, Loaders, Validators, FontManager
- **Game**: ECSコンポーネント/マネージャ（Entity/Skill/Stage）、シーン管理、基本システム（Movement/Attack/Skill/Rendering）
- **Editor**: ImGui + rlImGui ベースのデータ編集用アプリ
- 旧実装は `old/` にアーカイブ（ビルド対象外）

## 実行ターゲット

| 実行ファイル | 役割 | 出力例 (Debug) | 状態 |
|-------------|------|----------------|------|
| SimpleTDCGame | ゲーム本体 | `build/game/Debug/SimpleTDCGame.exe` | ✅ 動作確認済み |
| SimpleTDCEditor | JSONデータエディタ | `build/editor/Debug/SimpleTDCEditor.exe` | ⚠️ ImGui v1.89.9 + rlImGuiでビルド（互換性注意） |

トップレベルCMakeプロジェクト名は `SimpleTDCGame_NewArch` です。

## ディレクトリ構成

```
Simple-TDC-GameProject/
├── CMakeLists.txt            # ルート（SimpleTDCGame_NewArch）
├── shared/                   # Shared Layer (Core/Data)
├── game/                     # ゲーム本体
├── editor/                   # データエディタ
├── assets/                   # config.json, definitions/*.json, fonts/
├── docs/                     # 設計/ロードマップ/要件
└── old/                      # 旧実装のアーカイブ
```

## ビルド

### 前提
- CMake 3.19 以上
- C++17 対応コンパイラ（MSVC / GCC / Clang）
- Git（FetchContentで依存を取得）

### すぐ試す（Ninja・推奨）
```powershell
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

### Visual Studio ジェネレータ
```powershell
cmake --preset vs2022-debug
cmake --build --preset vs2022-debug
```

### 汎用（手動指定）
```powershell
cmake -B build -S .
cmake --build build --config Debug
```

ビルド後、`build/game/<Config>/SimpleTDCGame.exe` と `build/editor/<Config>/SimpleTDCEditor.exe` を実行してください。`assets/` は自動コピーされます。

## 依存ライブラリ（FetchContentで自動取得）

| ライブラリ | バージョン | 用途 |
|-----------|-----------|------|
| EnTT | v3.12.2 | ECS |
| nlohmann/json | v3.11.3 | JSON |
| Raylib | 5.5 | 入力・描画 |
| Dear ImGui | v1.89.9 | エディタUI |
| rlImGui | main | Raylib-ImGui統合 |

## データとフォント
- 定義ファイル：`assets/definitions/`（entities/abilities/stages/waves などの *_debug.json を同梱）
- 設定：`assets/config.json`
- スキーマ：`assets/schemas/*.schema.json`
- フォント：`assets/fonts/NotoSansJP-Medium.ttf`（日本語UI用、フォント未取得時は警告）

## 現在の進捗（v0.2.0）
- Shared Layer：Context / Event / FileWatcher / Loader / Validator / FontManager 実装済み
- Game Layer：Movement / Attack / Skill / Rendering の土台、SceneManager と Title/TDGameScene の最小実装
- Editor Layer：ImGui + rlImGui 統合、ビルドは ImGui v1.89.9 固定で確認
- FHD固定レンダリング（1920x1080）、日本語UI準備中（フォント統合完了済み、文言は随時更新）

詳細は `docs/RECONSTRUCTION_REPORT.md` と `docs/IMPLEMENTATION_ROADMAP.md` を参照してください。

## ドキュメント
- 再構成レポート: `docs/RECONSTRUCTION_REPORT.md`
- 実装ロードマップ: `docs/IMPLEMENTATION_ROADMAP.md`
- 技術要件: `docs/TECHNICAL_REQUIREMENTS.md`
- 設計資料: `docs/design/Architecture_Complete.md` ほか `docs/design/*.md`
- 旧資料: `docs/old/`（参照のみ、ビルド対象外）

## コントリビューション
歓迎します。`CONTRIBUTING.md` を参照のうえ、フィーチャーブランチでPRを送ってください。

## ライセンス
[MIT License](LICENSE)
