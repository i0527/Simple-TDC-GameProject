# EnTT 関数リファレンス チートシート

EnTT (Entity Component System) の主要なAPIをまとめたクイックリファレンスです。

## 目次

- [基本設定](#基本設定)
- [Registry (レジストリ)](#registry-レジストリ)
  - [エンティティの作成・削除](#エンティティの作成削除)
  - [コンポーネント操作](#コンポーネント操作)
  - [コンポーネントの取得](#コンポーネントの取得)
  - [コンポーネントの存在確認](#コンポーネントの存在確認)
  - [ストレージ操作](#ストレージ操作)
  - [コンテキスト変数](#コンテキスト変数)
- [View (ビュー)](#view-ビュー)
  - [ビューの作成](#ビューの作成)
  - [ビューのイテレーション](#ビューのイテレーション)
  - [ビューの操作](#ビューの操作)
- [Group (グループ)](#group-グループ)
- [Observer (オブザーバー)](#observer-オブザーバー)
- [Signal/Delegate](#signaldelegate)
- [Resource (リソース管理)](#resource-リソース管理)
- [Meta (リフレクション)](#meta-リフレクション)
- [Core Utilities](#core-utilities)
- [よく使うパターン](#よく使うパターン)

---

## 基本設定

```cpp
#include <entt/entt.hpp>

// レジストリの作成
entt::registry registry;

// エンティティ型の定義
using entity_type = entt::entity;
```

---

## Registry (レジストリ)

### エンティティの作成・削除

```cpp
// エンティティの作成
auto entity = registry.create();
auto entity2 = registry.create(entity);  // ヒント付き

// 複数エンティティの作成
std::vector<entt::entity> entities(10);
registry.create(entities.begin(), entities.end());

// エンティティの削除（全コンポーネントも削除）
registry.destroy(entity);
registry.destroy(entity, version);  // バージョン指定

// 範囲削除
auto view = registry.view<Component>();
registry.destroy(view.begin(), view.end());

// エンティティの解放（孤立エンティティ用）
registry.release(entity);
registry.release(entity, version);
registry.release(view.begin(), view.end());

// エンティティの有効性確認
bool valid = registry.valid(entity);
size_type count = registry.alive();  // 生存エンティティ数
```

### コンポーネント操作

```cpp
// コンポーネントの追加（構築）
registry.emplace<Position>(entity, 10.0f, 20.0f);
registry.emplace<Velocity>(entity);

// 複数エンティティに同じコンポーネントを追加
std::vector<entt::entity> entities = {...};
registry.insert<Position>(entities.begin(), entities.end(), Position{0, 0});

// コンポーネントの削除
registry.remove<Position>(entity);
registry.remove<Position>(entities.begin(), entities.end());

// コンポーネントの削除（破棄）
registry.erase<Position>(entity);
registry.erase<Position>(entities.begin(), entities.end());

// 条件付き削除
registry.erase_if<Position>(entity, [](const auto &pos) {
    return pos.x < 0;
});

// コンポーネントの置き換え
registry.replace<Position>(entity, 30.0f, 40.0f);
registry.patch<Position>(entity, [](auto &pos) {
    pos.x += 10;
});
```

### コンポーネントの取得

```cpp
// コンポーネントの取得（参照）
auto &pos = registry.get<Position>(entity);
const auto &pos = registry.get<const Position>(entity);

// コンポーネントの取得（ポインタ、存在しない場合はnullptr）
auto *pos = registry.try_get<Position>(entity);
const auto *pos = registry.try_get<const Position>(entity);

// 複数コンポーネントの取得
auto [pos, vel] = registry.get<Position, Velocity>(entity);

// 全コンポーネントの取得
auto components = registry.get(entity);  // tuple<Component&...>
```

### コンポーネントの存在確認

```cpp
// コンポーネントの存在確認
bool has_pos = registry.all_of<Position>(entity);
bool has_both = registry.all_of<Position, Velocity>(entity);
bool has_any = registry.any_of<Position, Velocity>(entity);
bool has_none = registry.none_of<Position>(entity);
```

### ストレージ操作

```cpp
// ストレージの取得
auto &storage = registry.storage<Position>();

// ストレージの存在確認
bool exists = registry.storage<Position>();

// ストレージのサイズ
size_type size = registry.size<Position>();

// ストレージのクリア
registry.clear<Position>();

// ストレージの圧縮
registry.compact<Position>();

// 全ストレージのクリア
registry.clear();
```

### コンテキスト変数

```cpp
// コンテキスト変数の設定
registry.ctx().emplace<GameState>(state);
registry.ctx().insert_or_assign<GameState>(state);

// コンテキスト変数の取得
auto &state = registry.ctx().get<GameState>();
const auto &state = registry.ctx().get<const GameState>();

// コンテキスト変数の取得（ポインタ）
auto *state = registry.ctx().find<GameState>();

// コンテキスト変数の削除
registry.ctx().erase<GameState>();

// コンテキスト変数の存在確認
bool exists = registry.ctx().contains<GameState>();
```

---

## View (ビュー)

### ビューの作成

```cpp
// 基本ビュー（指定コンポーネントを持つエンティティ）
auto view = registry.view<Position>();
auto view = registry.view<Position, Velocity>();
auto view = registry.view<const Position, Velocity>();  // const指定

// 除外ビュー（指定コンポーネントを持たないエンティティ）
auto view = registry.view<Position>(entt::exclude<Dead>);
auto view = registry.view<Position, Velocity>(entt::exclude<Dead, Inactive>);

// ランタイムビュー（実行時にコンポーネントを指定）
entt::runtime_view view{};
view.iterate(registry.storage<Position>())
    .iterate(registry.storage<Velocity>())
    .exclude(registry.storage<Dead>());
```

### ビューのイテレーション

```cpp
auto view = registry.view<Position, Velocity>();

// 方法1: each() コールバック
view.each([](const auto &pos, auto &vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});

// 方法2: each() 拡張コールバック（エンティティも取得）
view.each([](const auto entity, const auto &pos, auto &vel) {
    // entity, pos, vel を使用
});

// 方法3: range-based for（C++20スタイル）
for(auto [entity, pos, vel] : view.each()) {
    // entity, pos, vel を使用
}

// 方法4: エンティティのみイテレーション
for(auto entity : view) {
    auto &pos = view.get<Position>(entity);
    auto &vel = view.get<Velocity>(entity);
}

// 方法5: 前方イテレータ
for(auto it = view.begin(); it != view.end(); ++it) {
    auto entity = *it;
    auto &pos = view.get<Position>(entity);
}
```

### ビューの操作

```cpp
auto view = registry.view<Position>();

// ビューのサイズ
size_type count = view.size();

// ビューが空かどうか
bool empty = view.empty();

// エンティティがビューに含まれるか
bool contains = view.contains(entity);

// コンポーネントの取得
auto &pos = view.get<Position>(entity);
const auto &pos = view.get<const Position>(entity);

// コンポーネントの取得（ポインタ）
auto *pos = view.try_get<Position>(entity);

// ビューの再構築（不要、自動的に更新される）
// view.refresh();  // 通常は不要
```

---

## Group (グループ)

```cpp
// フルオーナーグループ（最高性能、順序固定）
auto group = registry.group<Position, Velocity>();

// パーシャルオーナーグループ
auto group = registry.group<Position>(entt::get<Velocity>);

// ノンオーナーグループ
auto group = registry.group<>(entt::get<Position, Velocity>);

// グループのイテレーション
group.each([](const auto &pos, const auto &vel) {
    // 処理
});

// グループのイテレーション（エンティティ付き）
group.each([](const auto entity, const auto &pos, const auto &vel) {
    // 処理
});

// range-based for
for(auto [entity, pos, vel] : group.each()) {
    // 処理
}
```

---

## Observer (オブザーバー)

```cpp
// オブザーバーの作成
entt::observer observer{};

// コンポーネント追加を監視
observer.connect(registry, entt::collector.group<Position>());

// コンポーネント削除を監視
observer.connect(registry, entt::collector.group<Position>().where([](const auto &entity) {
    return registry.get<Position>(entity).x > 100;
}));

// 変更を監視
observer.connect(registry, entt::collector.update<Position>());

// 変更の取得
for(auto entity : observer) {
    // 変更されたエンティティを処理
}

// オブザーバーのクリア
observer.disconnect();
observer.clear();
```

---

## Signal/Delegate

### Delegate (デリゲート)

```cpp
// デリゲートの作成
entt::delegate<void(int, float)> delegate;

// 関数の登録
void func(int a, float b) { /* ... */ }
delegate.connect<&func>();

// メンバ関数の登録
struct MyClass {
    void method(int a, float b) { /* ... */ }
};
MyClass instance;
delegate.connect<&MyClass::method>(&instance);

// ラムダの登録
delegate.connect([](int a, float b) { /* ... */ });

// デリゲートの呼び出し
delegate(10, 3.14f);

// デリゲートの解除
delegate.disconnect();
```

### Signal (シグナル)

```cpp
// シグナルの作成
entt::sigh<void(int, float)> signal;

// リスナーの接続
auto connection = signal.connect([](int a, float b) {
    // 処理
});

// シグナルの発火
signal.publish(10, 3.14f);

// リスナーの切断
connection.disconnect();
signal.disconnect(connection);
```

### Dispatcher (ディスパッチャー)

```cpp
// ディスパッチャーの作成
entt::dispatcher dispatcher;

// イベント構造体の定義
struct MyEvent {
    int value;
};

// イベントハンドラの登録
dispatcher.sink<MyEvent>().connect([](const MyEvent &event) {
    // 処理
});

// イベントの送信
dispatcher.trigger(MyEvent{42});
dispatcher.enqueue(MyEvent{42});  // キューに追加
dispatcher.update();  // キュー内のイベントを処理
```

---

## Resource (リソース管理)

### Cache (キャッシュ)

```cpp
// リソースキャッシュの作成
entt::resource_cache<Texture> cache;

// リソースの読み込み
auto handle = cache.load<TextureLoader>("texture.png");

// リソースの取得
auto handle = cache.handle("texture.png");
if(handle) {
    auto &texture = *handle;
}

// リソースの存在確認
bool exists = cache.contains("texture.png");

// リソースの削除
cache.erase("texture.png");

// キャッシュのクリア
cache.clear();
```

### Loader (ローダー)

```cpp
// カスタムローダーの実装
struct TextureLoader : entt::resource_loader<TextureLoader, Texture> {
    std::shared_ptr<Texture> load(const std::string &path) const {
        // リソースの読み込み処理
        return std::make_shared<Texture>(path);
    }
};

// ローダーの使用
entt::resource_cache<Texture> cache;
auto handle = cache.load<TextureLoader>("texture.png");
```

---

## Meta (リフレクション)

```cpp
// 型の登録
entt::meta<Position>()
    .type("position"_hs)
    .data<&Position::x>("x"_hs)
    .data<&Position::y>("y"_hs);

// 型の解決
auto type = entt::resolve<Position>();
auto type = entt::resolve("position"_hs);

// プロパティの取得
auto prop = type.data("x"_hs);
auto value = prop.get(instance);

// 関数の呼び出し
auto func = type.func("update"_hs);
func.invoke(instance, args...);

// メタデータの取得
auto meta = type.prop("name"_hs);
std::string name = meta.value().cast<std::string>();
```

---

## Core Utilities

### Any

```cpp
// anyの作成
entt::any any_value{42};
entt::any any_value = entt::make_any<int>(42);

// anyの取得
int value = any_value.cast<int>();
int *ptr = any_value.try_cast<int>();

// anyの型確認
bool is_int = any_value.type() == entt::type_id<int>();

// anyの代入
entt::any other{100};
any_value.assign(other);
```

### Hashed String

```cpp
// ハッシュ文字列の作成
entt::hashed_string name{"position"};
auto hash = name.value();

// リテラル演算子
using namespace entt::literals;
auto hash = "position"_hs;
```

### Type Info

```cpp
// 型IDの取得
auto id = entt::type_id<Position>();
auto name = id.name();

// 型情報の比較
bool same = entt::type_id<Position>() == entt::type_id<Velocity>();
```

---

## よく使うパターン

### 基本的なECSループ

```cpp
void Update(entt::registry &registry, float deltaTime) {
    // 位置と速度を持つエンティティを更新
    auto view = registry.view<Position, Velocity>();
    
    view.each([deltaTime](auto &pos, auto &vel) {
        pos.x += vel.dx * deltaTime;
        pos.y += vel.dy * deltaTime;
    });
}
```

### エンティティの生成

```cpp
entt::entity CreatePlayer(entt::registry &registry, float x, float y) {
    auto entity = registry.create();
    
    registry.emplace<Position>(entity, x, y);
    registry.emplace<Velocity>(entity, 0.0f, 0.0f);
    registry.emplace<Health>(entity, 100);
    registry.emplace<PlayerTag>(entity);
    
    return entity;
}
```

### エンティティの削除

```cpp
void RemoveDeadEntities(entt::registry &registry) {
    auto view = registry.view<Health>();
    
    view.each([](auto entity, const auto &health) {
        if(health.current <= 0) {
            registry.destroy(entity);
        }
    });
}
```

### コンポーネントの条件付き処理

```cpp
void ProcessEnemies(entt::registry &registry) {
    // 敵タグを持ち、死んでいないエンティティ
    auto view = registry.view<EnemyTag, Health>(
        entt::exclude<Dead>
    );
    
    for(auto [entity, health] : view.each()) {
        // 敵の処理
    }
}
```

### システムの実装

```cpp
class MovementSystem {
public:
    explicit MovementSystem(entt::registry &registry)
        : registry_(registry) {}
    
    void Update(float deltaTime) {
        auto view = registry_.view<Position, Velocity>();
        
        view.each([deltaTime](auto &pos, const auto &vel) {
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
        });
    }
    
private:
    entt::registry &registry_;
};
```

### イベント駆動パターン

```cpp
// イベント定義
struct CollisionEvent {
    entt::entity entity1;
    entt::entity entity2;
};

// システム内でイベントを送信
void CollisionSystem::CheckCollisions(entt::registry &registry) {
    auto view = registry.view<Position, Collider>();
    
    for(auto [e1, pos1, col1] : view.each()) {
        for(auto [e2, pos2, col2] : view.each()) {
            if(e1 != e2 && CheckCollision(pos1, pos2)) {
                registry.ctx().get<entt::dispatcher>()
                    .trigger(CollisionEvent{e1, e2});
            }
        }
    }
}

// イベントハンドラの登録
dispatcher.sink<CollisionEvent>().connect([](const CollisionEvent &event) {
    // 衝突処理
});
```

### リソース管理パターン

```cpp
// リソースローダー
struct TextureLoader : entt::resource_loader<TextureLoader, Texture> {
    std::shared_ptr<Texture> load(const std::string &path) const {
        return std::make_shared<Texture>(LoadTexture(path.c_str()));
    }
};

// リソースの使用
void RenderSystem::LoadTextures(entt::registry &registry) {
    auto &cache = registry.ctx().emplace<entt::resource_cache<Texture>>();
    
    auto texture_handle = cache.load<TextureLoader>("player.png");
    if(texture_handle) {
        // テクスチャを使用
        DrawTexture(*texture_handle, 0, 0, WHITE);
    }
}
```

### コンテキスト変数の使用

```cpp
// ゲーム状態の管理
struct GameState {
    int score = 0;
    int level = 1;
};

void InitializeGame(entt::registry &registry) {
    registry.ctx().emplace<GameState>();
}

void UpdateScore(entt::registry &registry, int points) {
    auto &state = registry.ctx().get<GameState>();
    state.score += points;
}
```

---

## 検索インデックス

### カテゴリ別検索

- **エンティティ操作**: `create`, `destroy`, `valid`, `alive`
- **コンポーネント操作**: `emplace`, `get`, `remove`, `erase`, `patch`, `replace`
- **存在確認**: `all_of`, `any_of`, `none_of`, `contains`
- **ビュー**: `view`, `each`, `get`, `size`
- **グループ**: `group`
- **オブザーバー**: `observer`, `connect`, `disconnect`
- **シグナル**: `sigh`, `connect`, `publish`
- **リソース**: `resource_cache`, `load`, `handle`
- **コンテキスト**: `ctx`, `emplace`, `get`, `erase`

### よく使う関数

- **エンティティ作成**: `registry.create()`
- **コンポーネント追加**: `registry.emplace<Component>(entity, args...)`
- **コンポーネント取得**: `registry.get<Component>(entity)`
- **ビュー作成**: `registry.view<Component1, Component2>()`
- **イテレーション**: `view.each([](auto &comp) { ... })`
- **存在確認**: `registry.all_of<Component>(entity)`
- **削除**: `registry.destroy(entity)`, `registry.remove<Component>(entity)`

---

**注意**: このチートシートは EnTT の主要な機能をまとめています。詳細なパラメータや使用例については、[EnTT公式ドキュメント](https://github.com/skypjack/entt)を参照してください。

