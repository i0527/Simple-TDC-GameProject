# JSON スキーマ設計 - 確定版

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 1.0.0（確定版）  
**最終更新**: 2025-12-08

---

## 📑 目次

1. [設計方針](#設計方針)
2. [キャラクター定義スキーマ](#キャラクター定義スキーマ)
3. [スキル定義スキーマ](#スキル定義スキーマ)
4. [ステージ定義スキーマ](#ステージ定義スキーマ)
5. [エフェクト定義スキーマ](#エフェクト定義スキーマ)
6. [敵定義スキーマ](#敵定義スキーマ)
7. [ファイル構成](#ファイル構成)

---

## 設計方針

### 基本戦略

```yaml
実装アプローチ:
  優先度: シンプルさ重視 → 最小限の機能で v1 リリース
  拡張性: フラット構造 + タグ/フラグ運用で柔軟性確保
  バランス調整: JSON で全パラメータ外部化
  エディタ: ImGui で直感的に編集可能な構造
```

### 選択方針サマリー

| 項目 | 選択 | 理由 |
|------|------|------|
| **キャラクター定義** | A: フラット構造 | シンプル・高速実装。敵も同じ構造で統一 |
| **スキル定義** | A: シンプル型 | 3 種別（パッシブ/インタラプト/イベント）で対応可能 |
| **ステージ定義** | A: ウェーブベース | タイムスケジュール管理が直感的 |
| **エフェクト定義** | A: 統合型 | 種類少ない ↔ 高頻度使用の最適バランス |
| **敵との共通化** | A: 共通化 | `is_enemy` フラグ + タグで敵/プレイヤーを分離 |

### 描画モード運用方針（v1 時点）

- 取り扱い: `draw_type` は `"sprite"` / `"parts_animation"` を許容するが、v1 実装は **スプライトのみ対応**。`parts_animation` は未実装扱いとし、プレースホルダー描画＋警告ログで「気付きやすい失敗」にする。  
- 入力検証: `draw_type: "parts_animation"` を検知したらロード時に WARN を出し、実行時は矩形描画にフォールバックする（クラッシュさせないが、見た目で異常が分かる）。  
- 拡張余地: 将来パーツアニメを導入する際は、この分岐に実装を差し込むだけで済むよう、レンダラ側は `draw_type` → ハンドラのディスパッチ構造にしておく。  
- エディタ連携: エディタでパーツアニメをスプライトシートに焼く運用も許容し、その場合は `draw_type: "sprite"` として出力させる。  
- テスト観点: 「`parts_animation` 指定で WARN が出ること」「フォールバック描画になること」をスモークテストに追加しておくと検知しやすい。

---

## キャラクター定義スキーマ

### スキーマ構造

```json
{
  "characters": [
    {
      "id": "char_001",
      "name": "炎猫",
      "description": "火炎系の基本ユニット",
      "rarity": 2,
      "type": "main",
      "is_enemy": false,
      "cost": 300,
      "cooldown": 8.5,
      "stats": {
        "hp": 120,
        "attack": 45,
        "attack_speed": 1.2,
        "range": 150
      },
      "draw_type": "parts_animation",
      "display": {
        "sprite_sheet": "sprites/char_001/sheet.png",
        "animation_idle": {
          "type": "parts_animation",
          "parts": [
            { "part_name": "body", "frame_start": 0, "frame_count": 4 },
            { "part_name": "eyes", "frame_start": 4, "frame_count": 4 }
          ],
          "fps": 8,
          "loop": true
        },
        "animation_attack": {
          "type": "parts_animation",
          "parts": [
            { "part_name": "body", "frame_start": 8, "frame_count": 6 },
            { "part_name": "weapon", "frame_start": 0, "frame_count": 6 }
          ],
          "fps": 10,
          "loop": false
        }
      },
      "skills": [
        {
          "id": "skill_001",
          "type": "passive"
        }
      ],
      "abilities": [
        {
          "id": "ability_001",
          "slot": 0
        }
      ],
      "evolution": {
        "evolve_to": "char_001_evo1",
        "required_materials": [
          { "item_id": "material_fire_001", "count": 3 }
        ],
        "level_required": 20
      },
      "tags": []
    }
  ]
}
```

### 項目詳細

| 項目 | 型 | 説明 |
|------|-----|------|
| `id` | string | ユニット ID（一意） |
| `name` | string | 表示名 |
| `description` | string | 説明テキスト |
| `rarity` | int (1-5) | レアリティ |
| `type` | string | `"main"` (出撃制限) / `"sub"` (無制限召喚) |
| `is_enemy` | bool | `true` なら敵（プレイヤー側は `false`） |
| `cost` | int | CP コスト（敵は使用しない） |
| `cooldown` | float | クールタイム秒数 |
| `stats` | object | HP・攻撃力・射程など |
| `draw_type` | string | `"parts_animation"` (パーツアニメ) / `"sprite"` (スプライト) |
| `display` | object | アニメーション定義（詳細後述） |
| `skills` | array | スキル参照（パッシブ/イベント型） |
| `abilities` | array | アビリティ参照（インタラプト型、敵は空配列） |
| `evolution` | object | 進化条件・進化先 |
| `tags` | array | 敵タグ（例：`["boss"]`, `["air"]`） |

#### Asepriteスプライト指定（新）

- `display.sprite_actions`: `action -> Aseprite JSONパス` を記述。JSON側の `meta.frameTags` 名とアクション名（例: `"idle"`, `"walk"`, `"attack"`, `"death"`) を一致させておくと自動でタグを拾う。  
- `display.atlas_texture`: 画像パス（任意）。空の場合はAseprite JSONの `meta.image` を使用して読み込む。  
- 再生速度・ループ可否はゲーム内デフォルト（例: idle 8fps ループ、walk 12fps ループ、attack/death 非ループ）を適用。`frames[*].duration` があればそれを優先。`frameTags` が無い場合は全フレームを単一クリップとしてWARNしつつ再生。
- `display.icon`: 編成UIなどで使うアイコン画像（任意）。空の場合は `icon.png` などを同フォルダから自動検出。  

サンプル:

```json
"display": {
  "atlas_texture": "assets/sprites/hero.png",
  "sprite_actions": {
    "idle": "assets/sprites/hero_idle.json",
    "walk": "assets/sprites/hero_walk.json",
    "attack": "assets/sprites/hero_attack.json",
    "death": "assets/sprites/hero_death.json"
  },
  "icon": "assets/sprites/hero_icon.png"
}
```

### ディレクトリ運用（mainCharacters / subCharacters）

- キャラ定義をスプライトと同じディレクトリに配置：  
  `assets/mainCharacters/<charId>/character.json`（または `*.character.json`）  
  `assets/mainCharacters/<charId>/sheet.png`  
  `assets/mainCharacters/<charId>/idle.json` / `walk.json` / `attack.json` ...  
- サブキャラも同様に `assets/subCharacters/<charId>/...`。分類は便宜上で、ロード動作は同一。  
- `character.json` 内の `display.sprite_actions` や `atlas_texture` は **フォルダ相対パス** で記述し、ローダーが親ディレクトリで解決する。  
- Aseprite JSONの `meta.image` が相対パスの場合も同一ディレクトリで解決。  
- 既存の `assets/definitions/entities_*.json` も従来どおり読み込まれる（後方互換）。ID衝突時は後勝ちでWARNを出す運用を推奨。

### アニメーション定義

#### パーツアニメーション型（推奨）

```json
{
  "animation_idle": {
    "type": "parts_animation",
    "parts": [
      {
        "part_name": "body",
        "frame_start": 0,
        "frame_count": 4,
        "layer": 1
      },
      {
        "part_name": "eyes",
        "frame_start": 4,
        "frame_count": 4,
        "layer": 2
      }
    ],
    "fps": 8,
    "loop": true
  }
}
```

**メリット**:

- パーツごと独立的にアニメーション制御可能
- メモリ効率が良い
- エディタで「パーツの表示/非表示」「レイヤー順」を変更可能

#### スプライトアニメーション型（代替案）

```json
{
  "animation_walk": {
    "type": "sprite",
    "sprite_sheet": "sprites/char_001/walk.png",
    "frame_width": 64,
    "frame_height": 64,
    "frame_count": 8,
    "fps": 10,
    "loop": true
  }
}
```

**メリット**:

- シンプルな実装
- グラフィック制作者が直感的に作成可能

---

## スキル定義スキーマ

### スキル 3 種類

```yaml
スキルの分類:
  passive (パッシブ):
    説明: "常時発動のスキル"
    発動: "自動（常時または特定条件）"
    例: "HP自動回復 / 攻撃時に追加ダメージ / 敵接近時に速度UP"

  interrupt (インタラプト):
    説明: "プレイヤー手動発動"
    対象: "メインキャラのアビリティ枠のみ"
    例: "攻撃力アップ / 敵グループを一時停止 / 範囲攻撃"

  event (イベント):
    説明: "条件付き自動発動"
    例: "HP20%以下で自動回復 / ボス倒した時ボーナス発動 / N秒ごとに攻撃"
```

### スキーマ構造

```json
{
  "skills": [
    {
      "id": "skill_001",
      "name": "炎撃",
      "description": "前方範囲に火炎ダメージを与える",
      "skill_type": "passive",
      "trigger": "on_attack",
      "effect": {
        "type": "damage",
        "damage_type": "fire",
        "base_damage": 30,
        "scaling": "attack * 1.5",
        "range": 200,
        "area_radius": 100
      }
    },
    {
      "id": "skill_002",
      "name": "攻撃力アップ",
      "description": "味方全体の攻撃力を一時的に上げる",
      "skill_type": "interrupt",
      "cooldown": 30,
      "cost": 500,
      "effect": {
        "type": "buff",
        "target": "all_allies",
        "stat": "attack",
        "value": 50,
        "duration": 20
      }
    },
    {
      "id": "skill_003",
      "name": "生存本能",
      "description": "HP が 20% 以下になると自動発動、HP を 30% 回復",
      "skill_type": "event",
      "trigger": "on_hp_below_20",
      "cooldown": 60,
      "effect": {
        "type": "heal",
        "target": "self",
        "heal_value": 30,
        "heal_type": "percentage"
      }
    }
  ]
}
```

### スキル種別ごとの項目

#### パッシブ型（passive）

```json
{
  "id": "skill_001",
  "name": "炎撃",
  "skill_type": "passive",
  "trigger": "on_attack",
  "effect": { /* ... */ }
}
```

| trigger | 説明 |
|---------|------|
| `on_attack` | 攻撃時に発動 |
| `on_take_damage` | ダメージを受けた時に発動 |
| `on_kill_enemy` | 敵を倒したときに発動 |
| `continuous` | 常時（時間経過で定期発動） |

#### インタラプト型（interrupt）

```json
{
  "id": "skill_002",
  "name": "攻撃力アップ",
  "skill_type": "interrupt",
  "cooldown": 30,
  "cost": 500,
  "effect": { /* ... */ }
}
```

**注**: 使用時にプレイヤーが手動発動。クールタイムとコストを指定。

#### イベント型（event）

```json
{
  "id": "skill_003",
  "name": "生存本能",
  "skill_type": "event",
  "trigger": "on_hp_below_20",
  "cooldown": 60,
  "effect": { /* ... */ }
}
```

| trigger | 説明 |
|---------|------|
| `on_hp_below_20` | HP が 20% 以下で発動 |
| `on_hp_below_50` | HP が 50% 以下で発動 |
| `on_time_interval` | 一定時間ごとに発動 |
| `on_wave_start` | ウェーブ開始時に発動 |

---

## ステージ定義スキーマ

### スキーマ構造

```json
{
  "stages": [
    {
      "id": "stage_001",
      "name": "初心者ステージ 1",
      "chapter": 1,
      "difficulty": "easy",
      "description": "最初のバトル。敵の動きに慣れましょう",
      "recommended_power": 100,
      "width": 2000,
      "player_base_hp": 1000,
      "time_limit": 300,
      "waves": [
        {
          "wave_id": "wave_001",
          "spawn_time": 0,
          "enemy_groups": [
            {
              "enemy_id": "enemy_001",
              "count": 3,
              "spawn_interval": 2.0
            }
          ]
        },
        {
          "wave_id": "wave_002",
          "spawn_time": 15,
          "enemy_groups": [
            {
              "enemy_id": "enemy_001",
              "count": 2,
              "spawn_interval": 2.0
            },
            {
              "enemy_id": "enemy_002",
              "count": 1,
              "spawn_interval": 0
            }
          ]
        },
        {
          "wave_id": "wave_003",
          "spawn_time": 35,
          "enemy_groups": [
            {
              "enemy_id": "enemy_boss_001",
              "count": 1,
              "spawn_interval": 0
            }
          ]
        }
      ],
      "rewards": {
        "gold": 500,
        "exp": 100,
        "items": [
          {
            "item_id": "material_fire_001",
            "drop_rate": 0.5
          }
        ]
      }
    }
  ]
}
```

### 項目詳細

| 項目 | 型 | 説明 |
|------|-----|------|
| `id` | string | ステージ ID |
| `name` | string | 表示名 |
| `chapter` | int | チャプター番号 |
| `difficulty` | string | `"easy"` / `"normal"` / `"hard"` |
| `description` | string | ステージ説明（ホーム画面に表示） |
| `recommended_power` | int | 推奨戦力 |
| `width` | int | ステージの横幅（ピクセル） |
| `player_base_hp` | int | プレイヤー拠点の HP |
| `time_limit` | int | 時間制限（秒）。`0` なら無制限 |
| `waves` | array | ウェーブ定義 |
| `rewards` | object | クリア報酬（ゴールド・経験値・アイテム） |

### ウェーブ構造

```json
{
  "wave_id": "wave_001",
  "spawn_time": 0,
  "enemy_groups": [
    {
      "enemy_id": "enemy_001",
      "count": 3,
      "spawn_interval": 2.0
    }
  ]
}
```

| 項目 | 説明 |
|------|------|
| `spawn_time` | ウェーブ開始時刻（ステージ開始からの秒数） |
| `enemy_groups` | 敵グループの配列 |
| `enemy_id` | 敵 ID（enemies.json で定義） |
| `count` | 敵の数 |
| `spawn_interval` | 敵の出現間隔（秒）。`2.0` なら 2 秒ごとに 1 体出現 |

---

## エフェクト定義スキーマ

### スキーマ構造

```json
{
  "effects": [
    {
      "id": "effect_fire_explosion",
      "name": "火炎爆発",
      "type": "particle",
      "duration": 1.5,
      "animation": {
        "sprite_sheet": "sprites/effects/explosion_fire.png",
        "frame_width": 64,
        "frame_height": 64,
        "frame_count": 12,
        "fps": 10,
        "loop": false
      },
      "sound": {
        "sfx_id": "sfx_explosion_fire",
        "volume": 0.8,
        "play_delay": 0.1
      },
      "physics": {
        "scale_start": 1.0,
        "scale_end": 0.5,
        "fade_in": 0.2,
        "fade_out": 0.3
      }
    },
    {
      "id": "effect_hit_flash",
      "name": "ヒットエフェクト（フラッシュ）",
      "type": "flash",
      "duration": 0.2,
      "color": [255, 255, 255],
      "intensity": 0.8,
      "sound": {
        "sfx_id": "sfx_hit",
        "volume": 0.6
      }
    },
    {
      "id": "effect_heal",
      "name": "回復エフェクト",
      "type": "particle",
      "duration": 1.0,
      "animation": {
        "sprite_sheet": "sprites/effects/heal.png",
        "frame_width": 64,
        "frame_height": 64,
        "frame_count": 8,
        "fps": 10
      },
      "sound": {
        "sfx_id": "sfx_heal",
        "volume": 0.5,
        "play_delay": 0.0
      }
    }
  ]
}
```

### 項目詳細

| 項目 | 型 | 説明 |
|------|-----|------|
| `id` | string | エフェクト ID |
| `name` | string | 表示名 |
| `type` | string | `"particle"` (パーティクル) / `"flash"` (フラッシュ) / `"beam"` (ビーム) |
| `duration` | float | エフェクト継続時間（秒） |
| `animation` | object | アニメーション定義 |
| `sound` | object | サウンド定義 |
| `physics` | object | 物理パラメータ（スケール・フェード） |

### エフェクト種類

#### Particle（パーティクル）

最一般的なエフェクト。爆発・煙・光など。

```json
{
  "type": "particle",
  "animation": {
    "sprite_sheet": "sprites/effects/explosion.png",
    "frame_count": 12,
    "fps": 10
  },
  "physics": {
    "scale_start": 1.0,
    "scale_end": 0.5,
    "fade_out": 0.3
  }
}
```

#### Flash（フラッシュ）

画面全体またはユニットを光で明滅。ヒット感表現。

```json
{
  "type": "flash",
  "color": [255, 255, 255],
  "intensity": 0.8,
  "duration": 0.2
}
```

#### Beam（ビーム）

レーザー・光線効果。

```json
{
  "type": "beam",
  "animation": { /* ... */ },
  "physics": {
    "width": 50,
    "length": 500
  }
}
```

---

## 敵定義スキーマ

### 方針：キャラクターと共通化

敵は「`is_enemy: true` のキャラクター」として定義。タグやフラグで敵/プレイヤーを分離。

### スキーマ構造

```json
{
  "enemies": [
    {
      "id": "enemy_001",
      "name": "弱い敵",
      "description": "基本的な敵ユニット",
      "is_enemy": true,
      "type": "sub",
      "rarity": 1,
      "cost": 0,
      "cooldown": 0,
      "stats": {
        "hp": 80,
        "attack": 20,
        "attack_speed": 1.0,
        "range": 100
      },
      "draw_type": "sprite",
      "display": {
        "sprite_sheet": "sprites/enemies/enemy_001.png",
        "animation_walk": {
          "type": "sprite",
          "frame_width": 64,
          "frame_height": 64,
          "frame_count": 4,
          "fps": 8
        },
        "animation_attack": {
          "type": "sprite",
          "frame_width": 64,
          "frame_height": 64,
          "frame_count": 6,
          "fps": 10
        }
      },
      "ai": {
        "behavior": "walk_and_attack",
        "target_priority": "closest",
        "movement_speed": 100
      },
      "skills": [
        {
          "id": "skill_004",
          "type": "passive"
        }
      ],
      "abilities": [],
      "tags": [],
      "rewards": {
        "gold": 50,
        "exp": 25
      }
    },
    {
      "id": "enemy_boss_001",
      "name": "ボス敵",
      "description": "強力なボス敵",
      "is_enemy": true,
      "type": "main",
      "rarity": 5,
      "stats": {
        "hp": 500,
        "attack": 80,
        "attack_speed": 1.5,
        "range": 150
      },
      "draw_type": "parts_animation",
      "display": {
        "sprite_sheet": "sprites/enemies/boss_001/sheet.png",
        "animation_idle": {
          "type": "parts_animation",
          "parts": [
            { "part_name": "body", "frame_start": 0, "frame_count": 4 }
          ],
          "fps": 8
        }
      },
      "ai": {
        "behavior": "boss_attack_pattern",
        "target_priority": "closest",
        "movement_speed": 80,
        "attack_pattern": "alternate"
      },
      "skills": [
        {
          "id": "skill_005",
          "type": "passive"
        },
        {
          "id": "skill_006",
          "type": "event"
        }
      ],
      "abilities": [],
      "tags": ["boss"],
      "rewards": {
        "gold": 500,
        "exp": 300,
        "items": [
          { "item_id": "rare_material_001", "drop_rate": 0.3 }
        ]
      }
    }
  ]
}
```

### 敵専用項目

| 項目 | 型 | 説明 |
|------|-----|------|
| `is_enemy` | bool | 常に `true` |
| `ai` | object | AI 挙動定義 |
| `ai.behavior` | string | `"walk_and_attack"` / `"boss_attack_pattern"` / `"dodge"` など |
| `ai.target_priority` | string | `"closest"` / `"weakest"` / `"strongest"` |
| `ai.movement_speed` | int | 敵の移動速度（ピクセル/秒） |
| `abilities` | array | 常に空配列（敵はアビリティを持たない） |
| `tags` | array | 敵タグ（例：`["boss"]`, `["air"]`, `["water"]`） |
| `rewards` | object | 敵を倒したときの報酬 |

### AI 挙動の種類

```yaml
behavior_types:
  walk_and_attack:
    説明: "プレイヤーに向かって歩き、攻撃範囲内で攻撃"
    用途: "通常の敵"

  boss_attack_pattern:
    説明: "パターン化された攻撃（複数の異なる攻撃を順序立てて実行）"
    用途: "ボス敵"

  dodge:
    説明: "プレイヤーの攻撃を避けながら移動"
    用途: "素早い敵"

  fixed_position:
    説明: "移動しない（砲台タイプ）"
    用途: "遠距離攻撃固定敵"
```

---

## ファイル構成

### ディレクトリ構成

```
data/
├─ characters/
│  └─ characters.json         # キャラクター定義（プレイヤー・敵共通）
├─ enemies/
│  └─ enemies.json            # 敵定義（characters.json と同じ構造）
├─ skills/
│  └─ skills.json             # スキル定義（パッシブ・イベント・インタラプト）
├─ stages/
│  └─ stages.json             # ステージ定義（ウェーブ構造）
├─ effects/
│  └─ effects.json            # エフェクト定義
└─ sounds/
   └─ sounds.json             # サウンド定義（BGM・SE）
```

### JSON ファイルの責務

| ファイル | 責務 |
|---------|------|
| `characters.json` | プレイヤー側のキャラクター定義 |
| `enemies.json` | 敵キャラクター定義（`is_enemy: true`） |
| `skills.json` | 全スキル定義（3 種別共通） |
| `stages.json` | ステージ・ウェーブ定義 |
| `effects.json` | パーティクル・フラッシュなどのエフェクト |
| `sounds.json` | BGM・SE 定義 |

---

## JSON 仕様の実装上のポイント

### 🎯 Point 1: フラット構造とタグの活用

```json
{
  "id": "char_001",
  "is_enemy": false,
  "type": "main",
  "tags": ["fire_type", "ranged"],
  "skills": [ /* 参照 ID のみ */ ]
}
```

**メリット**:

- 構造がシンプル
- エディタで直感的に編集
- タグで拡張可能（新しいタグを追加するだけ）

### 🎯 Point 2: 参照は ID で統一

```json
{
  "skills": [
    { "id": "skill_001", "type": "passive" }
  ],
  "abilities": [
    { "id": "ability_001", "slot": 0 }
  ]
}
```

**メリット**:

- スキル・アビリティを外部ファイルで管理
- 修正時に 1 ヶ所変更で全 JSON に反映

### 🎯 Point 3: パラメータは全て外部化

```json
{
  "stats": {
    "hp": 120,
    "attack": 45,
    "attack_speed": 1.2,
    "range": 150
  }
}
```

**メリット**:

- JSON 編集だけでバランス調整可能
- コード変更不要

### 🎯 Point 4: アニメーション型の柔軟性

```json
{
  "draw_type": "parts_animation",  // または "sprite"
  "display": { /* アニメーション定義 */ }
}
```

**メリット**:

- 同じスキーマで複数の描画方式に対応
- 今後新しい描画方式を追加しやすい

---

## 次のステップ

### エディタ実装への橋渡し

このスキーマをベースに、ImGui エディタを実装します：

- [ ] JSON パースを EnTT + nlohmann/json で実装
- [ ] 各スキーマの C++ 構造体を定義（deserialize 機能付き）
- [ ] ImGui UI で JSON 編集可能にする
- [ ] ホットリロード機能実装（F1 キー起動）
- [ ] バリデーション機能（不正な ID 参照の検出など）

### アーキテクチャ層への統合

```
Core Layer
  ├─ JSON Loader (nlohmann/json)
  ├─ Resource Manager
  └─ File Watcher (ホットリロード)

Game Layer
  ├─ Character Manager
  ├─ Enemy Manager
  ├─ Skill System
  ├─ Stage Manager
  └─ Effect Manager

TD Layer (ECS)
  ├─ Character Entity
  ├─ Enemy Entity
  ├─ Skill Component
  └─ Rendering Component
```

---

## 参考：JSON 編集時のチェックリスト

### キャラクター追加時

- [ ] `id` は一意か？
- [ ] `skills` の ID は `skills.json` に存在するか？
- [ ] `abilities` の ID は存在するか？ （メインのみ）
- [ ] `stats` の `range` は妥当か？
- [ ] `evolution` の `evolve_to` は存在するか？
- [ ] `draw_type` と `display` の内容は一致しているか？

### スキル追加時

- [ ] `id` は一意か？
- [ ] `skill_type` は `"passive"` / `"interrupt"` / `"event"` のいずれか？
- [ ] `trigger` はスキルタイプに合致しているか？
- [ ] `effect.type` は既知の型か？
- [ ] `cooldown` や `cost` は interrupt のみに指定しているか？

### ステージ追加時

- [ ] `id` は一意か？
- [ ] `waves` の `spawn_time` は昇順か？
- [ ] `enemy_id` は `enemies.json` に存在するか？
- [ ] `width` はゲーム画面の想定幅と一致しているか？
- [ ] `time_limit` は妥当か？
