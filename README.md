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
- **CI/CD**: GitHub Actions（Windows自動ビルド）

## プロジェクト構成
```
Simple-TDC-GameProject/
├── CMakeLists.txt          # CMakeビルド設定
├── src/                    # ソースコード
│   ├── main.cpp           # エントリーポイント
│   ├── Game.cpp           # ゲームメインループ
│   └── Systems.cpp        # ECSシステム実装
├── include/                # ヘッダーファイル
│   ├── Game.h
│   ├── Components.h       # ECSコンポーネント定義
│   └── Systems.h          # ECSシステム定義
├── external/               # 外部ライブラリ（FetchContentで管理）
├── assets/                 # ゲームアセット
│   └── config.json        # 設定ファイル
└── .github/
    ├── workflows/         # GitHub Actions設定
    ├── ISSUE_TEMPLATE/    # Issueテンプレート
    └── copilot-instructions.md  # コーディング規約
```

## ビルド手順

### 必要な環境
- CMake 3.15以上
- C++17対応コンパイラ
  - Windows: Visual Studio 2019以上、MinGW、または Clang
  - Linux: GCC 7以上、または Clang 5以上
  - macOS: Xcode 10以上、または Clang 5以上

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

## 開発方針

### ECSアーキテクチャ
このプロジェクトではEnTTライブラリを使用したECSアーキテクチャを採用しています。

- **Entity（エンティティ）**: ゲーム内のオブジェクトを表す一意のID
- **Component（コンポーネント）**: データのみを持つシンプルな構造体
- **System（システム）**: コンポーネントに対するロジックを実装

### データ駆動設計
ゲームの設定やデータはJSON形式で外部ファイルとして管理し、コードの変更なしにゲームの挙動を調整できるようにします。

### コーディング規約
詳細なコーディング規約については、[.github/copilot-instructions.md](.github/copilot-instructions.md)を参照してください。

主な規約:
- EnTTコンポーネントはシンプルなデータ構造として定義
- JSON解析は必ずtry-catchで囲む
- 1コミット = 1機能単位
- ブランチ戦略: GitFlow

## 貢献方法
プロジェクトへの貢献を歓迎します！詳細は[CONTRIBUTING.md](CONTRIBUTING.md)をご覧ください。

## ライセンス
このプロジェクトはMITライセンスの下で公開されています。

## 参考リンク
- [EnTT Documentation](https://github.com/skypjack/entt/wiki)
- [Raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [nlohmann/json Documentation](https://json.nlohmann.me/)