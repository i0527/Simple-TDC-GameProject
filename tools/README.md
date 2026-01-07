# ビルドツール

Visual Studio 2022 Build Tools を使用して、コマンドラインからプロジェクトをビルド＆テストするためのスクリプト集です。

## 必要な環境

- **Visual Studio 2022** または **Visual Studio 2022 Build Tools**
  - C++ デスクトップ開発ワークロード
- **CMake** (3.19以上)
- **Ninja** (推奨、なくてもビルド可能)

## スクリプト一覧

| ファイル | 説明 |
|---------|------|
| `build_debug.bat` | デバッグビルド (コマンドプロンプト用) |
| `build_release.bat` | リリースビルド (コマンドプロンプト用) |
| `build.ps1` | PowerShell用ビルドスクリプト（全機能対応） |
| `run_game.bat` | ビルド済みゲームの実行 |

## 使用方法

### コマンドプロンプト

```batch
REM デバッグビルド
tools\build_debug.bat

REM クリーンビルド（ビルドディレクトリ削除後に再構築）
tools\build_debug.bat clean

REM リリースビルド
tools\build_release.bat

REM ゲーム実行
tools\run_game.bat
tools\run_game.bat release
```

### PowerShell

```powershell
# デバッグビルド
.\tools\build.ps1

# リリースビルド
.\tools\build.ps1 -BuildType Release

# クリーンビルド
.\tools\build.ps1 -Clean

# ビルドして実行
.\tools\build.ps1 -Run

# 組み合わせ
.\tools\build.ps1 -BuildType Release -Clean -Run
```

## ビルド出力先

| ビルドタイプ | 出力ディレクトリ |
|-------------|----------------|
| Debug | `build_cli/game/` |
| Release | `build_release/game/` |

## トラブルシューティング

### CMakeが見つからない

CMakeをインストールして、PATHに追加してください:
- https://cmake.org/download/

### Ninjaが見つからない

Ninjaがない場合は自動的にVisual Studioジェネレータを使用します。
Ninjaを使いたい場合:
- https://github.com/ninja-build/ninja/releases

### vcvarsall.batが見つからない

Visual Studio 2022 または Build Tools で「C++によるデスクトップ開発」ワークロードをインストールしてください。

## 注意事項

- これらのスクリプトはVS2022を自動検出します (`vswhere.exe` 使用)
- Ninjaが利用可能な場合は高速ビルドのためNinjaを優先使用します
- ビルドディレクトリは IDE のビルドディレクトリ (`build/`) とは別に作成されます
