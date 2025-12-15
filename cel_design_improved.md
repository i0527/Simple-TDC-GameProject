# Raylib セルアニメーション基盤 設計書 - Simple-TDC-GameProject統合版

## 概要

Simple-TDC-GameProject のアーキテクチャに統合したセルアニメーション基盤です。  
ECS（EnTT）とデータドリブン設計に基づき、ホットリロード対応・スキーマ検証・Shared Layer統合を実現します。

---

## 全体方針

| 項目 | 仕様 |
|------|------|
| **表現方式** | カットアウトアニメ（分割パーツ＋親子ボーン＋平行移動/回転/拡縮） |
| **Root パーツ** | `torso`（胴体） |
| **最大パーツ数** | 16個（固定 12 + 自由 4） |
| **同時セルアニメキャラ上限** | 6体（ECS の Entity 数に準じる） |
| **描画方式** | 共通アトラス 1 枚 → `DrawTexturePro` で描画 |
| **データ形式** | JSON（Shared Layer 統合） |
| **ホットリロード** | ✅ FileWatcher対応 |
| **スキーマ検証** | ✅ JSON Schema対応 |
| **ECS統合** | ✅ CelAnimationComponent |

---

## アーキテクチャ統合

### 既存フレームワークの活用

```
Shared Layer
├── Core/
│   ├── GameContext          ← テクスチャ・フォント管理
│   ├── EventSystem          ← アニメーション終了イベント
│   └── FileWatcher          ← JSON 定義ホットリロード
├── Data/
│   ├── DefinitionRegistry   ← キャッシュ・クエリ
│   ├── Loaders/
│   │   └── CelAnimationLoader (新規)
│   └── Validators/
│       └── CelAnimationValidator (新規)

Game Layer
└── Components/
    ├── CelAnimationComponent   ← Entity へ付加
    ├── AnimationStateComponent ← 再生状態（現在時刻など）
    └── TransformComponent      ← 既存、ワールド座標
```

### ECS コンポーネント定義

**CelAnimationComponent**

```cpp
// game/include/components/CelAnimationComponent.h
namespace SimpleTDC::Game {

struct CelAnimationComponent {
    std::string characterId;        // "player", "enemy_1" など
    std::string currentClipName;    // 再生中のアニメ名
    float currentTime = 0.0f;       // アニメーション内の時刻 (秒)
    bool isPlaying = true;
    bool isLooping = true;          // ループするか
    float speed = 1.0f;             // 再生速度（1.0 = 通常速度）
    
    // アニメーション終了時のコールバック
    std::optional<std::function<void()>> onClipEnd;
};

}
```

**AnimationStateComponent**

```cpp
struct AnimationStateComponent {
    std::string prevClipName;       // 遷移前のアニメーション
    float transitionTime = 0.0f;    // フェード時間（秒、0 = 即時）
    float elapsedTransitionTime = 0.0f;
};
```

---

## パーツ構成と命名規則

### 共通 12 パーツ（固定）

| パーツ名 | 説明 | 親 |
|---------|------|-----|
| `head` | 頭（顔＋前髪のベース） | torso |
| `hair` | 前髪・横髪など追加分 | head |
| `hairBack` | 後ろ髪 | head |
| `torso` | 胴体（首〜腰まで）**[root]** | null |
| `armL` | 左上腕 | torso |
| `armL2` | 左前腕＋手 | armL |
| `armR` | 右上腕 | torso |
| `armR2` | 右前腕＋手 | armR |
| `legL` | 左太もも | torso |
| `legL2` | 左すね＋足 | legL |
| `legR` | 右太もも | torso |
| `legR2` | 右すね＋足 | legR |

### 自由 4 パーツ（キャラごとに選択）

| パーツ名 | 説明 | 推奨親 |
|---------|------|--------|
| `weapon` | 武器・手に持つ物 | armR2 or armL2 |
| `skirt` | スカート全体・腰のフリル | torso |
| `tail` | 尻尾 | torso |
| `accessory` | マント・リボン・エフェクト等 | torso or head |

**命名規則**: 自由枠が足りない場合は `weapon2`, `cape`, `backpack` など拡張可（ただし統一性を保つ）

---

## 親子関係（ボーン構造）

```
torso (root)
 ├─ head
 │   ├─ hair
 │   └─ hairBack
 ├─ armL
 │   └─ armL2
 │       └─ weapon (オプション：手に装備する武器)
 ├─ armR
 │   └─ armR2
 ├─ legL
 │   └─ legL2
 ├─ legR
 │   └─ legR2
 ├─ skirt (オプション)
 ├─ tail (オプション)
 └─ accessory (オプション)
```

---

## データ構造（JSON形式）

### ファイル配置

```
assets/
├── definitions/
│   └── characters/
│       ├── characters_parts.json       (パーツ定義＋リグ)
│       ├── characters_animations.json  (アニメーションクリップ)
│       ├── character_player_debug.json (キャラクター定義・参照用)
│       └── schemas/
│           ├── celanimation_parts.schema.json
│           ├── celanimation_clips.schema.json
│           └── celanimation_character.schema.json
├── textures/
│   └── characters_atlas.png            (1024×1024以上推奨)
└── ...
```

### 1. パーツ定義ファイル（characters_parts.json）

```jsonc
{
  "version": "1.0",
  "texture": "assets/textures/characters_atlas.png",
  "parts": [
    {
      "name": "torso",
      "src": {
        "x": 0,
        "y": 0,
        "w": 128,
        "h": 128
      },
      "pivot": {
        "x": 64,
        "y": 80
      },
      "parent": null,
      "bindOffset": {
        "x": 0,
        "y": 0
      },
      "zIndex": 5  // 描画順序（低いほど奥）
    },
    {
      "name": "head",
      "src": {
        "x": 128,
        "y": 0,
        "w": 96,
        "h": 96
      },
      "pivot": {
        "x": 48,
        "y": 80
      },
      "parent": "torso",
      "bindOffset": {
        "x": 0,
        "y": -40
      },
      "zIndex": 6
    },
    {
      "name": "weapon",
      "src": {
        "x": 500,
        "y": 200,
        "w": 50,
        "h": 80
      },
      "pivot": {
        "x": 25,
        "y": 10
      },
      "parent": "armR2",
      "bindOffset": {
        "x": 10,
        "y": 0
      },
      "zIndex": 7
    }
    // ... 他のパーツ
  ]
}
```

**フィールド説明**:

- `version`: データフォーマットバージョン
- `texture`: アトラステクスチャの相対パス（GameContext 経由でロード）
- `src`: アトラス内の矩形 (x, y, 幅, 高さ)
- `pivot`: パーツ画像内の回転・スケール原点（ピクセル単位）
- `parent`: 親パーツ名（null = root）
- `bindOffset`: テンプレート骨格との座標差分（リグ調整で更新）
- `zIndex`: 同じフレーム内での描画順序（オプション、デフォルト = パーツ定義順）

### 2. アニメーション定義ファイル（characters_animations.json）

```jsonc
{
  "version": "1.0",
  "clips": [
    {
      "name": "idle",
      "length": 0.6,
      "loop": true,
      "tracks": [
        {
          "part": "torso",
          "keys": [
            {
              "t": 0.0,
              "pos": { "x": 0, "y": 0 },
              "rot": 0.0,
              "scale": 1.0
            },
            {
              "t": 0.3,
              "pos": { "x": 0, "y": -3 },
              "rot": 0.0,
              "scale": 1.0
            },
            {
              "t": 0.6,
              "pos": { "x": 0, "y": 0 },
              "rot": 0.0,
              "scale": 1.0
            }
          ]
        },
        {
          "part": "hairBack",
          "keys": [
            { "t": 0.0, "rot": -4.0 },
            { "t": 0.3, "rot": 4.0 },
            { "t": 0.6, "rot": -4.0 }
          ]
        },
        {
          "part": "tail",
          "keys": [
            { "t": 0.0, "rot": 0.0 },
            { "t": 0.15, "rot": -8.0 },
            { "t": 0.3, "rot": 0.0 },
            { "t": 0.45, "rot": 8.0 },
            { "t": 0.6, "rot": 0.0 }
          ]
        }
      ],
      "events": [
        {
          "name": "idle_end",
          "time": 0.6
        }
      ]
    },
    {
      "name": "walk",
      "length": 0.5,
      "loop": true,
      "tracks": [
        {
          "part": "torso",
          "keys": [
            { "t": 0.0, "pos": { "x": 0, "y": 0 }, "rot": 0.0, "scale": 1.0 },
            { "t": 0.25, "pos": { "x": 0, "y": -4 }, "rot": 0.0, "scale": 1.0 },
            { "t": 0.5, "pos": { "x": 0, "y": 0 }, "rot": 0.0, "scale": 1.0 }
          ]
        },
        {
          "part": "legL",
          "keys": [
            { "t": 0.0, "rot": 0.0 },
            { "t": 0.25, "rot": 25.0 },
            { "t": 0.5, "rot": 0.0 }
          ]
        },
        {
          "part": "legR",
          "keys": [
            { "t": 0.0, "rot": -25.0 },
            { "t": 0.25, "rot": 0.0 },
            { "t": 0.5, "rot": -25.0 }
          ]
        }
      ]
    },
    {
      "name": "attack",
      "length": 0.35,
      "loop": false,
      "tracks": [
        {
          "part": "torso",
          "keys": [
            { "t": 0.0, "pos": { "x": 0, "y": 0 }, "rot": -10.0, "scale": 1.0 },
            { "t": 0.15, "pos": { "x": 5, "y": -5 }, "rot": 15.0, "scale": 1.0 },
            { "t": 0.35, "pos": { "x": 0, "y": 0 }, "rot": 0.0, "scale": 1.0 }
          ]
        },
        {
          "part": "armR",
          "keys": [
            { "t": 0.0, "rot": 0.0 },
            { "t": 0.15, "rot": 90.0 },
            { "t": 0.35, "rot": 0.0 }
          ]
        },
        {
          "part": "weapon",
          "keys": [
            { "t": 0.0, "rot": 0.0 },
            { "t": 0.15, "rot": 90.0 },
            { "t": 0.35, "rot": 0.0 }
          ]
        }
      ],
      "events": [
        {
          "name": "attack_hit",
          "time": 0.2,
          "metadata": {
            "damage": 10,
            "knockback": 50
          }
        },
        {
          "name": "attack_end",
          "time": 0.35
        }
      ]
    },
    {
      "name": "hit",
      "length": 0.2,
      "loop": false,
      "tracks": [
        {
          "part": "torso",
          "keys": [
            { "t": 0.0, "pos": { "x": -10, "y": 0 }, "rot": -15.0, "scale": 1.0 },
            { "t": 0.1, "pos": { "x": -5, "y": 0 }, "rot": -8.0, "scale": 1.0 },
            { "t": 0.2, "pos": { "x": 0, "y": 0 }, "rot": 0.0, "scale": 1.0 }
          ]
        }
      ]
    },
    {
      "name": "death",
      "length": 0.8,
      "loop": false,
      "tracks": [
        {
          "part": "torso",
          "keys": [
            { "t": 0.0, "pos": { "x": 0, "y": 0 }, "rot": 0.0, "scale": 1.0 },
            { "t": 0.4, "pos": { "x": 0, "y": 20 }, "rot": 90.0, "scale": 0.8 },
            { "t": 0.8, "pos": { "x": 0, "y": 20 }, "rot": 90.0, "scale": 0.6 }
          ]
        }
      ],
      "events": [
        {
          "name": "death_end",
          "time": 0.8
        }
      ]
    }
  ]
}
```

**フィールド説明**:

- `clips[].name`: アニメーション名
- `clips[].length`: アニメーション総長（秒）
- `clips[].loop`: ループするか
- `clips[].tracks`: パーツごとのトラック
  - `tracks[].part`: パーツ名
  - `tracks[].keys`: キーフレーム配列
    - `t`: キーの時刻（秒）
    - `pos`: ローカル平行移動（x, y ピクセル、オプション）
    - `rot`: ローカル回転（度数法、時計回りが正、オプション）
    - `scale`: ローカルスケール（1.0 = 等倍、オプション）
    - `alpha`: アルファ値（0.0〜1.0、オプション）
- `clips[].events`: アニメーション内のイベント（攻撃判定・効果音など）
  - `time`: イベント発火時刻
  - `metadata`: ゲーム側で処理するデータ

### 3. キャラクター定義ファイル（character_player_debug.json）

ゲーム側がキャラを参照する際のメタデータ：

```jsonc
{
  "id": "player",
  "name": "プレイヤー",
  "characterPartsDefinition": "characters_parts.json",
  "animationClips": "characters_animations.json",
  "atlas": "assets/textures/characters_atlas.png",
  "defaultAnimation": "idle",
  "animationSpeed": 1.0,
  "defaultScale": 2.0,
  "canFaceLeft": true,
  "parts": [
    "torso", "head", "hair", "hairBack",
    "armL", "armL2", "armR", "armR2",
    "legL", "legL2", "legR", "legR2",
    "weapon", "skirt", "tail", "accessory"
  ]
}
```

---

## ランタイム計算フロー

### フレーム更新処理（簡略化版）

```cpp
// game/src/systems/CelAnimationSystem.cpp

namespace SimpleTDC::Game {

class CelAnimationSystem {
public:
    void Update(entt::registry& registry, float dt) {
        auto view = registry.view<CelAnimationComponent, AnimationStateComponent>();
        
        for (auto entity : view) {
            auto& animComp = view.get<CelAnimationComponent>(entity);
            auto& stateComp = view.get<AnimationStateComponent>(entity);
            
            // 1. 現在のアニメーションクリップを取得
            auto* clip = GetAnimationClip(animComp.characterId, animComp.currentClipName);
            if (!clip) continue;
            
            // 2. 時刻を進める
            if (animComp.isPlaying) {
                animComp.currentTime += dt * animComp.speed;
                
                // ループ判定
                if (animComp.currentTime >= clip->length) {
                    if (animComp.isLooping) {
                        animComp.currentTime = std::fmod(animComp.currentTime, clip->length);
                    } else {
                        animComp.currentTime = clip->length;
                        animComp.isPlaying = false;
                        
                        // アニメーション終了イベント
                        if (animComp.onClipEnd) {
                            animComp.onClipEnd();
                        }
                    }
                }
                
                // イベント発火チェック
                FireAnimationEvents(clip, animComp.currentTime);
            }
        }
    }

private:
    AnimationClip* GetAnimationClip(const std::string& charId, const std::string& clipName) {
        // DefinitionRegistry から取得（キャッシュ済み）
        return registry->Get<AnimationClip>(charId + "/" + clipName);
    }
    
    void FireAnimationEvents(const AnimationClip* clip, float currentTime) {
        // イベント時刻にヒットしたら EventSystem に publish
        // → Skill/Attack系システムが subscribe して処理
    }
};

}
```

### 描画処理（RaylibIntegration）

```cpp
// game/src/systems/CelRenderSystem.cpp

void CelRenderSystem::Render(entt::registry& registry, GameContext* context) {
    auto view = registry.view<CelAnimationComponent, TransformComponent>();
    
    for (auto entity : view) {
        auto& animComp = view.get<CelAnimationComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        
        DrawCelCharacter(
            context,
            animComp.characterId,
            animComp.currentClipName,
            animComp.currentTime,
            transform.position,
            transform.rotation,
            animComp.speed
        );
    }
}

void CelRenderSystem::DrawCelCharacter(
    GameContext* context,
    const std::string& characterId,
    const std::string& clipName,
    float animTime,
    Vector2 worldPos,
    float worldRot,
    float playSpeed
) {
    // 1. パーツ定義とアニメーションクリップを取得
    auto* partsData = context->definitionRegistry->Get<CelPartsDefinition>(characterId);
    auto* clipData = context->definitionRegistry->Get<AnimationClip>(characterId + "/" + clipName);
    Texture2D atlas = context->resourceManager->GetTexture(partsData->texturePath);
    
    // 2. ボーン世界座標を計算（キャッシュ）
    std::unordered_map<std::string, BoneTransform> boneWorldTransforms;
    for (const auto& part : partsData->parts) {
        Vector2 localPos = SampleTrackVec2(clipData, part.name, "pos", animTime);
        float localRot = SampleTrackFloat(clipData, part.name, "rot", animTime);
        float localScale = SampleTrackFloat(clipData, part.name, "scale", animTime, 1.0f);
        
        // bindOffset を加える
        localPos = Vector2Add(localPos, part.bindOffset);
        
        // 親の world 行列を取得
        BoneTransform boneTransform;
        if (part.parent) {
            const auto& parentTransform = boneWorldTransforms[part.parent];
            Vector2 rotatedPos = RotateVector(localPos, parentTransform.rot);
            boneTransform.pos = Vector2Add(parentTransform.pos, rotatedPos);
            boneTransform.rot = parentTransform.rot + localRot;
            boneTransform.scale = localScale;  // スケールは子で独立
        } else {
            // root パーツ（torso）
            boneTransform.pos = Vector2Add(worldPos, localPos);
            boneTransform.rot = worldRot + localRot;
            boneTransform.scale = localScale;
        }
        
        boneWorldTransforms[part.name] = boneTransform;
    }
    
    // 3. zIndex でソートして描画
    std::vector<const CelPart*> sortedParts(partsData->parts);
    std::sort(sortedParts.begin(), sortedParts.end(), 
        [](const CelPart* a, const CelPart* b) { return a->zIndex < b->zIndex; });
    
    for (const auto& part : sortedParts) {
        const auto& boneTransform = boneWorldTransforms[part->name];
        
        Rectangle sourceRec = {
            (float)part->src.x, (float)part->src.y,
            (float)part->src.w, (float)part->src.h
        };
        Rectangle destRec = {
            boneTransform.pos.x, boneTransform.pos.y,
            part->src.w * boneTransform.scale,
            part->src.h * boneTransform.scale
        };
        Vector2 origin = { (float)part->pivot.x, (float)part->pivot.y };
        
        DrawTexturePro(
            atlas,
            sourceRec,
            destRec,
            origin,
            boneTransform.rot,
            WHITE
        );
    }
}
```

---

## Shared Layer統合

### CelAnimationLoader

```cpp
// shared/include/data/loaders/CelAnimationLoader.h

namespace SimpleTDC::Shared {

class CelAnimationLoader : public IDefinitionLoader {
public:
    bool Load(const std::string& path, nlohmann::json& outJson) override;
    std::string GetDefinitionType() const override { return "celanimation"; }
};

}
```

### CelAnimationValidator

```cpp
// shared/include/data/validators/CelAnimationValidator.h

class CelAnimationValidator : public IDefinitionValidator {
public:
    ValidationResult Validate(const nlohmann::json& definition) override;
    std::string GetSchemaPath() const override {
        return "assets/definitions/characters/schemas/celanimation_parts.schema.json";
    }
};
```

---

## ホットリロード対応

FileWatcher が `assets/definitions/characters/` の変更を検出すると：

```cpp
// shared/src/core/FileWatcher.cpp

void FileWatcher::OnFileChanged(const std::string& path) {
    if (path.find("celanimation") != std::string::npos) {
        // DefinitionRegistry のキャッシュをクリア
        registry->Reload(path);
        
        // イベント発火
        eventSystem->Publish<CelAnimationReloadedEvent>();
    }
}
```

---

## アニメーション種別と設計方針

### idle（待機）
- **長さ**: 0.6 秒ループ
- **動き**: torso のバウンス、tail/hair の軽い振動
- **用途**: デフォルト待機

### walk（歩行）
- **長さ**: 0.4〜0.6 秒ループ
- **動き**: 脚の交互回転、skirt の遅延振動
- **用途**: 移動中

### attack（攻撃）
- **長さ**: 0.3〜0.5 秒（ループなし）
- **イベント**: `attack_hit` イベント（t=0.2）で攻撃判定発火
- **用途**: 敵への攻撃アニメ

### hit（被弾）
- **長さ**: 0.2 秒（ループなし）
- **動き**: トルソー後退＋回転
- **用途**: ダメージ反応

### death（死亡）
- **長さ**: 0.8 秒（ループなし）
- **イベント**: `death_end` イベント（t=0.8）で消滅処理
- **用途**: キャラ消滅演出

---

## アセット制作パイプライン

### Step 1: Clip Studio でパーツ分割・描画

1. 256×256 キャンバスでキャラを描画
2. レイヤーをパーツ名で分割：
   - `head`, `torso`, `armL`, `armL2`, ...
   - レイヤー名 = パーツ名

### Step 2: PNG 書き出し

Clip Studio から「素材を書き出し」で各パーツを個別PNG化

### Step 3: Aseprite でアトラス化

1. Aseprite で新規 1024×1024 ファイル作成
2. 各パーツPNGを配置＋Sliceで原点設定
3. CLIで書き出し：

```bash
aseprite -b characters.aseprite \
  --sheet characters_atlas.png \
  --data characters_atlas.json \
  --format json-array
```

### Step 4: JSON 処理

生成された `characters_atlas.json` から：
- `src` (x, y, w, h) を抽出
- `pivot` を設定
- `characters_parts.json` を生成

---

## ゲーム内リグ調整ツール（推奨）

SimpleTDCEditor で実装：

```cpp
// editor/src/panels/CelAnimationEditor.cpp

class CelAnimationEditorPanel {
public:
    void Render();
    void OnPartSelected(const std::string& partName);
    void OnPivotDragged(Vector2 delta);
    void OnBindOffsetDragged(Vector2 delta);
    void SaveAdjustments();
    
private:
    std::string selectedPart;
    AnimationClip currentClip;
    // ...
};
```

---

## パフォーマンス最適化

### バッチング効果

- 1キャラ = 最大 16 DrawTexturePro 呼び出し
- 6体 = 96 呼び出し（誤差レベル）
- テクスチャ切り替えなし（アトラス利用）

### キャッシング戦略

```cpp
// DefinitionRegistry でキャッシュ
registry->Get<CelPartsDefinition>("player");
registry->Get<AnimationClip>("player/attack");
```

---

## ディレクトリ構成例

```
Simple-TDC-GameProject/
├── shared/
│   ├── include/
│   │   └── data/
│   │       ├── loaders/
│   │       │   └── CelAnimationLoader.h
│   │       └── validators/
│   │           └── CelAnimationValidator.h
│   └── src/
│       └── data/
│           ├── loaders/
│           │   └── CelAnimationLoader.cpp
│           └── validators/
│               └── CelAnimationValidator.cpp
├── game/
│   ├── include/
│   │   ├── components/
│   │   │   ├── CelAnimationComponent.h
│   │   │   └── AnimationStateComponent.h
│   │   └── systems/
│   │       ├── CelAnimationSystem.h
│   │       └── CelRenderSystem.h
│   └── src/
│       └── systems/
│           ├── CelAnimationSystem.cpp
│           └── CelRenderSystem.cpp
├── editor/
│   ├── include/
│   │   └── panels/
│   │       └── CelAnimationEditorPanel.h
│   └── src/
│       └── panels/
│           └── CelAnimationEditorPanel.cpp
└── assets/
    ├── definitions/
    │   └── characters/
    │       ├── characters_parts.json
    │       ├── characters_animations.json
    │       ├── character_*.json
    │       └── schemas/
    │           ├── celanimation_parts.schema.json
    │           ├── celanimation_clips.schema.json
    │           └── celanimation_character.schema.json
    ├── textures/
    │   └── characters_atlas.png
    └── ...
```

---

## 実装チェックリスト

### Phase 1: Shared Layer統合
- [ ] `CelAnimationLoader` 実装
- [ ] `CelAnimationValidator` + Schema 作成
- [ ] `DefinitionRegistry` にキャッシュロジック追加

### Phase 2: Game Layer実装
- [ ] `CelAnimationComponent`, `AnimationStateComponent` 定義
- [ ] `CelAnimationSystem` 実装（更新・イベント）
- [ ] `CelRenderSystem` 実装（Raylib描画統合）

### Phase 3: エディタ統合
- [ ] `CelAnimationEditorPanel` 実装（ImGui UI）
- [ ] リグ調整機能（Pivot / BindOffset ドラッグ）

### Phase 4: テスト＆最適化
- [ ] 複数キャラの同時描画テスト
- [ ] ホットリロード動作確認
- [ ] パフォーマンス測定

---

## 今後の拡張予定

| 機能 | 優先度 | 説明 |
|------|--------|------|
| IK（逆運動学） | 低 | 脚・腕の自動曲げ |
| Ease-in/out 補間 | 中 | より自然なアニメーション |
| メッシュ変形 | 低 | ボーンに基づく頂点変形 |
| Sprite Baking | 低 | 最適化用：アニメをスプライトシートにベイク |
| Audio Sync | 中 | 音声タイミングとの同期 |
| State Machine | 中 | アニメーション状態遷移管理 |

---

## 参考資料

- [EnTT Documentation](https://skypjack.github.io/entt/)
- [Raylib DrawTexturePro](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [JSON Schema Specification](https://json-schema.org/)
- カットアウトアニメーション（セルアニメ）一般的な手法

---

**Version**: 1.0 (Simple-TDC-GameProject統合版)  
**Last Updated**: 2025-12-15  
**Maintainer**: SimpleTDC Development Team
