# NetHack風ローグライクシステム設計書

## 概要

本ドキュメントは、NetHack-GameProjectにおけるローグライクダンジョン攻略ゲームの設計仕様を定義する。  
既存のRaylib + EnTT基盤（Core/Game層）を活用し、新規`Roguelike`層を追加してNetHack風のゲームプレイを実現する。

---

## 1. プロジェクト目標

### 1.1 ゲームコンセプト

- **NetHack（ローグライク）の再現**: ターン制・グリッドベース・永続死
- **GUI＋日本語対応**: ASCII表示ではなく、正方形タイルにテキスト描画
- **データ駆動設計**: モンスター・アイテム・ダンジョン全てをJSON定義
- **将来的な統合**: Simple-TDC-GameProject（タワーディフェンス）と仲間キャラを共有

### 1.2 技術スタック

| 項目 | 技術 | 備考 |
|------|------|------|
| 言語 | C++17 | 既存プロジェクト継続 |
| ビルド | CMake 3.15+ | 既存継続 |
| ECS | EnTT v3.12.2 | 既存継続 |
| JSON | nlohmann/json v3.11.2 | 既存継続 |
| レンダリング | Raylib 5.0 | 既存継続 |
| UI | Dear ImGui v1.90.1 | 既存継続 |

### 1.3 NetHackからの参考要素

| 要素 | 説明 | 実装優先度 |
|------|------|-----------|
| ターン制移動 | プレイヤー行動→敵行動の繰り返し | Phase 1 |
| グリッドマップ | タイルベースのダンジョン | Phase 1 |
| 視界(FOV) | 見えない場所は暗く表示 | Phase 2 |
| ランダム生成 | ダンジョンの自動生成 | Phase 2 |
| アイテム | 拾う・使う・投げる | Phase 3 |
| 戦闘 | 隣接攻撃・遠距離攻撃 | Phase 3 |
| 空腹度 | 時間経過で減少 | Phase 4 |
| 経験値/レベル | 成長システム | Phase 4 |
| 未識別アイテム | 使うまで効果不明 | Phase 5 |

---

## 2. アーキテクチャ設計

### 2.1 レイヤー構造

```
┌─────────────────────────────────────────────────────────────────┐
│                      Application Layer                          │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                         GameNew                              ││
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       ││
│  │  │ GameContext  │  │ World        │  │ SystemRunner │       ││
│  │  │ (DI Container)│  │ (registry)   │  │ (実行順管理)  │       ││
│  │  └──────────────┘  └──────────────┘  └──────────────┘       ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
          │                    │                    │
          ▼                    ▼                    ▼
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────┐
│   Core Layer    │  │   Game Layer    │  │  Roguelike Layer    │
│   (既存・参照)   │  │   (既存・参照)   │  │     ★新規作成★      │
├─────────────────┤  ├─────────────────┤  ├─────────────────────┤
│ • Position      │  │ • Sprite        │  │ • GridPosition      │
│ • Velocity      │  │ • Animation     │  │ • TurnActor         │
│ • GameContext   │  │ • SpriteSheet   │  │ • Tile              │
│ • World         │  │ • EntityFactory │  │ • MapData           │
│ • SystemRunner  │  │ • GameRenderer  │  │ • FieldOfView       │
│ • Definitions   │  │                 │  │ • Inventory         │
│                 │  │                 │  │ • Equipment         │
│                 │  │                 │  │ • MonsterAI         │
│                 │  │                 │  │ • CombatStats       │
│                 │  │                 │  │ • Hunger            │
│                 │  │                 │  │                     │
│                 │  │                 │  │ Systems:            │
│                 │  │                 │  │ • TurnSystem        │
│                 │  │                 │  │ • GridMoveSystem    │
│                 │  │                 │  │ • FOVSystem         │
│                 │  │                 │  │ • AISystem          │
│                 │  │                 │  │ • CombatSystem      │
│                 │  │                 │  │ • DungeonGenerator  │
│                 │  │                 │  │ • TileRenderer      │
└─────────────────┘  └─────────────────┘  └─────────────────────┘
```

### 2.2 Core層との関係（将来のリファクタリング考慮）

現時点では**Core層のコードを直接編集しない**方針とする。
Roguelike層から`#include`で参照し、必要に応じて継承・ラップする。

```cpp
// 例: Core層のGameContextをそのまま使用
#include "Core/GameContext.h"
#include "Core/World.h"

// Roguelike固有のサービスを追加登録
context.Register<Roguelike::TurnManager>();
context.Register<Roguelike::DungeonManager>();
```

将来Core層が変更された場合：

1. Roguelike層側で差分を吸収するアダプタを作成
2. または共通Core層を別リポジトリ化（Git Submodule等）

---

## 3. タイルレンダリング設計

### 3.1 方針

NetHackはASCII文字（半角）でマップを表現するが、本プロジェクトでは：

1. **正方形タイル**（32x32 または 64x64 ピクセル）を使用
2. **RenderTexture**に背景色＋文字を描画してテクスチャキャッシュ
3. 後から**画像テクスチャに差し替え可能**な設計

### 3.2 タイル表現（NetHack互換）

| 記号 | 意味 | 背景色 | 文字色 | 日本語名 |
|------|------|--------|--------|----------|
| `.` | 床（部屋） | 暗灰色 | 薄灰色 | 床 |
| `#` | 通路 | 暗灰色 | 茶色 | 通路 |
| `-` | 壁（横） | 黒 | 白 | 壁 |
| `\|` | 壁（縦） | 黒 | 白 | 壁 |
| `+` | ドア（閉） | 茶色 | 黄色 | 扉 |
| `'` | ドア（開） | 茶色 | 黄色 | 開扉 |
| `<` | 上り階段 | 灰色 | 白 | 上階段 |
| `>` | 下り階段 | 灰色 | 白 | 下階段 |
| `@` | プレイヤー | - | 白 | 自分 |
| 各種 | モンスター | - | 種類別 | - |

### 3.3 TileRenderer クラス設計

```cpp
// include/Roguelike/Rendering/TileRenderer.h

namespace Roguelike {

struct TileAppearance {
    char symbol;           // 表示文字（'@', '#', '.' など）
    Color foreground;      // 文字色
    Color background;      // 背景色
};

class TileRenderer {
public:
    static constexpr int TILE_SIZE = 32;  // 正方形タイルサイズ
    
    void Initialize(const Font& font);
    void Shutdown();
    
    // タイルテクスチャを取得（キャッシュから or 生成）
    Texture2D GetTileTexture(const TileAppearance& appearance);
    
    // マップ全体を描画
    void RenderMap(const MapData& map, 
                   int offsetX, int offsetY,
                   int viewportWidth, int viewportHeight);
    
    // 単一タイルを描画
    void RenderTile(int screenX, int screenY, 
                    const TileAppearance& appearance);

private:
    // キャッシュキー生成
    uint64_t MakeCacheKey(char symbol, Color fg, Color bg);
    
    // RenderTextureからテクスチャ生成
    Texture2D GenerateTileTexture(const TileAppearance& appearance);
    
    Font font_;
    std::unordered_map<uint64_t, Texture2D> textureCache_;
};

} // namespace Roguelike
```

### 3.4 描画フロー

```
1. ゲーム開始時
   └─ TileRenderer::Initialize() でフォントロード

2. 毎フレーム描画
   ├─ プレイヤー視点を中心にビューポート計算
   ├─ 視界内のタイルをループ
   │   ├─ MapData から TileAppearance 取得
   │   ├─ TileRenderer::GetTileTexture() でキャッシュ確認/生成
   │   └─ DrawTexture() で描画
   └─ エンティティ（プレイヤー、モンスター）を上に描画

3. テクスチャキャッシュ
   ├─ 初回アクセス時に RenderTexture で生成
   ├─ symbol + foreground + background をキーにハッシュ
   └─ 以降はキャッシュから即座に取得
```

---

## 4. コンポーネント設計

### 4.1 グリッド/マップ関連

```cpp
// include/Roguelike/Components/GridComponents.h

namespace Roguelike::Components {

// グリッド座標（タイル単位）
struct GridPosition {
    int x;
    int y;
};

// タイルの種類
enum class TileType : uint8_t {
    Void = 0,       // 何もない（未生成）
    Floor,          // 床
    Wall,           // 壁
    Corridor,       // 通路
    DoorClosed,     // 閉じたドア
    DoorOpen,       // 開いたドア
    StairsUp,       // 上り階段
    StairsDown,     // 下り階段
    Water,          // 水
    Lava,           // 溶岩
};

// 単一タイル情報
struct Tile {
    TileType type = TileType::Void;
    bool explored = false;   // 一度でも見たか
    bool visible = false;    // 現在視界内か
    entt::entity occupant = entt::null;  // タイル上のエンティティ
    entt::entity item = entt::null;      // 落ちているアイテム
};

// マップ全体データ（シングルトン的に1つ）
struct MapData {
    int width;
    int height;
    int currentFloor;       // 現在の階層
    std::vector<Tile> tiles;
    
    Tile& At(int x, int y) { return tiles[y * width + x]; }
    const Tile& At(int x, int y) const { return tiles[y * width + x]; }
    bool InBounds(int x, int y) const { 
        return x >= 0 && x < width && y >= 0 && y < height; 
    }
    bool IsWalkable(int x, int y) const;
    bool BlocksVision(int x, int y) const;
};

} // namespace Roguelike::Components
```

### 4.2 ターン制関連

```cpp
// include/Roguelike/Components/TurnComponents.h

namespace Roguelike::Components {

// ターン行動を持つエンティティ
struct TurnActor {
    int speed = 100;         // 行動速度（高いほど頻繁に行動）
    int energy = 0;          // 蓄積エネルギー（100で行動可能）
    bool isPlayer = false;   // プレイヤーか
};

// 行動待ちキュー（TurnManagerで管理）
// 最もenergyが高いアクターから順に行動

// プレイヤー入力待ち状態
struct AwaitingInput {
    // タグコンポーネント（入力待ち中のプレイヤーを示す）
};

// 行動コマンド（プレイヤーまたはAIが設定）
struct ActionCommand {
    enum class Type {
        None,
        Move,       // 移動
        Attack,     // 攻撃
        Wait,       // 待機
        PickUp,     // 拾う
        Use,        // 使う
        Drop,       // 落とす
        Open,       // ドアを開ける
        Close,      // ドアを閉める
        Descend,    // 階段を降りる
        Ascend,     // 階段を昇る
    };
    
    Type type = Type::None;
    int targetX = 0;
    int targetY = 0;
    entt::entity targetEntity = entt::null;
    int itemSlot = -1;
};

} // namespace Roguelike::Components
```

### 4.3 視界（FOV）関連

```cpp
// include/Roguelike/Components/VisionComponents.h

namespace Roguelike::Components {

// 視界を持つエンティティ
struct FieldOfView {
    int radius = 8;          // 視界半径
    bool needsUpdate = true; // 再計算が必要か
    std::vector<bool> visible; // 可視タイルフラグ（マップサイズ）
};

} // namespace Roguelike::Components
```

### 4.4 戦闘関連

```cpp
// include/Roguelike/Components/CombatComponents.h

namespace Roguelike::Components {

// 戦闘ステータス
struct CombatStats {
    int maxHP = 10;
    int currentHP = 10;
    int attack = 1;      // 基本攻撃力
    int defense = 0;     // 基本防御力
    int level = 1;
    int experience = 0;
    int experienceToNext = 20;
};

// モンスターAI
struct MonsterAI {
    enum class State {
        Sleeping,    // 眠っている（視界に入るまで）
        Wandering,   // 徘徊中
        Chasing,     // プレイヤー追跡中
        Fleeing,     // 逃走中
    };
    State state = State::Sleeping;
    entt::entity target = entt::null;
};

// 死亡フラグ
struct Dead {
    // タグコンポーネント
};

} // namespace Roguelike::Components
```

### 4.5 アイテム/インベントリ関連

```cpp
// include/Roguelike/Components/ItemComponents.h

namespace Roguelike::Components {

// アイテム種別
enum class ItemType : uint8_t {
    Weapon,
    Armor,
    Potion,
    Scroll,
    Food,
    Gold,
    Misc,
};

// アイテム基本情報
struct Item {
    std::string id;          // 定義ID
    std::string name;        // 表示名
    ItemType type;
    int quantity = 1;        // スタック数
    bool identified = false; // 識別済みか
};

// 装備可能アイテム
struct Equippable {
    enum class Slot {
        Weapon,
        Armor,
        Ring,
        Amulet,
    };
    Slot slot;
    int attackBonus = 0;
    int defenseBonus = 0;
};

// インベントリ
struct Inventory {
    static constexpr int MAX_SLOTS = 52;  // a-z, A-Z
    std::array<entt::entity, MAX_SLOTS> items;
    int gold = 0;
    
    int FindEmptySlot() const;
    int CountItems() const;
};

// 装備中
struct Equipment {
    entt::entity weapon = entt::null;
    entt::entity armor = entt::null;
    entt::entity ring = entt::null;
    entt::entity amulet = entt::null;
};

// 空腹度
struct Hunger {
    int current = 1000;      // 現在値
    int max = 1000;          // 最大値
    
    enum class State {
        Satiated,    // 満腹
        Normal,      // 普通
        Hungry,      // 空腹
        Weak,        // 衰弱
        Fainting,    // 気絶寸前
        Starving,    // 餓死寸前
    };
    State GetState() const;
};

} // namespace Roguelike::Components
```

---

## 5. システム設計

### 5.1 ターンシステム

```cpp
// include/Roguelike/Systems/TurnSystem.h

namespace Roguelike::Systems {

class TurnManager {
public:
    enum class State {
        ProcessingTurns,  // ターン処理中
        AwaitingInput,    // プレイヤー入力待ち
        Animating,        // アニメーション中（将来用）
    };
    
    void Update(Core::World& world, float deltaTime);
    
    State GetState() const { return state_; }
    entt::entity GetCurrentActor() const { return currentActor_; }
    
private:
    void GiveEnergy(Core::World& world);
    entt::entity FindNextActor(Core::World& world);
    void ProcessAction(Core::World& world, entt::entity actor);
    
    State state_ = State::ProcessingTurns;
    entt::entity currentActor_ = entt::null;
};

} // namespace Roguelike::Systems
```

### 5.2 ターン処理フロー

```
┌─────────────────────────────────────────────────────────────────┐
│                        TurnManager                              │
├─────────────────────────────────────────────────────────────────┤
│  1. GiveEnergy()                                                │
│     └─ 全TurnActorにspeedを加算                                 │
│                                                                 │
│  2. FindNextActor()                                             │
│     └─ energy >= 100 のアクターを探す                           │
│        ├─ なし → 1に戻る                                        │
│        ├─ プレイヤー → AwaitingInput状態へ                      │
│        └─ モンスター → AIが行動決定                             │
│                                                                 │
│  3. ProcessAction()                                             │
│     ├─ ActionCommandを読み取り                                  │
│     ├─ 行動を実行（移動/攻撃/etc）                              │
│     ├─ energy -= 100                                            │
│     └─ 2に戻る                                                  │
└─────────────────────────────────────────────────────────────────┘

プレイヤー入力待ち中:
┌─────────────────────────────────────────────────────────────────┐
│  InputSystem                                                    │
│  ├─ 方向キー → ActionCommand{Move, dx, dy} を設定              │
│  ├─ スペース → ActionCommand{Wait} を設定                      │
│  ├─ 'g' → ActionCommand{PickUp} を設定                         │
│  └─ etc...                                                      │
│                                                                 │
│  ActionCommand設定後 → AwaitingInputタグ削除                    │
│  TurnManager → ProcessingTurns状態に戻る                        │
└─────────────────────────────────────────────────────────────────┘
```

### 5.3 主要システム一覧

| システム | 説明 | Phase |
|----------|------|-------|
| `TurnSystem` | ターン進行管理 | 1 |
| `InputSystem` | プレイヤー入力→ActionCommand変換 | 1 |
| `GridMovementSystem` | グリッド移動処理 | 1 |
| `FOVSystem` | 視界計算（Shadowcasting） | 2 |
| `DungeonGenerator` | ダンジョン自動生成 | 2 |
| `AISystem` | モンスターAI（追跡・徘徊） | 3 |
| `CombatSystem` | 戦闘処理 | 3 |
| `ItemSystem` | アイテム拾得・使用 | 3 |
| `HungerSystem` | 空腹度管理 | 4 |
| `LevelUpSystem` | 経験値・レベルアップ | 4 |
| `IdentificationSystem` | 未識別アイテム管理 | 5 |

---

## 6. ダンジョン生成設計

### 6.1 生成アルゴリズム（BSP分割法）

```
1. マップ全体を1つの領域として開始
2. 領域を縦または横に分割（再帰的に）
3. 各末端領域に部屋を生成
4. 隣接する部屋同士を通路で接続
5. 階段・ドアを配置
6. モンスター・アイテムを配置
```

### 6.2 生成パラメータ（JSON定義）

```json
// assets/definitions/dungeons/floor_1.dungeon.json
{
    "id": "floor_1",
    "name": "地下1階",
    "width": 80,
    "height": 24,
    "generation": {
        "algorithm": "bsp",
        "minRoomSize": 4,
        "maxRoomSize": 10,
        "splitDepth": 4
    },
    "spawns": {
        "monsters": [
            { "id": "rat", "min": 2, "max": 5 },
            { "id": "kobold", "min": 1, "max": 3 }
        ],
        "items": [
            { "id": "potion_healing", "min": 0, "max": 2 },
            { "id": "food_ration", "min": 1, "max": 2 }
        ]
    },
    "features": {
        "stairsUp": true,
        "stairsDown": true,
        "doorChance": 0.5
    }
}
```

---

## 7. JSON定義スキーマ

### 7.1 モンスター定義

```json
// assets/definitions/monsters/kobold.monster.json
{
    "id": "kobold",
    "name": "コボルド",
    "symbol": "k",
    "color": { "r": 255, "g": 165, "b": 0 },
    "description": "小柄だが狡猾な人型生物",
    
    "stats": {
        "maxHP": 8,
        "attack": 3,
        "defense": 1,
        "speed": 100,
        "experience": 10
    },
    
    "ai": {
        "behavior": "aggressive",
        "sightRadius": 6
    },
    
    "drops": [
        { "itemId": "gold", "chance": 0.5, "min": 1, "max": 10 }
    ],
    
    "tags": ["humanoid", "small"]
}
```

### 7.2 アイテム定義

```json
// assets/definitions/items/potion_healing.item.json
{
    "id": "potion_healing",
    "name": "回復の薬",
    "unidentifiedName": "赤い薬",
    "symbol": "!",
    "color": { "r": 255, "g": 0, "b": 0 },
    "type": "potion",
    "description": "HPを回復する薬",
    
    "effects": [
        {
            "type": "heal",
            "value": 20,
            "message": "{name}は傷が癒えた！"
        }
    ],
    
    "weight": 1,
    "value": 50
}
```

### 7.3 ディレクトリ構造

```
assets/definitions/
├── monsters/
│   ├── rat.monster.json
│   ├── kobold.monster.json
│   └── orc.monster.json
├── items/
│   ├── potions/
│   │   ├── potion_healing.item.json
│   │   └── potion_poison.item.json
│   ├── scrolls/
│   ├── weapons/
│   └── armor/
├── dungeons/
│   ├── floor_1.dungeon.json
│   └── floor_2.dungeon.json
└── tiles/
    └── tile_definitions.json
```

---

## 8. UI設計

### 8.1 操作体系

本ゲームは**初心者にも分かりやすいシンプル操作**を採用。  
NetHackの複雑なキーバインドではなく、エルローグ/RPGツクール風の直感的な操作。

#### 基本操作

| キー | 動作 | 説明 |
|------|------|------|
| 矢印キー | 移動 | 上下左右4方向に移動 |
| Enter / Space | 決定/調べる | 足元メニューを開く、選択を決定 |
| ESC | キャンセル | メニューを閉じる、前に戻る |
| I | 持ち物 | インベントリ画面を開く |
| W | 待機 | その場で1ターン待機 |

#### 操作フロー

```
[通常探索モード]
  ├─ 矢印キー → 移動（隣に敵がいれば自動攻撃）
  ├─ Enter/Space → 足元メニューを開く
  │   ├─ 階段降りる/昇る（階段の上のとき）
  │   ├─ アイテムを拾う（アイテムの上のとき）
  │   └─ 待機
  ├─ I → インベントリ画面
  │   ├─ ↑↓で選択
  │   ├─ Enterで操作メニュー
  │   │   ├─ 使う
  │   │   ├─ 装備/外す
  │   │   ├─ 捨てる
  │   │   └─ やめる
  │   └─ ESCで閉じる
  └─ W → 待機（1ターン消費）
```

#### 廃止した操作（NetHackからの変更）

以下のNetHack式操作は廃止し、メニュー選択に統合：

| 旧操作 | 新操作 |
|--------|--------|
| `g` / `,` (拾う) | Enter → メニュー「拾う」 |
| `>` / `<` (階段) | Enter → メニュー「降りる/昇る」 |
| `d` (落とす) | I → アイテム選択 → 「捨てる」 |
| `u` (使う) | I → アイテム選択 → 「使う」 |
| `hjkl` (移動) | 矢印キーのみ |
| `yubn` (斜め) | 未サポート（4方向のみ） |

### 8.2 画面レイアウト

```
┌─────────────────────────────────────────────────────────────────┐
│  Roguelike RPG - 地下1階                              HP: 20/20 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│     ########                                                    │
│     #......#          ######                                    │
│     #..@...#          #....#                                    │
│     #......############....#                                    │
│     #......+..........+....#                                    │
│     #......#          #..k.#                                    │
│     #..>...#          #....#                                    │
│     ########          ######                                    │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│ メッセージログ:                                                  │
│ > あなたはコボルドを攻撃した。                                    │
│ > コボルドに5ダメージを与えた。                                   │
│ > コボルドがあなたを攻撃してきた！                                │
├─────────────────────────────────────────────────────────────────┤
│ [操作ガイド]                                                     │
│ [矢印] 移動  [Enter] 調べる  [I] 持ち物  [ESC] 戻る  [W] 待機    │
└─────────────────────────────────────────────────────────────────┘
```

### 8.3 メニューUI

#### 足元アクションメニュー

階段やアイテムの上でEnterを押すと表示：

```
┌────────────────────┐
│ どうする？          │
├────────────────────┤
│ ▶ 階段を降りる     │
│   アイテムを拾う   │
│   待機             │
├────────────────────┤
│ [↑↓]選択 [Enter]決定│
│ [ESC]戻る          │
└────────────────────┘
```

#### インベントリ画面

カーソル選択式のアイテム一覧：

```
┌──────────────────────────────────────────┐
│ 持ち物                      所持金: 100G │
├──────────────────────────────────────────┤
│ ▶ ! 回復薬                              │
│   ) ダガー [E]                           │
│   [ 革鎧 [E]                             │
│   % 携帯食料 x3                          │
│                                          │
├──────────────────────────────────────────┤
│ [↑↓]選択 [Enter]決定 [ESC]戻る           │
└──────────────────────────────────────────┘
```

#### アイテム操作メニュー

インベントリでアイテムを選択後に表示：

```
┌──────────────────┐
│ どうする？        │
├──────────────────┤
│ ▶ 使う           │
│   装備/外す      │
│   捨てる         │
│   やめる         │
└──────────────────┘
```

---

## 9. 実装ロードマップ（Phase分け）

---

### Phase 1: 基本移動とターン制 【動作確認ポイント】

**目標**: プレイヤーがグリッド上を移動でき、ターンが進行することを確認

#### TODO リスト

- [ ] **1.1** ディレクトリ構造作成
  - [ ] `include/Roguelike/` フォルダ作成
  - [ ] `include/Roguelike/Components/` 作成
  - [ ] `include/Roguelike/Systems/` 作成
  - [ ] `include/Roguelike/Rendering/` 作成
  - [ ] `src/Roguelike/` フォルダ作成
  - [ ] CMakeLists.txt にソース追加

- [ ] **1.2** 基本コンポーネント実装
  - [ ] `GridPosition` コンポーネント
  - [ ] `TileType` 列挙型
  - [ ] `Tile` 構造体
  - [ ] `MapData` 構造体（固定マップ）
  - [ ] `TurnActor` コンポーネント
  - [ ] `ActionCommand` コンポーネント
  - [ ] `AwaitingInput` タグコンポーネント

- [ ] **1.3** TileRenderer実装
  - [ ] `TileAppearance` 構造体
  - [ ] `TileRenderer::Initialize()` - フォント読み込み
  - [ ] `TileRenderer::GenerateTileTexture()` - RenderTexture生成
  - [ ] `TileRenderer::GetTileTexture()` - キャッシュ管理
  - [ ] `TileRenderer::RenderMap()` - マップ描画

- [ ] **1.4** ターンシステム実装
  - [ ] `TurnManager` クラス
  - [ ] `TurnManager::GiveEnergy()` - エネルギー供給
  - [ ] `TurnManager::FindNextActor()` - 次のアクター選択
  - [ ] `TurnManager::ProcessAction()` - 行動実行

- [ ] **1.5** 入力システム実装
  - [ ] `InputSystem` - キー入力処理
  - [ ] 方向キー → 移動コマンド変換
  - [ ] スペースキー → 待機コマンド

- [ ] **1.6** 移動システム実装
  - [ ] `GridMovementSystem` - 移動処理
  - [ ] 壁判定（移動不可）
  - [ ] `MapData::occupant` 更新

- [ ] **1.7** テストシーン作成
  - [ ] `RoguelikeScene` クラス
  - [ ] 固定マップ生成（5x5の部屋）
  - [ ] プレイヤーエンティティ生成
  - [ ] GameNewへの統合

#### Phase 1 動作確認チェックリスト

```
□ ゲーム起動時に固定マップが表示される
□ プレイヤー（@）が画面中央に表示される
□ 方向キーでプレイヤーが移動する
□ 壁にぶつかると移動できない
□ スペースキーで待機できる
□ ターン数がカウントアップされる（デバッグ表示）
```

---

### Phase 2: ダンジョン生成と視界 【動作確認ポイント】

**目標**: ランダムダンジョンが生成され、視界システムが動作する

#### TODO リスト

- [ ] **2.1** ダンジョン生成器実装
  - [ ] `DungeonGenerator` クラス
  - [ ] BSP分割アルゴリズム
  - [ ] 部屋生成ロジック
  - [ ] 通路接続ロジック
  - [ ] ドア配置
  - [ ] 階段配置（上り・下り）

- [ ] **2.2** 視界(FOV)システム実装
  - [ ] `FieldOfView` コンポーネント
  - [ ] `FOVSystem` - Shadowcasting実装
  - [ ] `MapData::visible` 更新
  - [ ] `MapData::explored` 更新

- [ ] **2.3** 視界に応じた描画
  - [ ] 視界内: 通常描画
  - [ ] 探索済み視界外: 暗く描画
  - [ ] 未探索: 描画しない

- [ ] **2.4** 階移動
  - [ ] '>' キーで下り階段移動
  - [ ] '<' キーで上り階段移動
  - [ ] 新フロア生成

#### Phase 2 動作確認チェックリスト

```
□ ゲーム起動ごとに異なるダンジョンが生成される
□ 部屋と通路が接続されている
□ プレイヤー視界内のみ明るく表示される
□ 一度見た場所は暗く表示される（探索済み）
□ 見たことない場所は真っ黒
□ 下り階段(>)に乗って'>'キーで次の階へ行ける
□ 上り階段(<)に乗って'<'キーで前の階へ戻れる
```

---

### Phase 3: モンスターと戦闘 【動作確認ポイント】

**目標**: モンスターが出現し、戦闘できる

#### TODO リスト

- [ ] **3.1** モンスター定義ローダー
  - [ ] `MonsterDef` 構造体
  - [ ] `MonsterLoader` クラス
  - [ ] JSONからモンスター定義読み込み

- [ ] **3.2** モンスターAI実装
  - [ ] `MonsterAI` コンポーネント
  - [ ] `AISystem` - AI行動決定
  - [ ] 睡眠状態（視界外）
  - [ ] 追跡状態（プレイヤー発見時）
  - [ ] 徘徊状態

- [ ] **3.3** 戦闘システム実装
  - [ ] `CombatStats` コンポーネント
  - [ ] `CombatSystem` - ダメージ計算
  - [ ] 隣接攻撃（移動先に敵がいたら攻撃）
  - [ ] ダメージメッセージ

- [ ] **3.4** 死亡処理
  - [ ] `Dead` タグコンポーネント
  - [ ] `DeathSystem` - 死亡エンティティ処理
  - [ ] 経験値獲得
  - [ ] ドロップアイテム

- [ ] **3.5** メッセージログ
  - [ ] `MessageLog` クラス
  - [ ] 戦闘メッセージ表示
  - [ ] スクロール表示

#### Phase 3 動作確認チェックリスト

```
□ ダンジョンにモンスター(k等)が出現する
□ モンスターに向かって移動すると攻撃になる
□ モンスターがプレイヤーを追いかけてくる
□ 戦闘メッセージが表示される
□ モンスターを倒すと消える
□ プレイヤーのHPが減ると表示が変わる
□ HPが0になるとゲームオーバー
```

---

### Phase 4: アイテムとインベントリ 【動作確認ポイント】

**目標**: アイテムを拾って使用できる

#### TODO リスト

- [ ] **4.1** アイテム定義ローダー
  - [ ] `ItemDef` 構造体
  - [ ] `ItemLoader` クラス
  - [ ] JSONからアイテム定義読み込み

- [ ] **4.2** アイテムコンポーネント
  - [ ] `Item` コンポーネント
  - [ ] `Equippable` コンポーネント
  - [ ] `Inventory` コンポーネント
  - [ ] `Equipment` コンポーネント

- [ ] **4.3** アイテムシステム（メニュー操作）
  - [ ] Enter → 足元メニュー → 「拾う」でアイテム拾得
  - [ ] I → インベントリ画面 → 「捨てる」でドロップ
  - [ ] I → インベントリ画面表示（カーソル選択式）

- [ ] **4.4** アイテム使用
  - [ ] インベントリ → アイテム選択 → 「使う」
  - [ ] ポーション使用（HP回復等）
  - [ ] スクロール使用
  - [ ] 食料使用

- [ ] **4.5** 装備システム
  - [ ] インベントリ → 装備品選択 → 「装備/外す」
  - [ ] 装備による能力値変化

#### Phase 4 動作確認チェックリスト

```
□ 床にアイテム(!)が落ちている
□ アイテムの上でEnter → 「拾う」でアイテムを拾える
□ Iキーでインベントリ画面が開く
□ ↑↓でアイテムを選択できる
□ Enterでアイテム操作メニューが開く
□ 「使う」で回復薬などを使用できる
□ 「装備/外す」で武器防具を装備できる
□ 「捨てる」でアイテムを床に落とせる
□ ESCでメニューを閉じられる
```

---

### Phase 5: 成長と空腹度 【動作確認ポイント】

**目標**: レベルアップと空腹度システム

#### TODO リスト

- [ ] **5.1** 経験値/レベルシステム
  - [ ] 経験値獲得処理
  - [ ] レベルアップ判定
  - [ ] レベルアップ時の能力値上昇

- [ ] **5.2** 空腹度システム
  - [ ] `Hunger` コンポーネント
  - [ ] `HungerSystem` - 毎ターン減少
  - [ ] 空腹状態表示
  - [ ] 餓死判定

- [ ] **5.3** 食事
  - [ ] 食料アイテム
  - [ ] 食事による空腹度回復

#### Phase 5 動作確認チェックリスト

```
□ モンスター撃破で経験値を獲得する
□ 経験値が溜まるとレベルアップする
□ レベルアップでHP/攻撃力が上がる
□ ターン経過で空腹度が減少する
□ 空腹になると警告メッセージが出る
□ 食料を食べると空腹度が回復する
□ 餓死するとゲームオーバー
```

---

### Phase 6: 未識別アイテムと追加要素 【動作確認ポイント】

**目標**: NetHackらしい未識別システムと追加機能

#### TODO リスト

- [ ] **6.1** 未識別システム
  - [ ] 未識別アイテム表示（「赤い薬」等）
  - [ ] 使用による識別
  - [ ] 識別の巻物

- [ ] **6.2** ドア操作
  - [ ] 'o'キーでドアを開ける
  - [ ] 'c'キーでドアを閉める
  - [ ] ドアを壊す

- [ ] **6.3** 投擲
  - [ ] 't'キーでアイテム投げる
  - [ ] 投擲ダメージ

- [ ] **6.4** セーブ/ロード
  - [ ] ゲーム状態のJSON保存
  - [ ] ロード機能

#### Phase 6 動作確認チェックリスト

```
□ 初見アイテムは「？の薬」等と表示される
□ 使用すると識別される
□ 識別の巻物で識別できる
□ ドアを開閉できる
□ アイテムを投げられる
□ ゲームをセーブ/ロードできる
```

---

## 10. ファイル構成（最終形）

```
nethack-GameProject/
├── include/
│   ├── Core/                    # 既存（参照のみ）
│   ├── Game/                    # 既存（参照のみ）
│   └── Roguelike/               # ★新規作成
│       ├── Components/
│       │   ├── GridComponents.h
│       │   ├── TurnComponents.h
│       │   ├── VisionComponents.h
│       │   ├── CombatComponents.h
│       │   └── ItemComponents.h
│       ├── Systems/
│       │   ├── TurnSystem.h
│       │   ├── InputSystem.h
│       │   ├── GridMovementSystem.h
│       │   ├── FOVSystem.h
│       │   ├── AISystem.h
│       │   ├── CombatSystem.h
│       │   ├── ItemSystem.h
│       │   ├── HungerSystem.h
│       │   └── DeathSystem.h
│       ├── Generators/
│       │   ├── DungeonGenerator.h
│       │   └── BSPTree.h
│       ├── Rendering/
│       │   ├── TileRenderer.h
│       │   └── MinimapRenderer.h
│       ├── Managers/
│       │   ├── DungeonManager.h
│       │   ├── TurnManager.h
│       │   └── MessageLog.h
│       ├── Loaders/
│       │   ├── MonsterLoader.h
│       │   ├── ItemLoader.h
│       │   └── DungeonConfigLoader.h
│       ├── Definitions/
│       │   ├── MonsterDef.h
│       │   ├── ItemDef.h
│       │   └── DungeonDef.h
│       └── RoguelikeScene.h
│
├── src/
│   └── Roguelike/               # ★新規作成
│       ├── Components/
│       ├── Systems/
│       ├── Generators/
│       ├── Rendering/
│       ├── Managers/
│       ├── Loaders/
│       └── RoguelikeScene.cpp
│
└── assets/
    └── definitions/
        ├── monsters/            # ★新規作成
        ├── items/               # ★新規作成
        ├── dungeons/            # ★新規作成
        └── tiles/               # ★新規作成
```

---

## 11. 参考資料

### NetHack関連

- [NetHack公式](https://www.nethack.org/) - オリジナルゲーム
- [NetHack Wiki](https://nethackwiki.com/) - 詳細情報
- [Roguelike Tutorial](http://www.roguebasin.com/index.php/Complete_Roguelike_Tutorial,_using_python%2Blibtcod) - ローグライク実装チュートリアル

### アルゴリズム

- [BSP Dungeon Generation](http://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation) - ダンジョン生成
- [Shadowcasting FOV](http://www.roguebasin.com/index.php/FOV_using_recursive_shadowcasting) - 視界計算
- [A* Pathfinding](http://www.roguebasin.com/index.php/A*) - 経路探索

### 技術

- [EnTT Wiki](https://github.com/skypjack/entt/wiki) - ECSライブラリ
- [Raylib](https://www.raylib.com/) - ゲームライブラリ
- [nlohmann/json](https://github.com/nlohmann/json) - JSONライブラリ

---

## 12. 変更履歴

| 日付 | 変更内容 |
|------|----------|
| 2025-11-30 | 初版作成 |
