# キャラクターアセット配置テンプレート

このフォルダは、キャラクターアセットをどのように配置するかのテンプレートです。

## ディレクトリ構造

新システムでは、シンプルなスプライトシートベースの構造を使用します：

```
assets/characters/
├── template/              # このテンプレートフォルダ
│   ├── README.md         # このファイル
│   └── CharacterName/    # キャラクター名のフォルダ（例: HatSlime）
│       ├── icon.png      # キャラクターアイコン（必須）
│       ├── move.png      # 移動アニメーション用スプライトシート（オプション）
│       └── attack.png    # 攻撃アニメーション用スプライトシート（オプション）
└── [実際のキャラクター名]/  # 実際のキャラクターフォルダ
    └── ...
```

**注意**: パス構造は柔軟です。`assets/characters/`から始まり、その後の構造は自由に決められます。
例: `assets/characters/sub/HatSlime/icon.png` や `assets/characters/sub/HatSlime/dev/walk/walk.png`

## ファイル説明

### 必須ファイル

- **icon.png**: キャラクターのアイコン画像
  - サイズ: 推奨64x64または128x128
  - 形式: PNG（透明背景推奨）
  - パス例: `assets/characters/CharacterName/icon.png`

### オプションファイル

- **move.png**: 移動アニメーション用スプライトシート
  - 横方向にフレームを並べたスプライトシート
  - パス例: `assets/characters/CharacterName/move.png` または `assets/characters/CharacterName/animations/walk/walk.png`

- **attack.png**: 攻撃アニメーション用スプライトシート
  - 横方向にフレームを並べたスプライトシート
  - パス例: `assets/characters/CharacterName/attack.png` または `assets/characters/CharacterName/animations/attack/attack.png`

## characters.jsonでの参照方法

`data/characters.json`では、以下のようにパスとフレーム情報を指定します：

```json
{
  "id": "char_sub_hatslime_001",
  "name": "帽子スライム",
  "sprites": {
    "icon_path": "assets/characters/sub/HatSlime/icon.png",
    "move": {
      "sheet_path": "assets/characters/sub/HatSlime/dev/walk/walk.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 8,
      "frame_duration": 0.1
    },
    "attack": {
      "sheet_path": "assets/characters/sub/HatSlime/dev/attack/attack.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 6,
      "frame_duration": 0.1
    }
  }
}
```

### スプライトシートの仕様

各スプライトシートには以下の情報が必要です：

- **frame_width**: 1フレームの幅（ピクセル）
- **frame_height**: 1フレームの高さ（ピクセル）
- **frame_count**: スプライトシート内の総フレーム数
- **frame_duration**: 各フレームの表示時間（秒）

**スプライトシートの配置方法**:
- フレームは横方向（左から右）に並べます
- 例: 64x64のフレームが8個ある場合、スプライトシートのサイズは 512x64 (64×8) になります

## パス規則

- パスは`assets/characters/`から始まります
- その後の構造は自由です（例: `sub/HatSlime/`, `CharacterName/`, `Category/CharacterName/`など）
- パス内の区切り文字は`/`（スラッシュ）を使用します（Windowsでも`/`を使用）
- キャラクター名のフォルダ名は、`characters.json`の`id`フィールドと対応させる必要はありませんが、分かりやすい名前を推奨します

## 新しいキャラクターを追加する手順

1. `data/assets/characters/`配下にキャラクター名のフォルダを作成（構造は自由）
2. `icon.png`を配置（必須）
3. 必要に応じてスプライトシートファイル（`move.png`, `attack.png`など）を配置
4. `data/characters.json`にキャラクター定義を追加し、以下を指定：
   - `sprites.icon_path`: アイコンパス
   - `sprites.move`: 移動アニメーション情報（sheet_path, frame_width, frame_height, frame_count, frame_duration）
   - `sprites.attack`: 攻撃アニメーション情報（同上）

## 例: シンプルな構造

```
assets/characters/
└── MyCharacter/
    ├── icon.png
    ├── move.png      # 64x64のフレームが8個 → 512x64のスプライトシート
    └── attack.png    # 64x64のフレームが6個 → 384x64のスプライトシート
```

対応する`characters.json`の設定：

```json
{
  "sprites": {
    "icon_path": "assets/characters/MyCharacter/icon.png",
    "move": {
      "sheet_path": "assets/characters/MyCharacter/move.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 8,
      "frame_duration": 0.1
    },
    "attack": {
      "sheet_path": "assets/characters/MyCharacter/attack.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 6,
      "frame_duration": 0.1
    }
  }
}
```

## 例: 階層構造

```
assets/characters/
└── sub/
    └── HatSlime/
        ├── icon.png
        └── dev/
            ├── walk/
            │   └── walk.png
            └── attack/
                └── attack.png
```

対応する`characters.json`の設定：

```json
{
  "sprites": {
    "icon_path": "assets/characters/sub/HatSlime/icon.png",
    "move": {
      "sheet_path": "assets/characters/sub/HatSlime/dev/walk/walk.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 8,
      "frame_duration": 0.1
    },
    "attack": {
      "sheet_path": "assets/characters/sub/HatSlime/dev/attack/attack.png",
      "frame_width": 64,
      "frame_height": 64,
      "frame_count": 6,
      "frame_duration": 0.1
    }
  }
}
```
