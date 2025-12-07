# TDシステム設計

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）のタワーディフェンスシステム設計を定義します。

## 目的

- 直線型TD（にゃんこ大戦争タイプ）の実装
- マップ型TD（アークナイツタイプ）の実装
- JSON駆動によるゲーム内容の拡張
- 共通システムの再利用

## 文書構成

- 1: システム概要（LineTD / MapTD の位置付けと共通システム）
- 2: 直線型TD 概要（詳細は付録A）
- 3以降: MapTD・共通基盤・運用メモ
- 付録A: 直線型TD詳細（旧 `linea-td-detailed-design.md` を統合）

---

## 1. システム概要

### 1.1 2種類のTDモード

#### 直線型TD（LineTD）

- 複数のレーン（横方向）に敵が進軍
- ユニットをレーン上に配置
- 敵はレーンの端から端まで一直線に進む
- 例: にゃんこ大戦争

#### マップ型TD（MapTD）

- グリッドベースのマップ
- 敵は定義されたパスに沿って進軍
- ユニットをグリッド上に配置
- 例: アークナイツ

### 1.2 共通システム

両方のTDモードで共有するシステム:

- リソース管理（コスト/ゴールド）
- 波（Wave）管理
- スポーン管理
- 戦闘システム
- 勝敗判定

### 1.3 モード切替と SystemRunner 再構成（戦闘シーン Enter）

- モード判定: 戦闘シーン Enter 時に `TDGameMode`（LineTD/MapTD）を読み取り、`SystemRunner` を該当セットで再構成する。シーン遷移と分離し、戦闘シーン内の処理として完結させる。
- 再構成手順: (1) 現行システム停止・破棄 → (2) モード別システムセットを登録（LineTD: レーン移動/スポーン等、MapTD: パス探索/タイル占有等） → (3) モード固有リソース・Prefab・タイル/レーン定義を再ロードしバインド → (4) 入力マップ/カメラ設定/レンダリングプロファイルを再適用。
- 共有データ: ステージ定義・編成・ユーザ設定は保持して引き継ぐ。スポーンテーブル/タイル情報はモード固有としてリロードし、旧モードのキャッシュは解放する。
- HotReload/検証: モード固有の定義も適用前に検証。失敗時は旧モードの状態を維持し、エラー通知のみ行う。
- UI/状態連動: Ready/Playing/Paused/Victory/Defeat の `GameState` 変化に合わせ、モード別 UI レイヤと入力バインドを差し替える。
- 切替ルール: TDモード切替は必ず戦闘シーンの Exit→Enter を経由して反映する。それ以外のシーンでは SystemRunner 再登録を行わず、UI差し替えのみを前提とする。

---

## 2. 直線型TD（LineTD）設計
>
> 直線型TDの詳細な運用/JSON例は付録A（旧 `linea-td-detailed-design.md` 統合）を参照。

### 2.1 コンポーネント定義

```cpp
namespace New::Domain::TD::LineTD::Components {

// レーン情報
struct Lane {
    int laneIndex = 0;  // レーン番号（0始まり）
    float yPosition = 0.0f;  // Y座標
    float startX = 0.0f;  // 開始X座標
    float endX = 1920.0f;  // 終了X座標
};

// レーン上移動
struct LaneMovement {
    int laneIndex = 0;
    float speed = 100.0f;
    float currentX = 0.0f;
    bool movingRight = true;  // true: 右へ, false: 左へ
};

// レーン配置可能
struct LanePlaceable {
    int laneIndex = 0;
    float xPosition = 0.0f;
};

} // namespace New::Domain::TD::LineTD::Components
```

> 詳細な運用フローや JSON 例は重複を避けるため `linea-td-detailed-design.md` を参照。ここでは方針と責務の概要のみ記載する。

### 2.2 システム実装

#### LaneMovementSystem

```cpp
namespace New::Domain::TD::LineTD::Systems {

class LaneMovementSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        auto& world = context.GetWorld();
        auto view = world.View<LaneMovement, Transform>();
        
        for (auto entity : view) {
            auto& movement = view.get<LaneMovement>(entity);
            auto& transform = view.get<Transform>(entity);
            
            // レーン情報取得
            auto* lane = world.GetComponent<Lane>(entity);
            if (!lane) continue;
            
            // 移動
            if (movement.movingRight) {
                movement.currentX += movement.speed * deltaTime;
                if (movement.currentX >= lane->endX) {
                    movement.currentX = lane->endX;
                    // 到達イベント発火
                    world.Emit(ReachedEndEvent{entity});
                }
            } else {
                movement.currentX -= movement.speed * deltaTime;
                if (movement.currentX <= lane->startX) {
                    movement.currentX = lane->startX;
                    world.Emit(ReachedEndEvent{entity});
                }
            }
            
            // Transform更新
            transform.x = movement.currentX;
            transform.y = lane->yPosition;
        }
    }
    
    int GetUpdatePriority() const override { return 100; }
};

} // namespace New::Domain::TD::LineTD::Systems
```

#### LineTDSpawnSystem

```cpp
namespace New::Domain::TD::LineTD::Systems {

class LineTDSpawnSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        auto& spawnManager = context.GetSpawnManager();
        auto& world = context.GetWorld();
        
        // スポーン待機中のエンティティを処理
        spawnManager.ProcessSpawns(world, context, deltaTime);
    }
    
    int GetUpdatePriority() const override { return 50; }
};

} // namespace New::Domain::TD::LineTD::Systems
```

### 2.3 マネージャー実装

#### LineTDWaveManager

```cpp
namespace New::Domain::TD::LineTD::Managers {

class LineTDWaveManager {
public:
    // 波開始
    void StartWave(const WaveDef& waveDef, Core::World& world, Core::GameContext& context) {
        currentWave_ = &waveDef;
        waveTimer_ = 0.0f;
        isActive_ = true;
        
        // スポーンエントリを準備
        for (const auto& spawn : waveDef.spawns) {
            if (spawn.lane >= 0) {  // レーン指定あり
                PendingSpawn pending;
                pending.entityId = spawn.entityId;
                pending.lane = spawn.lane;
                pending.delay = spawn.delay;
                pending.count = spawn.count;
                pending.interval = spawn.interval;
                pendingSpawns_.push_back(pending);
            }
        }
        
        world.Emit(WaveStartedEvent{waveDef.id});
    }
    
    // 更新
    void Update(Core::World& world, Core::GameContext& context, float deltaTime) {
        if (!isActive_) return;
        
        waveTimer_ += deltaTime;
        
        // スポーン処理
        for (auto it = pendingSpawns_.begin(); it != pendingSpawns_.end();) {
            if (waveTimer_ >= it->delay && it->count > 0) {
                // スポーン実行
                SpawnEntity(world, context, *it);
                
                it->count--;
                if (it->count > 0) {
                    it->delay += it->interval;
                } else {
                    it = pendingSpawns_.erase(it);
                    continue;
                }
            }
            ++it;
        }
        
        // 波完了チェック
        if (pendingSpawns_.empty() && AreAllEnemiesDefeated(world)) {
            CompleteWave(world);
        }
    }
    
private:
    struct PendingSpawn {
        std::string entityId;
        int lane = 0;
        float delay = 0.0f;
        int count = 1;
        float interval = 1.0f;
    };
    
    const WaveDef* currentWave_ = nullptr;
    float waveTimer_ = 0.0f;
    bool isActive_ = false;
    std::vector<PendingSpawn> pendingSpawns_;
    
    void SpawnEntity(Core::World& world, Core::GameContext& context, const PendingSpawn& pending);
    bool AreAllEnemiesDefeated(Core::World& world);
    void CompleteWave(Core::World& world);
};

} // namespace New::Domain::TD::LineTD::Managers
```

---

## 3. マップ型TD（MapTD）設計

### 3.1 コンポーネント定義

```cpp
namespace New::Domain::TD::MapTD::Components {

// グリッド位置
struct GridPosition {
    int x = 0;
    int y = 0;
};

// パス追従
struct PathFollower {
    std::string pathId;  // パスID
    float progress = 0.0f;  // パス進行度（0.0-1.0）
    float speed = 100.0f;
};

// グリッド配置可能
struct GridPlaceable {
    int gridX = 0;
    int gridY = 0;
    bool canPlace = true;
};

} // namespace New::Domain::TD::MapTD::Components
```

### 3.2 パス定義（JSON）

```json
{
  "paths": [
    {
      "id": "path_01",
      "name": "メインパス",
      "waypoints": [
        {"x": 0, "y": 540},
        {"x": 200, "y": 540},
        {"x": 200, "y": 300},
        {"x": 600, "y": 300},
        {"x": 600, "y": 780},
        {"x": 1920, "y": 780}
      ]
    }
  ]
}
```

### 3.3 システム実装

#### PathFollowerSystem

```cpp
namespace New::Domain::TD::MapTD::Systems {

class PathFollowerSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        auto& world = context.GetWorld();
        auto& pathManager = context.GetPathManager();
        auto view = world.View<PathFollower, Transform>();
        
        for (auto entity : view) {
            auto& follower = view.get<PathFollower>(entity);
            auto& transform = view.get<Transform>(entity);
            
            // パス取得
            const auto* path = pathManager.GetPath(follower.pathId);
            if (!path || path->waypoints.empty()) continue;
            
            // 進行度更新
            float pathLength = CalculatePathLength(*path);
            float distance = follower.speed * deltaTime;
            follower.progress += distance / pathLength;
            
            if (follower.progress >= 1.0f) {
                follower.progress = 1.0f;
                // 到達イベント
                world.Emit(ReachedEndEvent{entity});
            }
            
            // 位置計算
            Vector2 position = InterpolatePath(*path, follower.progress);
            transform.x = position.x;
            transform.y = position.y;
        }
    }
    
private:
    float CalculatePathLength(const PathDef& path);
    Vector2 InterpolatePath(const PathDef& path, float progress);
};

} // namespace New::Domain::TD::MapTD::Systems
```

#### GridPlacementSystem

```cpp
namespace New::Domain::TD::MapTD::Systems {

class GridPlacementSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        // グリッド配置の処理
        // マウスクリックでユニット配置
    }
    
    // グリッド座標からワールド座標へ変換
    Vector2 GridToWorld(int gridX, int gridY) const {
        return {
            gridX * gridCellSize_ + gridOffsetX_,
            gridY * gridCellSize_ + gridOffsetY_
        };
    }
    
    // ワールド座標からグリッド座標へ変換
    GridPosition WorldToGrid(float x, float y) const {
        return {
            static_cast<int>((x - gridOffsetX_) / gridCellSize_),
            static_cast<int>((y - gridOffsetY_) / gridCellSize_)
        };
    }

private:
    float gridCellSize_ = 64.0f;
    float gridOffsetX_ = 0.0f;
    float gridOffsetY_ = 0.0f;
};

} // namespace New::Domain::TD::MapTD::Systems
```

### 3.4 データモデル（MapTD）

- `paths[]`: Waypoint の配列でパスを定義（上記例）。
- `tiles[]`: グリッド座標と地形タイプ/Buildableフラグを保持。
- `spawnPoints[]`: 侵入位置と紐づく `pathId` を指定。
- `buildableRules`: 配置可能なタワー種別、コスト、上限など。

#### JSON例（Map + Buildable + Spawn）

```json
{
  "paths": [
    {
      "id": "path_main",
      "waypoints": [
        {"x": 0, "y": 540}, {"x": 320, "y": 540},
        {"x": 320, "y": 320}, {"x": 1280, "y": 320},
        {"x": 1280, "y": 820}, {"x": 1920, "y": 820}
      ]
    }
  ],
  "tiles": [
    {"x": 5, "y": 5, "type": "ground", "buildable": true},
    {"x": 6, "y": 5, "type": "ground", "buildable": true},
    {"x": 10, "y": 10, "type": "path", "buildable": false}
  ],
  "spawnPoints": [
    {"x": 0, "y": 540, "pathId": "path_main"}
  ],
  "buildableRules": [
    {"towerId": "archer_t1", "cost": 20, "maxPerMap": 20},
    {"towerId": "cannon_t1", "cost": 30, "maxPerMap": 10}
  ]
}
```

### 3.5 システム概要（MapTD）

- `PathFollowerSystem`: パスに沿って敵を移動。到達時イベントを発火。
- `GridPlacementSystem`: グリッド上の配置可否判定・コスト消費・占有管理。
- `MapTDWaveManager`: Wave/Spawn を管理し、SpawnRequest を生成（LineTDと同様のイベントを再利用）。
- `TileOccupancySystem`（将来）：タワー配置状態を同期し、衝突・経路封鎖をチェック。

### 3.6 実装チェックリスト（MapTD）

- [ ] パス/タイル/スポーン JSON をロードし、スキーマ検証を通す。
- [ ] `PathFollowerSystem` でパス補間・到達イベントを実装。
- [ ] `GridPlacementSystem` で配置可否判定とコスト消費を実装。
- [ ] `MapTDWaveManager` で MapTD 用の波/スポーンを生成。
- [ ] UI 連携: 配置結果・コスト・ウェーブ進行・ボス予告をイベント発火。

### 3.4 マネージャー実装

#### MapTDWaveManager

```cpp
namespace New::Domain::TD::MapTD::Managers {

class MapTDWaveManager {
public:
    // 波開始
    void StartWave(const WaveDef& waveDef, Core::World& world, Core::GameContext& context) {
        currentWave_ = &waveDef;
        waveTimer_ = 0.0f;
        isActive_ = true;
        
        for (const auto& spawn : waveDef.spawns) {
            if (!spawn.path.empty()) {  // パス指定あり
                PendingSpawn pending;
                pending.entityId = spawn.entityId;
                pending.pathId = spawn.path;
                pending.delay = spawn.delay;
                pending.count = spawn.count;
                pending.interval = spawn.interval;
                pendingSpawns_.push_back(pending);
            }
        }
        
        world.Emit(WaveStartedEvent{waveDef.id});
    }
    
    void Update(Core::World& world, Core::GameContext& context, float deltaTime) {
        // 直線型TDと同様の実装
        // ただし、laneの代わりにpathを使用
    }

private:
    struct PendingSpawn {
        std::string entityId;
        std::string pathId;  // レーンの代わりにパスID
        float delay = 0.0f;
        int count = 1;
        float interval = 1.0f;
    };
    
    const WaveDef* currentWave_ = nullptr;
    float waveTimer_ = 0.0f;
    bool isActive_ = false;
    std::vector<PendingSpawn> pendingSpawns_;
};

} // namespace New::Domain::TD::MapTD::Managers
```

---

## 4. 共通システム

### 4.1 リソース管理（ResourceManager）

```cpp
namespace New::Domain::TD {

class ResourceManager {
public:
    // コスト管理
    void SetCost(float cost) { currentCost_ = cost; }
    float GetCost() const { return currentCost_; }
    bool CanAfford(float cost) const { return currentCost_ >= cost; }
    bool SpendCost(float cost) {
        if (!CanAfford(cost)) return false;
        currentCost_ -= cost;
        return true;
    }
    void AddCost(float amount) {
        currentCost_ = std::min(currentCost_ + amount, maxCost_);
    }
    
    // コスト自動回復
    void Update(float deltaTime) {
        if (costRegenRate_ > 0.0f) {
            AddCost(costRegenRate_ * deltaTime);
        }
    }
    
    void Initialize(float startingCost, float regenRate, float maxCost) {
        currentCost_ = startingCost;
        costRegenRate_ = regenRate;
        maxCost_ = maxCost;
    }

private:
    float currentCost_ = 0.0f;
    float costRegenRate_ = 0.0f;
    float maxCost_ = 9999.0f;
};

} // namespace New::Domain::TD
```

### 4.2 戦闘システム（CombatSystem）

```cpp
namespace New::Domain::TD::Systems {

class CombatSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        auto& world = context.GetWorld();
        
        // 攻撃可能なユニットと敵のペアを検索
        auto attackers = world.View<AttackComponent, Transform>();
        auto enemies = world.View<HealthComponent, Transform>();
        
        for (auto attacker : attackers) {
            auto& attack = attackers.get<AttackComponent>(attacker);
            auto& attackerPos = attackers.get<Transform>(attacker);
            
            // クールダウン更新
            if (attack.cooldown > 0.0f) {
                attack.cooldown -= deltaTime;
                continue;
            }
            
            // 範囲内の敵を検索
            entt::entity target = FindNearestEnemyInRange(world, attackerPos, attack.range);
            if (target != entt::null) {
                // 攻撃実行
                ExecuteAttack(world, attacker, target, attack);
                attack.cooldown = attack.maxCooldown;
            }
        }
    }

private:
    entt::entity FindNearestEnemyInRange(Core::World& world, const Transform& pos, float range);
    void ExecuteAttack(Core::World& world, entt::entity attacker, entt::entity target, const AttackComponent& attack);
};

} // namespace New::Domain::TD::Systems
```

### 4.3 勝敗判定（GameStateManager）

```cpp
namespace New::Domain::TD::Managers {

class GameStateManager {
public:
    void Initialize() {
        isGameOver_ = false;
        isVictory_ = false;
    }
    
    void Update(Core::World& world, const WaveManager& waveManager, float deltaTime) {
        if (isGameOver_) return;
        
        // 敗北条件: プレイヤーHPが0
        if (playerHealth_ <= 0.0f) {
            isGameOver_ = true;
            isVictory_ = false;
            world.Emit(GameOverEvent{false});
            return;
        }
        
        // 勝利条件: 全波クリア
        if (waveManager.IsAllWavesCompleted() && AreAllEnemiesDefeated(world)) {
            isGameOver_ = true;
            isVictory_ = true;
            world.Emit(GameOverEvent{true});
            return;
        }
    }
    
    void SetPlayerHealth(float health, float maxHealth) {
        playerHealth_ = health;
        maxPlayerHealth_ = maxHealth;
    }
    
    void TakeDamage(float damage) {
        playerHealth_ = std::max(0.0f, playerHealth_ - damage);
    }

private:
    float playerHealth_ = 100.0f;
    float maxPlayerHealth_ = 100.0f;
    bool isGameOver_ = false;
    bool isVictory_ = false;
    
    bool AreAllEnemiesDefeated(Core::World& world);
};

} // namespace New::Domain::TD::Managers
```

---

## 5. システム統合

### 5.1 システム登録

```cpp
// LineTD用システム登録
void RegisterLineTDSystems(Core::SystemRunner& systemRunner) {
    systemRunner.RegisterSystem(std::make_unique<LineTD::Systems::LaneMovementSystem>());
    systemRunner.RegisterSystem(std::make_unique<LineTD::Systems::LineTDSpawnSystem>());
    systemRunner.RegisterSystem(std::make_unique<Systems::CombatSystem>());
    systemRunner.RegisterSystem(std::make_unique<Systems::DeathCheckSystem>());
}

// MapTD用システム登録
void RegisterMapTDSystems(Core::SystemRunner& systemRunner) {
    systemRunner.RegisterSystem(std::make_unique<MapTD::Systems::PathFollowerSystem>());
    systemRunner.RegisterSystem(std::make_unique<MapTD::Systems::GridPlacementSystem>());
    systemRunner.RegisterSystem(std::make_unique<Systems::CombatSystem>());
    systemRunner.RegisterSystem(std::make_unique<Systems::DeathCheckSystem>());
}
```

### 5.2 ゲームモード切り替え

```cpp
namespace New::Application {

enum class TDGameMode {
    LineTD,
    MapTD
};

class TDGameScene {
public:
    void SetGameMode(TDGameMode mode) {
        gameMode_ = mode;
        
        // システム再登録
        auto& systemRunner = context_.GetSystemRunner();
        systemRunner.Clear();
        
        if (mode == TDGameMode::LineTD) {
            RegisterLineTDSystems(systemRunner);
            waveManager_ = std::make_unique<LineTD::Managers::LineTDWaveManager>();
        } else {
            RegisterMapTDSystems(systemRunner);
            waveManager_ = std::make_unique<MapTD::Managers::MapTDWaveManager>();
        }
        
        systemRunner.Initialize(context_);
    }

private:
    TDGameMode gameMode_ = TDGameMode::LineTD;
    Core::GameContext& context_;
    std::unique_ptr<IWaveManager> waveManager_;
};

} // namespace New::Application
```

---

## 6. JSON定義との連携

### 6.1 波定義の読み込み

```cpp
// WaveDefからWaveManagerに設定
void LoadWave(const WaveDef& waveDef, IWaveManager& waveManager, Core::World& world, Core::GameContext& context) {
    waveManager.StartWave(waveDef, world, context);
}
```

### 6.2 エンティティスポーン

```cpp
entt::entity SpawnEntity(const EntityDef& entityDef, Core::World& world, Core::GameContext& context, const SpawnParams& params) {
    auto entity = world.CreateEntity(entityDef.id);
    
    // 基本コンポーネント
    world.AddComponent<Transform>(entity, Transform{params.x, params.y});
    world.AddComponent<Sprite>(entity, LoadSprite(entityDef.sprite));
    world.AddComponent<Stats>(entity, entityDef.stats);
    
    // TD固有コンポーネント
    if (params.isLineTD) {
        world.AddComponent<LineTD::Components::LaneMovement>(entity, LaneMovement{
            params.lane, entityDef.stats.speed, params.x, true
        });
    } else {
        world.AddComponent<MapTD::Components::PathFollower>(entity, PathFollower{
            params.pathId, 0.0f, entityDef.stats.speed
        });
    }
    
    return entity;
}
```

---

## 7. 実装チェックリスト

### 7.1 直線型TD

- [ ] `Lane`コンポーネント実装
- [ ] `LaneMovement`コンポーネント実装
- [ ] `LaneMovementSystem`実装
- [ ] `LineTDSpawnSystem`実装
- [ ] `LineTDWaveManager`実装

### 7.2 マップ型TD

- [ ] `GridPosition`コンポーネント実装
- [ ] `PathFollower`コンポーネント実装
- [ ] `PathFollowerSystem`実装
- [ ] `GridPlacementSystem`実装
- [ ] `MapTDWaveManager`実装
- [ ] パス定義ローダー実装

### 7.3 共通システム

- [ ] `ResourceManager`実装
- [ ] `CombatSystem`実装
- [ ] `GameStateManager`実装
- [ ] 勝敗判定ロジック

---

## 8. 参考資料

- [コアアーキテクチャ設計](core-architecture-design.md)
- [JSONスキーマ設計](json-schema-design.md)
- [設計原則と制約](design-principles-and-constraints.md)
- 直線型TD詳細: `linea-td-detailed-design.md`（LineTD固有のJSON例・運用手順はこちらを参照）

## 9. メタ進行・経済ポリシー

- 通貨/素材シンク: 強化/解放/リトライ費用で消費させ、ステージ周回で過剰に溜まらないよう上限と減衰を設定。  
- 強化・解放コスト: 進行度に応じて逓増。インフレ抑制のため上限値と漸減係数を設ける（例: 成長率が一定閾値を超えたら逓減）。  
- 難易度モード: 解放条件をステージクリア数や達成目標で設定し、報酬倍率を明示（Normal=1.0, Hard=1.2, Extreme=1.4 など）。  
- デイリー/ウィークリー目標: 達成回数と報酬上限を設け、過剰周回を抑制。  
- 報酬経路: ゴールド/素材は周回で獲得、上記シンクで消費。報酬テーブルはスキーマで倍率/上限を持たせ、DevModeで確認可能にする。

## 10. 設定ポリシー（カテゴリ/保存/適用）

- カテゴリ: 音量（マスター/SE/BGM）、表示（解像度/フルスクリーン/FPS上限/VSync/UIスケール）、入力リマップ、アクセシビリティ（色覚プリセット/フォントサイズ/コントラスト/字幕・表示強度）。  
- デフォルト値例: 音量100%、解像度=システム推奨、VSync ON、FPS上限=60、UIスケール=1.0、色覚=標準、フォントサイズ=標準。  
- 適用タイミング: 音量/UIスケール/字幕・表示強度/色覚は即時反映。VSync/FPS上限/解像度/フルスクリーンは再初期化または次起動反映（再初期化可能ならその場で行う）。  
- 保存タイミング: 設定変更時に即時保存＋終了時にも保存。  
- 破損時フォールバック: バリデーション失敗は既定値で復旧し、警告表示。未知キーは無視、欠落は既定値補完。  
- 入力リマップ: 重複バインドを検出し警告。仮想座標対応のズーム/パンはステージごとに許可フラグを持つ。  
- アクセシビリティプレビュー: 色覚/フォントサイズ/コントラスト変更時に即時プレビュー、元に戻す「キャンセル/リセット」操作を提供。

## 11. セーブ/バックアップ/リカバリ方針

- 保存対象: 進行度（ステージクリア履歴、難易度解放、報酬獲得）と設定。  
- タイミング: Victory時と設定変更時に即時保存。  
- バックアップ: 直近数世代（例: 最新+3世代）をローテーション保持。  
- 破損時フォールバック: バリデーションに失敗したら最新バックアップにロールバックし、警告を出す。  
- クラッシュ/異常終了: 直近ステージ開始前のスナップショットへ戻す。  
- 部分適用: セーブの一部のみ破損時は健全部分を適用し、欠落箇所は既定値に戻す。  
- バージョン: `schemaVersion` は設けず最新版前提。破壊的変更はローダで検知し、互換変換がない場合は読み取り専用＋再保存要求。

## 12. シンプル必須要素（オフライン向け軽量版）

- 画面フロー（5画面＋戦闘）: タイトル → スロット選択(3固定) → ホーム → ステージ選択 → ステージ詳細 → 編成確認 → 戦闘 → リザルト。ホームから編成/強化/ショップ/設定に遷移。  
- 編成: メイン3 + サブ5 + アビリティ2。プリセット3固定。コストはメイン/サブ合算で上限判定。サブは低コスト帯を想定（例: サブ1–3、メイン3–8）。ロール警告はメイン+サブ合算で判定し、必要なら「メインに必須ロール1枠」程度を後追い追加。プリセット上書きは確認ダイアログ。  
- 強化/解放: 確定強化のみ。必要コスト表示とプレビューを必須、ロックで誤操作防止。  
- ショップ: 素材/ゴールドの2タブのみ。毎日リセット、所持上限表示、購入時は確認ダイアログ。  
- ミッション簡略: 固定タスクの進捗サマリと未受取通知のみ（詳細UIは省略）。  
- ステージ詳細: 推奨戦力/敵プレビュー/報酬/難易度解放条件（Easy/Normal/Hard）を表示。  
- 設定（最小）: 解像度+適用ボタン、VSync/FPS上限、UIスケール、音量（BGM/SE/マスター）、字幕ON/OFF。即時反映と再初期化項目を区分。  
- セーブ/スロット/バックアップ: スロット3固定。自動保存（Victory/設定変更）、ローテーションバックアップ、破損時はロールバック＋警告。  
- 誤操作防止: 高額購入/強化/プリセット上書き/売却は確認ダイアログ必須。空状態ガイド（編成未設定・ショップ在庫なし等）を表示。

## 13. 演出・フィードバック（最低限リスト）

- ヒット: 小エフェクト＋小SE。  
- 被弾: 中エフェクト＋中SE、カメラ振動なし。  
- 死亡: 中エフェクト＋中SE、短いカメラ振動(小)。  
- クリア: 大エフェクト（控えめな祝砲）＋勝利SE、振動なし。  
- 敗北: 低彩度フェード＋敗北SE、振動なし。  
- ボス出現: UIトースト＋SE、カメラ振動(小)を1回。  
- ウェーブ開始/終了: UIトースト＋短SE、振動なし。

## 14. エラー/例外パス（ロード・設定適用）

- ロード失敗/スキーマ不整合/ステージデータ欠落: メッセージ表示 → リトライ/スキップ/前回キャッシュへ復帰を選択肢として提示。  
- ステージ欠落時: そのステージをスキップし、次ステージを案内（オフライン用の代替動作）。  
- 設定適用失敗（解像度/フルスクリーン/VSync/FPS上限の再初期化失敗）: 元の設定に戻し、警告を表示。  
- HotReload失敗: キャッシュを保持し、エラーログ＋DevModeトーストで通知。

## 15. HotReload/検証ワークフロー（編成・ステージ）

- 監視パス: 編成プリセット/セーブ（main3+sub5+abilities2）、ステージ定義（難易度/報酬/推奨戦力/波）。  
- トリガ: ファイル変更検出 → スキーマ検証 → ネイティブ変換（プリセット/ステージ構造体） → 適用。  
- 部分失敗時: 検証失敗部分は適用せず、前回キャッシュを保持。「partial」ステータスで通知。  
- ステージ欠落/不整合: 該当ステージをスキップし、次ステージを案内。キャッシュがあればそれを使用。  
- ロールバック: 致命エラー時は全体を前回キャッシュへ戻し、リトライ/スキップを提示。  
- 通知: DevModeトースト＋ログに status=ok/reloaded/warn/error/partial で記録。

## 16. DevMode 運用メモ

- ライブ編集対象: Wave/Path/TD設定のJSON。検証に通った部分のみ適用し、失敗部分は前回キャッシュを維持する。警告はDevModeパネルに表示する。

## 17. データバージョニング

- TD関連JSONは `schemaVersion` を持たず最新版前提で運用する。破壊的変更時はスキーマとローダを同時に更新し、HotReload回帰で確認する。

## 18. セキュリティ/デバッグ運用メモ

- デバッグUI/チートコマンドは Dev/Debug ビルドのみ有効。リリースビルドでは無効化する。  
- セーブ改変検知は実装しない（同人規模につき不問）。  
- 詳細ログやコマンド有効化はビルドフラグで制御する。

---

**作成日**: 2025-12-06  
**最終更新**: 2025-12-07  
**バージョン**: 1.1  
**対象**: 新アーキテクチャ開発者向け

## 付録A: 直線型TD詳細（旧 `linea-td-detailed-design.md` 統合）

本付録は LineTD の詳細運用・JSON例・システム分解をまとめた簡易版です。本文の概要セクションから必要に応じて参照してください。

### A.1 コア概念（抜粋）

- レーン: `laneId` と `y` で識別。地上/空中を分離可能。
- 拠点ライン: 到達時にライフ減算する終端X。
- 前線位置: 味方/敵の押し合い境界。近接攻撃は前線基準。
- コスト: 時間で回復し、ユニット配置に消費。
- ウェーブ: スポーンイベント集合。ボス波を含む。

### A.2 JSONサンプル

```json
{
  "lanes": [
    { "id": 0, "y": 540, "type": "ground" },
    { "id": 1, "y": 420, "type": "air" }
  ],
  "cost": { "max": 50, "start": 10, "regenPerSec": 2.5 },
  "waves": [
    {
      "id": 1,
      "startTime": 0.0,
      "spawns": [
        { "time": 0.0, "lane": 0, "enemyId": "slime_s", "count": 3, "interval": 1.0 },
        { "time": 5.0, "lane": 0, "enemyId": "slime_m", "count": 1 },
        { "time": 8.0, "lane": 1, "enemyId": "bat_s", "count": 2, "interval": 0.5 }
      ],
      "boss": false
    },
    {
      "id": 2,
      "startTime": 25.0,
      "spawns": [
        { "time": 0.0, "lane": 0, "enemyId": "slime_boss", "count": 1 }
      ],
      "boss": true
    }
  ],
  "unitDeck": [
    { "slot": 0, "unitId": "soldier_s", "cost": 8, "cooldown": 1.5 },
    { "slot": 1, "unitId": "archer_s", "cost": 10, "cooldown": 2.0 }
  ]
}
```

### A.3 主要コンポーネント（LineTD）

- `LaneComponent`: `laneId`, `laneType`
- `LaneMovement`: `speed`, `currentX`, `movingRight`
- `LanePlaceable`: 配置可能位置
- `CostComponent`: `current`, `max`, `regenPerSec`
- `SpawnRequest`, `WaveState`, `PlacementCooldown`
- 通常の `Position/Transform` を併用

### A.4 主要システム（LineTD）

- `LaneManagerSystem`: `lanes[]` をロードし座標テーブル化、空地/空中のフィルタ提供
- `WaveSchedulerSystem`: 時刻順に SpawnRequest を生成、ボス通知イベント発火
- `SpawnSystem`: 敵/ユニット生成、初期位置設定
- `FrontlineSystem`: 押し合い境界を計算し近接前進を制御
- `MoveSystem`: 速度×方向で x 更新、到達時に拠点ダメージイベント
- `AttackSystem`: 近接は前線基準、遠隔は Projectile 発射（`combat-collision-design.md` と連携）
- `CostSystem`: コスト回復/消費、配置失敗イベント
- `UI Events`: `CostChanged`, `WaveProgress`, `UnitPlacementResult`, `BossIncoming`

### A.5 エディタ/ホットリロード

- StageEditor(F3) でレーン/コスト/ウェーブを編集（リスト＋タイムラインUI）
- JSON更新で `WaveScheduler`/`LaneManager` がリロード。失敗時はキャッシュ維持。

### A.6 実装チェックリスト（LineTD）

- [ ] `lanes`/`waves`/`cost` JSON ロードとスキーマ検証
- [ ] `LaneManagerSystem`/`WaveSchedulerSystem` 実装
- [ ] `SpawnSystem` で敵/ユニット生成
- [ ] `FrontlineSystem`/`MoveSystem` で押し合い・到達判定
- [ ] `AttackSystem` を衝突/弾システムと接続
- [ ] `CostSystem` と `PlacementCooldown` 管理
- [ ] UIイベント発火と疎結合連携
- [ ] StageEditor(F3) 対応: レーン/コスト/ウェーブ編集・保存
