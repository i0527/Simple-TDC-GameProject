# Game Definitions

このディレクトリには、ゲームのJSON定義ファイルを格納します。

## ディレクトリ構成

- **characters/** - キャラクター定義ファイル（*.char.json）
- **stages/** - ステージ定義ファイル（*.stage.json）
- **ui/** - UIレイアウト定義ファイル（*.ui.json）
- **skills/** - スキル定義ファイル（*.skill.json）
- **effects/** - エフェクト定義ファイル（*.effect.json）
- **sounds/** - サウンド定義ファイル（*.sound.json）

## WebUIエディター

これらの定義ファイルは、WebUIエディター（`tools/webui-editor/`）を使用してビジュアルに編集できます。

### 使用方法

1. WebUIエディターを起動:
   ```bash
   cd tools/webui-editor
   npm install
   npm run dev
   ```

2. ゲームサーバーをHTTPサーバー有効で起動:
   ```bash
   # ビルド後
   ./bin/SimpleTDCGame
   ```

3. ブラウザでエディターにアクセス:
   - WebUI: http://localhost:3000
   - API: http://localhost:8080/api/*

## 定義ファイルの例

### キャラクター定義 (characters/example.char.json)
```json
{
  "id": "example_character",
  "name": "サンプルキャラクター",
  "description": "これはサンプルです",
  "gameMode": "td",
  "rarity": "common",
  "stats": {
    "maxHP": 100,
    "attack": 10,
    "defense": 5,
    "speed": 1.0
  }
}
```

### ステージ定義 (stages/example.stage.json)
```json
{
  "id": "stage_1",
  "name": "ステージ1",
  "description": "最初のステージ",
  "laneCount": 3,
  "baseHP": 100,
  "waves": []
}
```

### スキル定義 (skills/example.skill.json)
```json
{
  "id": "fireball",
  "name": "ファイアボール",
  "description": "炎の球を放つ",
  "cooldown": 5.0,
  "targetType": "enemy",
  "effects": [
    {
      "type": "Damage",
      "value": 50
    }
  ]
}
```

詳細は各エディターのドキュメントを参照してください。
