# Cursor IDE 開発ガイド - Simple-TDC-GameProject

**バージョン**: 1.0  
**最終更新**: 2025-12-03  
**対象**: Cursor IDE + マルチAIエージェント開発  

---

## 📋 目次

1. [プロジェクト概要](#1-プロジェクト概要)
2. [Cursor IDE設定](#2-cursor-ide設定)
3. [マルチエージェント開発戦略](#3-マルチエージェント開発戦略)
4. [設計改善の要点](#4-設計改善の要点)
5. [推奨される設計パターン](#5-推奨される設計パターン)
6. [AIエージェント向けタスク分割](#6-aiエージェント向けタスク分割)
7. [コーディング規約と実装例](#7-コーディング規約と実装例)
8. [トラブルシューティング](#8-トラブルシューティング)
9. [よくある質問](#9-よくある質問)

---

## 1. プロジェクト概要

### 1.1 プロジェクトの目的

Simple-TDC-GameProjectは、**ECSアーキテクチャを採用したC++ゲームプロジェクト**です。データ駆動設計とモダンなC++開発手法を学ぶためのフレームワークを提供します。

### 1.2 3つのビルドターゲット

| ターゲット | 状態 | 用途 | 推奨度 |
|-----------|------|------|--------|
| **SimpleTDCGame** | 保守モード | 旧アーキテクチャ（非推奨） | ❌ |
| **SimpleTDCGame_NewArch** | アクティブ開発 | 新アーキテクチャ（推奨） | ✅ |
| **NetHackGame** | 開発中 | Roguelikeゲーム | 🚧 |

**重要**: 新規開発では必ず `SimpleTDCGame_NewArch` を使用してください。

### 1.3 技術スタック

```yaml
言語: C++17
ビルドシステム: CMake 3.19+
ECS: EnTT v3.12.2
JSON: nlohmann/json v3.11.2
レンダリング: Raylib 5.0
UI: raygui 4.0, Dear ImGui v1.90.1
```

### 1.4 アーキテクチャレイヤー

```
┌────────────────────────────────────────┐
│  Application Layer                      │
│  (Game, GameNew, RoguelikeGame)        │
└────────────────────────────────────────┘
              ↓
┌────────────────────────────────────────┐
│  Domain Layer                           │
│  (TD, Roguelike)                       │
└────────────────────────────────────────┘
              ↓
┌────────────────────────────────────────┐
│  Game Layer                             │
│  (Sprite, Animation, Scene)            │
└────────────────────────────────────────┘
              ↓
┌────────────────────────────────────────┐
│  Core Layer                             │
│  (ECS, DI, Rendering, Config)          │
└────────────────────────────────────────┘
              ↓
┌────────────────────────────────────────┐
│  External Libraries                     │
│  (EnTT, Raylib, nlohmann/json)         │
└────────────────────────────────────────┘
```

---

## 2. Cursor IDE設定

### 2.1 推奨設定

#### .cursorrules ファイル（プロジェクトルートに作成）

```markdown
# Simple-TDC-GameProject Cursor Rules

## 必須ルール

1. **新規開発では SimpleTDCGame_NewArch ターゲットを使用**
2. **ComponentsNew.h を使用（Components.h は非推奨）**
3. **Singleton パターンを避け、依存性注入（DI）を使用**
4. **JSON解析は必ず try-catch で囲む**
5. **ECSコンポーネントは POD 型として定義**

## コーディング規約

- クラス名: PascalCase
- 関数名: PascalCase
- 変数名: camelCase (プライベートメンバーは末尾に `_`)
- 定数: UPPER_CASE
- 名前空間: PascalCase

## ビルドコマンド

```bash
# 設定
cmake --preset ninja

# ビルド
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# 実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe
```

## 重要なドキュメント

- docs/EXECUTIVE_SUMMARY.md - プロジェクト全体概要
- docs/CODE_ANALYSIS.md - 詳細分析
- docs/REFACTORING_PLAN.md - 改善計画
- .github/copilot-instructions.md - コーディング規約
```

### 2.2 Cursor Composer 設定

#### プロジェクト固有のインストラクション

Cursor の Settings → Features → Composer Instructions に以下を設定：

```
このプロジェクトは C++17 の ECS ゲームプロジェクトです。

重要な制約:
1. 新規コードは SimpleTDCGame_NewArch ターゲット向けに書く
2. Singleton を使わず DI パターンを使用
3. ComponentsNew.h を使用（Components.h は非推奨）
4. JSON解析は try-catch 必須
5. ECS コンポーネントは POD 型のみ

ビルド: cmake --build --preset ninja-release --target SimpleTDCGame_NewArch
詳細: .cursor/CURSOR_DEVELOPMENT_GUIDE.md を参照
```

### 2.3 AIモデル選択戦略

| タスク種別 | 推奨モデル | 理由 |
|-----------|-----------|------|
| アーキテクチャ設計 | Claude 3.5 Sonnet | 深い理解と設計能力 |
| コード生成 | GPT-4 | 高速で正確 |
| リファクタリング | Claude 3.5 Sonnet | コンテキスト理解 |
| ドキュメント生成 | GPT-4 / Claude | どちらも優秀 |
| デバッグ | GPT-4 | 迅速な問題解決 |
| コードレビュー | Claude 3.5 Sonnet | 詳細な分析 |

---

## 3. マルチエージェント開発戦略

### 3.1 エージェント役割分担

Cursor では複数の AI エージェントを効果的に使い分けることで、開発効率を最大化できます。

#### エージェント1: アーキテクト

**役割**: システム設計、アーキテクチャ決定

**担当タスク**:
- 新機能の設計レビュー
- アーキテクチャパターンの提案
- リファクタリング計画の策定
- 依存関係の分析

**プロンプト例**:
```
SimpleTDCGame_NewArch に新しい敵AIシステムを追加したい。
ECS アーキテクチャに沿った設計を提案してください。

要件:
- 複数の敵タイプに対応
- 状態機械パターン
- 依存性注入を使用
```

#### エージェント2: コーダー

**役割**: コード実装、単体機能の開発

**担当タスク**:
- コンポーネント実装
- システム実装
- ユーティリティ関数作成
- 小規模なバグ修正

**プロンプト例**:
```
以下の設計に基づいて、EnemyAI コンポーネントとシステムを実装してください。

設計:
- EnemyAI コンポーネント (POD型)
- EnemyAISystem (Core::ISystem を継承)
- 依存: Transform, Velocity コンポーネント

ファイル:
- include/TD/Components/EnemyAI.h
- include/TD/Systems/EnemyAISystem.h
```

#### エージェント3: リファクター

**役割**: コード品質改善、重複削除

**担当タスク**:
- コードの重複削除
- Singleton の DI への変換
- 命名規則の統一
- パフォーマンス最適化

**プロンプト例**:
```
src/Game.cpp の ConfigManager::GetInstance() を
依存性注入パターンに変更してください。

条件:
- Singleton パターンを削除
- コンストラクタインジェクション使用
- 既存の動作を維持
```

#### エージェント4: ドキュメンター

**役割**: ドキュメント作成、コメント追加

**担当タスク**:
- API ドキュメント作成
- README 更新
- 設計書作成
- コードコメント追加

**プロンプト例**:
```
新しく実装した EnemyAISystem の設計書を作成してください。

含める内容:
- システム概要
- コンポーネント構成
- 処理フロー図
- 使用例
```

#### エージェント5: テスター

**役割**: テストコード作成、品質保証

**担当タスク**:
- 単体テスト作成
- 統合テスト作成
- エッジケースの検証
- テストカバレッジ向上

**プロンプト例**:
```
EnemyAISystem の単体テストを Google Test で作成してください。

テストケース:
- 正常な移動パターン
- 境界値チェック
- null ポインタ処理
```

### 3.2 マルチエージェント協調フロー

```
┌─────────────────────────────────────────────────────────┐
│ Step 1: 設計フェーズ                                       │
│   エージェント: アーキテクト                               │
│   成果物: 設計ドキュメント                                 │
└─────────────────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ Step 2: 実装フェーズ                                       │
│   エージェント: コーダー                                   │
│   成果物: 実装コード                                       │
└─────────────────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ Step 3: レビュー＆リファクタリング                          │
│   エージェント: リファクター                               │
│   成果物: 改善されたコード                                 │
└─────────────────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ Step 4: ドキュメンテーション                               │
│   エージェント: ドキュメンター                             │
│   成果物: ドキュメント                                     │
└─────────────────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ Step 5: テスト                                            │
│   エージェント: テスター                                   │
│   成果物: テストコード                                     │
└─────────────────────────────────────────────────────────┘
```

### 3.3 エージェント切り替えのベストプラクティス

#### 1. コンテキスト共有

各エージェントには前のエージェントの成果物を明示的に渡す：

```
【コーダーへの指示】
以下のアーキテクト設計に基づいて実装してください。

設計:
<アーキテクトの出力をペースト>

実装ファイル:
- include/TD/Components/EnemyAI.h
- include/TD/Systems/EnemyAISystem.h
```

#### 2. 段階的な作業

大きなタスクは小さく分割し、各エージェントに適切なサイズの作業を割り当てる：

```
❌ 悪い例:
「敵AIシステム全体を実装してください」

✅ 良い例:
1. アーキテクト: 敵AIシステムの設計
2. コーダー: EnemyAI コンポーネント実装
3. コーダー: EnemyAISystem 実装
4. リファクター: コードレビューと改善
5. ドキュメンター: API ドキュメント作成
6. テスター: 単体テスト作成
```

#### 3. 明確な成果物定義

各エージェントには明確な成果物を要求する：

```
【アーキテクトへ】
成果物: Markdown 形式の設計書

含める内容:
- クラス図
- シーケンス図
- データフロー
- 依存関係
```

---

## 4. 設計改善の要点

### 4.1 現状の主要課題

#### 🔴 最優先課題

| 課題 | 影響度 | 緊急度 | 対策 |
|-----|-------|-------|------|
| アーキテクチャの混在 | 高 | 高 | Strangler Fig Pattern で移行 |
| コンポーネント重複 | 高 | 高 | ComponentsNew.h に統一 |
| Singleton 過剰使用 | 中 | 中 | DI パターンに変換 |

#### 🟡 重要課題

| 課題 | 影響度 | 緊急度 | 対策 |
|-----|-------|-------|------|
| Roguelike 未実装 | 中 | 低 | Phase 1-3 段階的実装 |
| テストインフラ未整備 | 中 | 中 | Google Test 導入 |
| ドキュメント散在 | 中 | 高 | docs/ 配下に統合済み |

### 4.2 改善戦略: Strangler Fig Pattern

旧コードを一気に削除せず、段階的に置き換える：

```
Phase 1 (現在): 新旧共存
┣━ SimpleTDCGame (旧) - バグ修正のみ
┣━ SimpleTDCGame_NewArch (新) - アクティブ開発 ⭐
┗━ NetHackGame - 実装開始

Phase 2 (1-2ヶ月後): 新アーキ優先
┣━ SimpleTDCGame (旧) - 非推奨マーク
┣━ SimpleTDCGame_NewArch (新) - メイン開発
┗━ NetHackGame - Phase 1-3 完成

Phase 3 (3ヶ月後): 旧アーキ廃止
┣━ SimpleTDCGame (旧) - 削除
┣━ SimpleTDCGame (新) - リネーム
┗━ NetHackGame - 統合完了
```

### 4.3 コンポーネント統一計画

#### 現状

```cpp
// 旧 (非推奨)
include/Components.h
  - Position, Velocity, Sprite, Health...

// 新 (推奨)
include/ComponentsNew.h
  - include "Core/Components/Transform.h"
  - include "Core/Components/Renderable.h"
  - ...
```

#### 移行手順

1. **非推奨警告の追加**
   ```cpp
   // Components.h
   #pragma message("Components.h is deprecated. Use ComponentsNew.h")
   #include "ComponentsNew.h"
   ```

2. **全ファイルの置き換え**
   ```bash
   # 一括置換（Cursor の多段階編集機能を使用）
   Find: #include "Components.h"
   Replace: #include "ComponentsNew.h"
   ```

3. **1週間の移行期間後、Components.h を削除**

### 4.4 Singleton 削減計画

#### 問題のある Singleton 例

```cpp
// ❌ 悪い例: Singleton パターン
class ConfigManager {
public:
    static ConfigManager& GetInstance() {
        static ConfigManager instance;
        return instance;
    }
    
private:
    ConfigManager() = default;
};

// 使用箇所
void SomeFunction() {
    auto& config = ConfigManager::GetInstance();
    int value = config.GetInt("key");
}
```

#### 推奨される DI パターン

```cpp
// ✅ 良い例: 依存性注入
class ConfigManager {
public:
    explicit ConfigManager(const std::string& configPath) {
        // 初期化
    }
    
    int GetInt(const std::string& key) const;
};

// システムでの使用
class GameSystem : public Core::ISystem {
private:
    ConfigManager& config_;  // 参照として保持
    
public:
    explicit GameSystem(ConfigManager& config) 
        : config_(config) {}
    
    void Update(entt::registry& registry, float deltaTime) override {
        int value = config_.GetInt("key");
    }
};

// GameNew.cpp での初期化
class GameNew {
private:
    ConfigManager configManager_;
    std::vector<std::unique_ptr<Core::ISystem>> systems_;
    
public:
    GameNew() 
        : configManager_("assets/config.json") {
        // DI: ConfigManager を各システムに注入
        systems_.push_back(
            std::make_unique<GameSystem>(configManager_)
        );
    }
};
```

---

## 5. 推奨される設計パターン

### 5.1 ECS パターン（必須）

#### コンポーネント定義

```cpp
// ✅ 正しい: POD型コンポーネント
namespace Components {
    struct Transform {
        float x = 0.0f;
        float y = 0.0f;
        float rotation = 0.0f;
        float scale = 1.0f;
    };
    
    struct Velocity {
        float dx = 0.0f;
        float dy = 0.0f;
    };
    
    struct Health {
        int current = 100;
        int maximum = 100;
    };
}

// ❌ 間違い: メソッドを持つコンポーネント
struct Transform {
    float x, y;
    void Move(float dx, float dy) { x += dx; y += dy; }  // NG!
};
```

#### システム実装

```cpp
// include/Core/System.h
namespace Core {
    class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void ProcessInput(entt::registry& registry) {}
        virtual void Update(entt::registry& registry, float deltaTime) = 0;
        virtual void Render(entt::registry& registry) {}
    };
}

// include/TD/Systems/MovementSystem.h
namespace Systems {
    class MovementSystem : public Core::ISystem {
    public:
        void Update(entt::registry& registry, float deltaTime) override {
            auto view = registry.view<Components::Transform, Components::Velocity>();
            
            for (auto entity : view) {
                auto& transform = view.get<Components::Transform>(entity);
                auto& velocity = view.get<Components::Velocity>(entity);
                
                transform.x += velocity.dx * deltaTime;
                transform.y += velocity.dy * deltaTime;
            }
        }
    };
}
```

### 5.2 依存性注入パターン（推奨）

#### コンストラクタインジェクション

```cpp
// ✅ 推奨: コンストラクタインジェクション
class RenderSystem : public Core::ISystem {
private:
    ResourceManager& resourceManager_;
    ConfigManager& configManager_;
    
public:
    RenderSystem(ResourceManager& resources, ConfigManager& config)
        : resourceManager_(resources)
        , configManager_(config) {}
    
    void Render(entt::registry& registry) override {
        auto texture = resourceManager_.GetTexture("player");
        // ...
    }
};

// GameNew.cpp
class GameNew {
private:
    ConfigManager configManager_;
    ResourceManager resourceManager_;
    std::vector<std::unique_ptr<Core::ISystem>> systems_;
    
public:
    GameNew()
        : configManager_("assets/config.json")
        , resourceManager_(configManager_) {
        
        systems_.push_back(
            std::make_unique<RenderSystem>(resourceManager_, configManager_)
        );
    }
};
```

### 5.3 データ駆動設計パターン

#### JSON定義

```json
// assets/enemies/goblin.json
{
  "name": "Goblin",
  "components": {
    "transform": {
      "x": 100,
      "y": 200,
      "rotation": 0,
      "scale": 1.0
    },
    "health": {
      "current": 50,
      "maximum": 50
    },
    "sprite": {
      "texture": "goblin.png",
      "width": 32,
      "height": 32
    },
    "enemyAI": {
      "type": "patrol",
      "speed": 2.5,
      "detectionRange": 150
    }
  }
}
```

#### JSONからのエンティティ生成

```cpp
// include/Data/EntityFactory.h
namespace Data {
    class EntityFactory {
    private:
        ResourceManager& resourceManager_;
        
    public:
        explicit EntityFactory(ResourceManager& resources)
            : resourceManager_(resources) {}
        
        entt::entity CreateFromJson(
            entt::registry& registry,
            const std::string& jsonPath
        ) {
            auto entity = registry.create();
            
            try {
                std::ifstream file(jsonPath);
                nlohmann::json j = nlohmann::json::parse(file);
                
                // Transform コンポーネント
                if (j.contains("components") && j["components"].contains("transform")) {
                    auto& t = j["components"]["transform"];
                    registry.emplace<Components::Transform>(entity,
                        t["x"].get<float>(),
                        t["y"].get<float>(),
                        t["rotation"].get<float>(),
                        t["scale"].get<float>()
                    );
                }
                
                // Health コンポーネント
                if (j.contains("components") && j["components"].contains("health")) {
                    auto& h = j["components"]["health"];
                    registry.emplace<Components::Health>(entity,
                        h["current"].get<int>(),
                        h["maximum"].get<int>()
                    );
                }
                
                // Sprite コンポーネント
                if (j.contains("components") && j["components"].contains("sprite")) {
                    auto& s = j["components"]["sprite"];
                    auto texture = resourceManager_.GetTexture(s["texture"]);
                    registry.emplace<Components::Sprite>(entity,
                        texture,
                        s["width"].get<int>(),
                        s["height"].get<int>()
                    );
                }
                
            } catch (const nlohmann::json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                registry.destroy(entity);
                return entt::null;
            } catch (const std::exception& e) {
                std::cerr << "Error creating entity: " << e.what() << std::endl;
                registry.destroy(entity);
                return entt::null;
            }
            
            return entity;
        }
    };
}
```

### 5.4 シーンパターン

```cpp
// include/Game/Scene.h
namespace Game {
    class IScene {
    public:
        virtual ~IScene() = default;
        virtual void OnEnter() = 0;
        virtual void OnExit() = 0;
        virtual void ProcessInput() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;
    };
}

// include/TD/Scenes/BattleScene.h
namespace TD {
    class BattleScene : public Game::IScene {
    private:
        entt::registry registry_;
        std::vector<std::unique_ptr<Core::ISystem>> systems_;
        ResourceManager& resourceManager_;
        
    public:
        explicit BattleScene(ResourceManager& resources)
            : resourceManager_(resources) {}
        
        void OnEnter() override {
            // エンティティ初期化
            // システム登録
            systems_.push_back(
                std::make_unique<Systems::MovementSystem>()
            );
        }
        
        void OnExit() override {
            systems_.clear();
            registry_.clear();
        }
        
        void Update(float deltaTime) override {
            for (auto& system : systems_) {
                system->Update(registry_, deltaTime);
            }
        }
        
        void Render() override {
            for (auto& system : systems_) {
                system->Render(registry_);
            }
        }
    };
}
```

### 5.5 エラーハンドリングパターン

```cpp
// ✅ 推奨: try-catch + フォールバック
bool LoadConfig(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open: " << path << std::endl;
            return LoadDefaultConfig();  // フォールバック
        }
        
        nlohmann::json config = nlohmann::json::parse(file);
        
        // 設定を適用
        width_ = config.value("width", 800);  // デフォルト値指定
        height_ = config.value("height", 600);
        
        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return LoadDefaultConfig();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return LoadDefaultConfig();
    }
}

bool LoadDefaultConfig() {
    width_ = 800;
    height_ = 600;
    std::cerr << "Using default configuration" << std::endl;
    return true;
}
```

---

## 6. AIエージェント向けタスク分割

### 6.1 大きなタスクの分割例

#### ❌ 悪い例: 曖昧で大きすぎるタスク

```
「敵AIシステムを実装してください」
```

#### ✅ 良い例: 明確で小さなタスク

```
Phase 1: 設計（エージェント: アーキテクト）
タスク: 敵AIシステムの設計書を作成
成果物: docs/ENEMY_AI_DESIGN.md
内容:
- コンポーネント設計
- システム設計
- 状態機械設計
- データフロー図

Phase 2: コンポーネント実装（エージェント: コーダー）
タスク: EnemyAI コンポーネントを実装
成果物: include/TD/Components/EnemyAI.h
内容:
- EnemyAI 構造体定義（POD型）
- 状態enum定義
- 必要なデータメンバー

Phase 3: システム実装（エージェント: コーダー）
タスク: EnemyAISystem を実装
成果物: include/TD/Systems/EnemyAISystem.h
内容:
- Core::ISystem 継承
- Update メソッド実装
- 状態機械ロジック

Phase 4: リファクタリング（エージェント: リファクター）
タスク: コードレビューと改善
成果物: 改善されたコード
チェック項目:
- 命名規則準拠
- DI パターン使用
- エラーハンドリング

Phase 5: ドキュメント（エージェント: ドキュメンター）
タスク: API ドキュメント作成
成果物: docs/API/EnemyAISystem.md
内容:
- 使用方法
- サンプルコード
- 注意事項

Phase 6: テスト（エージェント: テスター）
タスク: 単体テスト作成
成果物: tests/TD/Systems/EnemyAISystemTest.cpp
内容:
- 正常系テスト
- 異常系テスト
- エッジケーステスト
```

### 6.2 タスクテンプレート

#### テンプレート1: 新機能実装

```markdown
## タスク: [機能名] 実装

### 目的
[機能の目的を1-2文で説明]

### 担当エージェント
[アーキテクト / コーダー / リファクター / ドキュメンター / テスター]

### 前提条件
- [ ] [依存する他のタスクが完了している]
- [ ] [必要なライブラリがインストールされている]

### 成果物
- [ファイル名1]: [説明]
- [ファイル名2]: [説明]

### 実装内容
1. [ステップ1]
2. [ステップ2]
3. [ステップ3]

### 制約条件
- [制約1]
- [制約2]

### チェックリスト
- [ ] コーディング規約に準拠
- [ ] エラーハンドリング実装
- [ ] ビルド成功
- [ ] 動作確認
```

#### テンプレート2: リファクタリング

```markdown
## タスク: [対象コード] リファクタリング

### 目的
[リファクタリングの目的]

### 担当エージェント
リファクター

### 対象ファイル
- [ファイル名1]
- [ファイル名2]

### 改善内容
- [ ] [改善項目1]
- [ ] [改善項目2]

### 制約条件
- 既存の動作を維持
- 外部APIを変更しない

### チェックリスト
- [ ] リファクタリング前のテスト作成
- [ ] リファクタリング実施
- [ ] テスト全てパス
- [ ] コードレビュー
```

### 6.3 並列実行可能なタスク

以下のタスクは独立しており、複数のエージェントで並列実行可能：

```
┌─────────────────────┐  ┌─────────────────────┐  ┌─────────────────────┐
│ タスクA:            │  │ タスクB:            │  │ タスクC:            │
│ MovementSystem実装  │  │ RenderSystem実装    │  │ CollisionSystem実装 │
│                     │  │                     │  │                     │
│ エージェント: コーダー1 │  │ エージェント: コーダー2 │  │ エージェント: コーダー3 │
└─────────────────────┘  └─────────────────────┘  └─────────────────────┘
         ↓                        ↓                        ↓
┌───────────────────────────────────────────────────────────────────────┐
│ タスクD: 統合とテスト                                                    │
│ エージェント: テスター                                                   │
└───────────────────────────────────────────────────────────────────────┘
```

### 6.4 順次実行が必要なタスク

依存関係があるタスクは順次実行：

```
┌─────────────────────┐
│ タスクA:            │
│ コンポーネント設計   │
│ エージェント: アーキテクト │
└─────────────────────┘
         ↓
┌─────────────────────┐
│ タスクB:            │
│ コンポーネント実装   │
│ エージェント: コーダー │
└─────────────────────┘
         ↓
┌─────────────────────┐
│ タスクC:            │
│ システム実装        │
│ エージェント: コーダー │
└─────────────────────┘
         ↓
┌─────────────────────┐
│ タスクD:            │
│ テスト作成          │
│ エージェント: テスター │
└─────────────────────┘
```

---

## 7. コーディング規約と実装例

### 7.1 命名規則

```cpp
// クラス名: PascalCase
class GameManager {};
class EnemyAISystem {};

// 関数名: PascalCase
void UpdatePosition(float x, float y);
bool IsAlive() const;

// 変数名: camelCase
int playerHealth = 100;
float deltaTime = 0.016f;

// プライベートメンバー: 末尾に _
class MyClass {
private:
    int value_;
    std::string name_;
};

// 定数: UPPER_CASE
const int MAX_ENEMIES = 100;
constexpr float PI = 3.14159f;

// 名前空間: PascalCase
namespace Components {}
namespace Systems {}
namespace TD {}
```

### 7.2 ファイル構成

```
include/
├── Core/                      # コア機能
│   ├── Components/
│   │   ├── Transform.h       # Transform コンポーネント
│   │   └── Renderable.h      # Renderable コンポーネント
│   ├── Systems/
│   │   └── ...
│   ├── DI/
│   │   └── ServiceLocator.h  # DI コンテナ（将来）
│   └── System.h              # ISystem インターフェース
├── Game/                      # ゲーム層
│   ├── Sprite/
│   ├── Animation/
│   └── Scene/
├── TD/                        # タワーディフェンス
│   ├── Components/
│   │   ├── Tower.h
│   │   ├── Enemy.h
│   │   └── Projectile.h
│   └── Systems/
│       ├── TowerSystem.h
│       ├── EnemySystem.h
│       └── ProjectileSystem.h
├── Roguelike/                 # ローグライク
│   ├── Components/
│   ├── Systems/
│   └── Generation/
├── Data/
│   └── EntityFactory.h        # JSON → Entity 変換
└── ComponentsNew.h            # コンポーネント統合ヘッダー
```

### 7.3 コメント規約

```cpp
/**
 * @brief システムの簡潔な説明
 * 
 * 詳細な説明をここに書く。複数行にわたっても良い。
 * 
 * @note 特記事項があればここに
 */
class MySystem : public Core::ISystem {
public:
    /**
     * @brief メソッドの説明
     * @param registry エンティティレジストリ
     * @param deltaTime フレーム間の経過時間（秒）
     */
    void Update(entt::registry& registry, float deltaTime) override;
    
private:
    // シンプルなコメント: 処理の意図を説明
    int counter_;  ///< カウンター（ドキュメント用インラインコメント）
};
```

### 7.4 エラーハンドリング実装例

```cpp
// ファイル読み込み
std::optional<nlohmann::json> LoadJsonFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return std::nullopt;
        }
        
        nlohmann::json j = nlohmann::json::parse(file);
        return j;
        
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
        return std::nullopt;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading " << path << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}

// 使用例
void LoadConfig() {
    auto config = LoadJsonFile("assets/config.json");
    if (config) {
        width_ = config->value("width", 800);
        height_ = config->value("height", 600);
    } else {
        // デフォルト値を使用
        width_ = 800;
        height_ = 600;
    }
}
```

### 7.5 メモリ管理

```cpp
// ✅ 推奨: スマートポインタ使用
class GameNew {
private:
    std::unique_ptr<ResourceManager> resourceManager_;
    std::vector<std::unique_ptr<Core::ISystem>> systems_;
    
public:
    GameNew() {
        resourceManager_ = std::make_unique<ResourceManager>();
        systems_.push_back(std::make_unique<MovementSystem>());
    }
    
    // デストラクタ不要（自動解放）
};

// ❌ 避ける: 生ポインタ
class BadExample {
private:
    ResourceManager* resourceManager_;  // NG
    
public:
    BadExample() {
        resourceManager_ = new ResourceManager();  // NG
    }
    
    ~BadExample() {
        delete resourceManager_;  // 忘れがち
    }
};
```

---

## 8. トラブルシューティング

### 8.1 ビルドエラー

#### エラー: "EnTT not found"

```bash
# 原因: FetchContent がライブラリを取得できていない

# 解決策1: ビルドキャッシュをクリア
rm -rf build
cmake --preset ninja
cmake --build --preset ninja-release

# 解決策2: 手動でライブラリを確認
ls build/_deps/
# entt-src, json-src, raylib-src が存在するか確認
```

#### エラー: "Singleton::GetInstance() not found"

```cpp
// 原因: Singleton を DI に変更中に参照が残っている

// 解決策: 全ファイルで検索して置き換え
// Cursor で Ctrl+Shift+F で検索

// Before:
auto& config = ConfigManager::GetInstance();

// After:
// コンストラクタで受け取る
class MySystem {
private:
    ConfigManager& config_;
public:
    MySystem(ConfigManager& config) : config_(config) {}
};
```

#### エラー: "Components.h: No such file"

```cpp
// 原因: ComponentsNew.h に移行中

// 解決策: インクルードを変更
// Before:
#include "Components.h"

// After:
#include "ComponentsNew.h"
```

### 8.2 実行時エラー

#### エラー: "Failed to load config.json"

```bash
# 原因: 実行ディレクトリに assets/ がない

# 解決策: ビルド後に assets/ が自動コピーされているか確認
ls build/ninja/bin/assets/

# 手動でコピー
cp -r assets build/ninja/bin/
```

#### エラー: "Texture not found"

```cpp
// 原因: リソースパスが間違っている

// 解決策: ResourceManager のデバッグ出力を有効化
void LoadTexture(const std::string& name, const std::string& path) {
    std::cout << "Loading texture: " << name << " from " << path << std::endl;
    
    if (!FileExists(path)) {
        std::cerr << "File not found: " << path << std::endl;
        return;
    }
    // ...
}
```

### 8.3 Cursor 固有の問題

#### 問題: AI が古いコードを参照する

```
症状: AI が Components.h を使ったコードを生成する

解決策:
1. .cursorrules に明記
   「ComponentsNew.h を使用してください」

2. プロンプトに明示
   「Components.h ではなく ComponentsNew.h を使ってください」

3. Cursor のインデックスを再構築
   Cmd/Ctrl + Shift + P → "Cursor: Reindex"
```

#### 問題: マルチエージェントでコンテキストが失われる

```
症状: 2つ目のエージェントが最初の設計を無視する

解決策:
1. 前の出力を明示的に次のプロンプトに含める

2. プロジェクトのドキュメントを参照させる
   「docs/ENEMY_AI_DESIGN.md の設計に従ってください」

3. .cursorrules に重要な制約を書く
```

---

## 9. よくある質問

### Q1: どのビルドターゲットを使うべきですか？

**A:** 新規開発では必ず `SimpleTDCGame_NewArch` を使用してください。

```bash
# ビルド
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# 実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe
```

### Q2: Components.h と ComponentsNew.h の違いは？

**A:** 
- `Components.h`: 旧アーキテクチャ用（非推奨）
- `ComponentsNew.h`: 新アーキテクチャ用（推奨）

新規コードでは必ず `ComponentsNew.h` を使用してください。

### Q3: Singleton を使っても良いですか？

**A:** いいえ。新アーキテクチャでは依存性注入（DI）パターンを使用してください。

```cpp
// ❌ 非推奨
auto& config = ConfigManager::GetInstance();

// ✅ 推奨
class MySystem {
private:
    ConfigManager& config_;
public:
    MySystem(ConfigManager& config) : config_(config) {}
};
```

### Q4: Cursor で複数のエージェントを使うコツは？

**A:** 
1. 各エージェントに明確な役割を与える
2. 前のエージェントの出力を次に明示的に渡す
3. タスクを小さく分割する
4. .cursorrules に重要な制約を書く

### Q5: JSON解析でエラーが出ます

**A:** JSON解析は必ず try-catch で囲んでください：

```cpp
try {
    nlohmann::json j = nlohmann::json::parse(file);
    // 処理
} catch (const nlohmann::json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    // フォールバック処理
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### Q6: Roguelike ゲームの実装状況は？

**A:** 設計は完了していますが、実装はほぼ未着手です。

実装ロードマップ:
- Phase 1: 基本移動とターン制（1週間）
- Phase 2: ダンジョン生成と視界（1週間）
- Phase 3: モンスターと戦闘（2週間）

詳細は `docs/ROGUELIKE_SYSTEM_DESIGN.md` を参照してください。

### Q7: テストはどうやって書きますか？

**A:** Google Test の導入を予定していますが、現在は未整備です。

将来の計画:
```cpp
// tests/TD/Systems/MovementSystemTest.cpp
#include <gtest/gtest.h>
#include "TD/Systems/MovementSystem.h"

TEST(MovementSystemTest, EntityMovesCorrectly) {
    entt::registry registry;
    auto entity = registry.create();
    
    registry.emplace<Components::Transform>(entity, 0.0f, 0.0f);
    registry.emplace<Components::Velocity>(entity, 1.0f, 0.0f);
    
    Systems::MovementSystem system;
    system.Update(registry, 1.0f);
    
    auto& transform = registry.get<Components::Transform>(entity);
    EXPECT_FLOAT_EQ(transform.x, 1.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
}
```

### Q8: ドキュメントはどこにありますか？

**A:** `docs/` ディレクトリに全てのドキュメントがあります：

必読:
- `docs/EXECUTIVE_SUMMARY.md` - プロジェクト概要
- `docs/CODE_ANALYSIS.md` - 詳細分析
- `docs/REFACTORING_PLAN.md` - 改善計画
- `docs/README.md` - ドキュメント索引

### Q9: パフォーマンスが悪いです

**A:** 以下を確認してください：

1. Release ビルドを使用
   ```bash
   cmake --build --preset ninja-release
   ```

2. V-Sync を有効化
   ```cpp
   SetTargetFPS(60);
   ```

3. プロファイリング（将来実装予定）
   - Tracy 統合
   - パフォーマンス測定

### Q10: 新しいライブラリを追加したいです

**A:** CMakeLists.txt で FetchContent を使用してください：

```cmake
FetchContent_Declare(
    NewLibrary
    GIT_REPOSITORY https://github.com/user/newlibrary.git
    GIT_TAG v1.0.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(NewLibrary)

target_link_libraries(SimpleTDCGame_NewArch PRIVATE NewLibrary)
```

---

## 付録A: クイックリファレンス

### ビルドコマンド

```bash
# 初回セットアップ
cmake --preset ninja

# ビルド（新アーキテクチャ）
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# ビルド（旧アーキテクチャ）
cmake --build --preset ninja-release --target SimpleTDCGame

# ビルド（Roguelike）
cmake --build --preset ninja-release --target NetHackGame

# クリーンビルド
rm -rf build
cmake --preset ninja
cmake --build --preset ninja-release
```

### よく使うディレクトリ

```
プロジェクトルート/
├── .cursor/              # Cursor IDE 設定
├── docs/                 # ドキュメント
├── include/              # ヘッダーファイル
│   ├── Core/
│   ├── Game/
│   ├── TD/
│   └── Roguelike/
├── src/                  # ソースファイル
├── assets/               # アセット
│   ├── config.json
│   ├── fonts/
│   └── textures/
└── build/                # ビルド出力（Git無視）
```

### 重要なファイル

| ファイル | 用途 |
|---------|------|
| .cursorrules | Cursor プロジェクトルール |
| .cursor/CURSOR_DEVELOPMENT_GUIDE.md | 本ガイド |
| .github/copilot-instructions.md | コーディング規約 |
| docs/EXECUTIVE_SUMMARY.md | プロジェクト概要 |
| include/ComponentsNew.h | コンポーネント定義 |
| include/Core/System.h | システムインターフェース |

---

## 付録B: 推奨リソース

### 公式ドキュメント

- [EnTT Wiki](https://github.com/skypjack/entt/wiki) - ECSライブラリ
- [Raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [nlohmann/json](https://json.nlohmann.me/)

### 学習リソース

- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - ゲームデザインパターン
- [Refactoring Guru](https://refactoring.guru/) - リファクタリング
- [RogueBasin](http://www.roguebasin.com/) - ローグライク開発

### Cursor 関連

- [Cursor Documentation](https://docs.cursor.com/)
- [Cursor AI Best Practices](https://docs.cursor.com/get-started/best-practices)

---

## 付録C: 変更履歴

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2025-12-03 | 1.0 | 初版作成 |

---

**このドキュメントは Cursor IDE でのマルチエージェント開発を最適化するために作成されました。**  
**質問やフィードバックがあれば、GitHub Issues でお知らせください。**

