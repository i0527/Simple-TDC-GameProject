# 🎮 Simple TDC Game - New Architecture

**バージョン**: 0.2.0  
**アーキテクチャ**: 完全分離型（Game + Editor）

[![Build Status](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml/badge.svg)](https://github.com/i0527/Simple-TDC-GameProject/actions/workflows/windows-build.yml)

---

## 📑 プロジェクト概要

Simple TDC Game は、「にゃんこ大戦争」風の横スクロール タワーディフェンスゲームです。  
**完全分離型アーキテクチャ** を採用し、**ゲーム本体** と **エディタ** が独立した実行ファイルとして動作します。

### ✨ 主な特徴

- ✅ **データドリブン設計**: JSON定義によるキャラクター・スキル・ステージ管理
- ✅ **ホットリロード**: JSON編集時にリアルタイムで反映
- ✅ **完全分離型**: ゲームとエディタが独立（軽量なリリースビルド）
- ✅ **ECS アーキテクチャ**: EnTT を使用した高性能なエンティティ管理
- ✅ **ImGui エディタ**: データ編集用の専用エディタ

---

## 🏗️ アーキテクチャ構成

```
┌──────────────────────────────────────────────────────────────┐
│                      Shared Layer                            │
│  ├─ Core (GameContext, EventSystem, FileWatcher)           │
│  └─ Data (Definitions, Loaders, Validators)                │
└──────────────┬──────────────────────────────────────────────┘
               │
        ┌──────┴──────┐
        ↓             ↓
┌─────────────────────┐  ┌──────────────────────────┐
│  Game Executable    │  │  Editor Executable       │
│  (SimpleTDCGame)    │  │  (SimpleTDCEditor)       │
│                     │  │                          │
│  ├─ Managers        │  │  ├─ ImGui Windows        │
│  ├─ ECS Components  │  │  ├─ Data Editors         │
│  ├─ ECS Systems     │  │  └─ Services             │
│  └─ Raylib Renderer │  └──────────────────────────┘
└─────────────────────┘
```

詳細は [Architecture_Complete.md](docs/design/Architecture_Complete.md) を参照してください。

---

## 📂 ディレクトリ構造

```
Simple-TDC-GameProject/
├── CMakeLists.txt                 # ルートCMake設定
├── README.md
├── shared/                        # 共有ライブラリ（Game + Editor共通）
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Core/                  # GameContext, EventSystem, FileWatcher
│   │   └── Data/                  # Definitions, Loaders, Validators
│   └── src/
├── game/                          # ゲーム本体
│   ├── CMakeLists.txt
│   ├── include/Game/
│   │   ├── Application/           # GameApp
│   │   ├── Components/            # ECS Components
│   │   ├── Managers/              # EntityManager, SkillManager, etc
│   │   └── Systems/               # ECS Systems
│   └── src/
│       └── main_game.cpp
├── editor/                        # エディタ
│   ├── CMakeLists.txt
│   ├── include/Editor/
│   │   ├── Application/           # EditorApp
│   │   ├── Windows/               # ImGui Windows
│   │   └── Services/              # Editor Services
│   └── src/
│       └── main_editor.cpp
├── assets/                        # アセット（JSON定義、画像、音声）
│   ├── config.json
│   └── definitions/
│       ├── entities_debug.json
│       ├── abilities_debug.json
│       ├── stages_debug.json
│       └── waves_debug.json
├── docs/design/                   # 設計ドキュメント
└── old/                           # 旧実装アーカイブ
```

---

## 🚀 ビルド手順

### 必要な環境

- **CMake**: 3.19 以上
- **C++ コンパイラ**: C++17 対応（MSVC, GCC, Clang）
- **Git**: FetchContent で依存関係を自動取得

### ビルドコマンド（Windows PowerShell）

```powershell
# ビルドディレクトリ作成
cmake -B build -S .

# ビルド実行
cmake --build build --config Debug

# 実行
.\build\game\Debug\SimpleTDCGame.exe      # ゲーム本体
.\build\editor\Debug\SimpleTDCEditor.exe  # エディタ
```

### 依存ライブラリ（自動取得）

| ライブラリ | バージョン | 用途 |
|-----------|----------|------|
| **EnTT** | v3.12.2 | ECS |
| **nlohmann/json** | v3.11.3 | JSON パース |
| **Raylib** | 5.5 | レンダリング/入力 |
| **Dear ImGui** | v1.89.9 | エディタUI |
| **rlImGui** | main branch | Raylib-ImGui統合 |

**注**: すべての依存関係はCMake FetchContentで自動ダウンロードされます。

---

## 📖 使い方

### ゲーム本体 (SimpleTDCGame)

1. `SimpleTDCGame.exe` を起動
2. タイトル画面で「続きから」または「新規ゲーム」を選択
3. ホーム画面から「ステージ選択」でTDゲーム開始

### エディタ (SimpleTDCEditor)

1. `SimpleTDCEditor.exe` を起動
2. メニューバーから編集したいウィンドウを選択（Entity/Skill/Stage Editor）
3. JSON定義を編集して保存
4. ゲーム本体が起動中であればホットリロードで即座に反映

---

## 🧪 開発状況

### Phase 1: 基盤構築 ✅ 完了

- [x] Shared Layer（Core + Data）実装
- [x] Game Layer 基盤（Managers + Components）
- [x] Editor Layer 基盤（ImGui統合）
- [x] CMake ビルドシステム構築
- [x] 解像度: FHD（1920x1080）固定
- [ ] 日本語フォント対応（NotoSansJP-Medium.ttf）
  - ひらがな、カタカナ、ASCII
  - JIS第一水準、JIS第二水準
  - Raylib/ImGuiデフォルトフォントとして設定
  - 全UI日本語表記

### Phase 2: ゲームロジック実装（予定）

- [ ] ECS Systems（Movement, Attack, Skill）
- [ ] Scene Manager（画面遷移）
- [ ] セーブ/ロード機能
- [ ] TD ゲームコア実装

### Phase 3: エディタ機能実装（予定）

- [ ] Entity Editor Window
- [ ] Skill Editor Window
- [ ] Stage Editor Window
- [ ] Wave Editor Window

---

## 📝 設計ドキュメント

詳細な設計資料は `docs/design/` を参照してください。

- [アーキテクチャ設計 - 完全分離型](docs/design/Architecture_Complete.md)
- [ゲームデザイン仕様書](docs/design/TD_GameDesign.md)
- [TD Layer ECS設計](docs/design/TD_Layer_ECS_Design.md)
- [Application Layer設計](docs/design/Application_Layer_Design.md)
- [Editor Layer設計](docs/design/Editor_Layer_Design.md)

---

## 🤝 コントリビューション

プロジェクトへの貢献を歓迎します！

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/new-feature`)
3. 変更をコミット (`git commit -m 'feat: Add new feature'`)
4. ブランチをプッシュ (`git push origin feature/new-feature`)
5. プルリクエストを作成

詳細は [CONTRIBUTING.md](CONTRIBUTING.md) を参照してください。

---

## 📄 ライセンス

[MIT License](LICENSE)

---

## 📧 連絡先

プロジェクトに関する質問や提案は、Issue または Pull Request でお願いします。
