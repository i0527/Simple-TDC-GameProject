# TD専用エンティティ動作システム設計

## 概要

にゃんこ大戦争のようなシンプルなTDゲームに特化したエンティティ動作システムの詳細設計。スプライトアニメーション、AI行動、エフェクト・サウンドの同期を JSON で定義できるようにする。

## システム構成

### 1. コンポーネント構造

```cpp
namespace TD {

/**
 * @brief エンティティの動作状態
 */
enum class EntityState {
    Idle,       // 待機
    Walk,       // 移動
    Attack,     // 攻撃
    Skill,      // スキル使用
    Damaged,    // 被ダメージ
    Death,      // 死亡
    Custom      // カスタム状態
};

/**
 * @brief アニメーションコンポーネント
 */
struct AnimationComponent {
    std::string currentAnimation;  // 現在のアニメーション名
    int currentFrame = 0;
    float frameTime = 0.0f;
    bool isPlaying = true;
    bool loop = true;
};

/**
 * @brief スプライトシート情報
 */
struct SpriteSheetInfo {
    Texture2D texture;
    int frameCount;
    float frameDuration;
    bool loop;
    Vector2 frameSize;
    std::vector<AnimationEvent> events;  // フレームイベント
};

/**
 * @brief エンティティ動作定義コンポーネント
 */
struct BehaviorComponent {
    std::string behaviorId;           // 動作定義ID
    EntityState currentState;         // 現在の状態
    EntityState previousState;        // 前の状態
    float stateTime = 0.0f;          // 現在の状態の経過時間
    
    // アニメーション
    std::map<std::string, SpriteSheetInfo> animations;
    
    // ステートマシン
    std::map<EntityState, StateDefinition> states;
    
    // 条件評価用パラメータ
    float attackCooldown = 0.0f;
    entt::entity targetEntity = entt::null;
};

/**
 * @brief 状態定義
 */
struct StateDefinition {
    std::string spriteName;              // 使用するスプライト
    float moveSpeed = 0.0f;              // 移動速度（0なら移動しない）
    std::vector<Transition> transitions; // 状態遷移定義
    
    // 状態開始時のアクション
    std::vector<std::string> onEnterActions;
    
    // 状態終了時のアクション
    std::vector<std::string> onExitActions;
};

/**
 * @brief 状態遷移定義
 */
struct Transition {
    ConditionType conditionType;
    EntityState targetState;
    
    // 条件パラメータ
    float range = 0.0f;
    float value = 0.0f;
    float probability = 1.0f;
    std::string cooldownId;
};

/**
 * @brief 条件タイプ
 */
enum class ConditionType {
    EnemyInRange,           // 敵が範囲内
    NoEnemyInRange,         // 敵が範囲外
    HpBelow,                // HP が指定値以下
    CooldownReady,          // クールダウン完了
    AnimationFinished,      // アニメーション終了
    DistanceToBase,         // 拠点までの距離
    Random,                 // ランダム
    Always                  // 常に真
};

/**
 * @brief アニメーションイベント
 */
struct AnimationEvent {
    int frame;                    // 発火するフレーム
    AnimationEventType type;      // イベントタイプ
    
    // イベントパラメータ（タイプによって使用される）
    float damageValue = 0.0f;
    std::string soundId;
    std::string effectId;
    Vector2 offset = {0, 0};
    std::string spawnEntityId;
    EntityState targetState = EntityState::Idle;
};

/**
 * @brief アニメーションイベントタイプ
 */
enum class AnimationEventType {
    Damage,         // ダメージを与える
    Sound,          // サウンド再生
    Effect,         // エフェクト生成
    Spawn,          // エンティティ生成
    Heal,           // HP回復
    StateChange     // 状態遷移
};

} // namespace TD
```

### 2. 動作システム

```cpp
namespace TD::Systems {

/**
 * @brief エンティティ動作システム
 * 
 * エンティティのステートマシンを実行し、
 * 状態遷移とアニメーション制御を行う
 */
class EntityBehaviorSystem : public Core::ISystem {
public:
    void ProcessInput(float deltaTime) override {}
    
    void Update(float deltaTime) override {
        auto view = world_->GetRegistry().view<
            BehaviorComponent,
            AnimationComponent,
            Game::Transform,
            TD::Stats
        >();
        
        for (auto entity : view) {
            auto& behavior = view.get<BehaviorComponent>(entity);
            auto& animation = view.get<AnimationComponent>(entity);
            auto& transform = view.get<Game::Transform>(entity);
            auto& stats = view.get<TD::Stats>(entity);
            
            // 状態時間更新
            behavior.stateTime += deltaTime;
            
            // クールダウン更新
            if (behavior.attackCooldown > 0.0f) {
                behavior.attackCooldown -= deltaTime;
            }
            
            // 状態遷移チェック
            CheckStateTransition(entity, behavior, transform, stats);
            
            // 現在の状態に応じた処理
            UpdateState(entity, behavior, animation, transform, deltaTime);
            
            // アニメーション更新
            UpdateAnimation(entity, behavior, animation, deltaTime);
        }
    }
    
    void Render() override {}
    
private:
    void CheckStateTransition(
        entt::entity entity,
        BehaviorComponent& behavior,
        const Game::Transform& transform,
        const TD::Stats& stats
    );
    
    void UpdateState(
        entt::entity entity,
        BehaviorComponent& behavior,
        AnimationComponent& animation,
        Game::Transform& transform,
        float deltaTime
    );
    
    void UpdateAnimation(
        entt::entity entity,
        BehaviorComponent& behavior,
        AnimationComponent& animation,
        float deltaTime
    );
    
    bool EvaluateCondition(
        const Transition& transition,
        entt::entity entity,
        const BehaviorComponent& behavior,
        const Game::Transform& transform,
        const TD::Stats& stats
    );
    
    void ChangeState(
        entt::entity entity,
        BehaviorComponent& behavior,
        AnimationComponent& animation,
        EntityState newState
    );
    
    void ProcessAnimationEvents(
        entt::entity entity,
        const BehaviorComponent& behavior,
        const AnimationComponent& animation
    );
};

} // namespace TD::Systems
```

## JSON定義フォーマット

### エンティティ動作定義

完全なエンティティ動作定義のJSONスキーマ:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "TD Entity Behavior Definition",
  "type": "object",
  "required": ["entityId", "name", "sprites", "stats", "behavior"],
  "properties": {
    "entityId": {
      "type": "string",
      "description": "エンティティの一意識別子"
    },
    "name": {
      "type": "string",
      "description": "エンティティ名（表示用）"
    },
    "description": {
      "type": "string",
      "description": "エンティティの説明"
    },
    "sprites": {
      "type": "object",
      "description": "スプライトアニメーション定義",
      "patternProperties": {
        "^[a-z_]+$": {
          "$ref": "#/definitions/spriteAnimation"
        }
      }
    },
    "stats": {
      "$ref": "#/definitions/stats"
    },
    "behavior": {
      "$ref": "#/definitions/behavior"
    },
    "sounds": {
      "$ref": "#/definitions/sounds"
    }
  },
  "definitions": {
    "spriteAnimation": {
      "type": "object",
      "required": ["texturePath", "frameCount", "frameDuration"],
      "properties": {
        "texturePath": {
          "type": "string",
          "description": "スプライトシートのパス"
        },
        "frameCount": {
          "type": "integer",
          "minimum": 1,
          "description": "フレーム数"
        },
        "frameDuration": {
          "type": "number",
          "minimum": 0.01,
          "description": "1フレームの表示時間（秒）"
        },
        "loop": {
          "type": "boolean",
          "default": true,
          "description": "ループ再生するか"
        },
        "events": {
          "type": "array",
          "description": "アニメーションイベント",
          "items": {
            "$ref": "#/definitions/animationEvent"
          }
        }
      }
    },
    "animationEvent": {
      "type": "object",
      "required": ["frame", "type"],
      "properties": {
        "frame": {
          "type": "integer",
          "minimum": 0,
          "description": "イベント発火フレーム"
        },
        "type": {
          "type": "string",
          "enum": ["damage", "sound", "effect", "spawn", "heal", "state_change"],
          "description": "イベントタイプ"
        },
        "value": {
          "type": "number",
          "description": "ダメージ/回復量"
        },
        "soundId": {
          "type": "string",
          "description": "サウンドID"
        },
        "effectId": {
          "type": "string",
          "description": "エフェクトID"
        },
        "offsetX": {
          "type": "number",
          "default": 0,
          "description": "X方向オフセット"
        },
        "offsetY": {
          "type": "number",
          "default": 0,
          "description": "Y方向オフセット"
        },
        "entityId": {
          "type": "string",
          "description": "生成するエンティティID"
        },
        "targetState": {
          "type": "string",
          "description": "遷移先の状態"
        }
      }
    },
    "stats": {
      "type": "object",
      "required": ["maxHp", "speed", "attackRange", "attackDamage"],
      "properties": {
        "maxHp": {
          "type": "number",
          "minimum": 1,
          "description": "最大HP"
        },
        "speed": {
          "type": "number",
          "minimum": 0,
          "description": "移動速度"
        },
        "attackRange": {
          "type": "number",
          "minimum": 0,
          "description": "攻撃範囲"
        },
        "attackCooldown": {
          "type": "number",
          "minimum": 0.1,
          "description": "攻撃クールダウン（秒）"
        },
        "attackDamage": {
          "type": "number",
          "minimum": 0,
          "description": "攻撃力"
        }
      }
    },
    "behavior": {
      "type": "object",
      "required": ["type", "states", "initialState"],
      "properties": {
        "type": {
          "type": "string",
          "enum": ["melee_fighter", "ranged_fighter", "support", "tank", "custom"],
          "description": "行動タイプ"
        },
        "states": {
          "type": "object",
          "description": "状態定義",
          "patternProperties": {
            "^[a-z_]+$": {
              "$ref": "#/definitions/stateDefinition"
            }
          }
        },
        "initialState": {
          "type": "string",
          "description": "初期状態"
        }
      }
    },
    "stateDefinition": {
      "type": "object",
      "required": ["sprite", "transitions"],
      "properties": {
        "sprite": {
          "type": "string",
          "description": "使用するスプライト名"
        },
        "moveSpeed": {
          "type": "number",
          "minimum": 0,
          "default": 0,
          "description": "この状態での移動速度"
        },
        "transitions": {
          "type": "array",
          "description": "状態遷移定義",
          "items": {
            "$ref": "#/definitions/transition"
          }
        },
        "onEnter": {
          "type": "array",
          "description": "状態開始時のアクション",
          "items": {
            "type": "string"
          }
        },
        "onExit": {
          "type": "array",
          "description": "状態終了時のアクション",
          "items": {
            "type": "string"
          }
        }
      }
    },
    "transition": {
      "type": "object",
      "required": ["condition", "targetState"],
      "properties": {
        "condition": {
          "type": "string",
          "enum": [
            "enemy_in_range",
            "no_enemy_in_range",
            "hp_below",
            "cooldown_ready",
            "animation_finished",
            "distance_to_base",
            "random",
            "always"
          ],
          "description": "遷移条件"
        },
        "targetState": {
          "type": "string",
          "description": "遷移先の状態"
        },
        "range": {
          "type": "number",
          "minimum": 0,
          "description": "範囲（enemy_in_range等で使用）"
        },
        "percentage": {
          "type": "number",
          "minimum": 0,
          "maximum": 100,
          "description": "HP割合（hp_below等で使用）"
        },
        "cooldownId": {
          "type": "string",
          "description": "クールダウンID"
        },
        "probability": {
          "type": "number",
          "minimum": 0,
          "maximum": 1,
          "description": "確率（random条件で使用）"
        }
      }
    },
    "sounds": {
      "type": "object",
      "description": "サウンド定義",
      "properties": {
        "spawn": {
          "type": "string",
          "description": "出現時のサウンド"
        },
        "attack": {
          "type": "string",
          "description": "攻撃時のサウンド"
        },
        "hit": {
          "type": "string",
          "description": "被ダメージ時のサウンド"
        },
        "death": {
          "type": "string",
          "description": "死亡時のサウンド"
        }
      }
    }
  }
}
```

## 実装例

### 近接戦士の定義

```json
{
  "entityId": "melee_warrior",
  "name": "近接戦士",
  "description": "前線で戦う基本的な近接ユニット",
  
  "sprites": {
    "idle": {
      "texturePath": "assets/sprites/warrior/idle.png",
      "frameCount": 4,
      "frameDuration": 0.25,
      "loop": true
    },
    "walk": {
      "texturePath": "assets/sprites/warrior/walk.png",
      "frameCount": 6,
      "frameDuration": 0.15,
      "loop": true
    },
    "attack": {
      "texturePath": "assets/sprites/warrior/attack.png",
      "frameCount": 8,
      "frameDuration": 0.1,
      "loop": false,
      "events": [
        {
          "frame": 4,
          "type": "damage",
          "value": 15
        },
        {
          "frame": 3,
          "type": "sound",
          "soundId": "sword_swing"
        },
        {
          "frame": 4,
          "type": "effect",
          "effectId": "slash_effect",
          "offsetX": 40,
          "offsetY": 0
        }
      ]
    },
    "damaged": {
      "texturePath": "assets/sprites/warrior/damaged.png",
      "frameCount": 3,
      "frameDuration": 0.1,
      "loop": false
    },
    "death": {
      "texturePath": "assets/sprites/warrior/death.png",
      "frameCount": 8,
      "frameDuration": 0.15,
      "loop": false,
      "events": [
        {
          "frame": 0,
          "type": "sound",
          "soundId": "warrior_death"
        }
      ]
    }
  },
  
  "stats": {
    "maxHp": 150,
    "speed": 60,
    "attackRange": 90,
    "attackCooldown": 1.8,
    "attackDamage": 15
  },
  
  "behavior": {
    "type": "melee_fighter",
    "initialState": "idle",
    "states": {
      "idle": {
        "sprite": "idle",
        "moveSpeed": 0,
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 90
          },
          {
            "condition": "always",
            "targetState": "walk"
          }
        ]
      },
      "walk": {
        "sprite": "walk",
        "moveSpeed": 60,
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 90
          }
        ]
      },
      "attack": {
        "sprite": "attack",
        "moveSpeed": 0,
        "transitions": [
          {
            "condition": "animation_finished",
            "targetState": "idle"
          }
        ],
        "onEnter": ["reset_attack_cooldown"]
      },
      "damaged": {
        "sprite": "damaged",
        "moveSpeed": 0,
        "transitions": [
          {
            "condition": "animation_finished",
            "targetState": "idle"
          }
        ]
      },
      "death": {
        "sprite": "death",
        "moveSpeed": 0,
        "transitions": []
      }
    }
  },
  
  "sounds": {
    "spawn": "warrior_spawn",
    "attack": "sword_swing",
    "hit": "warrior_hit",
    "death": "warrior_death"
  }
}
```

### 遠距離アーチャーの定義

```json
{
  "entityId": "archer",
  "name": "アーチャー",
  "description": "遠距離から弓で攻撃するユニット",
  
  "sprites": {
    "idle": {
      "texturePath": "assets/sprites/archer/idle.png",
      "frameCount": 4,
      "frameDuration": 0.25,
      "loop": true
    },
    "walk": {
      "texturePath": "assets/sprites/archer/walk.png",
      "frameCount": 6,
      "frameDuration": 0.15,
      "loop": true
    },
    "attack": {
      "texturePath": "assets/sprites/archer/attack.png",
      "frameCount": 10,
      "frameDuration": 0.12,
      "loop": false,
      "events": [
        {
          "frame": 5,
          "type": "sound",
          "soundId": "bow_release"
        },
        {
          "frame": 6,
          "type": "spawn",
          "entityId": "arrow_projectile",
          "offsetX": 30,
          "offsetY": -20
        }
      ]
    },
    "death": {
      "texturePath": "assets/sprites/archer/death.png",
      "frameCount": 6,
      "frameDuration": 0.15,
      "loop": false
    }
  },
  
  "stats": {
    "maxHp": 80,
    "speed": 50,
    "attackRange": 250,
    "attackCooldown": 2.5,
    "attackDamage": 20
  },
  
  "behavior": {
    "type": "ranged_fighter",
    "initialState": "idle",
    "states": {
      "idle": {
        "sprite": "idle",
        "moveSpeed": 0,
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 250
          },
          {
            "condition": "always",
            "targetState": "walk"
          }
        ]
      },
      "walk": {
        "sprite": "walk",
        "moveSpeed": 50,
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 250
          }
        ]
      },
      "attack": {
        "sprite": "attack",
        "moveSpeed": 0,
        "transitions": [
          {
            "condition": "animation_finished",
            "targetState": "idle"
          }
        ],
        "onEnter": ["reset_attack_cooldown"]
      },
      "death": {
        "sprite": "death",
        "moveSpeed": 0,
        "transitions": []
      }
    }
  },
  
  "sounds": {
    "spawn": "archer_ready",
    "attack": "bow_release",
    "hit": "archer_hit",
    "death": "archer_death"
  }
}
```

## エディタ統合

### ビジュアルステートマシンエディタ

エンティティエディタ内で、ステートマシンを視覚的に編集できるようにする:

```
┌─────────────────────────────────────────────┐
│ State Machine Editor                        │
├─────────────────────────────────────────────┤
│                                             │
│   ┌──────┐                                 │
│   │ Idle │────┐                            │
│   └──────┘    │                            │
│      │        │ enemy_in_range             │
│      │        ↓                            │
│      │   ┌────────┐                        │
│      └──→│ Attack │                        │
│  always  └────────┘                        │
│      ↓        │                            │
│   ┌──────┐   │ animation_finished          │
│   │ Walk │←──┘                            │
│   └──────┘                                 │
│                                             │
│ Selected State: Attack                      │
│ ┌─────────────────────────────────────────┐│
│ │ Sprite: attack                          ││
│ │ Move Speed: 0                           ││
│ │ On Enter: [reset_attack_cooldown]       ││
│ │                                         ││
│ │ Transitions:                            ││
│ │  • animation_finished → idle           ││
│ │    [Add Transition]                     ││
│ └─────────────────────────────────────────┘│
└─────────────────────────────────────────────┘
```

### アニメーションタイムラインエディタ

アニメーションイベントをタイムライン上で編集:

```
┌─────────────────────────────────────────────┐
│ Animation Timeline: attack                  │
├─────────────────────────────────────────────┤
│                                             │
│ Frame: 0   1   2   3   4   5   6   7      │
│        │───│───│───│───│───│───│───│       │
│        [◆] [◆] [◆] [♪] [⚔] [◆] [◆] [◆]    │
│                     │   │                   │
│                     │   └─ Damage (15)      │
│                     └───── Sound (swing)    │
│                                             │
│ Preview: ▶ ⏸ ⏹  Frame Rate: 10 FPS         │
│                                             │
│ Event at Frame 4:                           │
│ ┌─────────────────────────────────────────┐│
│ │ Type: Damage                            ││
│ │ Value: [15]▔                            ││
│ │                                         ││
│ │ [Delete Event]                          ││
│ └─────────────────────────────────────────┘│
└─────────────────────────────────────────────┘
```

## プレビューシステム

### 単体エンティティプレビュー

エディタでエンティティをリアルタイムプレビュー:

1. **アニメーション再生**: 各状態のスプライトアニメーション確認
2. **ステート遷移**: 条件に応じた状態遷移の動作確認
3. **イベント発火**: サウンド、エフェクトのタイミング確認
4. **パラメータ調整**: ステータスをリアルタイムで変更し、動作を確認

### 戦闘シミュレーション

複数エンティティでの戦闘をシミュレーション:

1. **配置**: プレイヤー側と敵側のエンティティを配置
2. **再生**: 実際の戦闘をシミュレーション
3. **デバッグ**: ダメージ、範囲、クールダウンを可視化
4. **調整**: バランス調整をリアルタイムで実施

## まとめ

このシステムにより、プログラマーでないレベルデザイナーでも、JSONを編集するだけで、にゃんこ大戦争風のシンプルなTDゲームのエンティティ動作を定義できる。ビジュアルエディタとホットリロードにより、即座にフィードバックを得ながら調整可能。
