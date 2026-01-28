# Emscripten Web ビルド クイックスタート

## ⚠️ 重要な注意事項

**WebAssemblyファイルは、HTTPサーバー経由でのみ読み込めます。**

以下のように直接開くことはできません：
```
❌ file:///Z:/kaken/build_web/game/CatTDGame.html  （CORS エラー）
```

必ずHTTPサーバーを起動してアクセスしてください：
```
✅ http://localhost:8000/game/CatTDGame.html
```

---

## 問題の原因

Emscriptenビルドで以下の問題が発生していました：

1. **dataフォルダがプリロードされない**
   - CMakeLists.txt の `--preload-file` 設定が不適切
   - 絶対パスでの指定が必要

2. **メモリ設定が不足**
   - `TOTAL_MEMORY=512MB` が古い記法
   - `INITIAL_MEMORY=256MB` + `ALLOW_MEMORY_GROWTH` に変更

3. **CORSエラー**
   - `file://` プロトコルでは WebAssembly を読み込めない
   - HTTPサーバーが必須

## 修正内容

### 1. CMakeLists.txt の修正

```cmake
# 修正前（問題あり）
"--preload-file ${PROJECT_ROOT_DIR}/assets@/assets"

# 修正後（正常動作）
if(EXISTS "${PROJECT_ROOT_DIR}/assets")
    message(STATUS "Preloading assets from: ${PROJECT_ROOT_DIR}/assets")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "--preload-file ${PROJECT_ROOT_DIR}/assets@/assets")
endif()

if(EXISTS "${PROJECT_ROOT_DIR}/data")
    message(STATUS "Preloading data from: ${PROJECT_ROOT_DIR}/data")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "--preload-file ${PROJECT_ROOT_DIR}/data@/data")
endif()
```

### 2. メモリ設定の最適化

```cmake
"-s INITIAL_MEMORY=256MB"        # 初期メモリ（旧 TOTAL_MEMORY）
"-s ALLOW_MEMORY_GROWTH=1"       # 動的メモリ拡張
```

### 3. デバッグフラグの改善

```cmake
"-s ASSERTIONS=1"                # アサーション有効化
"-s STACK_OVERFLOW_CHECK=2"      # スタックオーバーフローチェック
"-gsource-map"                   # ソースマップ生成
```

---

## ビルド & 実行手順

### 🚀 ワンコマンド実行（推奨）

```cmd
# EMSDK環境をアクティベート（初回のみ）
call C:\emsdk\emsdk_env.bat

# ビルド + HTTPサーバー起動 + ブラウザ自動起動
.\tools\build_web.bat debug run
```

このコマンドは以下を自動実行します：
1. ビルド
2. HTTPサーバー起動（ポート 8000）
3. ブラウザで `http://localhost:8000/game/CatTDGame.html` を自動で開く

### 📋 手動実行手順

#### Step 1: ビルド

```cmd
# EMSDK環境をアクティベート（初回のみ）
call C:\emsdk\emsdk_env.bat

# ビルド実行
.\tools\build_web.bat debug
```

#### Step 2: HTTPサーバー起動

```cmd
cd build_web
python -m http.server 8000
```

#### Step 3: ブラウザで開く

ブラウザで以下のURLを開きます：

```
http://localhost:8000/game/CatTDGame.html
```

---

## ビルド成功の確認

ビルドが成功すると、以下のファイルが生成されます：

```
build_web/game/
├── CatTDGame.html      # メインHTMLファイル
├── CatTDGame.js        # JavaScript実行ファイル
├── CatTDGame.wasm      # WebAssemblyバイナリ
└── CatTDGame.data      # プリロードされたアセット（重要！）
```

**CatTDGame.data のサイズ確認**:

```cmd
dir build_web\game\CatTDGame.data
```

- サイズが 0 KB の場合、アセットがプリロードされていません
- 正常な場合、数十MB〜数百MBのサイズになります

---

## トラブルシューティング

### 🚫 問題1: CORS エラー

**エラーメッセージ**:
```
Access to XMLHttpRequest at 'file:///Z:/kaken/build_web/game/CatTDGame.wasm' 
from origin 'null' has been blocked by CORS policy
```

**原因**: 
- `file://` プロトコルで直接HTMLファイルを開いた
- WebAssemblyはHTTPサーバー経由でのみ読み込める

**解決策**:

```cmd
# HTTPサーバーを起動
cd build_web
python -m http.server 8000

# ブラウザで開く
http://localhost:8000/game/CatTDGame.html
```

### 🚫 問題2: CatTDGame.data が生成されない

**原因**: 
- `assets` または `data` フォルダが存在しない
- パスの指定が間違っている

**解決策**:

```cmd
# フォルダの存在確認
dir assets
dir data

# ビルドログで以下のメッセージを確認：
# [OK] Assets directory found: Z:\kaken\assets
# [OK] Data directory found: Z:\kaken\data
```

### 🚫 問題3: ポート 8000 が既に使用中

**エラーメッセージ**:
```
OSError: [WinError 10048] 通常、各ソケット アドレスに対してプロトコル、
ネットワーク アドレス、またはポートのどれか 1 つのみを使用できます。
```

**解決策**:

```cmd
# 別のポートを使用
python -m http.server 9000

# ブラウザで開く
http://localhost:9000/game/CatTDGame.html
```

### 🚫 問題4: Python がインストールされていない

**エラーメッセージ**:
```
'python' は、内部コマンドまたは外部コマンド、
操作可能なプログラムまたはバッチ ファイルとして認識されていません。
```

**解決策A: Python をインストール**

1. [Python公式サイト](https://www.python.org/downloads/)からダウンロード
2. インストール時に "Add Python to PATH" にチェック

**解決策B: 他のHTTPサーバーを使用**

#### Node.js がある場合

```cmd
npx http-server build_web -p 8000
```

#### VSCode がある場合

1. VSCode で `build_web/game/CatTDGame.html` を開く
2. 右クリック → "Open with Live Server"

### 🚫 問題5: ブラウザで起動しない（黒い画面）

**原因**: 
- アセットが読み込めない
- メモリ不足
- JavaScriptエラー

**解決策**:

1. ブラウザの開発者ツール（F12）を開く
2. Console タブでエラーを確認
3. Network タブで `CatTDGame.data` が読み込まれているか確認

**よくあるエラー**:

```javascript
// アセット読み込み失敗
Uncaught Error: ENOENT: no such file or directory, open 'assets/...'
→ CatTDGame.data が正しくプリロードされていません

// メモリ不足
RuntimeError: memory access out of bounds
→ CMakeLists.txt で INITIAL_MEMORY を増やしてください
```

### 🚫 問題6: メモリ不足エラー

**解決策**:

CMakeLists.txt で `INITIAL_MEMORY` を増やす：

```cmake
"-s INITIAL_MEMORY=512MB"  # または1024MB
```

その後、再ビルド：

```cmd
.\tools\build_web.bat debug
```

---

## 代替HTTPサーバー

### Python がない場合の代替手段

#### 1. Node.js の http-server

```cmd
# インストール（初回のみ）
npm install -g http-server

# 起動
cd build_web
http-server -p 8000
```

#### 2. PHP の組み込みサーバー

```cmd
cd build_web
php -S localhost:8000
```

#### 3. VSCode Live Server

1. VSCode 拡張機能 "Live Server" をインストール
2. `build_web/game/CatTDGame.html` を開く
3. 右クリック → "Open with Live Server"

---

## デプロイ方法

### 静的ホスティングサービスにデプロイ

以下のファイルをアップロード：

```
build_web/game/
├── CatTDGame.html
├── CatTDGame.js
├── CatTDGame.wasm
└── CatTDGame.data
```

**推奨ホスティングサービス**:
- GitHub Pages
- Netlify
- Vercel
- Firebase Hosting

### GitHub Pages へのデプロイ例

```cmd
# build_web/game/ フォルダを gh-pages ブランチにコミット
cd build_web/game
git init
git add .
git commit -m "Deploy to GitHub Pages"
git branch -M gh-pages
git remote add origin https://github.com/yourusername/yourrepo.git
git push -u origin gh-pages --force
```

GitHub の設定で "Pages" → "Source" を "gh-pages" ブランチに設定

アクセス: `https://yourusername.github.io/yourrepo/CatTDGame.html`

---

## まとめ

### ✅ 正しい実行方法

```cmd
# 1. ビルド
call C:\emsdk\emsdk_env.bat
.\tools\build_web.bat debug run

# 2. ブラウザで開く（自動）
# http://localhost:8000/game/CatTDGame.html
```

### ❌ 間違った実行方法

```cmd
# ❌ 直接HTMLファイルを開く（CORS エラー）
start build_web\game\CatTDGame.html

# ❌ file:// プロトコル（WebAssembly 読み込めない）
file:///Z:/kaken/build_web/game/CatTDGame.html
```

---

**最終更新**: 2025-01-12  
**バージョン**: 1.2.0
