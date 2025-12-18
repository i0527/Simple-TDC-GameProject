# キャラクター・ステージ定義システム設計書

> **⚠️ 設計変更通知**  
> **キャラクター描画システムの設計が変更されました。**  
> 最新の設計については [`docs/CHARACTER_RENDERING_DESIGN.md`](../CHARACTER_RENDERING_DESIGN.md) を参照してください。  
> 
> **主な変更点**:
>
> - メインキャラクター: セルアニメ形式 → 256×256 スプライトシート
> - サブキャラクター: 可変スプライトシート → 128×128以下 スプライトシート統一
> - 描画パイプライン: 形式分岐 → 統一パイプライン（レイヤー分離アーキテクチャ）
>
> 本ドキュメントは旧設計の参考資料として保持されています。

## 概要

本ドキュメントは、にゃんこ大戦争風タワーディフェンスゲームにおけるキャラクター・ステージ定義システムの設計仕様を定義する。  
キャラクターのアニメーション、スキル、効果範囲などをJSON形式で外部定義し、ノードベースWebUIエディタで編集可能なシステムを構築する。

---

## 0. 現状分析と課題

### 0.1 現在のアーキテクチャ

```
┌─────────────────────────────────────────────────────────────────┐
│                          Game                                   │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ SceneManager    │  │ ConfigManager   │  │ InputManager    │  │
│  │ (Singleton)     │  │ (Singleton)     │  │ (Singleton)     │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
│  ┌─────────────────┐  ┌─────────────────┐                       │
│  │ ResourceManager │  │ UIManager       │                       │
│  │ (Singleton)     │  │ (Singleton)     │                       │
│  └─────────────────┘  └─────────────────┘                       │
│                              │                                   │
│                    ┌─────────▼─────────┐                        │
│                    │ entt::registry    │                        │
│                    │ (Game メンバー)    │                        │
│                    └───────────────────┘                        │
└─────────────────────────────────────────────────────────────────┘
```

### 0.2 現在の問題点

| 問題 | 詳細 | 影響 |
|------|------|------|
| **Singleton 過多** | 5つ以上のSingletonマネージャー | テスト困難、依存関係が暗黙的 |
| **シーン責務過大** | `IScene` 内でシステム呼び出し・入力処理・描画を直接実装 | コード重複、再利用性低下 |
| **システム二重定義** | `Systems.h` にクラス版と静的関数版が混在 | 混乱、保守性低下 |
| **エンティティ生成散在** | シーン内でハードコード | 新キャラ追加が困難 |
| **コンポーネント肥大化** | `SpriteAnimation` が多くのフィールドを持つ | 単一責任原則違反 |
| **イベント機構なし** | システム間通信が直接呼び出し | 疎結合化困難 |

### 0.3 改善方針

1. **依存性注入 (DI)**: Singletonを減らし、コンストラクタ経由で依存を注入
2. **レイヤー分離**: Core / Game / TD の3層に分離
3. **システム統一**: 静的関数方式に統一（EnTT推奨パターン）
4. **ファクトリパターン**: エンティティ生成を `EntityFactory` に集約
5. **イベント駆動**: `entt::dispatcher` でシステム間通信
6. **データ駆動**: JSON定義からエンティティを生成

---

## 1. システムアーキテクチャ

```
┌─────────────────────────────────────────────────────────────────┐
│                      WebUI Editor (Node-based)                  │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐  │
│  │ Character   │───▶│ Animation   │───▶│ Skill / Effect Area │  │
│  │ Properties  │    │ Timeline    │    │ Node Editor         │  │
│  └─────────────┘    └─────────────┘    └─────────────────────┘  │
│                              │                                   │
│                              ▼                                   │
│                    ┌─────────────────┐                          │
│                    │  JSON Export    │                          │
│                    └─────────────────┘                          │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                     assets/definitions/                         │
│  ├── characters/                                                │
│  │   ├── cupslime.character.json                                │
│  │   └── warrior_cat.character.json                             │
│  ├── stages/                                                    │
│  │   └── stage_01.stage.json                                    │
│  └── schemas/                                                   │
│      ├── character.schema.json                                  │
│      └── stage.schema.json                                      │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                     C++ Game Engine                             │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────┐  │
│  │ CharacterLoader │───▶│ EntityFactory   │───▶│ ECS Registry│  │
│  └─────────────────┘    └─────────────────┘    └─────────────┘  │
│  ┌─────────────────┐    ┌─────────────────┐                     │
│  │ StageLoader     │───▶│ WaveManager     │                     │
│  └─────────────────┘    └─────────────────┘                     │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 新アーキテクチャ（提案）

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Application Layer                               │
│  ┌─────────────────────────────────────────────────────────────────────────┐│
│  │                              Game                                        ││
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐                   ││
│  │  │ GameContext  │  │ World        │  │ SystemRunner │                   ││
│  │  │ (DI Container│  │ (registry +  │  │ (実行順管理)  │                   ││
│  │  │  + Services) │  │  dispatcher) │  │              │                   ││
│  │  └──────────────┘  └──────────────┘  └──────────────┘                   ││
│  └─────────────────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
          ┌───────────────────────────┼───────────────────────────┐
          ▼                           ▼                           ▼
┌─────────────────────┐   ┌─────────────────────┐   ┌─────────────────────┐
│    Core Layer       │   │    Game Layer       │   │     TD Layer        │
├─────────────────────┤   ├─────────────────────┤   ├─────────────────────┤
│ • Components (基本) │   │ • Components (ゲーム)│   │ • Components (TD)   │
│   - Position        │   │   - SpriteAnimation │   │   - Stats           │
│   - Velocity        │   │   - SpriteFrame     │   │   - Combat          │
│   - Scale           │   │   - SpriteTexture   │   │   - Team            │
│                     │   │                     │   │   - UnitState       │
│ • Systems (基本)    │   │ • Systems (ゲーム)  │   │ • Systems (TD)      │
│   - MovementSystem  │   │   - AnimationSystem │   │   - TargetingSystem │
│   - InputSystem     │   │   - SpriteRender    │   │   - CombatSystem    │
│                     │   │                     │   │   - UnitAISystem    │
│ • Services          │   │ • Loaders           │   │ • Managers          │
│   - ResourceManager │   │   - CharacterLoader │   │   - WaveManager     │
│   - ConfigManager   │   │   - StageLoader     │   │   - BattleManager   │
│   - SceneManager    │   │   - EntityFactory   │   │                     │
└─────────────────────┘   └─────────────────────┘   └─────────────────────┘
```

### 1.3 主要クラス設計

#### GameContext（依存性注入コンテナ）

```cpp
// include/Core/GameContext.h
class GameContext {
public:
    // サービス登録（Singleton ではなくインスタンス管理）
    template<typename T, typename... Args>
    T& Register(Args&&... args);
    
    // サービス取得
    template<typename T>
    T& Get();
    
    // 例: context.Get<Resources::ResourceManager>()
    
private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};
```

#### World（ECS ワールド）

```cpp
// include/Core/World.h
class World {
public:
    entt::registry& GetRegistry() { return registry_; }
    entt::dispatcher& GetDispatcher() { return dispatcher_; }
    
    // エンティティ操作ヘルパー
    entt::entity CreateEntity();
    void DestroyEntity(entt::entity entity);
    
    // イベント発行ヘルパー
    template<typename Event>
    void Emit(const Event& event);
    
private:
    entt::registry registry_;
    entt::dispatcher dispatcher_;
};
```

#### SystemRunner（システム実行管理）

```cpp
// include/Core/SystemRunner.h
class SystemRunner {
public:
    // システム登録（実行順序付き）
    enum class Phase { PreUpdate, Update, PostUpdate, Render };
    
    using SystemFunc = std::function<void(World&, float)>;
    void Register(Phase phase, SystemFunc system);
    
    // 実行
    void RunUpdate(World& world, float deltaTime);
    void RunRender(World& world);
    
private:
    std::map<Phase, std::vector<SystemFunc>> systems_;
};
```

### 1.4 イベント定義（EnTT Dispatcher 用）

```cpp
// include/Events/GameEvents.h
namespace Events {
    // ユニット関連
    struct UnitSpawned {
        entt::entity entity;
        std::string characterId;
        Components::Team::Type team;
    };
    
    struct UnitDied {
        entt::entity entity;
        entt::entity killer;  // entt::null if none
    };
    
    struct DamageDealt {
        entt::entity attacker;
        entt::entity target;
        int damage;
        bool isCritical;
    };
    
    // 戦闘関連
    struct WaveStarted {
        int waveNumber;
    };
    
    struct WaveCompleted {
        int waveNumber;
    };
    
    struct BattleEnded {
        bool playerWon;
    };
    
    // アニメーション関連
    struct AnimationFinished {
        entt::entity entity;
        std::string animationName;
    };
    
    struct AnimationHitFrame {
        entt::entity entity;
        std::string animationName;
        int frameIndex;
    };
}
```

### 1.5 コンポーネント分離設計

```cpp
// include/Core/Components.h - 汎用コンポーネント
namespace Core::Components {
    struct Position { float x, y; };
    struct Velocity { float dx, dy; };
    struct Scale { float x, y; };
    struct Rotation { float angle; };
}

// include/Game/Components.h - ゲーム基盤コンポーネント
namespace Game::Components {
    // スプライト参照（軽量）
    struct Sprite {
        std::string textureName;
        std::string currentFrame;
        Rectangle sourceRect;
    };
    
    // アニメーション状態（分離）
    struct AnimationState {
        std::string currentAnimation;
        size_t frameIndex;
        float elapsedTime;
        bool isPlaying;
    };
    
    // アニメーション定義参照（定義データへのポインタ）
    struct AnimationRef {
        const CharacterDefinition* definition;  // 読み取り専用参照
    };
}

// include/TD/Components.h - タワーディフェンス専用
namespace TD::Components {
    struct Stats {
        int hp, maxHp;
        int attack, defense;
        float moveSpeed, attackSpeed;
        float knockbackResistance;
    };
    
    struct Combat {
        enum class AttackType { Melee, Ranged };
        AttackType type;
        float range;
        float cooldown;
        float currentCooldown;
        entt::entity target;
    };
    
    struct Team {
        enum class Type { Player, Enemy };
        Type team;
    };
    
    struct UnitState {
        enum class State { Idle, Walking, Attacking, Stunned, Dead };
        State current;
        float stateTimer;
    };
    
    struct Hitbox {
        float width, height;
        float offsetX, offsetY;
    };
    
    // 拠点マーカー
    struct Base { int hp; int maxHp; };
    
    // ユニットマーカー
    struct Unit { std::string characterId; };
}
```

---

## 2. ディレクトリ構造（新アーキテクチャ対応）

```
Simple-TDC-GameProject/
├── assets/
│   ├── config.json                    # ゲーム全体設定
│   ├── atlas/                         # スプライトシート画像
│   │   ├── characters/
│   │   ├── effects/
│   │   └── ui/
│   ├── json/                          # Aseprite出力JSON（スプライト座標）
│   │   ├── characters/
│   │   └── effects/
│   ├── definitions/                   # ゲーム定義JSON
│   │   ├── characters/
│   │   │   └── *.character.json
│   │   ├── stages/
│   │   │   └── *.stage.json
│   │   └── schemas/
│   │       ├── character.schema.json
│   │       └── stage.schema.json
│   └── fonts/
│
├── include/
│   ├── Core/                          # ★ コア層（汎用）
│   │   ├── GameContext.h              # DI コンテナ
│   │   ├── World.h                    # ECS World ラッパー
│   │   ├── SystemRunner.h             # システム実行管理
│   │   ├── Components.h               # 基本コンポーネント
│   │   └── Events.h                   # イベント定義
│   │
│   ├── Game/                          # ★ ゲーム基盤層
│   │   ├── Components.h               # ゲームコンポーネント
│   │   ├── Systems/
│   │   │   ├── AnimationSystem.h
│   │   │   ├── SpriteRenderSystem.h
│   │   │   └── MovementSystem.h
│   │   ├── Loaders/
│   │   │   ├── CharacterLoader.h
│   │   │   └── StageLoader.h
│   │   └── EntityFactory.h
│   │
│   ├── TD/                            # ★ タワーディフェンス層
│   │   ├── Components.h               # TD専用コンポーネント
│   │   ├── Systems/
│   │   │   ├── TargetingSystem.h
│   │   │   ├── CombatSystem.h
│   │   │   ├── UnitAISystem.h
│   │   │   ├── DamageSystem.h
│   │   │   └── DeathSystem.h
│   │   ├── Managers/
│   │   │   ├── WaveManager.h
│   │   │   └── BattleManager.h
│   │   └── TDScene.h
│   │
│   ├── Services/                      # ★ サービス層（Singleton → DI）
│   │   ├── ResourceManager.h          # 既存を移動
│   │   ├── ConfigManager.h            # 既存を移動
│   │   ├── InputManager.h             # 既存を移動
│   │   └── SceneManager.h             # 既存を移動
│   │
│   └── UI/                            # UI層（既存維持）
│       └── UIManager.h
│
├── src/
│   ├── Core/
│   │   ├── GameContext.cpp
│   │   ├── World.cpp
│   │   └── SystemRunner.cpp
│   ├── Game/
│   │   ├── Systems/
│   │   ├── Loaders/
│   │   └── EntityFactory.cpp
│   ├── TD/
│   │   ├── Systems/
│   │   ├── Managers/
│   │   └── TDScene.cpp
│   ├── Services/
│   └── main.cpp
│
├── tools/                             # WebUIエディタ
│   └── entity-editor/
│       ├── package.json
│       ├── vite.config.ts
│       └── src/
│           ├── App.tsx
│           ├── components/
│           ├── nodes/
│           └── stores/
│
└── docs/
    └── CHARACTER_SYSTEM_DESIGN.md
```

### 2.1 レイヤー依存関係

```
        ┌─────────┐
        │   TD    │  ← タワーディフェンス固有ロジック
        └────┬────┘
             │ uses
        ┌────▼────┐
        │  Game   │  ← ゲーム共通機能（アニメーション、スプライト）
        └────┬────┘
             │ uses
        ┌────▼────┐
        │  Core   │  ← 汎用ECS・イベント・DI
        └────┬────┘
             │ uses
        ┌────▼────┐
        │Services │  ← リソース管理・設定・入力
        └─────────┘
```

**ルール**:

- 上位レイヤーは下位レイヤーに依存可能
- 下位レイヤーは上位レイヤーに依存不可
- 同一レイヤー内は相互参照可能（ただしインターフェース経由推奨）

---

## 3. JSONスキーマ定義

### 3.1 キャラクター定義 (`*.character.json`)

```json
{
  "$schema": "../schemas/character.schema.json",
  "id": "cupslime",
  "name": "カップスライム",
  "description": "基本的な近接ユニット",
  
  "sprite": {
    "atlasPath": "characters/cupslime.png",
    "jsonPath": "characters/cupslime.json"
  },
  
  "animations": {
    "idle": {
      "frames": ["cupslime_idle_0", "cupslime_idle_1", "cupslime_idle_2"],
      "frameRate": 8,
      "loop": true
    },
    "walk": {
      "frames": ["cupslime_walk_0", "cupslime_walk_1", "cupslime_walk_2", "cupslime_walk_3"],
      "frameRate": 12,
      "loop": true
    },
    "attack": {
      "frames": ["cupslime_atk_0", "cupslime_atk_1", "cupslime_atk_2"],
      "frameRate": 15,
      "loop": false,
      "hitFrame": 2
    },
    "death": {
      "frames": ["cupslime_death_0", "cupslime_death_1", "cupslime_death_2"],
      "frameRate": 10,
      "loop": false
    }
  },
  
  "stats": {
    "hp": 100,
    "attack": 15,
    "defense": 5,
    "moveSpeed": 120.0,
    "attackSpeed": 1.0,
    "knockbackResistance": 0.0
  },
  
  "combat": {
    "attackType": "melee",
    "attackRange": 50.0,
    "attackCooldown": 1.5
  },
  
  "hitbox": {
    "type": "rectangle",
    "width": 32,
    "height": 48,
    "offsetX": 0,
    "offsetY": 0
  },
  
  "skills": [
    {
      "id": "slime_punch",
      "name": "スライムパンチ",
      "description": "前方に強力なパンチを繰り出す",
      "cooldown": 5.0,
      "damage": 25,
      "damageMultiplier": 1.5,
      "animation": "attack",
      "effectArea": {
        "type": "rectangle",
        "width": 60,
        "height": 40,
        "offsetX": 30,
        "offsetY": 0
      },
      "effects": []
    }
  ],
  
  "unitInfo": {
    "cost": 100,
    "rechargeTime": 3.0,
    "rarity": "common",
    "category": "basic"
  },
  
  "tags": ["melee", "basic", "slime"]
}
```

### 3.2 ステージ定義 (`*.stage.json`)

```json
{
  "$schema": "../schemas/stage.schema.json",
  "id": "stage_01",
  "name": "はじまりの草原",
  "description": "チュートリアルステージ",
  
  "layout": {
    "width": 1600,
    "height": 600,
    "groundY": 450
  },
  
  "background": {
    "imagePath": "backgrounds/meadow.png",
    "scrollSpeed": 0.0
  },
  
  "bases": {
    "player": {
      "x": 50,
      "y": 450,
      "hp": 10000,
      "sprite": "player_base"
    },
    "enemy": {
      "x": 1550,
      "y": 450,
      "hp": 8000,
      "sprite": "enemy_base"
    }
  },
  
  "waves": [
    {
      "waveNumber": 1,
      "startDelay": 2.0,
      "enemies": [
        {
          "characterId": "cupslime",
          "count": 3,
          "spawnInterval": 1.5,
          "spawnDelay": 0.0
        }
      ]
    },
    {
      "waveNumber": 2,
      "startDelay": 10.0,
      "enemies": [
        {
          "characterId": "cupslime",
          "count": 5,
          "spawnInterval": 1.0,
          "spawnDelay": 0.0
        },
        {
          "characterId": "warrior_cat",
          "count": 2,
          "spawnInterval": 2.0,
          "spawnDelay": 3.0
        }
      ]
    }
  ],
  
  "rewards": {
    "clearGold": 500,
    "firstClearBonus": 1000
  },
  
  "conditions": {
    "victoryCondition": "destroy_enemy_base",
    "defeatCondition": "player_base_destroyed",
    "timeLimit": 0
  }
}
```

### 3.3 JSON Schema（バリデーション用）

#### `character.schema.json`

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "character.schema.json",
  "title": "Character Definition",
  "type": "object",
  "required": ["id", "name", "sprite", "animations", "stats", "combat", "hitbox", "unitInfo"],
  "properties": {
    "id": {
      "type": "string",
      "pattern": "^[a-z][a-z0-9_]*$",
      "description": "キャラクター識別子（英小文字・数字・アンダースコア）"
    },
    "name": {
      "type": "string",
      "description": "表示名"
    },
    "description": {
      "type": "string",
      "default": ""
    },
    "sprite": {
      "type": "object",
      "required": ["atlasPath", "jsonPath"],
      "properties": {
        "atlasPath": { "type": "string" },
        "jsonPath": { "type": "string" }
      }
    },
    "animations": {
      "type": "object",
      "additionalProperties": {
        "$ref": "#/definitions/Animation"
      }
    },
    "stats": {
      "$ref": "#/definitions/Stats"
    },
    "combat": {
      "$ref": "#/definitions/Combat"
    },
    "hitbox": {
      "$ref": "#/definitions/RectangleArea"
    },
    "skills": {
      "type": "array",
      "items": { "$ref": "#/definitions/Skill" },
      "default": []
    },
    "unitInfo": {
      "$ref": "#/definitions/UnitInfo"
    },
    "tags": {
      "type": "array",
      "items": { "type": "string" },
      "default": []
    }
  },
  "definitions": {
    "Animation": {
      "type": "object",
      "required": ["frames", "frameRate", "loop"],
      "properties": {
        "frames": {
          "type": "array",
          "items": { "type": "string" },
          "minItems": 1
        },
        "frameRate": {
          "type": "number",
          "minimum": 1,
          "maximum": 60
        },
        "loop": { "type": "boolean" },
        "hitFrame": {
          "type": "integer",
          "minimum": 0,
          "description": "攻撃判定が発生するフレーム番号"
        }
      }
    },
    "Stats": {
      "type": "object",
      "required": ["hp", "attack", "defense", "moveSpeed", "attackSpeed"],
      "properties": {
        "hp": { "type": "integer", "minimum": 1 },
        "attack": { "type": "integer", "minimum": 0 },
        "defense": { "type": "integer", "minimum": 0 },
        "moveSpeed": { "type": "number", "minimum": 0 },
        "attackSpeed": { "type": "number", "minimum": 0.1 },
        "knockbackResistance": { "type": "number", "minimum": 0, "maximum": 1, "default": 0 }
      }
    },
    "Combat": {
      "type": "object",
      "required": ["attackType", "attackRange", "attackCooldown"],
      "properties": {
        "attackType": {
          "type": "string",
          "enum": ["melee", "ranged"]
        },
        "attackRange": { "type": "number", "minimum": 0 },
        "attackCooldown": { "type": "number", "minimum": 0.1 }
      }
    },
    "RectangleArea": {
      "type": "object",
      "required": ["type", "width", "height"],
      "properties": {
        "type": { "const": "rectangle" },
        "width": { "type": "number", "minimum": 1 },
        "height": { "type": "number", "minimum": 1 },
        "offsetX": { "type": "number", "default": 0 },
        "offsetY": { "type": "number", "default": 0 }
      }
    },
    "Skill": {
      "type": "object",
      "required": ["id", "name", "cooldown", "damage", "effectArea"],
      "properties": {
        "id": { "type": "string" },
        "name": { "type": "string" },
        "description": { "type": "string", "default": "" },
        "cooldown": { "type": "number", "minimum": 0 },
        "damage": { "type": "integer", "minimum": 0 },
        "damageMultiplier": { "type": "number", "minimum": 0, "default": 1.0 },
        "animation": { "type": "string" },
        "effectArea": { "$ref": "#/definitions/RectangleArea" },
        "effects": {
          "type": "array",
          "items": { "$ref": "#/definitions/StatusEffect" },
          "default": []
        }
      }
    },
    "StatusEffect": {
      "type": "object",
      "required": ["type", "duration"],
      "properties": {
        "type": {
          "type": "string",
          "enum": ["slow", "stun", "poison", "buff_attack", "buff_defense"]
        },
        "duration": { "type": "number", "minimum": 0 },
        "value": { "type": "number", "default": 0 }
      }
    },
    "UnitInfo": {
      "type": "object",
      "required": ["cost", "rechargeTime"],
      "properties": {
        "cost": { "type": "integer", "minimum": 0 },
        "rechargeTime": { "type": "number", "minimum": 0 },
        "rarity": {
          "type": "string",
          "enum": ["common", "uncommon", "rare", "epic", "legendary"],
          "default": "common"
        },
        "category": {
          "type": "string",
          "default": "basic"
        }
      }
    }
  }
}
```

---

## 4. C++ 実装設計（新アーキテクチャ）

### 4.1 Core レイヤー

#### GameContext（依存性注入コンテナ）

```cpp
// include/Core/GameContext.h
#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

namespace Core {

class GameContext {
public:
    // サービス登録
    template<typename T, typename... Args>
    T& Register(Args&&... args) {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        services_[std::type_index(typeid(T))] = ptr;
        return *ptr;
    }
    
    // 既存インスタンスを登録
    template<typename T>
    void RegisterInstance(std::shared_ptr<T> instance) {
        services_[std::type_index(typeid(T))] = instance;
    }
    
    // サービス取得
    template<typename T>
    T& Get() {
        auto it = services_.find(std::type_index(typeid(T)));
        if (it == services_.end()) {
            throw std::runtime_error("Service not registered: " + std::string(typeid(T).name()));
        }
        return *std::static_pointer_cast<T>(it->second);
    }
    
    // サービス存在確認
    template<typename T>
    bool Has() const {
        return services_.find(std::type_index(typeid(T))) != services_.end();
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

} // namespace Core
```

#### World（ECS ワールドラッパー）

```cpp
// include/Core/World.h
#pragma once

#include <entt/entt.hpp>
#include <functional>

namespace Core {

class World {
public:
    // Registry アクセス
    entt::registry& Registry() { return registry_; }
    const entt::registry& Registry() const { return registry_; }
    
    // Dispatcher アクセス（イベント用）
    entt::dispatcher& Dispatcher() { return dispatcher_; }
    
    // エンティティ操作
    entt::entity Create() { return registry_.create(); }
    void Destroy(entt::entity entity) { registry_.destroy(entity); }
    bool Valid(entt::entity entity) const { return registry_.valid(entity); }
    
    // コンポーネント操作ヘルパー
    template<typename T, typename... Args>
    T& Emplace(entt::entity entity, Args&&... args) {
        return registry_.emplace<T>(entity, std::forward<Args>(args)...);
    }
    
    template<typename T>
    T* TryGet(entt::entity entity) {
        return registry_.try_get<T>(entity);
    }
    
    // イベント発行
    template<typename Event>
    void Emit(const Event& event) {
        dispatcher_.trigger(event);
    }
    
    template<typename Event>
    void Enqueue(const Event& event) {
        dispatcher_.enqueue(event);
    }
    
    void UpdateEvents() {
        dispatcher_.update();
    }
    
    // イベント購読
    template<typename Event, auto Func>
    void Subscribe() {
        dispatcher_.sink<Event>().template connect<Func>();
    }
    
    template<typename Event, typename T>
    void Subscribe(T& instance) {
        dispatcher_.sink<Event>().connect(instance);
    }

private:
    entt::registry registry_;
    entt::dispatcher dispatcher_;
};

} // namespace Core
```

#### SystemRunner（システム実行管理）

```cpp
// include/Core/SystemRunner.h
#pragma once

#include "World.h"
#include <functional>
#include <map>
#include <vector>
#include <string>

namespace Core {

class SystemRunner {
public:
    // システム実行フェーズ
    enum class Phase {
        PreUpdate,      // 入力処理、イベント処理
        Update,         // メインロジック
        PostUpdate,     // 後処理（死亡削除など）
        PreRender,      // 描画準備
        Render          // 描画
    };
    
    // システム関数型
    using UpdateSystem = std::function<void(World&, float)>;
    using RenderSystem = std::function<void(World&)>;
    
    // システム登録
    void RegisterUpdate(Phase phase, const std::string& name, UpdateSystem system) {
        updateSystems_[phase].push_back({name, system});
    }
    
    void RegisterRender(Phase phase, const std::string& name, RenderSystem system) {
        renderSystems_[phase].push_back({name, system});
    }
    
    // 実行
    void RunUpdate(World& world, float deltaTime) {
        for (auto phase : {Phase::PreUpdate, Phase::Update, Phase::PostUpdate}) {
            for (auto& [name, system] : updateSystems_[phase]) {
                system(world, deltaTime);
            }
        }
    }
    
    void RunRender(World& world) {
        for (auto phase : {Phase::PreRender, Phase::Render}) {
            for (auto& [name, system] : renderSystems_[phase]) {
                system(world);
            }
        }
    }

private:
    std::map<Phase, std::vector<std::pair<std::string, UpdateSystem>>> updateSystems_;
    std::map<Phase, std::vector<std::pair<std::string, RenderSystem>>> renderSystems_;
};

} // namespace Core
```

### 4.2 コンポーネント設計（レイヤー分離）

```cpp
// include/Core/Components.h - 汎用コンポーネント
#pragma once

namespace Core::Components {
    struct Position { float x, y; };
    struct Velocity { float dx, dy; };
    struct Scale { float x = 1.0f, y = 1.0f; };
    struct Rotation { float angle = 0.0f; };
    struct Active { bool value = true; };  // 有効/無効フラグ
}

// include/Game/Components.h - ゲーム基盤コンポーネント
#pragma once

#include <string>
#include <raylib.h>

namespace Game::Components {
    // スプライト描画用（軽量）
    struct Sprite {
        std::string textureName;
        Rectangle sourceRect;
    };
    
    // アニメーション状態（実行時データ）
    struct AnimationState {
        std::string currentAnimation;
        size_t frameIndex = 0;
        float elapsedTime = 0.0f;
        bool isPlaying = false;
        bool isLooping = true;
    };
    
    // キャラクター定義参照
    struct CharacterRef {
        std::string characterId;
    };
    
    // 描画順序
    struct RenderOrder {
        int layer = 0;      // レイヤー（背景:0, ユニット:1, UI:2）
        int order = 0;      // 同一レイヤー内順序
    };
    
    // 向き（左右反転用）
    struct Facing {
        enum class Direction { Left, Right };
        Direction direction = Direction::Right;
    };
}

// include/TD/Components.h - タワーディフェンス専用
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace TD::Components {
    // チーム識別
    struct Team {
        enum class Type { Player, Enemy };
        Type team;
    };
    
    // ステータス（実行時）
    struct Stats {
        int hp;
        int maxHp;
        int attack;
        int defense;
        float moveSpeed;
        float attackSpeed;
        float knockbackResistance;
    };
    
    // 戦闘情報
    struct Combat {
        enum class AttackType { Melee, Ranged };
        AttackType type;
        float range;
        float cooldown;
        float currentCooldown = 0.0f;
    };
    
    // ターゲット情報
    struct Target {
        entt::entity entity = entt::null;
        float distance = 0.0f;
    };
    
    // ユニット状態
    struct UnitState {
        enum class State { Idle, Walking, Attacking, Stunned, Dead };
        State current = State::Idle;
        State previous = State::Idle;
        float stateTimer = 0.0f;
    };
    
    // ヒットボックス
    struct Hitbox {
        float width;
        float height;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
    };
    
    // ノックバック
    struct Knockback {
        float distance;
        float duration;
        float timer = 0.0f;
        float direction;  // 1.0 = 右, -1.0 = 左
    };
    
    // スキル（実行時状態）
    struct SkillState {
        std::string skillId;
        float currentCooldown = 0.0f;
    };
    
    struct SkillSlots {
        std::vector<SkillState> skills;
    };
    
    // ユニット情報
    struct UnitInfo {
        int cost;
        float rechargeTime;
        std::string rarity;
        std::string category;
    };
    
    // === タグコンポーネント ===
    struct BaseTag {};         // 拠点
    struct UnitTag {};         // ユニット
    struct ProjectileTag {};   // 投射物
}
```

### 4.3 CharacterLoader（新設計）

```cpp
// include/Game/Loaders/CharacterLoader.h
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace Game {

// キャラクター定義（イミュータブル、共有可能）
struct CharacterDefinition {
    std::string id;
    std::string name;
    std::string description;
    
    struct SpriteInfo {
        std::string atlasPath;
        std::string jsonPath;
    } sprite;
    
    struct AnimationDef {
        std::vector<std::string> frames;
        float frameDuration;  // 1フレームの秒数
        bool loop;
        int hitFrame = -1;    // 攻撃判定フレーム（-1 = なし）
    };
    std::unordered_map<std::string, AnimationDef> animations;
    
    struct StatsDef {
        int hp;
        int attack;
        int defense;
        float moveSpeed;
        float attackSpeed;
        float knockbackResistance;
    } stats;
    
    struct CombatDef {
        std::string attackType;
        float attackRange;
        float attackCooldown;
    } combat;
    
    struct HitboxDef {
        float width;
        float height;
        float offsetX;
        float offsetY;
    } hitbox;
    
    struct SkillDef {
        std::string id;
        std::string name;
        float cooldown;
        int damage;
        float damageMultiplier;
        std::string animation;
        HitboxDef effectArea;
    };
    std::vector<SkillDef> skills;
    
    struct UnitInfoDef {
        int cost;
        float rechargeTime;
        std::string rarity;
        std::string category;
    } unitInfo;
    
    std::vector<std::string> tags;
};

class CharacterLoader {
public:
    // 単体読み込み
    bool Load(const std::string& characterId, 
              const std::string& basePath = "assets/definitions/characters/");
    
    // ディレクトリ内全読み込み
    int LoadAll(const std::string& directoryPath = "assets/definitions/characters/");
    
    // 取得（const参照、存在しなければnullptr）
    const CharacterDefinition* Get(const std::string& characterId) const;
    
    // 全ID取得
    std::vector<std::string> GetAllIds() const;
    
    // 存在確認
    bool Has(const std::string& characterId) const;
    
    // クリア
    void Clear();
    
private:
    bool ParseJson(const nlohmann::json& json, CharacterDefinition& outDef);
    
    // shared_ptr で共有可能に
    std::unordered_map<std::string, std::shared_ptr<CharacterDefinition>> definitions_;
};

} // namespace Game
```

### 4.4 EntityFactory（新設計）

```cpp
// include/Game/EntityFactory.h
#pragma once

#include "Core/World.h"
#include "Loaders/CharacterLoader.h"
#include <entt/entt.hpp>

namespace Resources { class ResourceManager; }

namespace Game {

class EntityFactory {
public:
    EntityFactory(Core::World& world, 
                  CharacterLoader& characterLoader,
                  Resources::ResourceManager& resourceManager);
    
    // ユニット生成（チーム指定）
    entt::entity CreateUnit(const std::string& characterId,
                            float x, float y,
                            TD::Components::Team::Type team);
    
    // 拠点生成
    entt::entity CreateBase(float x, float y, int hp,
                            TD::Components::Team::Type team,
                            const std::string& spriteName = "");
    
    // 投射物生成（将来拡張）
    entt::entity CreateProjectile(float x, float y,
                                  float targetX, float targetY,
                                  int damage,
                                  TD::Components::Team::Type team);

private:
    void SetupSprite(entt::entity entity, const CharacterDefinition& def);
    void SetupAnimation(entt::entity entity, const CharacterDefinition& def);
    void SetupStats(entt::entity entity, const CharacterDefinition& def);
    void SetupCombat(entt::entity entity, const CharacterDefinition& def);
    
    Core::World& world_;
    CharacterLoader& characterLoader_;
    Resources::ResourceManager& resourceManager_;
};

} // namespace Game
```

### 4.5 TD システム設計

```cpp
// include/TD/Systems/Systems.h
#pragma once

#include "Core/World.h"

namespace TD::Systems {

// ターゲット検索システム
// - 敵チームのユニットを検索し、最も近いターゲットを設定
void TargetingSystem(Core::World& world, float deltaTime);

// ユニットAIシステム
// - 状態遷移（Idle → Walking → Attacking）
// - ターゲットに応じた行動決定
void UnitAISystem(Core::World& world, float deltaTime);

// ユニット移動システム
// - 状態が Walking の場合のみ移動
// - Team に応じて左右方向を決定
void UnitMovementSystem(Core::World& world, float deltaTime);

// 戦闘システム
// - 攻撃クールダウン管理
// - 攻撃判定（hitFrame到達時）
// - ダメージイベント発行
void CombatSystem(Core::World& world, float deltaTime);

// ダメージ処理システム
// - DamageDealt イベントを処理
// - HP減少、ノックバック適用
void DamageSystem(Core::World& world, float deltaTime);

// 死亡処理システム
// - HP <= 0 のユニットを Dead 状態に
// - 死亡アニメーション完了後にエンティティ削除
void DeathSystem(Core::World& world, float deltaTime);

// ノックバックシステム
// - Knockback コンポーネントを持つエンティティを押し戻す
void KnockbackSystem(Core::World& world, float deltaTime);

// HPバー描画システム
void HealthBarRenderSystem(Core::World& world);

} // namespace TD::Systems
```

### 4.6 システム登録例

```cpp
// src/TD/TDScene.cpp

void TDScene::Initialize(Core::World& world, Core::SystemRunner& runner) {
    // Update フェーズ
    runner.RegisterUpdate(Core::SystemRunner::Phase::PreUpdate, 
        "Targeting", TD::Systems::TargetingSystem);
    
    runner.RegisterUpdate(Core::SystemRunner::Phase::Update,
        "UnitAI", TD::Systems::UnitAISystem);
    runner.RegisterUpdate(Core::SystemRunner::Phase::Update,
        "UnitMovement", TD::Systems::UnitMovementSystem);
    runner.RegisterUpdate(Core::SystemRunner::Phase::Update,
        "Combat", TD::Systems::CombatSystem);
    runner.RegisterUpdate(Core::SystemRunner::Phase::Update,
        "Animation", Game::Systems::AnimationSystem);
    
    runner.RegisterUpdate(Core::SystemRunner::Phase::PostUpdate,
        "Damage", TD::Systems::DamageSystem);
    runner.RegisterUpdate(Core::SystemRunner::Phase::PostUpdate,
        "Death", TD::Systems::DeathSystem);
    runner.RegisterUpdate(Core::SystemRunner::Phase::PostUpdate,
        "Knockback", TD::Systems::KnockbackSystem);
    
    // Render フェーズ
    runner.RegisterRender(Core::SystemRunner::Phase::Render,
        "SpriteRender", Game::Systems::SpriteRenderSystem);
    runner.RegisterRender(Core::SystemRunner::Phase::Render,
        "HealthBar", TD::Systems::HealthBarRenderSystem);
}
```

### 4.3 EntityFactory クラス

```cpp
// include/EntityFactory.h
#pragma once

#include <entt/entt.hpp>
#include "CharacterLoader.h"
#include "Components.h"

class ResourceManager;

class EntityFactory {
public:
    EntityFactory(entt::registry& registry, CharacterLoader& characterLoader, ResourceManager& resourceManager);
    
    // プレイヤーユニット生成
    entt::entity CreatePlayerUnit(const std::string& characterId, float x, float y);
    
    // 敵ユニット生成
    entt::entity CreateEnemyUnit(const std::string& characterId, float x, float y);
    
    // 拠点生成
    entt::entity CreateBase(Components::Team::Type team, float x, float y, int hp);
    
private:
    entt::entity CreateUnit(const std::string& characterId, float x, float y, Components::Team::Type team);
    void SetupAnimations(entt::entity entity, const CharacterDefinition& def);
    
    entt::registry& registry_;
    CharacterLoader& characterLoader_;
    ResourceManager& resourceManager_;
};
```

### 4.4 追加システム

```cpp
// include/Systems.h に追加

namespace Systems {
    // ターゲット検索システム
    void TargetingSystem(entt::registry& registry);
    
    // ユニットAIシステム（状態遷移）
    void UnitAISystem(entt::registry& registry, float deltaTime);
    
    // ユニット移動システム（TD用）
    void UnitMovementSystem(entt::registry& registry, float deltaTime);
    
    // 戦闘システム
    void CombatSystem(entt::registry& registry, float deltaTime);
    
    // スキルシステム
    void SkillSystem(entt::registry& registry, float deltaTime);
    
    // ダメージ処理システム
    void DamageSystem(entt::registry& registry);
    
    // 死亡処理システム
    void DeathSystem(entt::registry& registry);
    
    // ノックバックシステム
    void KnockbackSystem(entt::registry& registry, float deltaTime);
    
    // HP バー描画システム
    void HealthBarRenderSystem(entt::registry& registry);
}
```

---

## 5. WebUI エディタ設計（ノードベース）

### 5.1 技術スタック

| レイヤー | 技術 | 理由 |
|---------|------|------|
| フレームワーク | React 18 + TypeScript | コンポーネント指向、型安全 |
| ビルド | Vite | 高速HMR、軽量 |
| ノードエディタ | React Flow | NodeRED/ComfyUI風のノードベースUI実現 |
| UI | Tailwind CSS + shadcn/ui | 迅速なスタイリング |
| 状態管理 | Zustand | シンプルで軽量 |
| スキーマ検証 | Zod | TypeScript統合のバリデーション |
| スプライト描画 | Canvas API | 軽量なプレビュー描画 |

### 5.2 ノードタイプ設計

```
┌─────────────────────────────────────────────────────────────────┐
│                     Node Editor Canvas                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐      ┌─────────────────┐                  │
│  │ [Character]     │      │ [Animation]     │                  │
│  │ ─────────────── │      │ ─────────────── │                  │
│  │ id: cupslime    │      │ name: idle      │                  │
│  │ name: カップスライム│     │ frames: 3       │                  │
│  │                 │──────│ frameRate: 8    │                  │
│  │ ○ animations   │      │ loop: true      │                  │
│  │ ○ stats        │      └─────────────────┘                  │
│  │ ○ combat       │                                           │
│  │ ○ skills       │      ┌─────────────────┐                  │
│  └────────┬────────┘      │ [Animation]     │                  │
│           │               │ ─────────────── │                  │
│           │               │ name: attack    │                  │
│           └───────────────│ frames: 3       │                  │
│                           │ hitFrame: 2     │                  │
│  ┌─────────────────┐      └─────────────────┘                  │
│  │ [Stats]         │                                           │
│  │ ─────────────── │      ┌─────────────────┐                  │
│  │ hp: 100         │      │ [Skill]         │                  │
│  │ attack: 15      │──────│ ─────────────── │                  │
│  │ defense: 5      │      │ id: slime_punch │                  │
│  │ moveSpeed: 120  │      │ damage: 25      │                  │
│  └─────────────────┘      │ cooldown: 5.0   │                  │
│                           │                 │                  │
│  ┌─────────────────┐      │ ○ effectArea   │                  │
│  │ [Combat]        │      └────────┬────────┘                  │
│  │ ─────────────── │               │                           │
│  │ type: melee     │      ┌────────▼────────┐                  │
│  │ range: 50       │      │ [EffectArea]    │                  │
│  │ cooldown: 1.5   │      │ ─────────────── │                  │
│  └─────────────────┘      │ type: rectangle │                  │
│                           │ width: 60       │                  │
│                           │ height: 40      │                  │
│                           │ offsetX: 30     │                  │
│                           │ ┌─────────────┐ │                  │
│                           │ │ ■ Preview   │ │                  │
│                           │ └─────────────┘ │                  │
│                           └─────────────────┘                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 5.3 ノードタイプ一覧

| ノード | 入力 | 出力 | 説明 |
|--------|------|------|------|
| CharacterNode | - | animations, stats, combat, skills, hitbox | ルートノード、基本情報 |
| AnimationNode | character | - | アニメーション定義、フレームリスト |
| StatsNode | character | - | HP, 攻撃力などのステータス |
| CombatNode | character | - | 攻撃タイプ、射程、クールダウン |
| SkillNode | character | effectArea | スキル定義 |
| EffectAreaNode | skill | - | 効果範囲（矩形）、ビジュアルプレビュー付き |
| HitboxNode | character | - | 当たり判定 |
| SpritePreviewNode | - | - | スプライトシートプレビュー |

### 5.4 ステージエディタ用ノード

| ノード | 入力 | 出力 | 説明 |
|--------|------|------|------|
| StageNode | - | waves, bases, layout | ステージルートノード |
| WaveNode | stage | enemies | ウェーブ定義 |
| EnemySpawnNode | wave | - | 敵出現定義（キャラID、数、間隔） |
| BaseNode | stage | - | 拠点定義（座標、HP） |
| LayoutNode | stage | - | ステージサイズ、地面位置 |

### 5.5 ディレクトリ構造（WebUI）

```
tools/entity-editor/
├── package.json
├── vite.config.ts
├── tsconfig.json
├── tailwind.config.js
├── index.html
├── public/
│   └── assets/           # テスト用アセット
├── src/
│   ├── main.tsx
│   ├── App.tsx
│   ├── components/
│   │   ├── Layout/
│   │   │   ├── Header.tsx
│   │   │   ├── Sidebar.tsx
│   │   │   └── MainCanvas.tsx
│   │   ├── NodeEditor/
│   │   │   ├── NodeEditor.tsx
│   │   │   ├── NodeToolbar.tsx
│   │   │   └── ConnectionLine.tsx
│   │   ├── Panels/
│   │   │   ├── PropertiesPanel.tsx
│   │   │   ├── SpritePreview.tsx
│   │   │   └── EffectAreaPreview.tsx
│   │   └── Dialogs/
│   │       ├── ExportDialog.tsx
│   │       └── ImportDialog.tsx
│   ├── nodes/
│   │   ├── index.ts
│   │   ├── CharacterNode.tsx
│   │   ├── AnimationNode.tsx
│   │   ├── StatsNode.tsx
│   │   ├── CombatNode.tsx
│   │   ├── SkillNode.tsx
│   │   ├── EffectAreaNode.tsx
│   │   ├── HitboxNode.tsx
│   │   ├── StageNode.tsx
│   │   ├── WaveNode.tsx
│   │   └── EnemySpawnNode.tsx
│   ├── stores/
│   │   ├── useEditorStore.ts
│   │   ├── useCharacterStore.ts
│   │   └── useStageStore.ts
│   ├── types/
│   │   ├── character.ts
│   │   ├── stage.ts
│   │   └── nodes.ts
│   ├── utils/
│   │   ├── exportJson.ts
│   │   ├── importJson.ts
│   │   ├── validation.ts
│   │   └── spriteLoader.ts
│   └── schemas/
│       ├── characterSchema.ts   # Zod スキーマ
│       └── stageSchema.ts
└── README.md
```

---

## 6. プロトタイプ作成ロードマップ（新アーキテクチャ対応）

### Phase 0: アーキテクチャ基盤構築（2-3日）★ 新規追加

**目標**: 新レイヤー構造とDI基盤を構築

1. [ ] ディレクトリ構造作成（`include/Core/`, `include/Game/`, `include/TD/`）
2. [ ] `Core::GameContext` 実装（DIコンテナ）
3. [ ] `Core::World` 実装（registry + dispatcher ラッパー）
4. [ ] `Core::SystemRunner` 実装（フェーズ別システム実行）
5. [ ] 既存コードを新構造にマイグレーション
   - `Components.h` → `Core/Components.h` + `Game/Components.h`
   - `Systems.h` → `Game/Systems/` 配下に分離
6. [ ] イベント定義（`Core/Events.h`）

### Phase 1: JSON スキーマ確定 & サンプル作成（1-2日）✅ 完了

**目標**: JSON形式を確定し、C++ローダーのテスト用データを準備

1. [x] `assets/definitions/schemas/character.schema.json` 作成
2. [x] `assets/definitions/schemas/stage.schema.json` 作成
3. [x] `assets/definitions/characters/cupslime.character.json` サンプル作成
4. [x] `assets/definitions/stages/stage_01.stage.json` サンプル作成
5. [x] ディレクトリ構造作成

### Phase 2: ゲーム層ローダー実装（2-3日）

**目標**: JSONからキャラクター・ステージ情報を読み込み

1. [ ] `Game::CharacterLoader` 実装
   - JSON読み込み・パース
   - `CharacterDefinition` 構造体への変換
   - エラーハンドリング（デフォルト値フォールバック）
2. [ ] `Game::StageLoader` 実装
   - ステージJSON読み込み
   - ウェーブ情報パース
3. [ ] `Game::EntityFactory` 実装
   - `CreateUnit()` - キャラクター定義からエンティティ生成
   - コンポーネント自動付与
4. [ ] 既存 `ResourceManager` との統合
   - スプライトシート自動読み込み

### Phase 3: TD層コンポーネント・システム実装（3-5日）

**目標**: タワーディフェンス専用ロジック

1. [ ] TD コンポーネント実装（`TD/Components.h`）
   - `Stats`, `Combat`, `Team`, `UnitState`, `Target`
2. [ ] ターゲティングシステム
   - 敵チーム検索、最近接ターゲット選択
3. [ ] ユニットAIシステム
   - 状態遷移（Idle → Walking → Attacking → Dead）
4. [ ] 移動システム
   - Team に応じた移動方向
5. [ ] 戦闘システム
   - 攻撃クールダウン、ダメージ計算
   - `Events::DamageDealt` イベント発行
6. [ ] ダメージ・死亡処理システム
   - HPバー描画

### Phase 4: ウェーブ管理・バトルシーン（2-3日）

**目標**: ゲームフロー完成

1. [ ] `TD::WaveManager` 実装
   - ウェーブ進行管理
   - 敵スポーン処理
2. [ ] `TD::BattleManager` 実装
   - 勝敗判定
   - リソース（お金）管理
3. [ ] `TDScene` 実装
   - バトル画面全体の統合
   - UIとの連携

### Phase 5: WebUI エディタ基盤（2-3日）

**目標**: React Flow でノードベースエディタの基盤を構築

1. [ ] Vite + React + TypeScript プロジェクト作成
2. [ ] React Flow (@xyflow/react) 導入
3. [ ] Tailwind CSS + shadcn/ui 設定
4. [ ] Zustand ストア設計
5. [ ] 基本レイアウト（Header, Sidebar, NodeCanvas）

### Phase 6: キャラクター・ステージノード実装（3-4日）

**目標**: ノードベース編集機能

1. [ ] キャラクター系ノード
   - `CharacterNode`, `StatsNode`, `CombatNode`
   - `AnimationNode`, `SkillNode`, `EffectAreaNode`
2. [ ] ステージ系ノード
   - `StageNode`, `WaveNode`, `EnemySpawnNode`
3. [ ] ノード接続ロジック
4. [ ] JSON エクスポート/インポート
5. [ ] Zod バリデーション

### Phase 7: プレビュー機能 & 仕上げ（2-3日）

**目標**: ビジュアル編集機能

1. [ ] スプライトシート読み込み・表示
2. [ ] アニメーションプレビュー
3. [ ] 効果範囲ビジュアル描画
4. [ ] エラーハンドリング改善
5. [ ] ドキュメント作成

---

## 7. マイグレーション計画

### 7.1 既存コードの移行

| 現在 | 移行先 | 変更内容 |
|------|--------|----------|
| `include/Components.h` | `include/Core/Components.h` | 基本コンポーネントのみ残す |
| `include/AnimationSystem.h` | `include/Game/Systems/AnimationSystem.h` | namespace を `Game::Systems` に |
| `include/System.h` | 削除 | `ISystem` は廃止、静的関数に統一 |
| `include/SystemManager.h` | `include/Core/SystemRunner.h` | 新設計に置き換え |
| `src/Game.cpp` の `SampleScene` | `src/Game/Scenes/` に分離 | シーンを独立ファイルに |

### 7.2 Singleton の段階的排除

```cpp
// Before: Singleton アクセス
Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();

// After: GameContext 経由
auto& rm = context.Get<Resources::ResourceManager>();
```

**移行ステップ**:

1. `GameContext` を `Game` クラスに保持
2. 各マネージャーを `GameContext` に登録
3. シーン・システムに `GameContext` 参照を渡す
4. Singleton の `GetInstance()` を deprecated 警告に
5. 最終的に Singleton 削除

### 7.3 互換性維持

移行期間中は以下のマクロで切り替え可能に：

```cpp
#ifdef USE_NEW_ARCHITECTURE
    auto& rm = context.Get<Resources::ResourceManager>();
#else
    auto& rm = Resources::ResourceManager::GetInstance();
#endif
```

---

## 8. AIバイブコーディング向けプロンプト例

### Phase 0: Core層基盤実装

```
C++17 ECS (EnTT) プロジェクトで、依存性注入コンテナを実装してください。

要件:
- テンプレートベースの型安全なサービス登録/取得
- std::type_index を使用した型識別
- shared_ptr でサービスのライフタイム管理
- 存在しないサービス取得時は例外をスロー

命名規則:
- クラス名: PascalCase
- メンバー変数: camelCase + 末尾アンダースコア

[GameContext ヘッダーのスケルトンをここに貼り付け]
```

### Phase 2: CharacterLoader 実装

```
C++ ECS (EnTT) プロジェクトで、以下のJSONスキーマに対応した
Game::CharacterLoader クラスを実装してください。

- nlohmann/json を使用
- try-catch でエラーハンドリング（parse_error と exception の両方）
- 必須フィールド欠落時はログ出力してスキップ
- shared_ptr<CharacterDefinition> でキャッシュ
- 命名規則: クラス名 PascalCase、メンバー変数 camelCase + 末尾_

[character.schema.json の内容をここに貼り付け]
```

### Phase 5: ノードエディタ実装

```
React Flow を使用して、NodeRED/ComfyUI 風のノードベースエディタを
作成してください。

技術スタック:
- React 18 + TypeScript
- Vite
- React Flow (@xyflow/react)
- Tailwind CSS
- Zustand

ノードタイプ:
1. CharacterNode - キャラクター基本情報
2. StatsNode - ステータス値
3. SkillNode - スキル定義
4. EffectAreaNode - 効果範囲（矩形のみ）

要件:
- ノード間を線で接続可能
- 接続情報からJSONをエクスポート
- ダークテーマ対応
```

---

## 9. 参考資料

### C++ / ECS

- [EnTT Wiki](https://github.com/skypjack/entt/wiki) - ECSライブラリドキュメント
- [EnTT Dispatcher](https://github.com/skypjack/entt/wiki/Crash-Course:-events,-signals-and-everything-in-between) - イベントシステム
- [nlohmann/json](https://github.com/nlohmann/json) - JSONライブラリ
- [Raylib](https://www.raylib.com/) - ゲームライブラリ

### WebUI

- [React Flow](https://reactflow.dev/) - ノードベースUIライブラリ
- [Tailwind CSS](https://tailwindcss.com/) - CSSフレームワーク
- [shadcn/ui](https://ui.shadcn.com/) - UIコンポーネント
- [Zustand](https://github.com/pmndrs/zustand) - 状態管理
- [Zod](https://zod.dev/) - スキーマバリデーション

### 設計パターン

- [Dependency Injection in C++](https://www.cppstories.com/2018/07/dependency-injection-cpp/) - DIパターン解説
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - ゲーム設計パターン
- [ECS FAQ](https://github.com/SanderMertens/ecs-faq) - ECS設計のFAQ

---

## 10. 変更履歴

| 日付 | 変更内容 |
|------|----------|
| 2024-XX-XX | 初版作成 |
| 2024-XX-XX | 新アーキテクチャ（Core/Game/TD層分離）追加 |
| 2024-XX-XX | GameContext（DI）、World、SystemRunner 設計追加 |
| 2024-XX-XX | マイグレーション計画追加 |
