# Roguelikeゲーム起動時クラッシュレポート

**日時**: 2025年12月4日  
**ビルド構成**: Release  
**症状**: Roguelikeゲーム選択後にクラッシュ

---

## 📊 エラーサマリー

| 項目 | 値 |
|------|-----|
| **エラー種類** | ディレクトリ不在エラー |
| **影響範囲** | Roguelikeゲームシーン |
| **重大度** | 🔴 Critical（ゲーム起動不可） |
| **修正優先度** | ⚠️ 最高 |

---

## 🔴 エラー詳細

### エラーメッセージ

```
DataLoader Error [assets/definitions/maps]: Directory does not exist
```

### 発生タイミング

1. ✅ ゲーム起動（HomeScene表示）
2. ✅ Roguelikeゲーム選択
3. ❌ **クラッシュ発生** - `assets/definitions/maps` ディレクトリが存在しない

### エラー発生箇所

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**関数**: `Initialize()` → `Render()`  
**行**: 54-56（初期化）、300-340（レンダリング）

**問題のコード**:

```cpp
// Line 53-56: Initialize()
auto mapData = std::make_unique<Domain::Roguelike::Components::MapData>();
mapData->Initialize(80, 25);
g_mapData = mapData.get();  // ❌ ダングリングポインタ！
// ← mapDataがスコープを抜けると破棄される

// Line 295-296: Render()
if (!g_mapData) return;  // g_mapDataは既に無効
const auto& tiles = g_mapData->GetTiles();  // ❌ アクセス違反発生
```

**呼び出しチェーン**:

```
RoguelikeGameScene::Initialize()
  → std::make_unique<MapData>() 作成（ローカル変数）
  → g_mapData = mapData.get() （生ポインタを保存）
  → 関数終了 → mapData破棄 ❌
    → g_mapDataがダングリングポインタに

RoguelikeGameScene::Render()
  → g_mapData->GetTiles() 呼び出し
    → アクセス違反 (0xC0000005)
```

---

## 🔍 根本原因分析

### 主要な問題

1. **オブジェクトライフタイム管理の誤り** 🔴 重大
   - `std::unique_ptr<MapData>` をローカル変数として作成
   - 生ポインタ `g_mapData` に代入
   - 関数終了時に `unique_ptr` が破棄される → **ダングリングポインタ**
   - `Render()` で破棄済みメモリにアクセス → アクセス違反

2. **メモリ管理の設計ミス**
   - `GameContext::Register()` は参照を登録するだけでオブジェクトを保持しない
   - `MapData` のオーナーシップが不明確
   - グローバルポインタ `g_mapData` への依存

3. **C++ オブジェクト所有権の理解不足**

   ```cpp
   // 問題のパターン
   auto mapData = std::make_unique<MapData>();  // ローカル所有

## 💡 修正方針

### Option A: メンバー変数として保持（推奨）⭐

`RoguelikeGameScene` クラスに `MapData` を `std::unique_ptr` メンバーとして追加

**変更ファイル**: `include/Application/RoguelikeGameScene.h`

```cpp
class RoguelikeGameScene : public IScene {
private:
    std::string nextScene_;
    bool initialized_;
    int turnCount_;
    
    // ✅ 追加: MapDataをメンバーとして保持
    std::unique_ptr<Domain::Roguelike::Components::MapData> mapData_;
};
```

**変更ファイル**: `src/Application/RoguelikeGameScene.cpp`
---

### Option B: GameContextで管理（設計改善）

`GameContext` に `MapData` を `std::unique_ptr` として追加

**変更ファイル**: `include/Core/GameContext.h`

```cpp
class GameContext {
private:
    // 既存のメンバー...
    
    // ✅ 追加: Roguelike用MapData
    std::unique_ptr<Domain::Roguelike::Components::MapData> roguelikeMapData_;
    
public:
    // ✅ アクセサ追加
    Domain::Roguelike::Components::MapData* GetRoguelikeMapData() {
        return roguelikeMapData_.get();
    }
    
    void SetRoguelikeMapData(std::unique_ptr<Domain::Roguelike::Components::MapData> mapData) {
        roguelikeMapData_ = std::move(mapData);
    }
};
```

---

### Option C: グローバルポインタ削除（抜本的改善）

グローバル変数 `g_mapData` を削除し、すべて `World` または `GameContext` 経由でアクセス

**メリット**:

- グローバル変数の削除
- テストしやすい設計
- マルチスレッド対応

**デメリット**:

- 大規模なリファクタリング
- すべてのシステムの変更が必要ァイルの変更が必要
- 最小限の変更
- 即座に解決

**デメリット**:

- 根本的な解決ではない

---

### Option B: エラーハンドリング改善（中期対応）

**変更ファイル**: `src/Application/UnifiedGame.cpp`

```cpp
// 修正前
definitionLoader_->LoadAllMaps(definitionsPath + "/maps");

// 修正後
if (std::filesystem::exists(definitionsPath + "/maps")) {
    definitionLoader_->LoadAllMaps(definitionsPath + "/maps");
} else {
    std::cout << "Maps directory not found, skipping map definitions\n";
}
```

**メリット**:

- 柔軟なエラーハンドリング
- ディレクトリが無くても動作

**デメリット**:

- 他の定義ローダーにも同様の対応が必要

---

### Option C: オプショナル読み込み（長期対応）

**変更ファイル**: `include/Data/Loaders/DataLoaderBase.h`

```cpp
/**
 * @brief ディレクトリ内のファイルを読み込み（オプショナル）
 * @param optional trueの場合、ディレクトリが存在しなくてもエラーにしない
 */
int LoadDirectory(const std::string& directoryPath, 
                  const std::string& extension,
                  std::function<bool(const std::string&)> loadFunc,
                  bool optional = false) {
    if (!std::filesystem::exists(directoryPath)) {
        if (optional) {
            return 0; // エラーにせず0を返す
        }
        errorHandler_(directoryPath, "Directory does not exist");
        return -1;
    }
    // ...既存のロジック
}
```

**UnifiedGame.cpp** での使用:

```cpp
// オプショナル読み込み（ディレクトリが無くてもOK）
definitionLoader_->LoadAllMaps(definitionsPath + "/maps", true);
```

**メリット**:

- 設計的に正しい
- 再利用可能

**デメリット**:

- 複数ファイルの変更が必要

## 🚀 即座に実行すべき対応（Option A）

### 1. RoguelikeGameScene.h 修正 ⚠️ 優先度: 最高

**ファイル**: `include/Application/RoguelikeGameScene.h`

```cpp
// クラス定義内にメンバー変数を追加
private:
    std::string nextScene_;
    bool initialized_;
    int turnCount_;
    
    // ✅ 追加
    std::unique_ptr<Domain::Roguelike::Components::MapData> mapData_;
```

### 2. RoguelikeGameScene.cpp 修正 ⚠️ 優先度: 最高

## 📁 影響を受けるファイル

### 修正が必要なファイル（Option A - 推奨）⭐

**変更**:

1. `include/Application/RoguelikeGameScene.h` ⚠️ 優先度: 最高
   - メンバー変数 `mapData_` 追加

2. `src/Application/RoguelikeGameScene.cpp` ⚠️ 優先度: 最高
   - Line 53-56: `Initialize()` 内のMapData作成を修正
   - Line 370-375: `Shutdown()` 内のクリーンアップを修正

### 修正が必要なファイル（Option B）

**変更**:

1. `include/Core/GameContext.h` - MapData管理メソッド追加
2. `src/Application/RoguelikeGameScene.cpp` - GameContext経由でアクセス

### 修正が必要なファイル（Option C）

**変更**:

1. `include/Domain/Roguelike/Systems/*.h` - すべてのシステム

## 📝 検証手順

### 修正後の確認

1. **ビルド**

   ```bash
   cmake --build build --config Release
   ```

2. **実行テスト**

   ```bash
   .\build\bin\Release\SimpleTDCGame.exe
   ```

3. **動作確認**
   - ホーム画面で "2" キーを押下してRoguelike選択
   - ダンジョンが表示されることを確認
   - プレイヤー（白い円）が表示されることを確認
   - 敵（赤い円）が表示されることを確認
   - **クラッシュしないことを確認** ⚠️ 重要

4. **ゲームプレイテスト**
   - 矢印キーで移動できることを確認
   - ターン制で動作することを確認
   - ESCキーでホーム画面に戻れることを確認

5. **ログ確認**
   - アクセス違反が発生しないことを確認
   - 終了コードが0であることを確認

**変更**:

- `src/Application/UnifiedGame.cpp` (Line 117)

### 修正が必要なファイル（Option C）

**変更**:

1. `include/Data/Loaders/DataLoaderBase.h` - `LoadDirectory` にオプショナルパラメータ追加
2. `include/Data/Loaders/MapLoader.h` - `LoadAllMaps` にオプショナルパラメータ追加
3. `include/Core/DefinitionLoader.h` - `LoadAllMaps` にオプショナルパラメータ追加
4. `src/Application/UnifiedGame.cpp` - オプショナルフラグ指定

---

## 📝 検証手順

### 修正後の確認

1. **ビルド**

   ```bash
   cmake --build build --config Release
   ```

## 🔧 追加の推奨事項

### 1. 他のグローバルポインタも確認

**ファイル**: `src/Application/RoguelikeGameScene.cpp`

現在のグローバル変数:

```cpp
static Domain::Roguelike::Managers::TurnManager *g_turnManager = nullptr;  // ✅ GameContext管理
static Domain::Roguelike::Components::MapData *g_mapData = nullptr;       // ❌ 修正対象
static entt::entity g_playerEntity = entt::null;                          // ⚠️ 要検討
```

**推奨**:

- `g_turnManager`: GameContextで管理されているのでOK
- `g_mapData`: **修正必須**（今回対応）
- `g_playerEntity`: メンバー変数化を検討

### 2. 同様のパターンを探す

プロジェクト全体で同様の問題がないか確認:

```bash
# unique_ptrの生ポインタ取得パターンを検索
grep -r "\.get()" --include="*.cpp" | grep "unique_ptr"
```

**特に注意**:

- ローカルの `std::unique_ptr` から `.get()` で生ポインタを取得
- その生ポインタをグローバル変数やメンバー変数に保存

### 3. コードレビューチェックリスト

**メモリ管理**:

- [ ] `unique_ptr` のライフタイムが適切か
- [ ] 生ポインタの参照元オブジェクトが有効か
- [ ] RAII パターンに従っているか

**グローバル変数**:

- [ ] グローバル変数の必要性を確認

## 📚 関連情報

### 関連ファイル

- `src/Application/RoguelikeGameScene.cpp` - クラッシュ発生箇所
- `include/Application/RoguelikeGameScene.h` - シーン定義
- `include/Domain/Roguelike/Components/RoguelikeComponents.h` - MapData定義
- `include/Core/GameContext.h` - ゲームコンテキスト

### 関連するC++概念

- **RAII** (Resource Acquisition Is Initialization)
- **オブジェクトライフタイム管理**
- **スマートポインタ** (`std::unique_ptr`, `std::shared_ptr`)
- **ダングリングポインタ** (Dangling Pointer)

### 参考ドキュメント

## 🎯 次のアクション

### 即座に実行（10分）⚠️ 最優先

1. **RoguelikeGameScene.h 修正**

   ```cpp
   // include/Application/RoguelikeGameScene.h のprivateセクションに追加
   std::unique_ptr<Domain::Roguelike::Components::MapData> mapData_;
   ```

2. **RoguelikeGameScene.cpp 修正**

   ```cpp
   // Line 53-56: Initialize()内
   mapData_ = std::make_unique<Domain::Roguelike::Components::MapData>();
   mapData_->Initialize(80, 25);
   g_mapData = mapData_.get();
   
   // Line 370: Shutdown()内
   mapData_.reset();
   g_mapData = nullptr;
   ```

3. **再ビルド**

   ```powershell
   cmake --build build --config Release
   ```

4. **動作確認**
   - Roguelikeゲーム起動

---

## 🎓 学習ポイント

### C++ メモリ管理の教訓

1. **スマートポインタのライフタイム**

   ```cpp
   // ❌ 危険: ローカルのunique_ptrから生ポインタを保存
   void BadExample() {
       auto data = std::make_unique<Data>();
       g_globalPtr = data.get();  // ← 関数終了後に無効化
   }
   
   // ✅ 安全: メンバー変数として保持
   class GoodExample {
       std::unique_ptr<Data> data_;
   public:
       void Initialize() {
           data_ = std::make_unique<Data>();
           g_globalPtr = data_.get();  // ← オブジェクト存続中は有効
       }
   };
   ```

2. **RAII原則を守る**
   - リソースの取得は初期化時
   - リソースの解放は破棄時
   - スコープを抜けたら自動解放

3. **生ポインタの使用は慎重に**
   - 所有権がないことを明確に
   - ライフタイムを保証できる場合のみ

---

**レポート作成日**: 2025年12月4日（更新版）  
**ステータス**: 🔴 Critical - アクセス違反によるクラッシュ  
**推奨修正方法**: Option A（メンバー変数化） → Option B（GameContext管理）  
**根本原因**: ダングリングポインタ（unique_ptrライフタイム管理ミス）
5. **Option B検討** - GameContext管理への移行
6. **g_playerEntity 対応** - メンバー変数化
7. **他のグローバルポインタ確認** - 同様の問題がないか

### 長期対応（Phase 6）

8. **Option C実装** - グローバル変数削除
9. **コードレビュー** - メモリ管理パターンの統一
10. **ドキュメント更新** - C++メモリ管理ガイドライン作成

### 即座に実行（5分）

1. ✅ **maps ディレクトリ作成**

   ```powershell
   New-Item -ItemType Directory -Path "assets/definitions/maps" -Force
   New-Item -ItemType File -Path "assets/definitions/maps/.gitkeep" -Force
   ```

2. ✅ **再ビルド**

   ```powershell
   cmake --build build --config Release
   ```

3. ✅ **動作確認**
   - Roguelikeゲーム起動
   - クラッシュしないことを確認

### 短期対応（30分）

4. **Option B実装** - エラーハンドリング改善
5. **他ディレクトリ確認** - 全定義ディレクトリに `.gitkeep` 追加

### 長期対応（Phase 6）

6. **Option C実装** - オプショナル読み込み設計
7. **CMakeLists.txt更新** - ディレクトリ自動生成
8. **ドキュメント更新** - ディレクトリ構造記載

---

**レポート作成日**: 2025年12月4日  
**ステータス**: 🔴 Critical - 即座の対応が必要  
**推奨修正方法**: Option A（ディレクトリ作成）→ Option B（エラーハンドリング）
