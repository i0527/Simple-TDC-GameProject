# AI エージェント向けクイックリファレンス

**最終更新**: 2025-12-03  
**用途**: Cursor でのマルチエージェント開発における即座の参照

---

## ⚡ 最重要ルール（5秒で確認）

```
✅ DO:
1. SimpleTDCGame_NewArch ターゲットを使用
2. ComponentsNew.h をインクルード
3. 依存性注入（DI）パターンを使用
4. JSON解析は try-catch で囲む
5. ECS コンポーネントは POD 型

❌ DON'T:
1. Singleton パターンを使用
2. Components.h をインクルード
3. SimpleTDCGame での新規開発
4. コンポーネントにロジックを実装
5. 生ポインタでメモリ管理
```

---

## 🎭 エージェント役割別チートシート

### エージェント1: アーキテクト

**役割**: システム設計、アーキテクチャ決定

**入力フォーマット**:
```markdown
## タスク
[機能名]の設計を行ってください

## 要件
- [要件1]
- [要件2]

## 制約
- ECS アーキテクチャに準拠
- 依存性注入パターン使用
- レイヤー分離を維持
```

**出力フォーマット**:
```markdown
# [機能名] 設計書

## 概要
[1-2文で説明]

## アーキテクチャ
[レイヤー図]

## コンポーネント設計
### Component1
- データメンバー
- 用途

## システム設計
### System1
- 責務
- 依存関係
- 処理フロー

## データフロー
[図または説明]

## 実装ステップ
1. [ステップ1]
2. [ステップ2]
```

**チェックリスト**:
- [ ] ECS パターンに準拠
- [ ] レイヤー分離が明確
- [ ] 依存性が明示
- [ ] 実装可能な粒度

---

### エージェント2: コーダー

**役割**: コード実装

**テンプレート: コンポーネント実装**:
```cpp
// include/TD/Components/[ComponentName].h
#pragma once

namespace Components {
    /**
     * @brief [簡潔な説明]
     */
    struct [ComponentName] {
        // POD型のデータメンバーのみ
        float value1 = 0.0f;
        int value2 = 0;
        
        // ロジックは含めない
    };
}
```

**テンプレート: システム実装**:
```cpp
// include/TD/Systems/[SystemName].h
#pragma once
#include "Core/System.h"
#include <entt/entt.hpp>

namespace Systems {
    /**
     * @brief [簡潔な説明]
     */
    class [SystemName] : public Core::ISystem {
    private:
        // 依存性（参照で保持）
        ResourceManager& resources_;
        
    public:
        // コンストラクタインジェクション
        explicit [SystemName](ResourceManager& resources)
            : resources_(resources) {}
        
        void ProcessInput(entt::registry& registry) override {
            // 入力処理
        }
        
        void Update(entt::registry& registry, float deltaTime) override {
            auto view = registry.view<Components::A, Components::B>();
            
            for (auto entity : view) {
                auto& a = view.get<Components::A>(entity);
                auto& b = view.get<Components::B>(entity);
                
                // 更新ロジック
            }
        }
        
        void Render(entt::registry& registry) override {
            // 描画処理
        }
    };
}
```

**チェックリスト**:
- [ ] 命名規則準拠（PascalCase）
- [ ] DI パターン使用
- [ ] エラーハンドリング実装
- [ ] コメント記載
- [ ] インクルードガード

---

### エージェント3: リファクター

**役割**: コード品質改善

**チェックポイント**:
```markdown
## コード品質チェック

### 命名規則
- [ ] クラス名: PascalCase
- [ ] 関数名: PascalCase
- [ ] 変数名: camelCase
- [ ] プライベートメンバー: 末尾 _
- [ ] 定数: UPPER_CASE

### アーキテクチャ
- [ ] Singleton を使用していない
- [ ] 依存性注入を使用
- [ ] レイヤー分離が適切
- [ ] 循環依存なし

### エラーハンドリング
- [ ] JSON解析に try-catch
- [ ] ファイル操作にエラーチェック
- [ ] フォールバック処理実装

### メモリ管理
- [ ] スマートポインタ使用
- [ ] RAII パターン準拠
- [ ] メモリリーク無し

### コードの質
- [ ] 重複コード無し
- [ ] 適切なコメント
- [ ] マジックナンバー無し
```

**リファクタリングパターン**:
```cpp
// Before: Singleton
class BadManager {
public:
    static BadManager& GetInstance() { /* ... */ }
};
auto& manager = BadManager::GetInstance();

// After: DI
class GoodManager {
public:
    explicit GoodManager(Config& config) : config_(config) {}
private:
    Config& config_;
};
// コンストラクタインジェクション
```

---

### エージェント4: ドキュメンター

**役割**: ドキュメント作成

**テンプレート: API ドキュメント**:
```markdown
# [システム/コンポーネント名] API リファレンス

## 概要
[1-2文で説明]

## 使用方法

### 基本的な使い方
\```cpp
// サンプルコード
\```

### 高度な使い方
\```cpp
// サンプルコード
\```

## API リファレンス

### コンポーネント

#### ComponentName
\```cpp
struct ComponentName {
    Type member1;  ///< 説明
    Type member2;  ///< 説明
};
\```

### システム

#### SystemName
\```cpp
class SystemName : public Core::ISystem {
public:
    SystemName(Dependency& dep);
    void Update(entt::registry& registry, float deltaTime) override;
};
\```

**メソッド**:
- `Update()`: [説明]

## 使用例

### 例1: [ユースケース1]
\```cpp
// コード例
\```

### 例2: [ユースケース2]
\```cpp
// コード例
\```

## 注意事項
- [注意点1]
- [注意点2]

## 関連
- [関連システム/コンポーネント]
```

---

### エージェント5: テスター

**役割**: テストコード作成

**テンプレート: 単体テスト（Google Test）**:
```cpp
// tests/TD/Systems/[SystemName]Test.cpp
#include <gtest/gtest.h>
#include "TD/Systems/[SystemName].h"
#include "TD/Components/[ComponentName].h"

namespace {

class [SystemName]Test : public ::testing::Test {
protected:
    entt::registry registry;
    
    void SetUp() override {
        // テスト前の初期化
    }
    
    void TearDown() override {
        // テスト後のクリーンアップ
        registry.clear();
    }
};

// 正常系テスト
TEST_F([SystemName]Test, NormalCase) {
    // Arrange: テストデータ準備
    auto entity = registry.create();
    registry.emplace<Components::A>(entity, /* データ */);
    
    [SystemName] system;
    
    // Act: テスト対象実行
    system.Update(registry, 1.0f);
    
    // Assert: 結果検証
    auto& component = registry.get<Components::A>(entity);
    EXPECT_EQ(component.value, expected);
}

// 異常系テスト
TEST_F([SystemName]Test, EdgeCase) {
    // エッジケースのテスト
}

// 境界値テスト
TEST_F([SystemName]Test, BoundaryValue) {
    // 境界値のテスト
}

} // namespace
```

**テストカテゴリ**:
1. **正常系**: 期待される入力での動作
2. **異常系**: エラー条件での動作
3. **境界値**: 極端な値での動作
4. **エッジケース**: 特殊な条件での動作

---

## 🔧 頻出コードスニペット

### JSON解析
```cpp
std::optional<nlohmann::json> LoadJson(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open: " << path << std::endl;
            return std::nullopt;
        }
        return nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}
```

### エンティティ生成
```cpp
entt::entity CreateEntity(entt::registry& registry) {
    auto entity = registry.create();
    
    registry.emplace<Components::Transform>(entity, 0.0f, 0.0f);
    registry.emplace<Components::Velocity>(entity, 0.0f, 0.0f);
    registry.emplace<Components::Health>(entity, 100, 100);
    
    return entity;
}
```

### システム登録
```cpp
// GameNew.cpp
void RegisterSystems() {
    systems_.push_back(
        std::make_unique<Systems::InputSystem>()
    );
    systems_.push_back(
        std::make_unique<Systems::MovementSystem>()
    );
    systems_.push_back(
        std::make_unique<Systems::RenderSystem>(resourceManager_)
    );
}
```

---

## 📁 ファイル配置ルール

```
include/
├── Core/
│   ├── Components/      # コアコンポーネント
│   │   ├── Transform.h
│   │   └── Renderable.h
│   ├── Systems/         # コアシステム
│   └── System.h         # ISystem インターフェース
├── TD/
│   ├── Components/      # TD固有コンポーネント
│   │   ├── Tower.h
│   │   └── Enemy.h
│   └── Systems/         # TD固有システム
│       ├── TowerSystem.h
│       └── EnemySystem.h
├── Roguelike/
│   ├── Components/      # Roguelike固有
│   └── Systems/
└── ComponentsNew.h      # 統合ヘッダー

src/
├── main_new.cpp         # SimpleTDCGame_NewArch エントリポイント
├── GameNew.cpp          # GameNew 実装
└── ... 
```

---

## 🚀 ビルド＆実行コマンド

```bash
# 初回セットアップ
cmake --preset ninja

# ビルド（新アーキテクチャ）
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# 実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe

# クリーンビルド
rm -rf build && cmake --preset ninja && cmake --build --preset ninja-release
```

---

## 🐛 よくあるエラーと解決策

### エラー1: "Singleton::GetInstance() not found"
**原因**: Singleton を DI に変更中  
**解決**: コンストラクタインジェクションに変更

### エラー2: "Components.h: No such file"
**原因**: ComponentsNew.h に移行中  
**解決**: `#include "ComponentsNew.h"` に変更

### エラー3: "EnTT not found"
**原因**: FetchContent が失敗  
**解決**: `rm -rf build && cmake --preset ninja`

### エラー4: "Failed to load config.json"
**原因**: assets/ がコピーされていない  
**解決**: `cp -r assets build/ninja/bin/`

---

## 📚 参照ドキュメント優先度

1. **最優先**: `.cursor/CURSOR_DEVELOPMENT_GUIDE.md`
2. **次**: `docs/EXECUTIVE_SUMMARY.md`
3. **詳細**: `docs/CODE_ANALYSIS.md`
4. **規約**: `.github/copilot-instructions.md`

---

## 💡 タスク分割のヒント

### ❌ 悪い例（大きすぎる）
```
「敵AIシステムを実装してください」
```

### ✅ 良い例（適切なサイズ）
```
Phase 1: EnemyAI コンポーネント定義（アーキテクト）
Phase 2: EnemyAI コンポーネント実装（コーダー）
Phase 3: EnemyAISystem 設計（アーキテクト）
Phase 4: EnemyAISystem 実装（コーダー）
Phase 5: コードレビュー（リファクター）
Phase 6: ドキュメント作成（ドキュメンター）
Phase 7: テスト作成（テスター）
```

---

## 🎯 品質チェックリスト（提出前）

```markdown
- [ ] 命名規則準拠
- [ ] DI パターン使用（Singleton なし）
- [ ] ComponentsNew.h 使用
- [ ] エラーハンドリング実装
- [ ] ビルド成功
- [ ] 実行確認
- [ ] コメント記載
- [ ] ドキュメント更新
```

---

**このクイックリファレンスを常に参照して、効率的に開発を進めてください！**
