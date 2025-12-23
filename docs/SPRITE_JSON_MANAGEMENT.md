# キャラクターアニメーションスプライトJSON管理

## 概要

キャラクターのアニメーションスプライトは、Aseprite互換フォーマットのJSONで管理されます。
スプライトエディタ(`SpriteEditorWindow`)から生成・保存される仕組みを説明します。

## ディレクトリ構造

```
assets/characters/{character_name}/
├── entity.json                    # キャラクター定義
├── {character_name}.png           # メインテクスチャ
├── {character_name}_icon.png      # アイコン画像
└── animations/                    # ← アニメーションJSON格納先
    ├── idle.json                  # idle アクション用JSON
    ├── walk.json                  # walk アクション用JSON
    ├── attack.json                # attack アクション用JSON
    └── death.json                 # death アクション用JSON
```

## JSONフォーマット（Aseprite互換）

生成されるJSON構造：

```json
{
  "frames": [
    {
      "filename": "0",
      "frame": {"x": 0, "y": 0, "w": 64, "h": 64},
      "rotated": false,
      "trimmed": false,
      "spriteSourceSize": {"x": 0, "y": 0, "w": 64, "h": 64},
      "sourceSize": {"w": 64, "h": 64},
      "duration": 100
    },
    ...
  ],
  "meta": {
    "app": "SimpleTDC Editor",
    "version": "1.0",
    "image": "idle.png",
    "frameW": 64,
    "frameH": 64,
    "frames": 8,
    "columns": 4,
    "rows": 2,
    "yOffset": 0,
    "pivotX": 0.5,
    "pivotY": 1.0,
    "mirror": {
      "horizontal": false,
      "vertical": false
    }
  }
}
```

## JSONメタデータの詳細

### meta セクション

| キー | 型 | 説明 | 用途 |
|------|-----|------|------|
| `app` | string | 生成ツール名 | 情報のみ |
| `version` | string | フォーマットバージョン | 互換性確認 |
| `image` | string | テクスチャファイル名 | frame解決時に使用 |
| `frameW` | int | 各フレームの幅(px) | グリッド検証 |
| `frameH` | int | 各フレームの高さ(px) | グリッド検証 |
| `frames` | int | 総フレーム数 | フレーム数検証 |
| `columns` | int | グリッド列数 | レイアウト復元用 |
| `rows` | int | グリッド行数 | レイアウト復元用 |
| `yOffset` | int | Y軸オフセット | フレーム位置計算 |
| `pivotX` | float (0.0-1.0) | 水平原点(0=左, 1=右) | 回転・反転時の基準 |
| `pivotY` | float (0.0-1.0) | 垂直原点(0=上, 1=下) | 回転・反転時の基準 |
| `mirror` | object | デフォルト反転設定 | 描画時の反転 |

## 生成フロー

### 1. スプライトエディタで設定

```
[スプライトエディタ]
  ├─ PNG画像パス指定
  ├─ JSON出力ファイル名指定
  ├─ フレームパラメータ設定
  │  ├─ frameW/frameH （フレームサイズ）
  │  ├─ frames （フレーム数）
  │  ├─ columns/rows （グリッドレイアウト）
  │  ├─ yOffset （Y軸補正）
  │  ├─ pivotX/pivotY （原点位置）
  │  └─ mirrorH/mirrorV （反転設定）
  └─ [生成] ボタン クリック
```

### 2. バリデーション

生成前に以下を検証：

- PNG画像が指定されている
- JSON出力ファイル名が指定されている  
- frameW > 0
- frameH > 0
- frames > 0
- durationMs > 0
- PNG画像ファイルが存在する

失敗時は具体的なエラーメッセージを表示：

```
✖ フレーム幅(frameW)が0以下です
✖ PNG画像が見つかりません: assets/characters/main/idle.png
```

### 3. JSON生成

グリッド配置を計算して`frames`配列を構築：

```cpp
cols = (columns > 0) ? columns : frames
rows = (frames + cols - 1) / cols

frame[i]:
  col = i % cols
  row = i / cols
  x = col * frameW
  y = row * frameH + yOffset
```

### 4. ファイル保存

```
保存先: assets/characters/{character_name}/animations/{json_filename}
```

### 5. entity.jsonへの登録

`sprite_actions` マップに追加：

```json
{
  "display": {
    "sprite_actions": {
      "idle": "animations/idle.json",
      "walk": "animations/walk.json",
      "attack": "animations/attack.json",
      "death": "animations/death.json"
    }
  }
}
```

## ゲーム側での読み込み

### CharacterFactory

JSON読み込み時：

1. `sprite_actions` から JSONパス取得
2. JSONファイルをパース
3. `meta.mirror` から `Animation.mirrorHByAction/mirrorVByAction` に設定
4. `frames` 配列から frame情報を抽出

```cpp
// meta パラメータ読込例
bool mirrorH = false, mirrorV = false;
if (j.contains("meta") && j["meta"].contains("mirror")) {
    auto& mirror = j["meta"]["mirror"];
    mirrorH = mirror.value("horizontal", false);
    mirrorV = mirror.value("vertical", false);
}
```

### RenderingSystem

描画時の反転処理：

```cpp
// Transform側の反転
bool flipH = transform.flipH;
bool flipV = transform.flipV;

// Animation側のアクション別反転を合成
if (currentAction在り) {
    if (animation.mirrorHByAction.count(currentAction)) {
        flipH = flipH != animation.mirrorHByAction[currentAction];
    }
}

// 描画時：負の幅/高さで反転
float width = flipH ? -abs(frameWidth) : abs(frameWidth);
float height = flipV ? -abs(frameHeight) : abs(frameHeight);

// 原点を中央に設定
Vector2 origin = {abs(frameWidth)/2, abs(frameHeight)/2};

DrawTexturePro(texture, srcRect, dstRect, origin, rotation, tint);
```

## PreviewWindow との連携

### アクション選択時の動作

1. ユーザーがアクション（idle/walk/等）を選択
2. `LoadActionFrames()` で対応するJSONを読込
3. `current_frames_` ベクタに frame情報を格納
4. 再生ループで frame タイマー管理
5. `DrawTexturePro()` で画像を描画

### JSON読込パス

```cpp
// entity定義からアクションのJSONパス取得
auto it = entityDef.display.sprite_actions.find(action_name);
if (it != entityDef.display.sprite_actions.end()) {
    // entity.jsonからの相対パス解決
    if (!std::filesystem::exists(relative_path)) {
        absolute_path = entity_dir / relative_path;
    }
    // JSONパース → current_frames_ に格納
}
```

## エラーハンドリング

### JSON生成時のエラー

| エラー | 原因 | 対応 |
|--------|------|------|
| PNG画像が見つかりません | PNG指定パスが無効 | パスを確認、ドラッグ&ドロップで設定 |
| animationsディレクトリ作成失敗 | パーミッション不足 | プロジェクトフォルダの権限確認 |
| JSONファイル書き込み失敗 | ディスク容量不足またはパーミッション | ディスク空き容量確認 |
| JSON生成エラー | 例外発生 | デバッグログで詳細確認 |

### JSON読込時のエラー

| エラー | 原因 | 対応 |
|--------|------|------|
| JSONが見つからない | sprite_actions の相対パスが無効 | entity.json の sprite_actions を確認 |
| JSONフォーマットエラー | meta が不正 | JSON をエディタで確認 |
| frameW/frameH が0 | meta の設定不正 | スプライトエディタで再生成 |

## ベストプラクティス

1. **命名規則**: `{action_name}.json`
   - good: `idle.json`, `walk.json`, `attack.json`
   - bad: `anim.json`, `sprite_01.json`

2. **パラメータ設定**
   - pivotX/pivotY は反転・回転時の基準になるため重要
   - yOffset は複数アクションで高さが異なる場合に使用

3. **テクスチャ管理**
   - 1アクション＝1 PNG画像推奨
   - 複数アクションを同じPNGに配置しない（管理複雑化）

4. **バージョン管理**
   - JSON ファイルはバージョン管理対象に含める
   - PNG 画像はバージョン管理対象から除外（`.gitignore` に追加）

5. **ドキュメント**
   - 特殊なパラメータを使用した場合は、コミットメッセージに記載

## 参考資料

- [Aseprite JSON フォーマット](https://www.aseprite.org/)
- [EntityDef スキーマ](./EDITOR_ARCHITECTURE_SPLIT_PLAN.md)
