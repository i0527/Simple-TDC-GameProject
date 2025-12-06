# JSONスキーマ設計

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）で使用するJSON定義ファイルのスキーマを定義します。

## 目的

- データ駆動設計の実現
- JSONファイルでゲームコンテンツを拡張可能にする
- スキーマバリデーションによる型安全性の確保
- 内部エディタでの編集を容易にする

---

## 1. エンティティ定義（entities.json）

### 1.1 スキーマ概要

エンティティ定義は、ゲーム内のユニット、敵、弾丸などの基本情報を定義します。

### 1.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "entities": {
      "type": "array",
      "items": {
        "$ref": "#/definitions/Entity"
      }
    }
  },
  "definitions": {
    "Entity": {
      "type": "object",
      "required": ["id", "name", "type"],
      "properties": {
        "id": {
          "type": "string",
          "description": "エンティティの一意ID"
        },
        "name": {
          "type": "string",
          "description": "表示名（日本語可）"
        },
        "type": {
          "type": "string",
          "enum": ["unit", "enemy", "projectile", "tower", "obstacle"],
          "description": "エンティティタイプ"
        },
        "sprite": {
          "$ref": "#/definitions/Sprite"
        },
        "stats": {
          "$ref": "#/definitions/Stats"
        },
        "abilities": {
          "type": "array",
          "items": {
            "type": "string"
          },
          "description": "能力IDのリスト"
        },
        "animations": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/definitions/Animation"
          }
        }
      }
    },
    "Sprite": {
      "type": "object",
      "required": ["texture"],
      "properties": {
        "texture": {
          "type": "string",
          "description": "テクスチャファイルパス"
        },
        "width": {
          "type": "integer",
          "minimum": 1
        },
        "height": {
          "type": "integer",
          "minimum": 1
        },
        "originX": {
          "type": "number",
          "default": 0.5
        },
        "originY": {
          "type": "number",
          "default": 0.5
        }
      }
    },
    "Stats": {
      "type": "object",
      "properties": {
        "health": {
          "type": "number",
          "minimum": 0,
          "default": 100
        },
        "maxHealth": {
          "type": "number",
          "minimum": 0,
          "default": 100
        },
        "attack": {
          "type": "number",
          "minimum": 0,
          "default": 10
        },
        "defense": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "speed": {
          "type": "number",
          "minimum": 0,
          "default": 100
        },
        "cost": {
          "type": "number",
          "minimum": 0,
          "default": 0
        }
      }
    },
    "Animation": {
      "type": "object",
      "required": ["frames"],
      "properties": {
        "frames": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/AnimationFrame"
          }
        },
        "loop": {
          "type": "boolean",
          "default": true
        },
        "speed": {
          "type": "number",
          "minimum": 0.01,
          "default": 1.0
        }
      }
    },
    "AnimationFrame": {
      "type": "object",
      "required": ["x", "y", "width", "height", "duration"],
      "properties": {
        "x": {
          "type": "integer",
          "minimum": 0
        },
        "y": {
          "type": "integer",
          "minimum": 0
        },
        "width": {
          "type": "integer",
          "minimum": 1
        },
        "height": {
          "type": "integer",
          "minimum": 1
        },
        "duration": {
          "type": "number",
          "minimum": 0.01
        }
      }
    }
  }
}
```

### 1.3 実装例

```json
{
  "entities": [
    {
      "id": "player_unit_01",
      "name": "プレイヤーユニット1",
      "type": "unit",
      "sprite": {
        "texture": "assets/textures/units/player01.png",
        "width": 64,
        "height": 64,
        "originX": 0.5,
        "originY": 0.5
      },
      "stats": {
        "health": 100,
        "maxHealth": 100,
        "attack": 20,
        "defense": 5,
        "speed": 150,
        "cost": 50
      },
      "abilities": ["ability_attack_01", "ability_heal_01"],
      "animations": {
        "idle": {
          "frames": [
            {"x": 0, "y": 0, "width": 64, "height": 64, "duration": 0.2},
            {"x": 64, "y": 0, "width": 64, "height": 64, "duration": 0.2}
          ],
          "loop": true,
          "speed": 1.0
        },
        "walk": {
          "frames": [
            {"x": 0, "y": 64, "width": 64, "height": 64, "duration": 0.15},
            {"x": 64, "y": 64, "width": 64, "height": 64, "duration": 0.15},
            {"x": 128, "y": 64, "width": 64, "height": 64, "duration": 0.15}
          ],
          "loop": true,
          "speed": 1.0
        }
      }
    },
    {
      "id": "enemy_slime_01",
      "name": "スライム",
      "type": "enemy",
      "sprite": {
        "texture": "assets/textures/enemies/slime.png",
        "width": 48,
        "height": 48
      },
      "stats": {
        "health": 50,
        "maxHealth": 50,
        "attack": 10,
        "defense": 2,
        "speed": 80
      },
      "abilities": ["ability_attack_basic"]
    }
  ]
}
```

### 1.4 C++構造体定義

```cpp
namespace New::Data::Definitions {

struct AnimationFrame {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    float duration = 0.1f;
};

struct Animation {
    std::vector<AnimationFrame> frames;
    bool loop = true;
    float speed = 1.0f;
};

struct Sprite {
    std::string texture;
    int width = 0;
    int height = 0;
    float originX = 0.5f;
    float originY = 0.5f;
};

struct Stats {
    float health = 100.0f;
    float maxHealth = 100.0f;
    float attack = 10.0f;
    float defense = 0.0f;
    float speed = 100.0f;
    float cost = 0.0f;
};

enum class EntityType {
    Unit,
    Enemy,
    Projectile,
    Tower,
    Obstacle
};

struct EntityDef {
    std::string id;
    std::string name;
    EntityType type;
    Sprite sprite;
    Stats stats;
    std::vector<std::string> abilities;
    std::unordered_map<std::string, Animation> animations;
};

} // namespace New::Data::Definitions
```

---

## 2. 波定義（waves.json）

### 2.1 スキーマ概要

波定義は、タワーディフェンスの各波（Wave）で出現する敵の種類、数、タイミングを定義します。

### 2.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "waves": {
      "type": "array",
      "items": {
        "$ref": "#/definitions/Wave"
      }
    }
  },
  "definitions": {
    "Wave": {
      "type": "object",
      "required": ["id", "name", "spawns"],
      "properties": {
        "id": {
          "type": "string"
        },
        "name": {
          "type": "string"
        },
        "spawns": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/Spawn"
          }
        },
        "reward": {
          "$ref": "#/definitions/Reward"
        }
      }
    },
    "Spawn": {
      "type": "object",
      "required": ["entityId", "count", "delay"],
      "properties": {
        "entityId": {
          "type": "string",
          "description": "エンティティID"
        },
        "count": {
          "type": "integer",
          "minimum": 1
        },
        "delay": {
          "type": "number",
          "minimum": 0,
          "description": "前のスポーンからの遅延（秒）"
        },
        "interval": {
          "type": "number",
          "minimum": 0.1,
          "default": 1.0,
          "description": "同一エンティティのスポーン間隔（秒）"
        },
        "lane": {
          "type": "integer",
          "minimum": 0,
          "description": "レーン番号（直線型TD用）"
        },
        "path": {
          "type": "string",
          "description": "パスID（マップ型TD用）"
        }
      }
    },
    "Reward": {
      "type": "object",
      "properties": {
        "gold": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "exp": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "items": {
          "type": "array",
          "items": {
            "type": "object",
            "required": ["itemId", "count"],
            "properties": {
              "itemId": {
                "type": "string"
              },
              "count": {
                "type": "integer",
                "minimum": 1
              }
            }
          }
        }
      }
    }
  }
}
```

### 2.3 実装例

```json
{
  "waves": [
    {
      "id": "wave_01",
      "name": "第1波",
      "spawns": [
        {
          "entityId": "enemy_slime_01",
          "count": 5,
          "delay": 0.0,
          "interval": 1.0,
          "lane": 0
        },
        {
          "entityId": "enemy_slime_01",
          "count": 3,
          "delay": 3.0,
          "interval": 0.8,
          "lane": 1
        }
      ],
      "reward": {
        "gold": 100,
        "exp": 50
      }
    },
    {
      "id": "wave_02",
      "name": "第2波",
      "spawns": [
        {
          "entityId": "enemy_slime_01",
          "count": 10,
          "delay": 0.0,
          "interval": 0.5,
          "lane": 0
        },
        {
          "entityId": "enemy_goblin_01",
          "count": 2,
          "delay": 5.0,
          "interval": 2.0,
          "lane": 0
        }
      ],
      "reward": {
        "gold": 200,
        "exp": 100
      }
    }
  ]
}
```

### 2.4 C++構造体定義

```cpp
namespace New::Data::Definitions {

struct Spawn {
    std::string entityId;
    int count = 1;
    float delay = 0.0f;
    float interval = 1.0f;
    int lane = 0;  // 直線型TD用
    std::string path;  // マップ型TD用（オプション）
};

struct Reward {
    float gold = 0.0f;
    float exp = 0.0f;
    std::vector<std::pair<std::string, int>> items;  // itemId, count
};

struct WaveDef {
    std::string id;
    std::string name;
    std::vector<Spawn> spawns;
    Reward reward;
};

} // namespace New::Data::Definitions
```

---

## 3. 能力定義（abilities.json）

### 3.1 スキーマ概要

能力定義は、エンティティが使用できるスキルや特殊能力を定義します。

### 3.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "abilities": {
      "type": "array",
      "items": {
        "$ref": "#/definitions/Ability"
      }
    }
  },
  "definitions": {
    "Ability": {
      "type": "object",
      "required": ["id", "name", "type"],
      "properties": {
        "id": {
          "type": "string"
        },
        "name": {
          "type": "string"
        },
        "description": {
          "type": "string"
        },
        "type": {
          "type": "string",
          "enum": ["attack", "heal", "buff", "debuff", "summon", "special"]
        },
        "cooldown": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "cost": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "range": {
          "type": "number",
          "minimum": 0,
          "default": 0
        },
        "effects": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/Effect"
          }
        },
        "targetType": {
          "type": "string",
          "enum": ["self", "ally", "enemy", "point", "area"],
          "default": "enemy"
        }
      }
    },
    "Effect": {
      "type": "object",
      "required": ["type"],
      "properties": {
        "type": {
          "type": "string",
          "enum": ["damage", "heal", "buff", "debuff", "status"]
        },
        "value": {
          "type": "number",
          "description": "効果値（ダメージ量、回復量など）"
        },
        "duration": {
          "type": "number",
          "minimum": 0,
          "default": 0,
          "description": "持続時間（秒、0で即時）"
        },
        "statusEffectId": {
          "type": "string",
          "description": "ステータス効果ID（statusタイプ時）"
        }
      }
    }
  }
}
```

### 3.3 実装例

```json
{
  "abilities": [
    {
      "id": "ability_attack_01",
      "name": "通常攻撃",
      "description": "敵にダメージを与える",
      "type": "attack",
      "cooldown": 1.0,
      "cost": 0,
      "range": 100,
      "targetType": "enemy",
      "effects": [
        {
          "type": "damage",
          "value": 20
        }
      ]
    },
    {
      "id": "ability_heal_01",
      "name": "ヒール",
      "description": "味方を回復する",
      "type": "heal",
      "cooldown": 5.0,
      "cost": 20,
      "range": 150,
      "targetType": "ally",
      "effects": [
        {
          "type": "heal",
          "value": 50
        }
      ]
    },
    {
      "id": "ability_buff_attack",
      "name": "攻撃力強化",
      "description": "攻撃力を一時的に上げる",
      "type": "buff",
      "cooldown": 10.0,
      "cost": 30,
      "range": 0,
      "targetType": "self",
      "effects": [
        {
          "type": "buff",
          "value": 1.5,
          "duration": 10.0
        }
      ]
    }
  ]
}
```

### 3.4 C++構造体定義

```cpp
namespace New::Data::Definitions {

enum class AbilityType {
    Attack,
    Heal,
    Buff,
    Debuff,
    Summon,
    Special
};

enum class TargetType {
    Self,
    Ally,
    Enemy,
    Point,
    Area
};

enum class EffectType {
    Damage,
    Heal,
    Buff,
    Debuff,
    Status
};

struct Effect {
    EffectType type;
    float value = 0.0f;
    float duration = 0.0f;
    std::string statusEffectId;  // statusタイプ時
};

struct AbilityDef {
    std::string id;
    std::string name;
    std::string description;
    AbilityType type;
    float cooldown = 0.0f;
    float cost = 0.0f;
    float range = 0.0f;
    TargetType targetType = TargetType::Enemy;
    std::vector<Effect> effects;
};

} // namespace New::Data::Definitions
```

---

## 4. UIレイアウト定義（ui_layout.json）
>
> 現状スキーマ未整備のため要追加。ネイティブの `UIDefinitions.h` と型・アンカー表記を合わせ、`slot` を含むタイプ列挙を定義すること（`gamedev_libs_guide.md` の対応表を参照）。
> スキーマはデータ宣言のみを担い、実行ロジック（レイアウト構築、バインディング解決、イベント通知、ビヘイビア適用）はネイティブ/ローダ側で行う。

### 4.1 スキーマ概要

UIレイアウト定義は、ゲーム内のUI要素の配置、スタイル、アニメーションを定義します。

### 4.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "layouts": {
      "type": "array",
      "items": {
        "$ref": "#/definitions/UILayout"
      }
    }
  },
  "definitions": {
    "UILayout": {
      "type": "object",
      "required": ["id", "elements"],
      "properties": {
        "id": {
          "type": "string"
        },
        "name": {
          "type": "string"
        },
        "elements": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/UIElement"
          }
        }
      }
    },
    "UIElement": {
      "type": "object",
      "required": ["type", "id"],
      "properties": {
        "id": {
          "type": "string"
        },
        "type": {
          "type": "string",
          "enum": ["panel", "button", "label", "image", "progress", "list", "grid", "slot"]
        },
        "position": {
          "$ref": "#/definitions/Position"
        },
        "size": {
          "$ref": "#/definitions/Size"
        },
        "style": {
          "$ref": "#/definitions/UIStyle"
        },
        "children": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/UIElement"
          }
        },
        "properties": {
          "type": "object",
          "additionalProperties": true
        }
      }
    },
    "Position": {
      "type": "object",
      "properties": {
        "x": {
          "type": "number"
        },
        "y": {
          "type": "number"
        },
        "anchor": {
          "type": "string",
          "enum": ["top-left", "top-center", "top-right", "center-left", "center", "center-right", "bottom-left", "bottom-center", "bottom-right"],
          "default": "top-left"
        }
      }
    },
    "Size": {
      "type": "object",
      "properties": {
        "width": {
          "type": "number"
        },
        "height": {
          "type": "number"
        }
      }
    },
    "UIStyle": {
      "type": "object",
      "properties": {
        "backgroundColor": {
          "type": "string",
          "pattern": "^#[0-9A-Fa-f]{6}$"
        },
        "textColor": {
          "type": "string",
          "pattern": "^#[0-9A-Fa-f]{6}$"
        },
        "fontSize": {
          "type": "integer",
          "minimum": 8
        },
        "borderWidth": {
          "type": "integer",
          "minimum": 0
        },
        "borderColor": {
          "type": "string",
          "pattern": "^#[0-9A-Fa-f]{6}$"
        }
      }
    }
  }
}
```

### 4.5 UIローダ/バリデータ責務

- フロー: **スキーマ検証 → ネイティブ表現への変換（マッピング） → バインディング/イベント通知**。スキーマはデータ宣言のみに徹し、ローダが `UIElementDef`/`UILayoutDef` を構築してネイティブへ橋渡しする。
- `ValidateUILayoutDef` は構造・型・必須キーのチェックまで。ビヘイビア解決や `behaviorKey` による差し替えは別Registry（例: `UIBehaviorRegistry`）が担う。
- List/Grid/Slot は UI 定義ではデータ/見た目のみを保持し、ロジック差し替えは `behaviorKey` で行う（例: スクロール付きリスト/ページンググリッドなどを交換）。
- バインディング式は簡易式のみ（プレースホルダ参照＋単純な加減算/三項演算子程度）を許容し、複雑計算はコード側へ委譲。デフォルト値の解決はローダ側責務（未指定時にネイティブの既定値を埋める）。

### 4.3 実装例

```json
{
  "layouts": [
    {
      "id": "game_hud",
      "name": "ゲームHUD",
      "elements": [
        {
          "id": "top_panel",
          "type": "panel",
          "position": {
            "x": 0,
            "y": 0,
            "anchor": "top-left"
          },
          "size": {
            "width": 1920,
            "height": 100
          },
          "style": {
            "backgroundColor": "#2C2C2C",
            "borderWidth": 2,
            "borderColor": "#FFFFFF"
          },
          "children": [
            {
              "id": "gold_label",
              "type": "label",
              "position": {
                "x": 20,
                "y": 20,
                "anchor": "top-left"
              },
              "style": {
                "textColor": "#FFD700",
                "fontSize": 24
              },
              "properties": {
                "text": "ゴールド: {gold}",
                "binding": "gameState.gold"
              }
            },
            {
              "id": "health_bar",
              "type": "progress",
              "position": {
                "x": 200,
                "y": 20,
                "anchor": "top-left"
              },
              "size": {
                "width": 300,
                "height": 30
              },
              "style": {
                "backgroundColor": "#FF0000",
                "textColor": "#FFFFFF"
              },
              "properties": {
                "maxValue": 100,
                "currentValue": "{player.health}",
                "maxValueBinding": "{player.maxHealth}"
              }
            }
          ]
        }
      ]
    }
  ]
}
```

### 4.4 C++構造体定義

```cpp
namespace New::Data::Definitions {

enum class UIElementType {
    Panel,
    Button,
    Label,
    Image,
    Progress,
    List,
    Grid
};

enum class Anchor {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    Anchor anchor = Anchor::TopLeft;
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

struct Color {
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;
};

struct UIStyle {
    Color backgroundColor = {40, 40, 40, 255};
    Color textColor = {255, 255, 255, 255};
    int fontSize = 16;
    int borderWidth = 0;
    Color borderColor = {255, 255, 255, 255};
};

struct UIElement {
    std::string id;
    UIElementType type;
    Position position;
    Size size;
    UIStyle style;
    std::vector<UIElement> children;
    std::unordered_map<std::string, nlohmann::json> properties;
};

struct UILayoutDef {
    std::string id;
    std::string name;
    std::vector<UIElement> elements;
};

} // namespace New::Data::Definitions
```

---

## 5. 状態定義（states.json）

### 5.1 スキーマ概要

キャラクターの状態遷移テーブルとアニメーションイベントを定義します。`motion-animation-integration-guide.md` の StateSystem が参照し、`AnimationRegistryCache` がキャッシュします。

### 5.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["characterId", "states"],
  "properties": {
    "characterId": { "type": "string" },
    "states": {
      "type": "array",
      "items": { "$ref": "#/definitions/StateEntry" }
    }
  },
  "definitions": {
    "StateEntry": {
      "type": "object",
      "required": ["state", "clips"],
      "properties": {
        "state": { "type": "string" },
        "clips": {
          "type": "array",
          "items": { "type": "string" },
          "minItems": 1
        },
        "interruptible": { "type": "boolean", "default": true },
        "priority": {
          "type": "string",
          "enum": ["Idle", "Move", "Attack", "Hit", "Death"],
          "default": "Idle"
        },
        "blendDuration": { "type": "number", "minimum": 0, "default": 0 },
        "onComplete": {
          "type": "object",
          "properties": {
            "toState": { "type": "string" }
          }
        },
        "transitions": {
          "type": "array",
          "items": { "$ref": "#/definitions/Transition" },
          "default": []
        },
        "events": {
          "type": "array",
          "items": { "$ref": "#/definitions/AnimEvent" },
          "default": []
        }
      }
    },
    "Transition": {
      "type": "object",
      "required": ["trigger", "toState"],
      "properties": {
        "trigger": { "type": "string" },
        "toState": { "type": "string" },
        "interruptible": { "type": "boolean", "default": true }
      }
    },
    "AnimEvent": {
      "type": "object",
      "required": ["frame", "type"],
      "properties": {
        "frame": { "type": "integer", "minimum": 0 },
        "type": { "type": "string", "enum": ["hitbox_on", "hitbox_off", "particle", "sound", "custom"] },
        "args": {
          "type": "object",
          "properties": {
            "particleId": { "type": "string" },
            "offset": {
              "type": "array",
              "items": { "type": "number" },
              "minItems": 2,
              "maxItems": 2,
              "default": [0, 0]
            },
            "scale": { "type": "number", "minimum": 0, "default": 1.0 },
            "rotation": { "type": "number" }
          },
          "additionalProperties": true
        }
      }
    }
  }
}
```

### 5.3 実装メモ

- `priority` は `StatePriority` に対応（Idle/Move/Attack/Hit/Death）。
- `events` は `AnimationEventSystem` で処理（例: `hitbox_on/off`, `particle_*`）。
- 共有プリセットは `assets/characters/shared/states_<archetype>.json` に置き、キャラ定義で参照。

---

## 6. パーティクルプリセット（effects.json）

### 6.1 スキーマ概要

パーティクルエフェクトのプリセットを定義します。`particle-effect-system-design.md` のローダと DevMode プレビューが参照します。

### 6.2 JSONスキーマ

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["presets"],
  "properties": {
    "presets": {
      "type": "object",
      "patternProperties": {
        "^[a-zA-Z0-9_-]+$": {
          "type": "object",
          "required": ["texture", "maxParticles"],
          "properties": {
            "texture": { "type": "string" },
            "maxParticles": { "type": "integer", "minimum": 1, "maximum": 4096 },
            "emission": {
              "type": "object",
              "properties": {
                "burst": { "type": "integer", "minimum": 1 },
                "rate": { "type": "number", "minimum": 0 },
                "duration": { "type": "number", "minimum": 0 }
              },
              "additionalProperties": false
            },
            "lifetime": {
              "type": "object",
              "properties": {
                "min": { "type": "number", "minimum": 0 },
                "max": { "type": "number", "minimum": 0 }
              },
              "required": ["min", "max"]
            },
            "speed": {
              "type": "object",
              "properties": {
                "min": { "type": "number" },
                "max": { "type": "number" }
              },
              "required": ["min", "max"]
            },
            "angle": {
              "type": "object",
              "properties": {
                "min": { "type": "number" },
                "max": { "type": "number" }
              },
              "required": ["min", "max"]
            },
            "size": {
              "type": "object",
              "properties": {
                "min": { "type": "number", "minimum": 0 },
                "max": { "type": "number", "minimum": 0 }
              },
              "required": ["min", "max"]
            },
            "gravity": { "type": "number", "default": 0 },
            "drag": { "type": "number", "minimum": 0, "default": 0 },
            "fadeOut": { "type": "boolean", "default": true },
            "color": {
              "type": "array",
              "items": { "type": "integer", "minimum": 0, "maximum": 255 },
              "minItems": 4,
              "maxItems": 4
            },
            "blend": { "type": "string", "enum": ["alpha", "add"], "default": "alpha" }
          },
          "additionalProperties": false
        }
      },
      "additionalProperties": false
    }
  },
  "additionalProperties": false
}
```

### 6.3 実装メモ

- `emission.burst` と `emission.rate` はどちらか片方でもよい（両方ある場合は併用）。
- `color` は `[r,g,b,a]` (0-255)。未指定時はローダで WHITE をデフォルト設定。
- HotReload 対象: `assets/effects/*.json`。

---

## 7. バリデーション

### 7.1 スキーマバリデーション

すべてのJSON定義ファイルは、読み込み時にスキーマバリデーションを実行します。

```cpp
namespace New::Data::Validators {

class SchemaValidator {
public:
    bool ValidateEntityDef(const nlohmann::json& json);
    bool ValidateWaveDef(const nlohmann::json& json);
    bool ValidateAbilityDef(const nlohmann::json& json);
    bool ValidateUILayoutDef(const nlohmann::json& json);
    
    std::vector<std::string> GetErrors() const { return errors_; }
    void ClearErrors() { errors_.clear(); }

private:
    std::vector<std::string> errors_;
    
    bool ValidateRequired(const nlohmann::json& json, const std::vector<std::string>& required);
    bool ValidateType(const nlohmann::json& json, const std::string& key, const std::string& expectedType);
    bool ValidateEnum(const nlohmann::json& json, const std::string& key, const std::vector<std::string>& allowed);
    bool ValidateRange(const nlohmann::json& json, const std::string& key, double min, double max);
};

} // namespace New::Data::Validators
```

### 7.2 エラーハンドリング

```cpp
// ローダーでの使用例
bool EntityLoader::LoadFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            lastError_ = "Failed to open file: " + path;
            return false;
        }
        
        nlohmann::json json;
        file >> json;
        
        // スキーマバリデーション
        SchemaValidator validator;
        if (!validator.ValidateEntityDef(json)) {
            lastError_ = "Schema validation failed:\n";
            for (const auto& error : validator.GetErrors()) {
                lastError_ += "  - " + error + "\n";
            }
            return false;
        }
        
        // パース
        // ...
        
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        lastError_ = "JSON parse error: " + std::string(e.what());
        return false;
    } catch (const std::exception& e) {
        lastError_ = "Error: " + std::string(e.what());
        return false;
    }
}
```

### 7.3 HotReload / Validation フロー

- 監視（`HotReloadSystem` でパス登録）
- 検証（`SchemaValidator` の UI/TD/アニメ各スキーマ）
- ローダ（ネイティブ構造体への変換とキャッシュ更新）
- 適用 / ロールバック（検証失敗時は前回キャッシュを保持）
- DevMode通知（バリデーション結果と差分をエディタパネルへ出す）

---

## 8. 実装チェックリスト

### 8.1 スキーマ定義（設計進捗）

- [x] entities.json スキーマ定義
- [x] waves.json スキーマ定義
- [x] abilities.json スキーマ定義
- [ ] ui_layout.json スキーマ定義（未整備: slot/type/anchor をネイティブと合わせて追加する）
- [x] states.json スキーマ定義（particleイベント拡張含む）
- [x] effects.json スキーマ定義

### 8.2 C++構造体

- [ ] EntityDef構造体実装
- [ ] WaveDef構造体実装
- [ ] AbilityDef構造体実装
- [ ] UILayoutDef構造体実装

### 8.3 ローダー実装

- [ ] EntityLoader実装
- [ ] WaveLoader実装
- [ ] AbilityLoader実装
- [ ] UILoader実装

### 8.4 バリデータ実装

- [ ] SchemaValidator実装
- [ ] 各スキーマのバリデーション関数
- [ ] エラーメッセージ生成

---

## 9. 参考資料

---

## 10. スキーマとネイティブ実装の表記差異メモ

- UI要素タイプ/アンカー対応表（スキーマ→ネイティブ）。ローダでマッピングし、スキーマはデータ宣言のみ:

| スキーマ | ネイティブ (`UIDefinitions.h`) | メモ |
| --- | --- | --- |
| panel | Panel | コンテナ |
| button | Button | - |
| label | Text | 表示テキスト |
| image | Image | - |
| progress | ProgressBar | HP/経験値等 |
| list | Container (+repeat) | ロジックは `behaviorKey` で差し替え |
| grid | Container (+repeat) | 同上 |
| slot | Slot | インベントリ等。スキーマにも追加済み |

| スキーマアンカー | ネイティブアンカー | 備考 |
| --- | --- | --- |
| top-left | top_left | - |
| top-center | topCenter | - |
| top-right | top_right | - |
| center-left | centerLeft | - |
| center | center | - |
| center-right | centerRight | - |
| bottom-left | bottom_left | - |
| bottom-center | bottomCenter | - |
| bottom-right | bottom_right | - |

- ローダで上記マッピングを許容し、変換後にネイティブへ渡す（スキーマ側では命名を固定する）。
- アンカー変換テーブルと%サイズ解釈（親基準で割合を適用し、アンカー原点で補正）は UI ローダ内で一元管理すること。

## 11. セーブ/互換性メモ

- セーブデータ（進行度・設定）は `schemaVersion` を持たず最新版前提。破壊的変更を行う場合はローダで検知し、互換変換がない場合は読み取り専用で警告し、新形式で再保存するようUIから促す。  
- セーブは Victory と設定変更時に書き出す。スキーマ更新時は旧フィールドを無視するよりも、移行処理またはエラー提示を優先してデータ破損を防ぐ。  
- ホットリロード時はセーブを自動変更しない（手動保存またはVictoryトリガでのみ更新）。

### 11.1 設定セクション（キーとデフォルト）

- `settings.audio.master` (100), `settings.audio.se`, `settings.audio.bgm`  
- `settings.display.resolution` (system), `fullscreen` (true/false), `fpsCap` (60), `vsync` (true), `uiScale` (1.0)  
- `settings.input.bindings`（リマップ表。未知キーは無視、欠落は既定値補完）  
- `settings.accessibility.colorPreset` (standard), `fontScale` (1.0), `contrast` (standard), `subtitle` (on/off), `intensity` (standard)  
- 互換ポリシー: 未知キーは無視、欠落は既定値補完。バリデーション失敗時は設定全体を既定値で復旧し警告。

### 11.2 セーブ（必須フィールドの軽量版メモ）

- `saveSlots`: 固定3スロット。  
- `partyPresets`: 3プリセット、各 `main[3]`, `sub[5]`, `abilities[2]`, `costCap`。欠落スロットは空で補完。  
- `settings`: 上記11.1の最小セット。  
- `backups`: ローテーションバックアップのメタ情報（保存時刻、元スロット）。  
- 互換ポリシー: 未知フィールドは無視、欠落は既定値で補完。破損時はバックアップへロールバックし警告。

### 11.3 エラーハンドリング補足（設定/ステージ）

- 未知キーは無視、欠落は既定値で補完。致命エラー時は読み取り専用で警告を表示し、新規保存を促す。  
- ステージデータ欠落時はスキップ/代替ステージを提示し、キャッシュがあれば前回データを再利用。  
- 設定適用の失敗はローダで検知し、設定を既定値に戻した上で警告。

### 11.5 セキュリティ/チート検知方針

- セーブ改変検知は行わない（未知キー無視・欠落補完のみ）。リリースでもそのまま許容する。

## 12. スキーマ統合方針（マスター運用）

- 本ドキュメントをスキーマ定義のマスターとし、audio / particle / camera / stage / ui / 編成（main3+sub5+abilities2） / 難易度 / 報酬 / settings を一元管理する。  
- 各設計ドキュメントにスキーマ断片を記す場合、ここへ対応表・差分を反映する運用とする。  
- 追加/変更時は: スキーマ更新 → バリデーション項目追記 → ネイティブ構造体/ローダの差分確認 → HotReload回帰で確認。

### 11.4 スキーマ検証ポイント（編成プリセット / ステージ）

- 編成プリセット: `main[3]`, `sub[5]`, `abilities[2]`, `costCap`。必須: 配列サイズと型。欠落スロットは空で補完。未知キーは無視。  
- ステージ定義: 必須 `id`, `displayName`, `life`, `difficulty`(hp/atk/reward), `waves`, `recommendedPower`, `rewards`, `difficultyUnlock`(Easy/Normal/Hard)。型/enumをチェック。  
- エラー扱い: 型/必須欠落は致命エラー（読み取り専用＋警告）。値範囲逸脱は警告扱いで既定値補完可。  
- HotReload: 検証→ネイティブ変換→適用。失敗部分は適用せず、前回キャッシュを保持。「partial」ステータスで通知。

- [コアアーキテクチャ設計](core-architecture-design.md)
- [内部エディタ設計](internal-editor-design.md)
- [設計原則と制約](design-principles-and-constraints.md)

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ開発者向け
