# Ninja を使った高速ビルド (Windows)

このドキュメントでは、Ninja ビルドシステムを使用してプロジェクトをより高速にビルドする方法を説明します。

## 概要

[Ninja](https://ninja-build.org/) は、高速な増分ビルドを重視した軽量ビルドシステムです。Visual Studio のビルドシステムと比較して、特に大規模プロジェクトでのビルド速度が向上します。

このプロジェクトでは、Ninja を**推奨**しますが、**必須ではありません**。Ninja がインストールされていない環境でも、従来の Visual Studio ジェネレータを使用してビルドできます。

## 🎉 自動ダウンロード機能

**CMake 3.19以降を使用している場合、Ninjaは自動的にダウンロードされます！**

CMake実行時にNinjaが見つからない場合：

1. `scripts/bootstrap-ninja.ps1`が自動的に実行されます
2. Ninjaが`.tools/bin/`にダウンロードされます
3. CMakeセッション内でPATHに追加されます

この機能を無効にする場合は、環境変数`NINJA_SKIP_AUTO_SETUP`を設定してください：

```powershell
$env:NINJA_SKIP_AUTO_SETUP = "1"
cmake --preset ninja-debug
```

## クイックスタート (Windows PowerShell)

### 方法1: 自動セットアップスクリプト（最も簡単・推奨）

```powershell
# Ninja のダウンロード、MSVC環境セットアップ、ビルドを自動実行
.\scripts\build-with-ninja.ps1

# Releaseビルドの場合
.\scripts\build-with-ninja.ps1 -Config Release
```

このスクリプトは以下を自動的に実行します:

- Ninja が未インストールの場合、自動ダウンロード
- Visual Studio 環境の自動検出とセットアップ
- CMake の設定とビルド実行

### 方法2: CMake Presetsを使用（Ninja自動ダウンロード対応）

```powershell
# CMakeがNinjaを自動的にダウンロードします（初回のみ）
cmake --preset ninja-debug
cmake --build build --config Debug

# Releaseビルド
cmake --preset ninja-release
cmake --build build --config Release
```

**注意**: この方法ではVisual Studio環境のセットアップは自動で行われないため、事前に「Developer Command Prompt for VS 2022」などから実行するか、方法1を使用してください。

### 方法3: 手動セットアップ

```powershell
# 1. Ninja をダウンロード（初回のみ）
.\scripts\bootstrap-ninja.ps1

# 2. MSVC環境セットアップとビルドを実行
.\scripts\setup-ninja-env.ps1 -Config Debug
```

## 詳細な使用方法

### CMake Presets

このプロジェクトは [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) をサポートしています。

利用可能なプリセット:

| Configure Preset | Build Preset | 説明 |
|------------------|--------------|------|
| `ninja` | `ninja-release` | Ninja を使用した Release ビルド |
| `ninja-debug` | `ninja-debug` | Ninja を使用した Debug ビルド |

使用例:

```powershell
# プリセット一覧を表示
cmake --list-presets

# Ninja Release ビルド
cmake --preset ninja
cmake --build --preset ninja-release

# Ninja Debug ビルド
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

### bootstrap-ninja.ps1 の動作

`scripts\bootstrap-ninja.ps1` は以下の動作を行います:

1. システムの PATH に `ninja` コマンドがあるか確認
2. ない場合は、GitHub Releases から Ninja バイナリをダウンロード
3. `.tools\bin\` ディレクトリに配置

**特徴:**

- システムに既に Ninja がある場合は何もしない（尊重する）
- 一度ダウンロードした Ninja は `.tools\bin\` に保持される
- システムの PATH は恒久的に変更しない

### setup-ninja-env.ps1 の動作

`scripts\setup-ninja-env.ps1` は以下の動作を行います:

1. `bootstrap-ninja.ps1` を呼び出してNinjaを確保
2. Visual Studio のインストール場所を自動検出（vswhere使用）
3. `vcvarsall.bat` を実行してMSVC環境を設定
4. CMake でプロジェクトを設定
5. ビルドを実行

**パラメータ:**

- `-Config <Debug|Release>`: ビルド構成（デフォルト: Debug）
- `-Clean`: ビルド前にbuildディレクトリをクリーンアップ

**使用例:**

```powershell
# Debugビルド（デフォルト）
.\scripts\setup-ninja-env.ps1

# Releaseビルド
.\scripts\setup-ninja-env.ps1 -Config Release

# クリーンビルド
.\scripts\setup-ninja-env.ps1 -Config Release -Clean
```

### build-with-ninja.ps1 の動作

`scripts\build-with-ninja.ps1` は `setup-ninja-env.ps1` のラッパーで、よりシンプルなインターフェースを提供します。

```powershell
# Debugビルド
.\scripts\build-with-ninja.ps1

# Releaseビルド
.\scripts\build-with-ninja.ps1 -Config Release
```

## Ninja バージョンの変更

デフォルトの Ninja バージョンは `v1.11.1` です。別のバージョンを使用するには、環境変数 `NINJA_VERSION` を設定します:

```powershell
$env:NINJA_VERSION = "v1.12.0"
.\scripts\bootstrap-ninja.ps1
```

新しいバージョンの Ninja をダウンロードする場合は、既存のバイナリを削除してからスクリプトを再実行してください:

```powershell
Remove-Item .\.tools\bin\ninja.exe
.\scripts\bootstrap-ninja.ps1
```

## セキュリティに関する注意

### スクリプト実行前の確認

- スクリプトは GitHub から直接バイナリをダウンロードします
- 実行前にスクリプトの内容を確認することを推奨します
- ダウンロード元: `https://github.com/ninja-build/ninja/releases`

### 検証方法

```powershell
# スクリプト内容の確認
Get-Content scripts\bootstrap-ninja.ps1

# 既存のNinjaを優先（すでにインストール済みの場合）
Get-Command ninja -ErrorAction SilentlyContinue
ninja --version
```

### セキュリティポリシーに関する注意

外部ダウンロードを許可しないセキュリティポリシーの環境では:

1. 手動で Ninja を公式サイトまたはパッケージマネージャからインストールする
2. システムの PATH に追加する
3. `bootstrap-ninja.ps1` は自動的にスキップされる

パッケージマネージャを使用したインストール:

```powershell
# Chocolatey
choco install ninja

# Scoop
scoop install ninja

# winget
winget install Ninja-build.Ninja
```

## トラブルシューティング

### CMake のバージョンが古い

CMake Presets を使用するには CMake 3.19 以上が必要です。古いバージョンの場合は:

```powershell
# 従来の方法でビルド
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### ビルドディレクトリをクリーンアップしたい

```powershell
Remove-Item -Recurse -Force build
cmake --preset ninja
cmake --build --preset ninja-release
```

## 既存のビルド方法との互換性

この変更は既存のビルドフローに影響を与えません:

```powershell
# 従来の方法（引き続き動作します）
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

## 参考リンク

- [Ninja 公式サイト](https://ninja-build.org/)
- [Ninja GitHub リポジトリ](https://github.com/ninja-build/ninja)
- [CMake Presets ドキュメント](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
