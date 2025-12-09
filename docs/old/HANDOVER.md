# ハンドオーバー文書: Simple-TDC-GameProject

## 1. 現在の実装状況

### Phase 0: コアアーキテクチャ ✅ 完了

- `Core::World` - ECS entt::registry ラッパー
- `Core::GameContext` - グローバルゲーム状態（時間、入力など）
- `Core::SystemRunner` - システム実行管理
- `Core::EntityFactory` - エンティティ作成ファクトリ

### Phase 1: TDシステム ✅ 完了

10個の専用システム実装：

- `LaneMovementSystem` - レーン上の移動
- `CombatDetectionSystem` - 戦闘範囲検出
- `CombatSystem` - 攻撃/ダメージ処理
- `HealthSystem` - HP管理
- `DeathSystem` - 死亡処理
- `ProjectileSystem` - 投射物
- `KnockbackSystem` - ノックバック
- `AbilitySystem` - 特殊能力
- `BaseHealthSystem` - 拠点HP
- `ResourceSystem` - お金/コスト

### Phase 2: ステージ統合 ✅ 完了

- `WaveManager` - ウェーブ管理
- `SpawnManager` - 敵スポーン制御
- `GameStateManager` - ゲーム状態（準備/戦闘/勝利/敗北）
- テストステージJSON作成
- ゲームフロー統合テスト

### Phase 2追加: フォールバック描画機能 ✅ 完了

- 画像/JSONが読み込めない場合のフォールバック描画
- 自動テクスチャ生成
- アニメーション動作確認機能

### Phase 2.5: ステージ反転修正 ✅ 完了

- **右側 = プレイヤー自陣**、**左側 = 敵陣**に変更
- 味方ユニット: 右側からスポーン、左へ移動
- 敵ユニット: 左側からスポーン、右へ移動
- ポーズ機能修正: ポーズ中はシステム停止

### Phase 3: UI/UX改善 ✅ 完了

- `TD::UI::GameUI` - UI統合クラス
- クリック可能デッキスロット
- ユニットHPバー描画
- コストゲージ、ウェーブ進行バー
- レーン背景、拠点UI
- ゲーム状態オーバーレイ

---

## 2. コンポーネント一覧（GameComponents.h）

### 移動・配置

```cpp
struct Lane { int laneIndex; };
struct LanePosition { float x; int lane; };
struct LaneMovement { float speed; int direction; };
```

### ステータス・戦闘

```cpp
struct Stats {
    float maxHealth;
    float currentHealth;
    float attackPower;
    float attackSpeed;
    float attackRange;
    float knockbackResist;
    float cost;
    float cooldown;
};

struct CombatState { ... };
struct Combat { entt::entity target; bool inCombat; };
struct ProjectileData { ... };
struct Knockback { float velocity; float deceleration; };
```

### 特殊効果

```cpp
struct Invincible { float duration; };
struct Frozen { float duration; float slowFactor; };
struct Poisoned { ... };
struct Ability { ... };
```

### 識別・フラグ

```cpp
struct Team { enum class Type { Player, Enemy } type; };
struct CharacterType { ... };
struct Dead {};
struct Spawning { float remainingTime; };
```

### フォールバック描画

```cpp
enum class FallbackShape { Circle, Triangle, Star, Diamond };

struct AnimationTestData {
    float animPhase = 0.0f;
    float bounceAmount = 4.0f;
    float stretchFactor = 1.0f;
};

struct FallbackVisual {
    Color baseColor = WHITE;
    float pulseSpeed = 2.0f;
    FallbackShape accentShape = FallbackShape::Circle;
    Color accentColor = YELLOW;
    bool showAnimation = true;
    AnimationTestData animTestData;
};
```

---

## 3. システムシグネチャ

すべてのシステムは統一シグネチャ：

```cpp
void SystemName(Core::World& world, Core::GameContext& ctx, float dt);
```

例：

```cpp
void LaneMovementSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    auto& reg = world.Registry();
    auto view = reg.view<LanePosition, LaneMovement>();
    for (auto entity : view) {
        auto& pos = view.get<LanePosition>(entity);
        auto& movement = view.get<LaneMovement>(entity);
        pos.x += movement.speed * movement.direction * dt;
    }
}
```

---

## 4. EntityFactory

### メソッド

```cpp
class EntityFactory {
    // 定義からキャラクター作成
    entt::entity CreateCharacter(const std::string& characterId, bool isEnemy);
    
    // フォールバックキャラクター作成（定義がない場合）
    entt::entity CreateFallbackCharacter(const std::string& characterId, bool isEnemy);
    
    // 投射物作成
    entt::entity CreateProjectile(...);
};
```

### フォールバック動作

`CreateCharacter()`は、キャラクター定義が見つからない場合に自動的に`CreateFallbackCharacter()`を呼び出し、デフォルト値でエンティティを作成します。

---

## 5. JSON定義フォーマット

### キャラクター定義 (`*.char.json`)

```json
{
    "id": "basic_cat",
    "name": "ネコ",
    "stats": {
        "health": 100,
        "attackPower": 20,
        "attackSpeed": 1.0,
        "attackRange": 50,
        "moveSpeed": 40,
        "knockbackResist": 0
    },
    "sprite": {
        "texture": "cat.png",
        "frameWidth": 64,
        "frameHeight": 64,
        "animations": {
            "idle": { "startFrame": 0, "frameCount": 4, "fps": 8 },
            "walk": { "startFrame": 4, "frameCount": 4, "fps": 10 },
            "attack": { "startFrame": 8, "frameCount": 3, "fps": 12 }
        }
    },
    "cost": 50,
    "cooldown": 2.0
}
```

### ステージ定義 (`*.stage.json`)

```json
{
    "id": "stage_01",
    "name": "Test Stage",
    "lanes": 3,
    "baseHealth": { "player": 1000, "enemy": 1000 },
    "startMoney": 500,
    "moneyRate": 10,
    "waves": [
        {
            "id": 1,
            "duration": 30,
            "spawns": [
                { "character": "basic_enemy", "lane": 1, "delay": 2.0 }
            ]
        }
    ],
    "availableUnits": ["basic_cat", "tank_cat"]
}
```

---

## 6. テストステージファイル

### test_stage.stage.json

基本的なテストステージ。正常なキャラクターIDを使用。

### stage_01.stage.json

3ウェーブ構成のサンプルステージ。

### fallback_test.stage.json

フォールバック機能テスト用。存在しないキャラクターID（`unknown_enemy_1`, `unknown_enemy_2`）を使用。

---

## 7. フォールバック描画システム

### FallbackHelpers名前空間（GameNew.cpp内）

```cpp
namespace FallbackHelpers {
    // プロシージャル生成テクスチャ
    Texture2D GenerateFallbackTexture(int width, int height, Color baseColor, 
                                       FallbackShape shape);
    
    // 静的フォールバック描画
    void DrawFallbackSprite(const Transform& transform, 
                           const FallbackVisual& visual);
    
    // アニメーション付きフォールバック描画
    void DrawFallbackAnimatedSprite(const Transform& transform, 
                                    FallbackVisual& visual, 
                                    float dt);
}
```

### SpriteRenderSystemの拡張

1. 通常のスプライト描画を試行
2. テクスチャがない場合 → FallbackVisualコンポーネントをチェック
3. FallbackVisualがあれば → フォールバック描画を実行

---

## 8. GameUI（Phase 3）

### TD::UI::GameUI クラス

```cpp
class GameUI {
public:
    void Initialize(int screenWidth, int screenHeight);
    void SetSlotClickCallback(SlotClickCallback callback);
    void HandleInput();  // クリック検出
    
    // 描画メソッド
    void DrawTopBar(WaveManager&, SpawnManager&, GameStateManager&);
    void DrawDeckSlots(SpawnManager&, GameContext&);
    void DrawUnitHealthBars(World&);
    void DrawLaneBackgrounds(WaveManager&);
    void DrawBases(GameStateManager&);
    void DrawControlsHelp();
    void DrawGameStateOverlay(GameStateManager&);
    
    int GetSelectedSlot() const;
    void SetSelectedSlot(int slot);
};
```

### UI機能

- **デッキスロット**: クリック＆キーボード対応、選択状態表示、クールダウンオーバーレイ
- **上部UI**: Wave進行バー、コストゲージ、拠点HPバー
- **ユニットHPバー**: 各ユニット上に表示
- **レーン背景**: 交互カラー、レーン番号表示
- **拠点**: 縦HPゲージ付き拠点描画
- **ゲーム状態**: Paused/Victory/Defeatオーバーレイ

---

## 9. ビルド設定

### ビルドターゲット

```
SimpleTDCGame_NewArch
```

### CMake設定

```bash
cmake --preset ninja-release
cmake --build out/build/ninja-release
```

### POST_BUILD

アセットは自動的に出力ディレクトリにコピーされます。

---

## 10. テスト方法

### 通常モードテスト

```cpp
// main_new.cpp
std::string stageId = "test_stage";  // または "stage_01"
```

### フォールバックモードテスト

```cpp
// main_new.cpp
std::string stageId = "fallback_test";
```

フォールバックモードでは：

- 存在しないキャラクターIDでもクラッシュしない
- カラフルな幾何学図形で代替描画
- アニメーション（パルス、バウンス）が動作

---

## 11. ファイル構造

```
include/
├── Core/
│   ├── World.h
│   ├── GameContext.h
│   ├── SystemRunner.h
│   └── EntityFactory.h
├── Game/
│   ├── Components/
│   │   └── GameComponents.h
│   └── Managers/
│       ├── WaveManager.h
│       ├── SpawnManager.h
│       └── GameStateManager.h
└── TD/
    └── UI/
        └── GameUI.h          # Phase 3: UI統合クラス

src/
├── main_new.cpp          # テストエントリポイント
├── GameNew.cpp           # ゲームクラス + 全システム
├── Core/
│   ├── World.cpp
│   ├── GameContext.cpp
│   ├── SystemRunner.cpp
│   └── EntityFactory.cpp
└── Game/
    └── Managers/
        ├── WaveManager.cpp
        ├── SpawnManager.cpp
        └── GameStateManager.cpp

assets/
└── definitions/
    ├── characters/
    │   ├── basic_cat.char.json
    │   ├── tank_cat.char.json
    │   └── basic_enemy.char.json
    └── stages/
        ├── test_stage.stage.json
        ├── stage_01.stage.json
        └── fallback_test.stage.json
```

---

## 12. 次のステップ

### Phase 4: FHD固定レンダリング & JSON UIシステム（計画中）

**目的**: 内部解像度をFHD(1920x1080)に固定し、UIをJSON定義可能にする

#### Phase 4A: FHDレンダリングシステム

- [ ] `RenderTexture`ベースのFHD固定レンダリング実装
- [ ] ウィンドウサイズに関係なくFHDで描画→スケーリング表示
- [ ] マウス座標のFHD空間への逆変換
- [ ] 全座標をFHD実数値で統一（相対座標計算不要）

```cpp
// 構想: GameRenderer クラス
class GameRenderer {
    static constexpr int RENDER_WIDTH = 1920;
    static constexpr int RENDER_HEIGHT = 1080;
    
    RenderTexture2D renderTarget_;
    
    void BeginRender();    // RenderTextureに描画開始
    void EndRender();      // スケーリングして画面に表示
    Vector2 ScreenToWorld(Vector2 screenPos);  // マウス座標変換
};
```

#### Phase 4B: JSON UI定義システム

- [ ] UI要素の基底クラス設計（`UIElement`, `UIPanel`, `UIButton`等）
- [ ] JSONからUIツリーを構築するパーサー
- [ ] イベントバインディングシステム（クリック、ホバー等）
- [ ] インラインC++実装との共存（ハイブリッド対応）

```json
// 構想: game_ui.ui.json
{
    "id": "main_hud",
    "type": "panel",
    "position": { "x": 80, "y": 5 },
    "size": { "width": 600, "height": 60 },
    "children": [
        {
            "type": "text",
            "id": "wave_label",
            "position": { "x": 10, "y": 10 },
            "text": "Wave: {wave_current}/{wave_total}",
            "fontSize": 22,
            "color": "#FFFFFF"
        },
        {
            "type": "progressBar",
            "id": "cost_gauge",
            "position": { "x": 200, "y": 25 },
            "size": { "width": 180, "height": 12 },
            "bind": "player.cost",
            "max": "player.maxCost",
            "fillColor": "#FFD700"
        }
    ]
}
```

### Phase 5: ImGui内蔵エディター（実装済）

**目的**: F1キーで起動するゲーム内エディターでデータ定義を視覚的に編集

#### Phase 5A: 基本機能（完了）

- [✓] ImGui + rlImGui統合
- [✓] EditorManagerクラス実装
- [✓] キャラクターエディター（閲覧）
- [✓] コンソールログ表示
- [✓] メニューバーシステム

```cpp
// F1キーでエディター起動
if (IsKeyPressed(KEY_F1)) {
    editor_->ToggleVisibility();
}
```

#### Phase 5B: 拡張機能（計画中）

- [ ] キャラクター編集機能
- [ ] ステージエディター
- [ ] UIレイアウトエディター
- [ ] スキル・エフェクト・サウンドエディター
- [ ] JSONファイル保存機能
PUT  /api/ui/layouts/{id}     # UI定義更新
```

#### Phase 5B: エンティティエディター

- [ ] キャラクター定義の作成/編集UI
- [ ] ステータス、アニメーション、能力の設定
- [ ] スプライトプレビュー
- [ ] JSON直接編集モード

#### Phase 5C: ステージエディター

- [ ] ノードベースのWave編集（NodeRED風）
- [ ] タイムライン形式のスポーン配置
- [ ] ステージ設定（レーン数、拠点HP等）
- [ ] プレビュー再生

#### Phase 5D: UIエディター

- [ ] ドラッグ&ドロップでUI配置
- [ ] FHD座標でのWYSIWYG編集
- [ ] プロパティパネル
- [ ] リアルタイムゲーム内プレビュー

### Phase 6: 追加機能

- [ ] より多くのキャラクター定義
- [ ] 特殊能力の実装
- [ ] エフェクトシステム
- [ ] サウンド統合

### Phase 7: 最適化

- [ ] 空間分割（Quadtree）
- [ ] オブジェクトプーリング
- [ ] バッチレンダリング

---

## 13. 注意事項

### コンポーネント設計

- `Combat`の`currentHealth`は削除され、`Stats::currentHealth`に統合
- `Invincible`は`remainingTime`ではなく`duration`を使用

### パス解決

- ビルド出力から実行時、アセットパスは`../assets/definitions`

### EnTT使用

- ビューの反復中にコンポーネント削除は避ける
- `reg.emplace_or_replace`で安全に更新

### フォールバック使用

- 開発中の仮アセットとして有用
- プロダクションでは適切なアセットを用意すること

### FHD固定レンダリング（Phase 4で実装予定）

**現状**: レンダリングはウィンドウサイズ依存（1280x720等）
**計画**: RenderTextureでFHD(1920x1080)に固定後、スケーリング表示

メリット:

- 全UI座標をFHD絶対値で指定可能
- エディターとゲームで座標系が完全一致
- 相対座標計算が不要
- 異なる画面サイズでも見た目が同一

---

## 14. 技術選定メモ

### 内蔵エディター技術スタック

- **UIフレームワーク**: Dear ImGui 1.90.1
- **Raylib統合**: rlImGui
- **アーキテクチャ**: EditorManagerパターン
- **データアクセス**: DefinitionRegistry/DefinitionLoader経由

### エディター機能

- キャラクターエディター（閲覧/編集）
- コンソールログ表示
- インスペクター（エンティティ詳細）
- F1キーで表示/非表示

### ファイル監視

- エディターでJSON保存 → ゲーム側で自動リロード（ホットリロード）
