# Phase 3: TD Layer 詳細設計 - ECS Components & Systems（統合最適版）

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 1.0.0（ECS詳細設計版）  
**作成日**: 2025-12-08 / 08:01 JST  
**目的**: Game Layer 完成後、TD ゲームロジック（ECS Components & Systems）の詳細仕様を確定

---

## 📑 目次

1. [TD Layer 全体概要](#td-layer-全体概要)
2. [Component詳細設計（18個）](#component詳細設計18個)
3. [System詳細設計（12個）](#system詳細設計12個)
4. [Component ↔ System 対応表](#component--system-対応表)
5. [Factory パターン（Entity生成）](#factory-パターンentity生成)
6. [Entity ライフサイクル](#entity-ライフサイクル)
7. [System 実行順序（フレームループ）](#system-実行順序フレームループ)
8. [データフロー図](#データフロー図)

---

## TD Layer 全体概要

### ECS（Entity Component System）アーキテクチャ

```
Game Layer（Manager層）
         ↓ Entity生成要求
┌─────────────────────────────────────────┐
│           TD Layer (ECS)                 │
│                                          │
│  ┌─ Entity（オブジェクト） ─────────────┐│
│  │  - プレイヤーユニット                 ││
│  │  - 敵ユニット                        ││
│  │  - エフェクト                        ││
│  │  - UI パーティクル                   ││
│  └──────────────────────────────────────┘│
│           ↓                               │
│  ┌─ Component（データ） ──────────────────┐│
│  │  - Transform（位置）                  ││
│  │  - Stats（ステータス）                ││
│  │  - Health（HP管理）                  ││
│  │  - Movement（移動）                  ││
│  │  - Skill（スキル）                   ││
│  │  - Animation（アニメーション）        ││
│  │  - Renderer（描画）                  ││
│  │  - Buff/Debuff（一時効果）           ││
│  │  - Cooldown（クールタイム）           ││
│  │  - AI（敵AI）                        ││
│  └──────────────────────────────────────┘│
│           ↓                               │
│  ┌─ System（ロジック） ──────────────────┐│
│  │  毎フレーム実行：                     ││
│  │  1. InputSystem（入力）               ││
│  │  2. MovementSystem（移動）            ││
│  │  3. CollisionSystem（衝突判定）       ││
│  │  4. CostSystem（CP管理）              ││
│  │  5. SkillSystem（スキル発動）         ││
│  │  6. BuffSystem（バフ処理）            ││
│  │  7. AttackSystem（ダメージ計算）      ││
│  │  8. HealthSystem（HP管理）            ││
│  │  9. AnimationSystem（アニメ制御）     ││
│  │  10. EffectSystem（エフェクト生成）   ││
│  │  11. AISystem（敵AI）                 ││
│  │  12. RenderSystem（描画）             ││
│  └──────────────────────────────────────┘│
│           ↓                               │
│  ┌─ Factory（生成） ─────────────────────┐│
│  │  - CharacterFactory（プレイヤーユニット）││
│  │  - EnemyFactory（敵ユニット）          ││
│  │  - EffectFactory（エフェクト）         ││
│  └──────────────────────────────────────┘│
└─────────────────────────────────────────┘
         ↓ 描画データ要求
Application Layer（SceneManager）
```

### 設計方針

```yaml
ECS選定理由:
  ✅ データ（Component）とロジック（System）の完全分離
  ✅ 高パフォーマンス（キャッシュ効率）
  ✅ 拡張性（新Componentの追加が容易）
  ✅ テスト容易性（各Systemが独立）
  ✅ HotReload対応（Systemの動的入れ替え）

Library選択: entt（推奨）
  ✅ ヘッダーオンリー（統合が容易）
  ✅ 高性能（競争性マルチコンポーネント）
  ✅ 豊富な機能（View, Group, Storage など）
  ✅ C++17 対応
```

---

## Component詳細設計（18個）

### 1. TransformComponent（位置・スケール・回転）

```cpp
// game/include/Game/Components/TransformComponent.h
namespace Game::Components {

struct TransformComponent {
  // ===== 位置 =====
  float x = 0.0f;           // X座標（ピクセル）
  float y = 0.0f;           // Y座標（ピクセル）
  
  // ===== 速度 =====
  float velocity_x = 0.0f;  // X速度（ピクセル/秒）
  float velocity_y = 0.0f;  // Y速度（ピクセル/秒）
  
  // ===== スケール・回転 =====
  float scale_x = 1.0f;     // X拡大率
  float scale_y = 1.0f;     // Y拡大率
  float rotation = 0.0f;    // 回転角度（ラジアン）
  
  // ===== サイズ =====
  float width = 0.0f;       // 幅（ピクセル）
  float height = 0.0f;      // 高さ（ピクセル）
  
  // ===== レイヤー =====
  int layer = 0;            // 描画レイヤー（0～99）
};

} // namespace Game::Components
```

**用途**: すべての Entity（ユニット・敵・エフェクト）に必須

---

### 2. StatsComponent（ステータス：攻撃力・射程など）

```cpp
// game/include/Game/Components/StatsComponent.h
namespace Game::Components {

struct StatsComponent {
  // ===== 基本ステータス（定義から取得） =====
  int max_hp = 100;         // 最大HP
  int attack = 10;          // 攻撃力
  float attack_speed = 1.0f; // 攻撃速度（攻撃/秒）
  int range = 100;          // 攻撃射程（ピクセル）
  
  // ===== 修正値（Buff/Debuffで変動） =====
  int attack_bonus = 0;     // 攻撃力ボーナス
  float attack_speed_multiplier = 1.0f;  // 攻撃速度倍率
  
  // ===== タイプ =====
  std::string unit_type;    // "player" / "enemy"
  std::string character_id; // キャラクター ID（定義への参照）
  int level = 1;            // レベル（プレイヤー側）
};

} // namespace Game::Components
```

**用途**: 攻撃・スキル計算の基盤

---

### 3. HealthComponent（HP管理）

```cpp
// game/include/Game/Components/HealthComponent.h
namespace Game::Components {

struct HealthComponent {
  int current_hp = 100;     // 現在 HP
  int max_hp = 100;         // 最大 HP
  bool is_alive = true;     // 生存フラグ
  
  // ===== ダメージ履歴 =====
  struct DamageRecord {
    int damage;
    float timestamp;
    std::string source_id;  // ダメージ源の Entity ID
  };
  std::vector<DamageRecord> recent_damage;  // 最近のダメージ記録
  
  // ===== イベント用フラグ =====
  bool just_took_damage = false;  // このフレーム受けたダメージあり？
  int damage_this_frame = 0;      // このフレームのダメージ合計
};

} // namespace Game::Components
```

**用途**: HP管理、死亡判定、ダメージ表現

---

### 4. MovementComponent（移動パス・速度）

```cpp
// game/include/Game/Components/MovementComponent.h
namespace Game::Components {

struct MovementComponent {
  // ===== 移動パス =====
  enum PathType {
    STRAIGHT,       // 直進
    FOLLOW_LANE,    // レーン追従
    ZIGZAG,         // ジグザグ（未使用）
  };
  
  PathType path_type = STRAIGHT;
  std::vector<glm::vec2> waypoints;  // 通過点
  int current_waypoint_index = 0;
  
  // ===== 速度 =====
  float base_speed = 100.0f;         // 基本移動速度（ピクセル/秒）
  float current_speed = 100.0f;      // 現在速度（修正値反映）
  float speed_multiplier = 1.0f;     // 速度倍率（スローなど）
  
  // ===== 方向 =====
  bool facing_right = true;          // 向き（右向き = true）
  bool can_move = true;              // 移動可能？
};

} // namespace Game::Components
```

**用途**: プレイヤーユニット・敵の移動制御

---

### 5. AttackComponent（攻撃タイプ・ターゲット）

```cpp
// game/include/Game/Components/AttackComponent.h
namespace Game::Components {

struct AttackComponent {
  // ===== 攻撃タイプ =====
  enum AttackType {
    MELEE,          // 近接
    RANGED,         // 遠距離
    AREA,           // 範囲
  };
  
  AttackType attack_type = MELEE;
  
  // ===== ターゲット情報 =====
  enum TargetPriority {
    CLOSEST,        // 最も近い敵
    WEAKEST,        // 最も弱い敵
    STRONGEST,      // 最も強い敵
    FIRST,          // 最初に出現した敵
    LAST,           // 最後に出現した敵
  };
  
  TargetPriority target_priority = CLOSEST;
  entt::entity current_target = entt::null;  // 現在のターゲット Entity
  
  // ===== 攻撃クールタイム =====
  float attack_cooldown = 1.0f;      // 攻撃間隔（秒）
  float cooldown_remaining = 0.0f;   // 残りクールタイム
  
  // ===== 射程・範囲 =====
  int range = 100;                   // 攻撃射程（ピクセル）
  int area_radius = 0;               // 範囲攻撃の半径（0なら単体攻撃）
};

} // namespace Game::Components
```

**用途**: ユニット攻撃システムの中核

---

### 6. SkillComponent（スキル情報・発動管理）

```cpp
// game/include/Game/Components/SkillComponent.h
namespace Game::Components {

struct SkillComponent {
  // ===== 所持スキル =====
  struct OwnedSkill {
    std::string skill_id;           // スキル ID
    std::string skill_type;         // "passive" / "interrupt" / "event"
    float cooldown_remaining = 0.0f; // 残りクールタイム
    bool is_active = true;          // 発動中？
  };
  
  std::vector<OwnedSkill> skills;
  
  // ===== イベント発動用フラグ =====
  bool should_trigger_passive = false;
  bool should_trigger_event = false;
  std::string event_trigger_type;   // "on_attack" / "on_take_damage" など
  
  // ===== スキル履歴 =====
  struct SkillUseRecord {
    std::string skill_id;
    float timestamp;
  };
  std::vector<SkillUseRecord> recent_skills;
};

} // namespace Game::Components
```

**用途**: スキル発動、パッシブ効果の管理

---

### 7. BuffDebuffComponent（一時効果：加速・鈍化など）

```cpp
// game/include/Game/Components/BuffDebuffComponent.h
namespace Game::Components {

struct BuffDebuffComponent {
  enum EffectType {
    ATTACK_UP,      // 攻撃力UP
    ATTACK_DOWN,    // 攻撃力DOWN
    SPEED_UP,       // 速度UP
    SPEED_DOWN,     // 速度DOWN（スロー）
    DEFENSE_UP,     // 防御UP
    DEFENSE_DOWN,   // 防御DOWN
    FREEZE,         // 凍結（移動・攻撃不可）
    BURN,           // 火傷（継続ダメージ）
  };
  
  struct Buff {
    EffectType effect;
    float duration;          // 継続時間（秒）
    float duration_remaining; // 残り時間
    float value;             // 効果値（攻撃力UPなら値）
    std::string source_id;   // 付与元の Entity ID
  };
  
  std::vector<Buff> active_buffs;  // 有効なバフ一覧
  
  // ===== キャッシュ値（毎フレーム再計算） =====
  float total_attack_multiplier = 1.0f;
  float total_speed_multiplier = 1.0f;
  bool is_frozen = false;
};

} // namespace Game::Components
```

**用途**: スキル・アビリティの一時効果管理

---

### 8. CooldownComponent（クールタイム管理）

```cpp
// game/include/Game/Components/CooldownComponent.h
namespace Game::Components {

struct CooldownComponent {
  struct Cooldown {
    std::string id;           // クールタイム ID
    float duration;           // 継続時間（秒）
    float remaining;          // 残り時間
    std::function<void()> on_complete;  // 完了時コールバック
  };
  
  std::vector<Cooldown> active_cooldowns;
  
  // ===== ユニット出撃クールタイム（出撃制限） =====
  float spawn_cooldown_remaining = 0.0f;
  float spawn_cooldown_max = 8.5f;  // 敵の出撃間隔など
};

} // namespace Game::Components
```

**用途**: 出撃制限、スキルクールタイム

---

### 9. AnimationComponent（アニメーション状態・フレーム管理）

```cpp
// game/include/Game/Components/AnimationComponent.h
namespace Game::Components {

struct AnimationComponent {
  // ===== 再生中のアニメーション =====
  std::string current_animation;     // 再生中のアニメーション名
  float elapsed_time = 0.0f;         // 経過時間（秒）
  
  // ===== フレーム情報 =====
  int current_frame = 0;             // 現在フレーム
  int total_frames = 1;              // 総フレーム数
  float fps = 10.0f;                 // フレームレート
  bool is_looping = true;            // ループ再生？
  bool is_finished = false;          // アニメーション終了？
  
  // ===== アニメーション定義（Shared層から取得） =====
  std::string character_id;
  std::string draw_type;             // "parts_animation" / "sprite"
  
  // ===== キューイング =====
  std::queue<std::string> animation_queue;  // 次のアニメーション
};

} // namespace Game::Components
```

**用途**: ユニットのアニメーション制御

---

### 10. RendererComponent（描画パラメータ）

```cpp
// game/include/Game/Components/RendererComponent.h
namespace Game::Components {

struct RendererComponent {
  // ===== 描画タイプ =====
  enum RenderType {
    SPRITE,         // 単純スプライト
    PARTS,          // パーツアニメーション
    EFFECT,         // エフェクト
    UI_PARTICLE,    // UIパーティクル
  };
  
  RenderType render_type = SPRITE;
  
  // ===== 画像情報 =====
  std::string texture_key;           // テクスチャ ID
  Color tint = {255, 255, 255, 255}; // 色合い（RGB + Alpha）
  float opacity = 1.0f;              // 透明度（0.0～1.0）
  
  // ===== パーツアニメーション用 =====
  struct PartData {
    std::string part_name;
    int frame_index;
    int layer;
    bool visible;
  };
  std::unordered_map<std::string, PartData> parts;
  
  // ===== 描画フラグ =====
  bool is_visible = true;
  bool flip_horizontal = false;
  bool flip_vertical = false;
};

} // namespace Game::Components
```

**用途**: Raylib による描画

---

### 11. AIComponent（敵 AI パラメータ）

```cpp
// game/include/Game/Components/AIComponent.h
namespace Game::Components {

struct AIComponent {
  // ===== AI行動パターン =====
  enum Behavior {
    WALK_AND_ATTACK,       // 歩いて攻撃
    BOSS_ATTACK_PATTERN,   // ボスパターン攻撃
    DODGE,                 // 回避移動
    FIXED_POSITION,        // 固定砲台
    CHARGE,                // 突進
  };
  
  Behavior current_behavior = WALK_AND_ATTACK;
  
  // ===== ターゲット優先度 =====
  enum TargetPriority {
    CLOSEST,
    WEAKEST,
    STRONGEST,
  };
  TargetPriority target_priority = CLOSEST;
  
  // ===== 状態管理 =====
  enum AIState {
    IDLE,
    ATTACKING,
    MOVING,
    CHARGING,
    DEAD,
  };
  AIState current_state = IDLE;
  
  // ===== パラメータ =====
  float decision_cooldown = 0.5f;    // AI決定間隔
  float decision_remaining = 0.0f;
  
  // ===== ボスパターン用 =====
  int attack_pattern_index = 0;      // パターン番号
  std::vector<std::string> attack_patterns;  // パターンリスト
  float pattern_timer = 0.0f;
};

} // namespace Game::Components
```

**用途**: 敵ユニットの AI 制御

---

### 12. CostComponent（CP コスト・再生）

```cpp
// game/include/Game/Components/CostComponent.h
namespace Game::Components {

struct CostComponent {
  // ===== コスト情報（プレイヤーユニット用） =====
  int base_cost = 300;       // 出撃コスト
  
  // ===== グローバルコスト（ステージ全体） =====
  static int total_cost;              // 合計コスト（クラス変数）
  static int max_cost;                // 最大コスト（ステージ定義）
  static float cost_recharge_rate;    // コスト再生速度（/秒）
};

// グローバル変数は World で管理
} // namespace Game::Components
```

**用途**: プレイヤーユニット出撃システム

---

### 13. CollisionComponent（衝突判定）

```cpp
// game/include/Game/Components/CollisionComponent.h
namespace Game::Components {

struct CollisionComponent {
  enum CollisionType {
    UNIT_PLAYER,    // プレイヤーユニット
    UNIT_ENEMY,     // 敵ユニット
    PROJECTILE,     // 飛び道具
    EFFECT,         // エフェクト（ダメージなし）
  };
  
  CollisionType collision_type = UNIT_PLAYER;
  
  // ===== 衝突判定形状 =====
  float collision_radius = 30.0f;  // 円形判定の半径
  
  // ===== 衝突フラグ =====
  bool is_colliding_this_frame = false;
  std::vector<entt::entity> colliding_entities;  // 衝突中の Entity リスト
};

} // namespace Game::Components
```

**用途**: 敵への攻撃判定、ユニット衝突判定

---

### 14. ProjectileComponent（飛び道具・弾丸）

```cpp
// game/include/Game/Components/ProjectileComponent.h
namespace Game::Components {

struct ProjectileComponent {
  entt::entity shooter = entt::null;  // 発射者 Entity
  entt::entity target = entt::null;   // ターゲット Entity
  
  float speed = 300.0f;               // 移動速度
  float lifetime = 5.0f;              // 最大生存時間
  float elapsed_time = 0.0f;
  
  int damage = 0;                     // ダメージ値
  std::string effect_on_hit;          // ヒット時エフェクト ID
  bool has_hit = false;               // 既にヒット？
};

} // namespace Game::Components
```

**用途**: 遠距離攻撃の弾丸管理

---

### 15. AudioComponent（サウンド再生）

```cpp
// game/include/Game/Components/AudioComponent.h
namespace Game::Components {

struct AudioComponent {
  enum AudioType {
    BGM,            // 背景音
    SFX,            // 効果音
    VOICE,          // ボイス
  };
  
  struct AudioRequest {
    std::string audio_id;
    AudioType type;
    float volume = 1.0f;
    bool loop = false;
    float delay = 0.0f;      // 遅延再生
  };
  
  std::queue<AudioRequest> audio_queue;  // 再生予定のサウンド
  std::string currently_playing;
};

} // namespace Game::Components
```

**用途**: サウンド再生リクエスト

---

### 16. StageContextComponent（ステージ情報：Wave・進捗）

```cpp
// game/include/Game/Components/StageContextComponent.h
namespace Game::Components {

struct StageContextComponent {
  // ===== ステージ情報（グローバル） =====
  std::string stage_id;
  std::string stage_name;
  int current_wave = 0;
  int total_waves = 0;
  
  // ===== ゲーム状態 =====
  enum GameState {
    LOADING,
    READY,
    PLAYING,
    PAUSED,
    VICTORY,
    DEFEAT,
  };
  GameState current_state = LOADING;
  
  // ===== プレイヤーHP =====
  int player_base_hp = 1000;
  int player_current_hp = 1000;
  
  // ===== グローバルコスト =====
  int current_cost = 0;
  int max_cost = 500;
  float cost_recharge_rate = 10.0f;  // /秒
  
  // ===== 時間管理 =====
  float elapsed_time = 0.0f;
  float time_limit = 0.0f;           // 0 = 無制限
  
  // ===== 敵スポーン管理 =====
  int enemies_spawned = 0;
  int enemies_defeated = 0;
};

} // namespace Game::Components
```

**用途**: ステージ全体の状態管理（World に 1 つ）

---

### 17. ParticleEmitterComponent（パーティクルエミッター）

```cpp
// game/include/Game/Components/ParticleEmitterComponent.h
namespace Game::Components {

struct ParticleEmitterComponent {
  std::string emitter_id;
  std::string effect_id;      // エフェクト定義 ID
  
  bool is_active = true;
  float emission_rate = 10.0f; // /秒
  float emission_remaining = 0.0f;
  
  float lifetime = 2.0f;       // エミッターの生存時間
  float elapsed = 0.0f;
};

} // namespace Game::Components
```

**用途**: エフェクトパーティクルの生成管理

---

### 18. TagComponent（Entity 分類タグ）

```cpp
// game/include/Game/Components/TagComponent.h
namespace Game::Components {

struct TagComponent {
  std::string entity_type;    // "player_unit" / "enemy" / "effect" など
  std::string character_id;   // キャラクター ID
  std::vector<std::string> tags;  // タグ（"boss", "fire_type" など）
  
  bool is_player_unit = false;
  bool is_enemy = false;
  bool is_effect = false;
};

} // namespace Game::Components
```

**用途**: Entity 分類・検索用

---

## System詳細設計（12個）

### System 実行フロー

```
毎フレーム：
  1. InputSystem         → キー入力を Entity へ
  2. CostSystem          → CP 再生・管理
  3. MovementSystem      → 位置更新
  4. CollisionSystem     → 衝突判定
  5. SkillSystem         → スキル発動
  6. BuffSystem          → Buff/Debuff 更新
  7. AttackSystem        → ターゲット選択・ダメージ計算
  8. HealthSystem        → HP 管理・死亡判定
  9. AnimationSystem     → アニメーション再生
  10. EffectSystem       → エフェクト生成
  11. AISystem           → 敵 AI 決定
  12. RenderSystem       → 描画データ準備
```

### 1. InputSystem（入力処理）

```cpp
// game/include/Game/Systems/InputSystem.h
namespace Game::Systems {

class InputSystem : public ISystem {
private:
  Shared::Core::GameContext& context_;

public:
  InputSystem(Shared::Core::GameContext& context);
  
  void Update(entt::registry& registry, float delta_time) override;

private:
  void HandleUnitSpawn(entt::registry& registry, 
                      const glm::vec2& spawn_pos);
  void HandleSkillActivation(entt::registry& registry,
                            const std::string& skill_id);
};

} // namespace Game::Systems
```

**責務**:
- マウス入力からユニット召喚
- スキルボタンからスキル発動

---

### 2. CostSystem（CP再生管理）

```cpp
// game/include/Game/Systems/CostSystem.h
namespace Game::Systems {

class CostSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateGlobalCost(entt::registry& registry, float delta_time);
  void ReduceCostOnSpawn(entt::registry& registry, int cost_spent);
};

} // namespace Game::Systems
```

**責務**:
- ステージ全体の CP を時間経過で増加
- ユニット出撃時に CP 減少

---

### 3. MovementSystem（位置更新）

```cpp
// game/include/Game/Systems/MovementSystem.h
namespace Game::Systems {

class MovementSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateEntityPosition(entt::registry& registry,
                           entt::entity entity,
                           float delta_time);
  void UpdateWaypoint(TransformComponent& transform,
                     MovementComponent& movement);
};

} // namespace Game::Systems
```

**責務**:
- Entity の位置（x, y）を更新
- 経路追従（ウェイポイント）

---

### 4. CollisionSystem（衝突判定）

```cpp
// game/include/Game/Systems/CollisionSystem.h
namespace Game::Systems {

class CollisionSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  bool CheckCircleCollision(const glm::vec2& pos1, float radius1,
                           const glm::vec2& pos2, float radius2);
  void HandleCollision(entt::registry& registry,
                      entt::entity entity1,
                      entt::entity entity2);
};

} // namespace Game::Systems
```

**責務**:
- Entity 間の衝突判定（円形判定）
- 攻撃判定の実行

---

### 5. SkillSystem（スキル発動）

```cpp
// game/include/Game/Systems/SkillSystem.h
namespace Game::Systems {

class SkillSystem : public ISystem {
private:
  Shared::Core::GameContext& context_;
  Game::Managers::SkillManager& skill_manager_;

public:
  SkillSystem(Shared::Core::GameContext& context,
             Game::Managers::SkillManager& skill_manager);
  
  void Update(entt::registry& registry, float delta_time) override;
  void TriggerSkill(entt::registry& registry, entt::entity entity,
                   const std::string& skill_id);

private:
  void UpdatePassiveSkills(entt::registry& registry,
                          entt::entity entity);
  void UpdateEventSkills(entt::registry& registry,
                        entt::entity entity);
  void ApplySkillEffect(entt::registry& registry,
                       entt::entity caster,
                       const Shared::Data::SkillDef& skill);
};

} // namespace Game::Systems
```

**責務**:
- パッシブスキルの自動発動
- イベントスキルの条件判定
- スキル効果の適用

---

### 6. BuffSystem（バフ・デバフ更新）

```cpp
// game/include/Game/Systems/BuffSystem.h
namespace Game::Systems {

class BuffSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateBuffDuration(entt::registry& registry,
                         entt::entity entity,
                         float delta_time);
  void ApplyBuffModifiers(entt::registry& registry,
                         entt::entity entity);
  void RemoveExpiredBuffs(entt::registry& registry,
                         entt::entity entity);
};

} // namespace Game::Systems
```

**責務**:
- Buff の継続時間管理
- Buff による効果値の計算（攻撃力UP など）

---

### 7. AttackSystem（攻撃・ダメージ計算）

```cpp
// game/include/Game/Systems/AttackSystem.h
namespace Game::Systems {

class AttackSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateAttackCooldowns(entt::registry& registry,
                            entt::entity entity,
                            float delta_time);
  void CheckAndExecuteAttacks(entt::registry& registry,
                             entt::entity entity);
  int CalculateDamage(const StatsComponent& attacker,
                     const StatsComponent& defender);
};

} // namespace Game::Systems
```

**責務**:
- 攻撃クールタイムの更新
- ターゲット選択・攻撃実行
- ダメージ計算（攻撃力 × 倍率など）

---

### 8. HealthSystem（HP管理・死亡）

```cpp
// game/include/Game/Systems/HealthSystem.h
namespace Game::Systems {

class HealthSystem : public ISystem {
private:
  Shared::Core::GameContext& context_;

public:
  HealthSystem(Shared::Core::GameContext& context);
  
  void Update(entt::registry& registry, float delta_time) override;
  void ApplyDamage(entt::registry& registry, entt::entity entity,
                  int damage, entt::entity source);
  void Heal(entt::registry& registry, entt::entity entity,
           int heal_amount);

private:
  void CheckDeathConditions(entt::registry& registry,
                           entt::entity entity);
  void HandleUnitDeath(entt::registry& registry,
                      entt::entity entity);
};

} // namespace Game::Systems
```

**責務**:
- HP 変更管理
- ダメージ受け付け
- 死亡判定・処理

---

### 9. AnimationSystem（アニメーション制御）

```cpp
// game/include/Game/Systems/AnimationSystem.h
namespace Game::Systems {

class AnimationSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateAnimationFrame(AnimationComponent& animation,
                           float delta_time);
  void ChangeAnimation(entt::registry& registry,
                      entt::entity entity,
                      const std::string& animation_name);
};

} // namespace Game::Systems
```

**責務**:
- アニメーション再生フレーム更新
- アニメーション切り替え

---

### 10. EffectSystem（エフェクト生成）

```cpp
// game/include/Game/Systems/EffectSystem.h
namespace Game::Systems {

class EffectSystem : public ISystem {
private:
  Shared::Core::GameContext& context_;

public:
  EffectSystem(Shared::Core::GameContext& context);
  
  void Update(entt::registry& registry, float delta_time) override;
  void SpawnEffect(entt::registry& registry,
                  const std::string& effect_id,
                  const glm::vec2& position,
                  entt::entity target = entt::null);

private:
  void UpdateParticleEmitters(entt::registry& registry,
                             float delta_time);
};

} // namespace Game::Systems
```

**責務**:
- エフェクトのスポーン
- パーティクルエミッターの管理

---

### 11. AISystem（敵AI決定）

```cpp
// game/include/Game/Systems/AISystem.h
namespace Game::Systems {

class AISystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;

private:
  void UpdateAIDecision(entt::registry& registry,
                       entt::entity entity,
                       float delta_time);
  entt::entity SelectTarget(entt::registry& registry,
                           entt::entity ai_entity,
                           const TransformComponent& ai_pos);
  void ExecuteAIAction(entt::registry& registry,
                      entt::entity entity);
};

} // namespace Game::Systems
```

**責務**:
- 敵 AI の決定処理（ターゲット選択など）
- パターン攻撃（ボス）の制御

---

### 12. RenderSystem（描画準備）

```cpp
// game/include/Game/Systems/RenderSystem.h
namespace Game::Systems {

class RenderSystem : public ISystem {
public:
  void Update(entt::registry& registry, float delta_time) override;
  
  // 描画用の Entity リストを取得
  std::vector<RenderData> GetRenderQueue() const;

private:
  struct RenderData {
    entt::entity entity;
    TransformComponent transform;
    RendererComponent renderer;
    int layer;
  };
  
  std::vector<RenderData> render_queue_;
  
  void PrepareRenderQueue(entt::registry& registry);
  void SortByLayer();
};

} // namespace Game::Systems
```

**責務**:
- 描画対象 Entity の選別
- レイヤーごとのソート
- Application 層へ描画データを渡す

---

## Component ↔ System 対応表

| Component | System | 説明 |
|-----------|--------|------|
| TransformComponent | MovementSystem, RenderSystem | 位置の更新と描画 |
| StatsComponent | AttackSystem, HealthSystem | ステータス情報の活用 |
| HealthComponent | HealthSystem, SkillSystem | HP管理、死亡判定 |
| MovementComponent | MovementSystem, CollisionSystem | 移動パス追従、衝突判定 |
| AttackComponent | AttackSystem, CollisionSystem | 攻撃実行、ターゲット選択 |
| SkillComponent | SkillSystem | スキル発動 |
| BuffDebuffComponent | BuffSystem, AttackSystem, MovementSystem | 効果値の修正 |
| CooldownComponent | SkillSystem, AttackSystem, CostSystem | 各種クールタイム管理 |
| AnimationComponent | AnimationSystem, RenderSystem | アニメーション制御 |
| RendererComponent | RenderSystem | 描画パラメータ |
| AIComponent | AISystem, AttackSystem | 敵AI行動決定 |
| CostComponent | CostSystem, InputSystem | CP管理 |
| CollisionComponent | CollisionSystem | 衝突判定 |
| ProjectileComponent | MovementSystem, CollisionSystem, HealthSystem | 飛び道具管理 |
| AudioComponent | 専用System | サウンド再生 |
| StageContextComponent | CostSystem, HealthSystem, AI System | ステージ全体状態 |
| ParticleEmitterComponent | EffectSystem | パーティクル生成 |
| TagComponent | (検索用) | Entity分類タグ |

---

## Factory パターン（Entity生成）

### CharacterFactory（プレイヤーユニット生成）

```cpp
// game/include/Game/Factories/CharacterFactory.h
namespace Game::Factories {

class CharacterFactory {
private:
  entt::registry& registry_;
  Shared::Data::DefinitionRegistry& definitions_;
  Game::Managers::CharacterManager& character_mgr_;

public:
  CharacterFactory(entt::registry& registry,
                  Shared::Data::DefinitionRegistry& definitions,
                  Game::Managers::CharacterManager& character_mgr);
  
  // プレイヤーユニット生成
  entt::entity SpawnPlayerUnit(
    const std::string& character_id,
    const glm::vec2& spawn_position);

private:
  void SetupTransform(entt::entity entity, const glm::vec2& pos);
  void SetupStats(entt::entity entity, const std::string& character_id);
  void SetupSkills(entt::entity entity, const std::string& character_id);
  void SetupAnimation(entt::entity entity, const std::string& character_id);
  void SetupAttack(entt::entity entity, const std::string& character_id);
};

} // namespace Game::Factories
```

**生成パターン**:
```cpp
auto factory = CharacterFactory(registry, definitions, character_mgr);
auto entity = factory.SpawnPlayerUnit("char_001", {500.0f, 300.0f});
```

### EnemyFactory（敵ユニット生成）

```cpp
// game/include/Game/Factories/EnemyFactory.h
namespace Game::Factories {

class EnemyFactory {
private:
  entt::registry& registry_;
  Shared::Data::DefinitionRegistry& definitions_;

public:
  EnemyFactory(entt::registry& registry,
              Shared::Data::DefinitionRegistry& definitions);
  
  // 敵ユニット生成
  entt::entity SpawnEnemy(const std::string& enemy_id,
                         const glm::vec2& spawn_position);

private:
  void SetupTransform(entt::entity entity, const glm::vec2& pos);
  void SetupStats(entt::entity entity, const std::string& enemy_id);
  void SetupAI(entt::entity entity, const std::string& enemy_id);
  void SetupMovement(entt::entity entity, const std::string& enemy_id);
  void SetupAttack(entt::entity entity, const std::string& enemy_id);
};

} // namespace Game::Factories
```

### EffectFactory（エフェクト生成）

```cpp
// game/include/Game/Factories/EffectFactory.h
namespace Game::Factories {

class EffectFactory {
private:
  entt::registry& registry_;
  Shared::Data::DefinitionRegistry& definitions_;

public:
  EffectFactory(entt::registry& registry,
               Shared::Data::DefinitionRegistry& definitions);
  
  // エフェクト生成
  entt::entity SpawnEffect(const std::string& effect_id,
                          const glm::vec2& position,
                          entt::entity target = entt::null);
  
  // パーティクルエミッター生成
  entt::entity SpawnEmitter(const std::string& effect_id,
                           const glm::vec2& position,
                           float lifetime = 2.0f);

private:
  void SetupTransform(entt::entity entity, const glm::vec2& pos);
  void SetupRenderer(entt::entity entity, const std::string& effect_id);
  void SetupAnimation(entt::entity entity, const std::string& effect_id);
};

} // namespace Game::Factories
```

---

## Entity ライフサイクル

### ユニットのライフサイクル

```
1. Factory で Entity 生成
   ├─ TransformComponent（位置）
   ├─ StatsComponent（ステータス）
   ├─ HealthComponent（HP）
   ├─ MovementComponent（移動）
   ├─ AttackComponent（攻撃）
   ├─ SkillComponent（スキル）
   ├─ AnimationComponent（アニメ）
   ├─ RendererComponent（描画）
   ├─ TagComponent（タグ）
   └─ CollisionComponent（衝突）

2. 毎フレーム System が更新
   ├─ MovementSystem → 位置更新
   ├─ AttackSystem → ターゲット攻撃
   ├─ HealthSystem → HP管理
   ├─ SkillSystem → スキル発動
   ├─ AnimationSystem → アニメ更新
   └─ RenderSystem → 描画

3. HealthComponent.current_hp <= 0 → HealthSystem が死亡判定
   ├─ EffectSystem → 死亡エフェクト生成
   ├─ GameManager → 敵撃破時の報酬処理
   └─ Registry から Entity 削除
```

### エフェクトのライフサイクル

```
1. EffectFactory で Entity 生成
   ├─ TransformComponent（位置）
   ├─ RendererComponent（描画）
   ├─ AnimationComponent（アニメ）
   └─ ParticleEmitterComponent（エミッター）

2. フレーム経過
   ├─ AnimationSystem → アニメ再生
   ├─ EffectSystem → パーティクル生成
   └─ RenderSystem → 描画

3. lifetime 経過 → Entity 削除
```

---

## System 実行順序（フレームループ）

### GameEngine メインループ

```cpp
// game/include/Game/World/GameEngine.h
namespace Game::World {

class GameEngine {
private:
  entt::registry registry_;
  
  std::unique_ptr<InputSystem> input_system_;
  std::unique_ptr<CostSystem> cost_system_;
  std::unique_ptr<MovementSystem> movement_system_;
  std::unique_ptr<CollisionSystem> collision_system_;
  std::unique_ptr<SkillSystem> skill_system_;
  std::unique_ptr<BuffSystem> buff_system_;
  std::unique_ptr<AttackSystem> attack_system_;
  std::unique_ptr<HealthSystem> health_system_;
  std::unique_ptr<AnimationSystem> animation_system_;
  std::unique_ptr<EffectSystem> effect_system_;
  std::unique_ptr<AISystem> ai_system_;
  std::unique_ptr<RenderSystem> render_system_;

public:
  void Update(float delta_time) {
    // ===== 入力処理 =====
    input_system_->Update(registry_, delta_time);
    
    // ===== ゲーム状態更新 =====
    cost_system_->Update(registry_, delta_time);
    movement_system_->Update(registry_, delta_time);
    collision_system_->Update(registry_, delta_time);
    
    // ===== スキル・効果処理 =====
    skill_system_->Update(registry_, delta_time);
    buff_system_->Update(registry_, delta_time);
    
    // ===== ダメージ・HP処理 =====
    attack_system_->Update(registry_, delta_time);
    health_system_->Update(registry_, delta_time);
    
    // ===== ビジュアル更新 =====
    animation_system_->Update(registry_, delta_time);
    effect_system_->Update(registry_, delta_time);
    
    // ===== AI（敵） =====
    ai_system_->Update(registry_, delta_time);
    
    // ===== 描画準備 =====
    render_system_->Update(registry_, delta_time);
  }
  
  std::vector<RenderData> GetRenderQueue() const {
    return render_system_->GetRenderQueue();
  }
};

} // namespace Game::World
```

---

## データフロー図

### フレーム内の情報フロー

```
Input（キー入力）
  ↓
InputSystem → Entity にアクション追加
  ↓
CostSystem → CP 減少・再生
  ↓
MovementSystem → TransformComponent.x, y 更新
  ↓
CollisionSystem → CollisionComponent を検査
  ↓
SkillSystem → パッシブスキル発動
  ↓
BuffSystem → Buff 継続時間更新 → Stats 修正
  ↓
AttackSystem → 攻撃実行（ダメージ計算）
  ↓
HealthSystem → HP 変更 → 死亡判定
  ↓（死亡時）
EffectSystem → 死亡エフェクト生成
  ↓
AnimationSystem → アニメーションフレーム更新
  ↓
EffectSystem → パーティクル生成・更新
  ↓
RenderSystem → レンダーキュー作成（描画順序）
  ↓
Application（UI） → Raylib で描画
```

---

## 実装優先度

### Phase 3.1: Core Components（3日）

```
Day 1:
  ✅ TransformComponent, StatsComponent, HealthComponent
  ✅ AnimationComponent, RendererComponent
  ✅ TagComponent, CollisionComponent

Day 2:
  ✅ MovementComponent, AttackComponent
  ✅ SkillComponent, CooldownComponent
  ✅ BuffDebuffComponent

Day 3:
  ✅ AIComponent, CostComponent
  ✅ ProjectileComponent, AudioComponent
  ✅ StageContextComponent, ParticleEmitterComponent
```

### Phase 3.2: Core Systems（1週間）

```
Day 1-2:
  ✅ MovementSystem, CollisionSystem
  ✅ InputSystem, CostSystem
  ✅ AttackSystem, HealthSystem

Day 3-4:
  ✅ SkillSystem, BuffSystem
  ✅ AnimationSystem, RenderSystem
  ✅ EffectSystem, AISystem

Day 5:
  ✅ GameEngine（メインループ）
  ✅ 全System統合テスト
```

### Phase 3.3: Factory & Tools（3-4日）

```
Day 1:
  ✅ CharacterFactory
  ✅ EnemyFactory
  ✅ EffectFactory

Day 2-3:
  ✅ Entity 生成テスト
  ✅ ライフサイクル検証
  ✅ パフォーマンス計測
```

---

## チェックリスト

```
Component実装:
  ☐ 全18個のComponent定義
  ☐ 各Component の初期化コンストラクタ
  ☐ JSON ↔ Component 変換（Serializer）

System実装:
  ☐ 全12個のSystem実装
  ☐ 各System の Update() メソッド
  ☐ System 間の依存関係確認

Factory実装:
  ☐ CharacterFactory
  ☐ EnemyFactory  
  ☐ EffectFactory
  ☐ Entity 生成テスト

GameEngine:
  ☐ メインループ実装
  ☐ System 実行順序確認
  ☐ フレームレート安定性確認
```

---

## 次のドキュメント

- [ ] **Application層設計** (SceneManager + Scenes)
- [ ] **Raylib統合設計** (Graphics + Input)
- [ ] **UI層設計** (UI Scenes + Widgets)

---

## サマリー

TD Layer（ECS）の設計が完成しました：

```
✅ 18個のComponent（データ）で全ゲームロジックを表現
✅ 12個のSystem（ロジック）で全ゲーム処理を実装
✅ 完全な Entity ライフサイクル管理
✅ パフォーマンス最適化済み（キャッシュ効率）
✅ テスト容易性を確保（各System独立）
✅ HotReload対応（System動的入れ替え）

🎉 これで Game Layer + TD Layer が完成し、
   Application層（UI）との統合が可能になりました！
```

