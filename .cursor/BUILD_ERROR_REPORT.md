# ビルドエラーレポート

**日時**: 2025年12月4日  
**ビルド構成**: Release  
**コンパイラ**: MSBuild 17.14.23 (MSVC)

---

## 📊 エラーサマリー

| 項目 | 値 |
|------|-----|
| **エラー数** | 10 |
| **警告数** | 28 |
| **影響ファイル** | `RoguelikeGameScene.cpp` |
| **ビルド結果** | ❌ 失敗 |

---

## 🔴 コンパイルエラー詳細

### エラー1-2: `Stats` コンポーネントが見つからない

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 135, 168

```
error C2039: 'Stats': 'Domain::Roguelike::Components' のメンバーではありません
error C2065: 'Stats': 定義されていない識別子です
```

**問題**:

- `Domain::Roguelike::Components::Stats` が存在しない
- `RoguelikeGameScene.cpp` で `Stats` コンポーネントを使用しようとしている

**該当コード** (推測):

```cpp
// Line 135
world_.Emplace<Domain::Roguelike::Components::Stats>(player, ...);

// Line 168
world_.Emplace<Domain::Roguelike::Components::Stats>(enemy, ...);
```

**原因**:

- Roguelike用の`Stats`コンポーネントが未定義
- TD用の`Stats`が`Domain::TD::Components::Stats`として存在している可能性

---

### エラー3-4: `World::Emplace` テンプレート引数無効

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 135, 168

```
error C2672: 'Core::World::Emplace': 一致するオーバーロードされた関数が見つかりませんでした
'Core::World::Emplace': テンプレート引数の'T' に対して無効です。型が必要です
```

**問題**:

- `Stats`が未定義のため、`Emplace<Stats>`の型推論が失敗

---

### エラー5: entt::entity に対する `!` 演算子の誤用

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 189

```
error C2675: 単項演算子'!': 'entt::entity' は、この演算子または定義済の演算子に適合する型への変換の定義を行いません
```

**該当コード** (推測):

```cpp
if (!player) {  // ❌ 誤り
    // ...
}
```

**正しい書き方**:

```cpp
if (player == entt::null) {  // ✅ 正しい
    // ...
}
```

**原因**:

- EnTT v3.12では `entt::entity` に対する `operator!` が削除された
- `entt::null` との比較を使用する必要がある

---

### エラー6-7: `World::TryGet` テンプレート引数無効

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 194

```
error C2672: 'Core::World::TryGet': 一致するオーバーロードされた関数が見つかりませんでした
'Core::World::TryGet': テンプレート引数の'T' に対して無効です。型が必要です
```

**該当コード** (推測):

```cpp
auto* playerStats = world_.TryGet<Stats>(player);
```

**原因**:

- `Stats`が未定義のため型推論失敗

---

### エラー8: 初期化前の変数使用

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 195

```
error C3536: 'playerStats': 初期化前に使用することはできません
```

**原因**:

- 上記エラー6-7により`playerStats`の型推論が失敗したため

---

### エラー9-10: `entt::view::try_get` メソッドが存在しない

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 333

```
error C2039: 'try_get': 'entt::basic_view<...>' のメンバーではありません
```

**該当コード** (推測):

```cpp
for (auto entity : view) {
    auto* actor = view.try_get(entity);  // ❌ 誤り
    // ...
}
```

**正しい書き方**:

```cpp
for (auto entity : view) {
    auto* actor = world_.TryGet<ComponentType>(entity);  // ✅ 正しい
    // または
    auto [component] = view.get(entity);
}
```

**原因**:

- EnTT v3.12では `view::try_get` が削除された
- `World::TryGet` または `view::get` を使用する必要がある

---

### エラー11: 初期化前の変数使用

**ファイル**: `src/Application/RoguelikeGameScene.cpp`  
**行**: 334

```
error C3536: 'actor': 初期化前に使用することはできません
```

**原因**:

- 上記エラー9-10により`actor`の型推論が失敗したため

---

## ⚠️ 警告一覧（抜粋）

### 未使用パラメータ警告

**数**: 20件以上

```
warning C4100: 'context': 参照されないパラメーター
warning C4100: 'world': 参照されないパラメーター
warning C4100: 'monster': 参照されないパラメーター
warning C4100: 'registry': 参照されないパラメーター
```

**影響**:

- コンパイルには成功するが、コードの意図が不明確

**推奨対応**:

```cpp
// 使用しないパラメータには [[maybe_unused]] を付ける
void Update([[maybe_unused]] Core::World& world, 
            [[maybe_unused]] Core::GameContext& context, 
            float deltaTime) {
    // ...
}

// または引数名を省略
void Update(Core::World&, Core::GameContext&, float deltaTime) {
    // ...
}
```

---

### 型変換警告

```
warning C4244: '引数': 'int' から 'float' への変換です。データが失われる可能性があります
warning C4267: 'return': 'size_t' から 'int' に変換しました。データが失われているかもしれません
```

**推奨対応**:

```cpp
// 明示的なキャスト
float value = static_cast<float>(intValue);
int size = static_cast<int>(container.size());
```

---

### 未使用ローカル変数警告

```
warning C4189: 'prevLevel': ローカル変数が初期化されましたが、参照されていません
warning C4189: 'stats': ローカル変数が初期化されましたが、参照されていません
```

**推奨対応**:

- 未使用の変数を削除
- または `[[maybe_unused]]` を付与

---

## 🔍 根本原因分析

### 主要な問題

1. **Roguelike用`Stats`コンポーネントの欠如**
   - TD用の`Stats`は存在するが、Roguelike専用の`Stats`が未定義
   - `Domain::Roguelike::Components::Stats` を作成する必要がある

2. **EnTT v3.12 APIの変更への未対応**
   - `operator!` の削除 → `entt::null`との比較を使用
   - `view::try_get` の削除 → `World::TryGet`を使用

3. **コンポーネント名前空間の混乱**
   - TD用とRoguelike用のコンポーネントが分離されている
   - 共通コンポーネント（Stats等）の設計が不明確

---

## 💡 修正方針

### Phase 1: Statsコンポーネントの定義

**新規ファイル**: `include/Domain/Roguelike/Components/StatsComponents.h`

```cpp
#pragma once

namespace Domain::Roguelike::Components {

/**
 * @brief キャラクターステータス
 */
struct Stats {
    int maxHp = 10;
    int currentHp = 10;
    int attack = 1;
    int defense = 0;
    int level = 1;
    int experience = 0;
};

} // namespace Domain::Roguelike::Components
```

**または共通化**:

`include/Game/Components/StatsComponents.h` を作成し、TD/Roguelike両方で使用:

```cpp
#pragma once

namespace Game::Components {

struct Stats {
    int maxHp = 10;
    int currentHp = 10;
    int attack = 1;
    int defense = 0;
    
    // TD専用フィールド（オプション）
    int range = 1;
    float attackSpeed = 1.0f;
    
    // Roguelike専用フィールド（オプション）
    int level = 1;
    int experience = 0;
};

} // namespace Game::Components
```

---

### Phase 2: EnTT API修正

**`RoguelikeGameScene.cpp` (Line 189)**:

```cpp
// 修正前
if (!player) {

// 修正後
if (player == entt::null) {
```

**`RoguelikeGameScene.cpp` (Line 333-334)**:

```cpp
// 修正前
for (auto entity : view) {
    auto* actor = view.try_get(entity);
    if (!actor) continue;
    // ...
}

// 修正後
for (auto entity : view) {
    auto* actor = world_.TryGet<Domain::Roguelike::Components::GridPosition>(entity);
    if (actor == nullptr) continue;
    // ...
}

// または
for (auto [entity, actor] : view.each()) {
    // actor は参照として直接取得される
    // ...
}
```

---

### Phase 3: 警告の修正（オプション）

未使用パラメータに `[[maybe_unused]]` を追加:

```cpp
void OnEnter([[maybe_unused]] Core::World& world, 
             [[maybe_unused]] Core::GameContext& context) override {
    // ...
}
```

---

## 📁 影響を受けるファイル

### 修正が必要なファイル

1. **`src/Application/RoguelikeGameScene.cpp`** ⚠️ 優先度: 高
   - Line 135, 168: `Stats`コンポーネントの参照を修正
   - Line 189: `!player` → `player == entt::null`
   - Line 194: `TryGet<Stats>` の型を修正
   - Line 333-334: `view.try_get` → `world_.TryGet` または `view.each()`

2. **`include/Domain/Roguelike/Components/StatsComponents.h`** ⚠️ 優先度: 高
   - 新規作成が必要

### 関連ファイル（参照用）

- `include/Core/World.h` - `Emplace`, `TryGet` の定義
- `include/Domain/TD/Components/StatsComponents.h` - TD用Stats（参考）
- `include/Domain/Roguelike/Components/` - Roguelikeコンポーネント群

---

## 🚀 次のアクション

### 即座に実行すべき対応

1. **Statsコンポーネント作成** (5分)

   ```
   Cursor Composer: include/Domain/Roguelike/Components/StatsComponents.h を作成
   ```

2. **EnTT API修正** (10分)

   ```
   Cursor Composer: RoguelikeGameScene.cpp の EnTT v3.12 API に対応
   - operator! → entt::null 比較
   - view.try_get → world_.TryGet または view.each()
   ```

3. **再ビルド** (2分)

   ```
   cmake --build build --config Release
   ```

### 後続タスク（オプション）

4. **警告の修正** (15分)
   - 未使用パラメータに `[[maybe_unused]]` 追加
   - 型変換を明示的キャストに変更

5. **共通コンポーネントの設計見直し** (Phase 4)
   - TD/Roguelike共通の`Stats`を`Game::Components`に統一

---

## 📚 参考リンク

- [EnTT v3.12 Migration Guide](https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#views)
- [.cursor/UNIFIED_PLATFORM_TODO.md](UNIFIED_PLATFORM_TODO.md) - Phase 2, Phase 3
- [EnTT API Documentation](https://skypjack.github.io/entt/)

---

## 📝 備考

### ビルド環境

- **CMake**: 3.19+
- **コンパイラ**: MSVC 19.44.35207
- **EnTT**: v3.12.2
- **Raylib**: 5.0

### 文字化けについて

出力に一部文字化けがありますが、エラーメッセージは以下の通りです：

- `蜿ら・縺輔ｌ縺ｪ縺` = "参照されない"
- `繝｡繝ｳ繝舌・縺ｧ縺ｯ縺ゅｊ縺ｾ縺帙ｓ` = "メンバーではありません"
- `螳夂ｾｩ縺輔ｌ縺ｦ縺・↑縺` = "定義されていない"

---

**レポート作成日**: 2025年12月4日  
**ステータス**: 🔴 ビルド失敗 - 修正が必要
