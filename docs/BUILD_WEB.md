# Emscripten (Web) ビルド手順

このドキュメントでは、Simple-TDC-GameProjectをWebAssembly(Emscripten)でビルドする方法を説明します。

## 前提条件

### 必須ツール

- **Emscripten SDK**: 最新バージョン（3.1.50以上推奨）
- **CMake**: 3.19以上
- **Ninja**: ビルドシステム
- **Python 3**: ローカルサーバー用（テスト実行時）

### Emscripten SDK のインストール

```bash
# Windows (PowerShell)
git clone https://github.com/emscripten-core/emsdk.git C:\emsdk
cd C:\emsdk
.\emsdk install latest
.\emsdk activate latest
.\emsdk_env.bat
```

**注意**: Emscripten SDK は毎回アクティベートが必要です。  
新しいターミナルを開くたびに `emsdk_env.bat` を実行してください。

---

## ビルド方法

### 簡易ビルドスクリプト

プロジェクトルートで以下のコマンドを実行：

```bash
# Releaseビルド
tools\build_web.bat release

# Debugビルド
tools\build_web.bat debug

# ビルド後にローカルサーバーで実行
tools\build_web.bat release run
```

### 手動ビルド（詳細制御が必要な場合）

```bash
# 1. Emscripten環境をアクティベート
call C:\emsdk\emsdk_env.bat

# 2. ビルドディレクトリ作成
mkdir build_web
cd build_web

# 3. CMake 構成
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake ^
    -DPLATFORM=Web ^
    ..

# 4. ビルド実行
cmake --build . --config Release

# 5. ローカルサーバー起動
python -m http.server 8000
```

ブラウザで `http://localhost:8000/game/CatTDGame.html` を開く

---

## ビルド設定

### メモリ設定

デフォルト設定（CMakeLists.txt）:

```cmake
"-s INITIAL_MEMORY=256MB"       # 初期メモリサイズ
"-s ALLOW_MEMORY_GROWTH=1"      # 動的メモリ拡張を有効化
```

大量のアセットを読み込む場合は、`INITIAL_MEMORY`を増やしてください：

```cmake
"-s INITIAL_MEMORY=512MB"       # 512MBに増量
```

### デバッグビルド

デバッグビルド時は以下のフラグが有効化されます：

```cmake
"-s ASSERTIONS=1"               # アサーション有効化
"-s STACK_OVERFLOW_CHECK=2"     # スタックオーバーフローチェック
"-gsource-map"                  # ソースマップ生成
"--source-map-base http://localhost:8000/"  # ソースマップのベースURL
```

### アセットのプリロード

`assets` と `data` フォルダは自動的にプリロードされます：

```cmake
"--preload-file ${PROJECT_ROOT}/assets@/assets"
"--preload-file ${PROJECT_ROOT}/data@/data"
```

---

## 出力ファイル

ビルド成功後、以下のファイルが生成されます：

```
build_web/game/
├── CatTDGame.html      # メインHTMLファイル
├── CatTDGame.js        # JavaScript実行ファイル
├── CatTDGame.wasm      # WebAssemblyバイナリ
└── CatTDGame.data      # プリロードされたアセット
```

---

## トラブルシューティング

### 問題1: CatTDGame.data が生成されない

**原因**: `assets` または `data` フォルダが存在しない

**解決策**:
1. プロジェクトルートに `assets` と `data` フォルダが存在するか確認
2. ビルドログで以下のメッセージを確認：
   ```
   [OK] Assets directory found: Z:\kaken\assets
   [OK] Data directory found: Z:\kaken\data
   ```

### 問題2: ブラウザでアセットが読み込めない

**原因**: ファイルシステムがマウントされていない

**解決策**:
1. ブラウザの開発者ツール（F12）でエラーを確認
2. 以下のようなエラーが出ている場合、`CatTDGame.data` が正しくプリロードされていません：
   ```
   Uncaught Error: ENOENT: no such file or directory, open 'assets/...'
   ```

3. CMakeLists.txt で `--preload-file` の設定を確認：
   ```cmake
   "--preload-file ${PROJECT_ROOT_DIR}/assets@/assets"
   "--preload-file ${PROJECT_ROOT_DIR}/data@/data"
   ```

### 問題3: メモリ不足エラー

**原因**: 初期メモリサイズが小さすぎる

**解決策**:
CMakeLists.txt で `INITIAL_MEMORY` を増やす：

```cmake
"-s INITIAL_MEMORY=512MB"  # または1024MB
```

### 問題4: ゲームが起動しない（黒い画面のまま）

**原因**: 以下のいずれか
- アセットの読み込み失敗
- JavaScriptエラー
- ログシステムの問題（Emscriptenでは無効化済み）

**解決策**:
1. ブラウザの開発者ツール（F12）を開く
2. Console タブでエラーを確認
3. Network タブで `CatTDGame.data` が正しく読み込まれているか確認

### 問題5: 音声が再生されない

**原因**: ブラウザのオートプレイポリシー

**解決策**:
1. ユーザー操作（クリックなど）後に音声を再生するように実装
2. Raylib の `InitAudioDevice()` が Emscripten で正しく動作するか確認

---

## パフォーマンス最適化

### Releaseビルド最適化

Releaseビルドでは以下の最適化が有効化されます：

```cmake
"-O3"              # 最高レベルの最適化
"--closure 1"      # Google Closure Compilerによる圧縮
```

### ファイルサイズ削減

1. **不要なアセットを削除**:
   - `assets` フォルダから使用していないファイルを削除

2. **テクスチャ圧縮**:
   - PNG画像をWebP形式に変換（Raylibは対応済み）

3. **音声圧縮**:
   - WAV音声をOGG形式に変換

---

## デプロイ

### 静的ホスティング

以下のファイルをWebサーバーにアップロード：

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

### MIMEタイプ設定

Webサーバーで以下のMIMEタイプを設定：

```
.wasm   -> application/wasm
.js     -> text/javascript
.data   -> application/octet-stream
```

---

## リソース

- [Emscripten 公式ドキュメント](https://emscripten.org/docs/)
- [Raylib Emscripten ポーティング](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5))
- [WebAssembly 公式サイト](https://webassembly.org/)

---

**最終更新**: 2025-01-12  
**バージョン**: 1.1.0
