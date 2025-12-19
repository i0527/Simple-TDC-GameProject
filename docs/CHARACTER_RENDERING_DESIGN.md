# キャラクター描画システム設計書

**バージョン**: 3.0（段階移行対応・最適化版）  
**最終更新**: 2025-12-18  
**対象**: Simple TDC Game Project  
**状態**: 🟡 実装中（GridSheetProvider → AsepriteJsonAtlasProvider → TexturePackerAtlasProvider への移行対応）

---

## 概要

本ドキュメントは、raylib上の2Dスプライトアニメーション管理の統一設計を定義します。

**設計方針**:

- **制作初期** → 「均一グリッド（JSON無し・256固定セル）」で最短実装
- **最適化段階** → 「Packed（trim + JSON）」「統合アトラス」へ段階移行
- **重要**: ゲームロジック側のコードは**変わらない**（Provider抽象化による吸収）

すべてのキャラクターをスプライトシート形式で統一し、**段階的な最適化を可能にします**。

---

## 1. 設計方針

### 1.1 統一描画フォーマット

| 項目 | 仕様 |
|------|------|
| **メインキャラクター** | 256×256 スプライトシート（グリッド → Packed → 統合アトラス） |
| **サブキャラクター** | 128×128以下 スプライトシート（同様） |
| **描画基準** | DrawTexturePro 統一（origin 基準点） |
| **アニメーション管理** | FrameRef + IFrameProvider 抽象化 |
| **足元揃え** | offset + origin 補正による安定化 |

### 1.2 段階移行プラン

```
Phase 1: GridSheetProvider（最短実装）
├─ 256固定セル、JSON無し
├─ 自前の clips.json 定義（start/length/fps）
├─ 足元揃え = 画像レイアウトルール
└─ 期間: 〜1週間

     ↓ テクスチャサイズ最適化の要求

Phase 2: AsepriteJsonAtlasProvider（Packed対応）
├─ Asepriteアトラス形式（json-array）
├─ trim/offset補正で位置ズレ解決
├─ 足元揃え = offset + origin調整
└─ 期間: 〜2週間

     ↓ 大量エフェクト・共存化

Phase 3: TexturePackerAtlasProvider（統合アトラス）
├─ 複数キャラを1テクスチャに統合
├─ テクスチャ切替削減 → 描画効率向上
└─ ゲームロジック不変（Provider差し替えのみ）
```

---

## 2. データモデル（中核）

### 2.1 FrameRef（描画の最小単位）

```cpp
// shared/include/Data/Graphics/FrameRef.h
struct FrameRef {
    Texture2D* tex;              // 使用するテクスチャ（raylib型）
    Rectangle src;               // テクスチャから切り出す矩形（sourceRect）
    Vector2 origin;              // DrawTexturePro の基準点（回転・足元基準）
    Vector2 offset;              // Packed/trim時の描画位置補正
    float durationSec;           // フレーム表示時間（秒）
    bool valid;                  // 有効フラグ
};
```

**設計の要点**:

- `tex` は `DrawTexturePro(*tex, src, dest, origin, rotation, tint)` へ直接投入可能
- `offset/origin` は「Grid期は(0,0)でOKだが、Packed移行で必須」の設計
- `durationSec` は各フレームの個別タイミング対応（Asepriteの個別duration対応）

### 2.2 AnimClip（アニメーション1本）

```cpp
struct AnimClip {
    std::string name;                        // "idle", "walk", "attack", "death"
    std::vector<FrameRef> frames;            // フレーム列
    bool loop;                               // ループフラグ
    float defaultFps;                        // デフォルトFPS
};
```

### 2.3 SpriteSet（1キャラ or 1アトラス単位）

```cpp
struct SpriteSet {
    std::unordered_map<std::string, AnimClip> clips;  // "idle" → AnimClip
    std::string debugName;                            // デバッグ用キャラ名
};
```

---

## 3. 抽象化レイヤ（IFrameProvider）

### 3.1 IFrameProvider インターフェース

```cpp
class IFrameProvider {
public:
    virtual ~IFrameProvider() = default;
    
    // クリップ存在確認
    virtual bool HasClip(const std::string& clipName) const = 0;
    
    // クリップのフレーム数
    virtual int GetFrameCount(const std::string& clipName) const = 0;
    
    // 指定クリップ・フレームインデックスからFrameRef取得
    virtual FrameRef GetFrame(const std::string& clipName, int frameIndex) const = 0;
    
    // クリップのデフォルトFPS
    virtual float GetClipFps(const std::string& clipName) const = 0;
    
    // クリップのループフラグ
    virtual bool IsLooping(const std::string& clipName) const = 0;
};
```

**利点**:

- ゲームコードは「state=walk、frameIndex=...」を渡すだけ
- 下位が「グリッド計算」「JSON参照」「Packed補正」を吸収
- Provider実装を差し替えるだけで段階移行可能
- **ゲームロジック変更なし**

---

## 4. Provider実装（段階移行対応）

### 4.1 GridSheetProvider（Phase 1: 最短実装）

```cpp
class GridSheetProvider : public IFrameProvider {
public:
    struct Config {
        int cellWidth;               // 例: 256
        int cellHeight;              // 例: 256
        int framesPerRow;            // 例: 16
    };
    
    GridSheetProvider(Texture2D texture, const Config& config);
    
    void RegisterClip(const std::string& name, int startIndex, int length, 
                     bool loop, float fps);
    
    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    float GetClipFps(const std::string& clipName) const override;
    bool IsLooping(const std::string& clipName) const override;

private:
    Texture2D texture_;
    Config config_;
    
    struct ClipDef {
        int startIndex;
        int length;
        bool loop;
        float fps;
    };
    std::unordered_map<std::string, ClipDef> clips_;
    
    Vector2 GetFootOrigin() const;  // 足元基準点計算
};
```

**使用例**:

```cpp
GridSheetProvider::Config cfg{256, 256, 16};  // 256セル、1行16フレーム
Texture2D tex = LoadTexture("assets/mainCharacters/Warrior/warrior.png");
GridSheetProvider provider(tex, cfg);

provider.RegisterClip("idle",   0,  8, true,  12.0f);
provider.RegisterClip("walk",   8,  8, true,  12.0f);
provider.RegisterClip("attack", 16, 12, false, 15.0f);
provider.RegisterClip("death",  28, 8, false,  10.0f);
```

**メリット**:

- 実装が単純、JSON依存なし
- 動作確認が容易
- テクスチャサイズ計算が明確

---

### 4.2 AsepriteJsonAtlasProvider（Phase 2: Packed対応）

```cpp
class AsepriteJsonAtlasProvider : public IFrameProvider {
public:
    AsepriteJsonAtlasProvider(Texture2D texture, const nlohmann::json& atlasJson);
    
    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    float GetClipFps(const std::string& clipName) const override;
    bool IsLooping(const std::string& clipName) const override;

private:
    Texture2D texture_;
    SpriteSet spriteSet_;
    float footOffsetY_;    // 足元オフセット
    
    Vector2 GetFootOrigin(const SpriteFrame& frame) const;
    Vector2 GetTrimOffset(const SpriteFrame& frame) const;
};
```

**入力（Aseprite JSON Array形式）**:

```json
{
  "frames": [
    {
      "filename": "idle_0",
      "frame": { "x": 0, "y": 0, "w": 256, "h": 256 },
      "trimmed": true,
      "spriteSourceSize": { "x": 10, "y": 20 },
      "sourceSize": { "w": 256, "h": 256 },
      "duration": 100
    }
  ],
  "meta": {
    "image": "character.png",
    "frameTags": [
      { "name": "idle", "from": 0, "to": 7, "direction": "forward" },
      { "name": "walk", "from": 8, "to": 15, "direction": "forward" }
    ]
  }
}
```

**メリット**:

- Asepriteの標準JSON形式に対応
- trim補正により、Packed化後も足元揃い安定
- frameTags で複数アニメ管理可能

---

### 4.3 TexturePackerAtlasProvider（Phase 3: 統合アトラス）

```cpp
class TexturePackerAtlasProvider : public IFrameProvider {
public:
    TexturePackerAtlasProvider(Texture2D atlasTexture, 
                              const nlohmann::json& packJson);
    
    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    // ... 他のメソッド同様
    
private:
    Texture2D atlasTexture_;
    std::unordered_map<std::string, SpriteSet> spriteSets_;
    std::unordered_map<std::string, std::string> clipToSetName_;
};
```

**メリット**:

- 小型キャラ200体 + エフェクト大量を1-2テクスチャに統合
- DrawTexturePro呼び出しのテクスチャ変更が大幅削減
- ゲームロジック不変（Provider差し替えのみ）

---

## 5. 描画層（Renderer）

### 5.1 統一描画関数

```cpp
class SpriteRenderer {
public:
    static void DrawSprite(
        const IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        const Vector2& worldPos,           // 足元座標
        const Vector2& scale = {1.0f, 1.0f},
        float rotation = 0.0f,
        Color tint = RAYWHITE
    );
};
```

**実装**:

```cpp
void SpriteRenderer::DrawSprite(
    const IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    const Vector2& worldPos,
    const Vector2& scale,
    float rotation,
    Color tint) {
    
    FrameRef ref = provider.GetFrame(clipName, frameIndex);
    if (!ref.valid) return;
    
    Rectangle dest = {
        worldPos.x + ref.offset.x * scale.x,
        worldPos.y + ref.offset.y * scale.y,
        ref.src.width * scale.x,
        ref.src.height * scale.y
    };
    
    DrawTexturePro(*ref.tex, ref.src, dest, ref.origin, rotation, tint);
}
```

---

## 6. ECS コンポーネント（リファクタリング）

### 6.1 Animation コンポーネント（新版）

```cpp
struct Animation {
    std::string currentClip = "idle";   // 現在のクリップ名
    int frameIndex = 0;                 // 現在のフレームインデックス
    float elapsedTime = 0.0f;           // 経過時間（秒）
    bool isPlaying = true;
};

struct Transform {
    float x = 0.0f, y = 0.0f;           // ワールド座標（足元基準）
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotation = 0.0f;
    bool flipH = false;
    bool flipV = false;
};

struct Sprite {
    IFrameProvider* provider = nullptr;  // FrameRef参照提供
};
```

---

## 7. システム実装（統一化）

### 7.1 AnimationSystem

```cpp
void AnimationSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Animation, Sprite>();
    
    for (auto entity : view) {
        auto& anim = view.get<Animation>(entity);
        auto& sprite = view.get<Sprite>(entity);
        
        if (!anim.isPlaying || !sprite.provider) continue;
        
        anim.elapsedTime += deltaTime;
        
        float fps = sprite.provider->GetClipFps(anim.currentClip);
        float frameDuration = 1.0f / fps;
        int nextFrameIndex = (int)(anim.elapsedTime / frameDuration);
        int frameCount = sprite.provider->GetFrameCount(anim.currentClip);
        
        if (nextFrameIndex >= frameCount) {
            if (sprite.provider->IsLooping(anim.currentClip)) {
                anim.elapsedTime = 0.0f;
                anim.frameIndex = 0;
            } else {
                anim.isPlaying = false;
                anim.frameIndex = frameCount - 1;
            }
        } else {
            anim.frameIndex = nextFrameIndex;
        }
    }
}
```

### 7.2 RenderingSystem

```cpp
void RenderingSystem::DrawEntities(
    entt::registry& registry,
    const IFrameProvider& provider) {
    
    auto view = registry.view<Transform, Animation, Team>();
    
    for (auto entity : view) {
        auto& transform = view.get<Transform>(entity);
        auto& anim = view.get<Animation>(entity);
        auto& team = view.get<Team>(entity);
        
        Color tint = (team.team == Team::Type::Player) ? BLUE : RED;
        
        SpriteRenderer::DrawSprite(
            provider,
            anim.currentClip,
            anim.frameIndex,
            Vector2{transform.x, transform.y},
            Vector2{transform.scaleX, transform.scaleY},
            transform.rotation,
            tint
        );
    }
}
```

---

## 8. 段階移行チェックリスト

### Grid → Packed への移行

- [ ] GridSheetProvider 実装・テスト
- [ ] AsepriteJsonAtlasProvider 実装
- [ ] Aseprite CLI json-array 出力フロー確立
- [ ] footOffsetY をドキュメント化
- [ ] trim フレームの offset/origin 補正確認
- [ ] Provider差し替えテスト（ゲームロジック不変確認）

### Packed → 統合アトラス への移行

- [ ] TexturePackerAtlasProvider 実装
- [ ] 複数キャラ統合定義（JSON）
- [ ] バッチング戦略立案
- [ ] パフォーマンステスト

---

## 9. アセット命名・管理規約

### Phase 1: Grid期

```
assets/mainCharacters/{name}/
├── {name}.png              # 256x256グリッド
└── clips.json              # クリップ定義
```

**clips.json**:

```json
{
  "config": {
    "cellWidth": 256,
    "cellHeight": 256,
    "framesPerRow": 16
  },
  "clips": [
    { "name": "idle", "startIndex": 0, "length": 8, "loop": true, "fps": 12 }
  ]
}
```

### Phase 2: Packed期

```
assets/mainCharacters/{name}/
├── {name}.png              # Packedアトラス
└── {name}.json             # Aseprite JSON Array
```

### Phase 3: 統合アトラス期

```
assets/atlases/
├── characters_main.png     # 統合テクスチャ
└── characters_main.json    # Texture Packer JSON
```

---

## 10. 実装ロードマップ

### Week 1: GridSheetProvider + 基本描画

- [ ] FrameRef/AnimClip/SpriteSet 定義
- [ ] IFrameProvider インターフェース
- [ ] GridSheetProvider 実装
- [ ] Animation/Transform/Sprite コンポーネント更新
- [ ] SpriteRenderer 実装
- [ ] AnimationSystem/RenderingSystem リファクタ
- [ ] グリッド形式アセットでテスト

### Week 2: AsepriteJsonAtlasProvider

- [ ] SpriteSheetLoader を FrameRef出力に更新
- [ ] AsepriteJsonAtlasProvider 実装
- [ ] offset/origin 補正ロジック
- [ ] Packed形式アセットでテスト
- [ ] Provider差し替えテスト

### Week 3-4: TexturePackerAtlasProvider + 最適化

- [ ] TexturePackerAtlasProvider 実装
- [ ] 複数キャラ統合
- [ ] バッチング戦略
- [ ] パフォーマンステスト

---

## 11. まとめ

### 設計のポイント

✅ **段階移行対応**: Grid → Packed → 統合アトラス に移行してもゲームロジック**不変**  
✅ **Provider抽象化**: 描画仕様の詳細を隠蔽  
✅ **足元揃え安定**: offset + origin 補正  
✅ **raylib統一**: DrawTexturePro に統一

---

**文責**: GitHub Copilot + ユーザー設計  
**次回更新**: GridSheetProvider 実装完了後
