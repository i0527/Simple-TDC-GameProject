# クイックフィックスガイド - 緊急対応項目

**作成日**: 2025-12-06  
**優先度**: 🔴 緊急  
**所要時間**: 1-2日

このガイドは、[ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md](ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md)で特定された**緊急対応が必要な問題**の解決手順を示します。

---

## 問題1: `World`クラスの設計vs実装の不一致 🔴

### 現状
- **設計文書**: `core-architecture-design.md`で`World`クラスが詳細に定義されている
- **実装**: `include/new/Core/World.h`が存在するが、実装が不完全または未使用
- **影響**: 開発者が混乱し、実装と設計の乖離が拡大

### 解決策A: `World`クラスを完全実装（推奨）

設計文書通りに`World`クラスを実装し、`GameContext`から分離する。

#### 実装手順

1. **`World.h`の更新**
```cpp
// include/new/Core/World.h
namespace New::Core {

class World {
public:
    explicit World(entt::registry& registry);
    ~World() = default;
    
    // エンティティ作成
    entt::entity CreateEntity();
    entt::entity CreateEntity(const std::string& name);
    
    // エンティティ削除
    void DestroyEntity(entt::entity entity);
    
    // エンティティ名管理
    void SetEntityName(entt::entity entity, const std::string& name);
    std::string GetEntityName(entt::entity entity) const;
    entt::entity FindEntityByName(const std::string& name) const;
    
    // コンポーネント操作（テンプレート）
    template<typename Component>
    Component& AddComponent(entt::entity entity, Component&& component) {
        return registry_.emplace<Component>(entity, std::forward<Component>(component));
    }
    
    template<typename Component>
    Component& GetComponent(entt::entity entity) {
        return registry_.get<Component>(entity);
    }
    
    template<typename Component>
    bool HasComponent(entt::entity entity) const {
        return registry_.all_of<Component>(entity);
    }
    
    template<typename Component>
    void RemoveComponent(entt::entity entity) {
        registry_.remove<Component>(entity);
    }
    
    // ビュー取得
    template<typename... Components>
    auto View() {
        return registry_.view<Components...>();
    }
    
    // レジストリ直接アクセス
    entt::registry& GetRegistry() { return registry_; }
    const entt::registry& GetRegistry() const { return registry_; }

private:
    entt::registry& registry_;  // 注: 参照を保持（所有権なし）
    std::unordered_map<std::string, entt::entity> nameToEntity_;
    std::unordered_map<entt::entity, std::string> entityToName_;
};

} // namespace New::Core
```

**重要**: `World`は`registry`の参照を保持するため、`registry`のライフタイムは`World`より長くなければなりません。`GameContext`での使用を想定しているため、`GameContext`が`registry`と`World`の両方を所有することで、この条件を満たします。

2. **`World.cpp`の実装**
```cpp
// src/new/Core/World.cpp
#include "Core/World.h"

namespace New::Core {

World::World(entt::registry& registry) : registry_(registry) {}

entt::entity World::CreateEntity() {
    return registry_.create();
}

entt::entity World::CreateEntity(const std::string& name) {
    auto entity = registry_.create();
    SetEntityName(entity, name);
    return entity;
}

void World::DestroyEntity(entt::entity entity) {
    // 名前マッピング削除
    auto it = entityToName_.find(entity);
    if (it != entityToName_.end()) {
        nameToEntity_.erase(it->second);
        entityToName_.erase(it);
    }
    registry_.destroy(entity);
}

void World::SetEntityName(entt::entity entity, const std::string& name) {
    // 既存の名前を削除
    auto it = entityToName_.find(entity);
    if (it != entityToName_.end()) {
        nameToEntity_.erase(it->second);
    }
    
    // 新しい名前を設定
    nameToEntity_[name] = entity;
    entityToName_[entity] = name;
}

std::string World::GetEntityName(entt::entity entity) const {
    auto it = entityToName_.find(entity);
    return (it != entityToName_.end()) ? it->second : "";
}

entt::entity World::FindEntityByName(const std::string& name) const {
    auto it = nameToEntity_.find(name);
    return (it != nameToEntity_.end()) ? it->second : entt::null;
}

} // namespace New::Core
```

3. **`GameContext`に`World`を追加**
```cpp
// include/new/Core/GameContext.h
class GameContext {
public:
    // ...既存メンバー...
    
    World& GetWorld() { return *world_; }
    const World& GetWorld() const { return *world_; }

private:
    entt::registry registry_;
    std::unique_ptr<World> world_;  // 追加
    // ...既存メンバー...
};

// src/new/Core/GameContext.cpp
bool GameContext::Initialize() {
    // World初期化
    world_ = std::make_unique<World>(registry_);
    
    // ...既存の初期化処理...
    return true;
}
```

#### 所要時間
- 実装: 2-3時間
- テスト: 1時間

---

### 解決策B: `World`クラスを削除（代替案）

`World`の機能を`GameContext`に統合し、設計文書を更新する。

#### 実装手順

1. **設計文書の更新**
   - `core-architecture-design.md`から`World`クラスのセクションを削除
   - `GameContext`に名前管理機能を統合する旨を記載

2. **`GameContext`の拡張**
```cpp
class GameContext {
public:
    // エンティティ作成（名前付き）
    entt::entity CreateEntity(const std::string& name = "");
    void DestroyEntity(entt::entity entity);
    
    // 名前管理
    void SetEntityName(entt::entity entity, const std::string& name);
    std::string GetEntityName(entt::entity entity) const;
    entt::entity FindEntityByName(const std::string& name) const;
    
private:
    std::unordered_map<std::string, entt::entity> nameToEntity_;
    std::unordered_map<entt::entity, std::string> entityToName_;
};
```

#### 所要時間
- 実装: 1-2時間
- 設計文書更新: 1時間

---

### 推奨: **解決策A**

理由:
- 設計文書の意図（レイヤー分離）を尊重
- `GameContext`の責務が肥大化しない
- 将来の拡張性が高い

---

## 問題2: 重複文書の統合 🔴

### 2.1 `design-principles*.md` の統合

#### 現状
- `design-principles.md` (462行) - 概要版
- `design-principles-and-constraints.md` (488行) - 詳細版
- 内容がほぼ重複、混乱の原因

#### 統合方針
詳細版を残し、概要版は削除して統合版にリンク

#### 手順

1. **`design-principles-and-constraints.md`を改名**
```bash
mv .cursor/new/design-principles-and-constraints.md \
   .cursor/new/design-principles-UNIFIED.md
```

2. **`design-principles.md`を削除し、リダイレクトに置換**
```bash
# design-principles.md
# 設計原則

このファイルは [design-principles-UNIFIED.md](design-principles-UNIFIED.md) に統合されました。

詳細版をご覧ください。
```

3. **他文書のリンクを更新**
```bash
# core-architecture-design.md 等から
design-principles.md → design-principles-UNIFIED.md
```

4. **00_INDEX.mdを更新**

#### 所要時間: 30分

---

### 2.2 `libs*.md` の統合

#### 現状
- `libs_guide.md` (502行)
- `gamedev_libs_guide.md` (693行)
- `libs-overview.md` (15行)

#### 統合方針
`libs_guide.md`を充実させ、`gamedev_libs_guide.md`を統合。`libs-overview.md`は簡易参照として維持。

#### 手順

1. **差分の抽出**
```bash
# gamedev_libs_guide.md で libs_guide.md にない情報を特定
# 例: Raylib固有の詳細、EnTT v3.12の変更点など
```

2. **`libs_guide.md`に統合**
```markdown
# ライブラリガイド（統合版）

## 目次
1. Raylib リソース管理
2. EnTT 注意点
3. nlohmann/json エラーハンドリング
4. ImGui + rlImGui 統合
5. raygui 使用方法

（gamedev_libs_guide.md の内容を統合）
```

3. **`raylib_resource_mgmt.md`とのクロスリファレンス**

4. **`gamedev_libs_guide.md`を削除し、リダイレクト化**

#### 所要時間: 1時間

---

### 2.3 `td-systems*.md` の統合

#### 現状
- `td-systems-design.md` (787行) - 全体設計
- `linea-td-detailed-design.md` (150行) - 直線TD詳細

#### 統合方針
`linea-td-detailed-design.md`を`td-systems-design.md`のセクションとして統合

#### 手順

1. **`td-systems-design.md`に直線TD詳細セクションを追加**
```markdown
## 2. 直線型TD（LineTD）設計

### 2.1 コンポーネント定義
（既存内容）

### 2.2 システム実装
（既存内容）

### 2.3 詳細設計（旧linea-td-detailed-design.md）
（linea-td-detailed-design.mdの内容を統合）
```

2. **`linea-td-detailed-design.md`を削除しリダイレクト化**

#### 所要時間: 30分

---

## 実施チェックリスト

### Day 1: `World`クラス実装
- [ ] World.h 更新
- [ ] World.cpp 実装
- [ ] GameContext に統合
- [ ] ユニットテスト作成
- [ ] 動作確認

### Day 1-2: 文書統合
- [ ] design-principles統合
- [ ] libs統合
- [ ] td-systems統合
- [ ] 00_INDEX.md更新
- [ ] リンク切れチェック

### Day 2: 検証とコミット
- [ ] ビルド確認
- [ ] テスト実行
- [ ] 文書の整合性確認
- [ ] コミット＆プッシュ

---

## 完了基準

✅ 以下がすべて達成されたら完了

1. `World`クラスが設計文書通りに実装され、動作する
2. 重複文書が統合され、リダイレクトが設定されている
3. すべてのクロスリファレンスが正しいパスを指している
4. ビルドが成功し、既存の動作に影響がない
5. `00_INDEX.md`が最新の文書構成を反映している

---

## トラブルシューティング

### Q: `World`実装後、既存のコードがビルドエラー

A: `GameContext`を使用している箇所で、以下の変更が必要な場合があります：
```cpp
// 修正前
auto entity = context.GetRegistry().create();

// 修正後
auto entity = context.GetWorld().CreateEntity();
```

### Q: 文書統合後、リンク切れが発生

A: 以下のコマンドで一括検索・置換：
```bash
# .cursor/new/ 配下の全.mdファイルでリンクチェック
grep -r "design-principles\.md" .cursor/new/*.md
grep -r "libs_guide\.md" .cursor/new/*.md
```

---

## 次のステップ

クイックフィックス完了後:
1. [IMPLEMENTATION_PHASES.md](IMPLEMENTATION_PHASES.md)のPhase 1継続
2. テスト戦略文書の作成（[ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md](ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md) § 3.3参照）
3. JSONスキーマファイル（.schema.json）の実装

---

**作成者**: AI開発アシスタント  
**最終更新**: 2025-12-06
