# キャラクター描画システム設計書

**バージョン**: 2.0  
**最終更新**: 2025-12-18  
**対象**: Simple TDC Game Project

---

## 概要

本ドキュメントは、キャラクター描画システムの統一設計を定義します。  
従来のメインキャラクター（セルアニメ形式）とサブキャラクター（スプライトシート形式）の二重管理を廃止し、すべてのキャラクターをスプライトシート形式で統一します。

---

## 1. 設計方針

### 1.1 統一描画フォーマット

| 項目 | 旧設計 | 新設計 |
|------|--------|--------|
| **メインキャラクター** | セルアニメ形式（個別PNG） | 256×256 スプライトシート |
| **サブキャラクター** | スプライトシート（可変サイズ） | 128×128以下 スプライトシート |
| **描画パイプライン** | 形式ごとに分岐 | 統一パイプライン |
| **アニメーション管理** | 形式依存 | アトラスベース統一 |

### 1.2 描画サイズ仕様

```
┌─────────────────────────────────────────────────┐
│ メインキャラクター: 256×256px                    │
│  - プレイヤーユニット                            │
│  - ボスキャラクター                              │
│  - 重要敵キャラクター                            │
│  - 高品質アニメーション（多フレーム）             │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│ サブキャラクター: 128×128px以下                  │
│  - 雑魚敵                                        │
│  - サポートキャラクター                          │
│  - エフェクトオブジェクト                        │
│  - シンプルアニメーション（少フレーム）           │
└─────────────────────────────────────────────────┘
```

### 1.3 レイヤー分離アーキテクチャ

```
┌──────────────────────────────────────────────────────────┐
│                  上位レイヤー (High-Level)                │
│  ┌────────────────────────────────────────────────────┐  │
│  │ CharacterDefinition (JSON定義)                     │  │
│  │  - アニメーションタグ                               │  │
│  │  - キャラクター属性                                 │  │
│  │  - スプライトパス                                   │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │ EntityFactory                                      │  │
│  │  - エンティティ生成                                 │  │
│  │  - コンポーネント初期化                             │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────┐
│                  中間レイヤー (Middle-Level)              │
│  ┌────────────────────────────────────────────────────┐  │
│  │ SpriteSheetAtlas (アトラスデータ)                   │  │
│  │  - フレーム位置・サイズ                             │  │
│  │  - アニメーションクリップ                           │  │
│  │  - メタデータ                                       │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │ ECS Components                                     │  │
│  │  - Sprite (テクスチャ参照)                          │  │
│  │  - Animation (実行時状態)                           │  │
│  │  - Transform (座標・スケール)                       │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────┐
│                  下位レイヤー (Low-Level)                 │
│  ┌────────────────────────────────────────────────────┐  │
│  │ RenderingSystem                                    │  │
│  │  - テクスチャキャッシュ                             │  │
│  │  - アトラスキャッシュ                               │  │
│  │  - 描画プリミティブ                                 │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │ Raylib Graphics API                                │  │
│  │  - LoadTexture()                                   │  │
│  │  - DrawTexturePro()                                │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

---

## 2. スプライトシート仕様

### 2.1 メインキャラクター（256×256）

**ファイル命名規則**:

```
assets/mainCharacters/{character_name}/{character_name}.png
assets/mainCharacters/{character_name}/{character_name}.json
```

**Aseprite エクスポート設定**:

```
サイズ: 256×256px (各フレーム)
形式: Array (横並び)
Padding: 0px
JSON形式: Array (with frames & tags)
```

**アニメーション推奨構成**:

```json
{
  "frames": [
    {
      "filename": "hero_idle_0",
      "frame": { "x": 0, "y": 0, "w": 256, "h": 256 },
      "duration": 100
    },
    // ... 省略
  ],
  "meta": {
    "frameTags": [
      {
        "name": "idle",
        "from": 0,
        "to": 7,
        "direction": "forward"
      },
      {
        "name": "walk",
        "from": 8,
        "to": 15,
        "direction": "forward"
      },
      {
        "name": "attack",
        "from": 16,
        "to": 23,
        "direction": "forward"
      },
      {
        "name": "death",
        "from": 24,
        "to": 31,
        "direction": "forward"
      }
    ]
  }
}
```

**推奨フレーム数**:

- `idle`: 8フレーム
- `walk`: 8フレーム
- `attack`: 8-12フレーム
- `death`: 8-16フレーム

### 2.2 サブキャラクター（128×128以下）

**ファイル命名規則**:

```
assets/subCharacters/{character_name}/{character_name}.png
assets/subCharacters/{character_name}/{character_name}.json
```

**Aseprite エクスポート設定**:

```
サイズ: 64×64px, 96×96px, 128×128px (キャラに応じて)
形式: Array (横並び)
Padding: 0px
JSON形式: Array (with frames & tags)
```

**推奨フレーム数**:

- `idle`: 4フレーム
- `walk`: 4-6フレーム
- `attack`: 4-6フレーム
- `death`: 4-6フレーム

---

## 3. データ構造定義

### 3.1 SpriteSheetAtlas（共通）

```cpp
// shared/include/Data/Loaders/SpriteSheetLoader.h

struct SpriteFrame {
    int x, y, w, h;                // テクスチャ上の位置・サイズ
    int sourceX, sourceY;          // 描画オフセット（トリミング用）
    int sourceW, sourceH;          // 元のキャンバスサイズ
    int durationMs;                // フレーム持続時間（ミリ秒）
    bool rotated;                  // 回転フラグ
    bool trimmed;                  // トリミングフラグ
};

struct SpriteAnimationClip {
    std::vector<int> frameIndices;  // フレームインデックス列
    bool loop;                      // ループフラグ
    std::string direction;          // "forward", "reverse", "pingpong"
    int fps;                        // フレームレート（デフォルト: 12）
};

struct SpriteSheetAtlas {
    std::string imagePath;                                   // テクスチャパス
    std::vector<SpriteFrame> frames;                         // 全フレーム
    std::unordered_map<std::string, SpriteAnimationClip> tags;  // アニメーションタグ
};
```

### 3.2 ECS Components（統一）

```cpp
// game/include/Game/Components/CoreComponents.h

struct Sprite {
    std::string texturePath;          // テクスチャファイルパス
    std::string atlasJsonPath;        // アトラスJSONパス
    const SpriteSheetAtlas* atlas;    // ロード済みアトラス参照
    Texture2D texture;                // Raylibテクスチャ
    bool loaded;                      // ロード済みフラグ
    bool failed;                      // ロード失敗フラグ
};

struct Animation {
    std::string currentAction;                               // 現在のアクション名（"idle", "walk"等）
    int atlasFrameIndex;                                     // 現在のフレームインデックス
    float atlasFrameTimer;                                   // フレームタイマー
    std::unordered_map<std::string, std::string> actionToJson;  // action → json path
    bool useAtlas;                                           // アトラス使用フラグ（常にtrue）
};

struct Transform {
    float x, y;                       // 座標
    float scaleX, scaleY;             // スケール
    float rotation;                   // 回転（度数法）
    bool flipH, flipV;                // 反転フラグ
};
```

### 3.3 EntityDefinition（JSON定義）

```cpp
// shared/include/Data/Definitions/EntityDef.h

struct EntityDef {
    std::string id;
    std::string name;
    std::string draw_type;  // 常に "sprite_sheet"
    
    struct Display {
        std::string atlas_texture;      // スプライトシートPNGパス
        std::unordered_map<std::string, std::string> sprite_actions;
        // 例: { "idle": "path/to/idle.json", "walk": "path/to/walk.json" }
        std::string icon;               // アイコン画像（UI用）
    } display;
    
    // ステータス、戦闘パラメータ等は省略
};
```

**JSON例**:

```json
{
  "id": "hero_warrior",
  "name": "戦士",
  "draw_type": "sprite_sheet",
  "display": {
    "atlas_texture": "assets/mainCharacters/Warrior/warrior.png",
    "sprite_actions": {
      "idle": "assets/mainCharacters/Warrior/warrior_idle.json",
      "walk": "assets/mainCharacters/Warrior/warrior_walk.json",
      "attack": "assets/mainCharacters/Warrior/warrior_attack.json",
      "death": "assets/mainCharacters/Warrior/warrior_death.json"
    },
    "icon": "assets/mainCharacters/Warrior/icon.png"
  }
}
```

---

## 4. 描画パイプライン

### 4.1 統一描画フロー

```
┌─────────────────────────────────────────────────────────┐
│ 1. エンティティ生成（EntityFactory）                     │
│    ├─ JSON定義ロード                                     │
│    ├─ Sprite コンポーネント作成                          │
│    │   ├─ texturePath = display.atlas_texture           │
│    │   └─ atlasJsonPath = sprite_actions["idle"]        │
│    ├─ Animation コンポーネント作成                       │
│    │   ├─ useAtlas = true                               │
│    │   ├─ currentAction = "idle"                        │
│    │   └─ actionToJson = sprite_actions                 │
│    └─ Transform コンポーネント作成                       │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ 2. リソースロード（RenderingSystem::LoadTextureIfNeeded)│
│    ├─ テクスチャキャッシュチェック                       │
│    ├─ LoadTexture() (Raylib)                           │
│    ├─ アトラスJSONロード                                 │
│    └─ キャッシュに保存                                   │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ 3. アニメーション更新（RenderingSystem::UpdateAnimation)│
│    ├─ 現在のアクション取得（anim.currentAction）         │
│    ├─ アトラスからクリップ取得（atlas->tags[action]）   │
│    ├─ フレーム進行（frameTimer += deltaTime）           │
│    ├─ フレームインデックス更新                           │
│    └─ ループ/終了処理                                    │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ 4. 描画（RenderingSystem::DrawEntities）                │
│    ├─ アトラスから現在フレーム取得                       │
│    │   frame = atlas->frames[frameIndices[frameIndex]]  │
│    ├─ ソース矩形作成                                     │
│    │   sourceRect = {frame.x, frame.y, frame.w, frame.h}│
│    ├─ 描画先矩形作成                                     │
│    │   destRect = {x + offsetX, y + offsetY, w, h}      │
│    └─ DrawTexturePro(texture, source, dest, origin, 0.0f)│
└─────────────────────────────────────────────────────────┘
```

### 4.2 描画システム実装例

```cpp
// game/src/Game/Systems/RenderingSystem.cpp

void RenderingSystem::UpdateAnimation(
    entt::registry& registry,
    float deltaTime
) {
    auto view = registry.view<Animation, Sprite>();
    
    for (auto entity : view) {
        auto& anim = view.get<Animation>(entity);
        auto& sprite = view.get<Sprite>(entity);
        
        if (!sprite.atlas || !anim.useAtlas) continue;
        
        // 現在のアクションに対応するクリップを取得
        auto it = sprite.atlas->tags.find(anim.currentAction);
        if (it == sprite.atlas->tags.end()) continue;
        
        const auto& clip = it->second;
        
        // フレームタイマー更新
        const auto& frame = sprite.atlas->frames[
            clip.frameIndices[anim.atlasFrameIndex]
        ];
        anim.atlasFrameTimer += deltaTime * 1000.0f;
        
        // フレーム切り替え判定
        if (anim.atlasFrameTimer >= frame.durationMs) {
            anim.atlasFrameTimer = 0.0f;
            anim.atlasFrameIndex++;
            
            // ループ処理
            if (anim.atlasFrameIndex >= clip.frameIndices.size()) {
                if (clip.loop) {
                    anim.atlasFrameIndex = 0;
                } else {
                    anim.atlasFrameIndex = clip.frameIndices.size() - 1;
                }
            }
        }
    }
}

void RenderingSystem::DrawEntities(
    entt::registry& registry,
    const Font& font
) {
    auto view = registry.view<Transform, Team, Stats, Sprite>();
    
    for (auto entity : view) {
        auto& transform = view.get<Transform>(entity);
        auto& sprite = view.get<Sprite>(entity);
        
        // テクスチャロード
        Texture2D* tex = LoadTextureIfNeeded(sprite);
        if (!tex) {
            // フォールバック: プレースホルダー矩形
            DrawFallbackRect(transform, view.get<Team>(entity));
            continue;
        }
        
        // アトラス描画
        if (sprite.atlas && registry.all_of<Animation>(entity)) {
            auto& anim = registry.get<Animation>(entity);
            DrawAtlasFrame(transform, sprite, anim, *tex);
        }
    }
}

void RenderingSystem::DrawAtlasFrame(
    const Transform& transform,
    const Sprite& sprite,
    const Animation& anim,
    Texture2D texture
) {
    auto it = sprite.atlas->tags.find(anim.currentAction);
    if (it == sprite.atlas->tags.end()) return;
    
    const auto& clip = it->second;
    const auto& frame = sprite.atlas->frames[
        clip.frameIndices[anim.atlasFrameIndex]
    ];
    
    // ソース矩形
    Rectangle source = {
        (float)frame.x,
        (float)frame.y,
        (float)frame.w,
        (float)frame.h
    };
    
    // 描画先矩形（スケール適用）
    Rectangle dest = {
        transform.x + frame.sourceX * transform.scaleX,
        transform.y + frame.sourceY * transform.scaleY,
        frame.w * transform.scaleX,
        frame.h * transform.scaleY
    };
    
    Vector2 origin = { 0, 0 };
    
    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
}
```

---

## 5. 柔軟性の確保

### 5.1 上位レイヤーでの変更容易性

**アニメーション切り替え**:

```cpp
// ゲームロジック層で簡単にアニメーション変更可能
void ChangeAnimation(entt::registry& registry, entt::entity entity, const std::string& action) {
    auto& anim = registry.get<Animation>(entity);
    if (anim.currentAction != action) {
        anim.currentAction = action;
        anim.atlasFrameIndex = 0;
        anim.atlasFrameTimer = 0.0f;
    }
}
```

**スケール変更**:

```cpp
// サブキャラを大きく見せたい場合
auto& transform = registry.get<Transform>(entity);
transform.scaleX = 1.5f;
transform.scaleY = 1.5f;
```

### 5.2 下位レイヤーの不変性

- **RenderingSystem**: 変更不要（常にアトラスベース描画）
- **テクスチャキャッシュ**: パス基準で動作（サイズ非依存）
- **アトラスローダー**: JSON形式さえ守れば動作

### 5.3 中間レイヤーの拡張性

**カスタムアニメーション再生**:

```cpp
struct CustomPlayback {
    int startFrame;
    int endFrame;
    int currentFrame;
    bool pingPong;
};

// ECSコンポーネントとして追加可能
```

**レイヤー合成**:

```cpp
struct LayeredSprite {
    std::vector<Sprite> layers;  // 複数レイヤー対応
    // 例: ボディ + 武器 + エフェクトを別々に管理
};
```

---

## 6. 移行計画

### 6.1 既存アセットの変換

**ステップ1: メインキャラクター変換**

```bash
# Asepriteスクリプトで一括エクスポート
aseprite --batch assets/mainCharacters/**/*.aseprite \
  --sheet {fullname}.png \
  --data {fullname}.json \
  --format json-array \
  --list-tags
```

**ステップ2: サブキャラクター変換**

```bash
# サイズを128×128以下に調整
aseprite --batch assets/subCharacters/**/*.aseprite \
  --sheet {fullname}.png \
  --data {fullname}.json \
  --format json-array \
  --list-tags
```

### 6.2 コード変更

1. **Animation コンポーネントの簡略化**
   - 旧形式フィールド削除（`columns`, `rows`, `frames_per_state`）
   - `useAtlas` を常にtrueに固定

2. **EntityFactory の更新**
   - グリッド形式の分岐削除
   - すべてアトラス形式で初期化

3. **RenderingSystem の簡略化**
   - グリッド描画コードの削除
   - アトラス描画に一本化

### 6.3 検証

```cpp
// テストケース
TEST_CASE("Unified Sprite Rendering") {
    // メインキャラクター（256×256）が正しく描画される
    auto mainChar = CreateEntity("hero_warrior");
    REQUIRE(mainChar.sprite.texture.width == 256);
    
    // サブキャラクター（128×128）が正しく描画される
    auto subChar = CreateEntity("slime_enemy");
    REQUIRE(subChar.sprite.texture.width == 128);
    
    // 両方とも同じ描画パイプラインを使用
    REQUIRE(mainChar.animation.useAtlas == true);
    REQUIRE(subChar.animation.useAtlas == true);
}
```

---

## 7. パフォーマンス考慮事項

### 7.1 テクスチャメモリ

**最大同時表示想定**:

```
メインキャラ: 10体 × (256×256×4) = 2.5MB
サブキャラ: 50体 × (128×128×4) = 3.2MB
合計: 約6MB（GPU VRAM）
```

**最適化手法**:

- テクスチャアトラス化（複数キャラを1枚に結合）
- 画面外カリング（描画スキップ）
- LOD（距離に応じてサブキャラをさらに縮小）

### 7.2 描画コール削減

**バッチング**:

```cpp
// 同じテクスチャを使用するエンティティをまとめて描画
std::map<Texture2D*, std::vector<entt::entity>> batches;
for (auto entity : view) {
    auto& sprite = view.get<Sprite>(entity);
    batches[&sprite.texture].push_back(entity);
}

for (auto& [texture, entities] : batches) {
    BeginTextureMode(*texture);
    for (auto entity : entities) {
        DrawEntity(entity);
    }
    EndTextureMode();
}
```

---

## 8. まとめ

### 8.1 利点

| 項目 | 効果 |
|------|------|
| **コード簡略化** | グリッド/アトラス分岐削除で保守性向上 |
| **アセット統一** | すべてAsepriteで管理可能 |
| **パイプライン安定** | 下位レイヤー変更不要 |
| **拡張性** | 上位レイヤーで柔軟なカスタマイズ |

### 8.2 今後の拡張

- **パーツアニメーション**: レイヤー合成で装備変更対応
- **シェーダー対応**: カラー変更、アウトライン、発光効果
- **3Dスプライト**: Z軸回転、深度ソート
- **エフェクト統合**: アタックエフェクトもスプライトシートで管理

---

**文責**: GitHub Copilot  
**承認**: プロジェクトリード  
**次回更新予定**: 実装完了後（フィードバック反映）
