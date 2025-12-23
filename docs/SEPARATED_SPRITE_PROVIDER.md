# 開発用分割スプライト管理システム

**バージョン**: 1.0  
**最終更新**: 2025-01-XX  
**対象**: Simple TDC Game Project  
**状態**: 実装完了

---

## 概要

開発時にスプライトシートを分割状態で管理し、状態アニメーションごとに個別のPNGファイルとJSON設定で管理するシステムです。本番時は既存の`AsepriteJsonAtlasProvider`を使用し、開発時のみ`SeparatedSpriteProvider`を使用します。

## 設計方針

- **開発モード**: 状態ごとに分割されたPNGファイルを管理（編集が容易）
- **本番モード**: 統合されたAsepriteスプライトシートを使用（パフォーマンス重視）
- **切り替え**: `EntityDef`の`use_dev_mode`フラグで制御
- **互換性**: `IFrameProvider`インターフェースにより、ゲームロジックは変更不要

## アセット構造

```
assets/
├── characters/
│   └── sub/
│       └── HatSlime/
│           ├── HatSlime.png              # 本番用統合スプライトシート
│           └── dev/                      # 開発用分割スプライト
│               ├── idle/
│               │   ├── idle_0.png
│               │   ├── idle_1.png
│               │   ├── idle_2.png
│               │   ├── idle_3.png
│               │   └── idle.json
│               ├── walk/
│               │   ├── walk_0.png
│               │   ├── walk_1.png
│               │   ├── walk_2.png
│               │   ├── walk_3.png
│               │   └── walk.json
│               ├── attack/
│               │   └── ...
│               └── death/
│                   └── ...
│
├── definitions/
│   └── entities/
│       └── characters/
│           └── HatSlime/
│               ├── entity.json           # エンティティ定義
│               └── dev_animation.json    # 開発用アニメーション設定
```

## JSONファイル形式

### dev_animation.json（開発用アニメーション設定）

開発用アニメーション設定のルートファイルです。各状態クリップの設定ファイルへの参照を定義します。

```json
{
  "character_id": "char_sub_hatslime_001",
  "sprite_base_path": "assets/characters/sub/HatSlime/dev",
  "clips": {
    "idle": {
      "config_path": "idle/idle.json"
    },
    "walk": {
      "config_path": "walk/walk.json"
    },
    "attack": {
      "config_path": "attack/attack.json"
    },
    "death": {
      "config_path": "death/death.json"
    }
  },
  "default_foot_offset_y": 32.0
}
```

**フィールド説明**:

- `character_id` (optional): キャラクターID（参照用）
- `sprite_base_path` (required): スプライトディレクトリのベースパス
- `clips` (required): アニメーションクリップの設定
  - `config_path`: `sprite_base_path`からの相対パスでクリップ設定JSONを指定
- `default_foot_offset_y` (optional): 全クリップのデフォルト足元オフセット（デフォルト: 0.0）

### 各状態クリップJSON（例: idle.json）

各アニメーション状態（idle, walk, attack, deathなど）の詳細設定を定義します。

```json
{
  "clip_name": "idle",
  "fps": 12.0,
  "loop": true,
  "frames": [
    {
      "file": "idle_0.png",
      "duration_ms": 83
    },
    {
      "file": "idle_1.png",
      "duration_ms": 83
    },
    {
      "file": "idle_2.png",
      "duration_ms": 83
    },
    {
      "file": "idle_3.png",
      "duration_ms": 83
    }
  ],
  "foot_offset_y": 32.0
}
```

**フィールド説明**:

- `clip_name` (optional): クリップ名（JSONファイル名から推測可能）
- `fps` (optional): デフォルトFPS（デフォルト: 12.0）
- `loop` (optional): ループ再生フラグ（デフォルト: true）
- `frames` (required): フレーム配列
  - `file`: PNGファイル名（クリップJSONと同じディレクトリに配置）
  - `duration_ms`: フレーム表示時間（ミリ秒）
- `foot_offset_y` (optional): 足元オフセット（デフォルト: `dev_animation.json`の`default_foot_offset_y`）

## EntityDefの設定

`EntityDef`の`display`構造体に以下のフィールドを追加：

```json
{
  "display": {
    "atlas_texture": "assets/characters/sub/HatSlime/HatSlime.png",
    "use_dev_mode": true,
    "dev_animation_config_path": "assets/definitions/entities/characters/HatSlime/dev_animation.json",
    ...
  }
}
```

**フィールド説明**:

- `use_dev_mode` (optional): 開発モードを使用する場合は`true`（デフォルト: `false`）
- `dev_animation_config_path` (optional): 開発用アニメーション設定JSONのパス

## 使用例

### 1. FrameProviderFactoryを使用したProvider生成

```cpp
#include "Game/Graphics/FrameProviderFactory.h"
#include "Data/Definitions/EntityDef.h"

// EntityDefから取得
const auto* entityDef = definitions.GetEntity("char_sub_hatslime_001");

// Providerを生成
auto provider = Game::Graphics::FrameProviderFactory::Create(*entityDef);
if (provider) {
    // 使用可能
    sprite.provider = provider.get();
    // プロバイダーを適切に管理（キャッシュなど）
}
```

### 2. NewRenderingSystemでの使用

```cpp
// エンティティ生成時にProviderを設定
auto provider = FrameProviderFactory::Create(*entityDef);
if (provider) {
    auto entity = registry.create();
    auto& sprite = registry.emplace<Game::Components::Sprite>(entity);
    sprite.provider = provider.get();
    
    // プロバイダーの所有権を管理（例: キャッシュに保存）
    provider_cache_[entityDef->id] = std::move(provider);
}
```

### 3. 開発モードと本番モードの切り替え

開発時は`entity.json`で`use_dev_mode: true`に設定し、本番時は`false`に戻すか、フィールドを削除します。

```json
// 開発時
{
  "display": {
    "use_dev_mode": true,
    "dev_animation_config_path": "...",
    ...
  }
}

// 本番時
{
  "display": {
    "use_dev_mode": false,  // またはフィールドを削除
    "atlas_texture": "...",
    ...
  }
}
```

## アーキテクチャ

```
EntityDef
  ├─ use_dev_mode flag
  └─ dev_animation_config_path
           │
           ▼
FrameProviderFactory
  ├─ use_dev_mode = true  → SeparatedSpriteProvider
  │   ├─ dev_animation.json を読み込み
  │   ├─ 各クリップJSONを読み込み
  │   └─ PNGファイルを遅延ロード
  │
  └─ use_dev_mode = false → AsepriteJsonAtlasProvider
      ├─ atlas_texture を読み込み
      └─ sprite_actions JSONを読み込み
```

## 実装詳細

### SeparatedSpriteProvider

- **IFrameProviderインターフェース**: 既存の描画システムと互換
- **遅延ロード**: テクスチャは`GetFrame`時に初めてロード（メモリ効率化）
- **ライフサイクル管理**: デストラクタでテクスチャを自動アンロード
- **エラーハンドリング**: JSON解析エラーやファイル不存在時の適切な処理

### FrameProviderFactory

- **自動選択**: `use_dev_mode`フラグに基づいて適切なProviderを選択
- **フォールバック**: 開発モードでファイルが見つからない場合、本番モードにフォールバック
- **エラーハンドリング**: 詳細なエラーメッセージを出力

## トラブルシューティング

### 問題: テクスチャが表示されない

**原因**:
- PNGファイルのパスが正しくない
- `sprite_base_path`と`config_path`の組み合わせが誤っている

**解決策**:
- `dev_animation.json`の`sprite_base_path`を確認
- 各クリップJSONの`config_path`が正しい相対パスか確認
- PNGファイルが実際に存在するか確認

### 問題: JSON解析エラー

**原因**:
- JSON形式が不正
- 必須フィールドが不足

**解決策**:
- JSON構文チェッカーで検証
- `dev_animation.schema.json`でスキーマ検証
- コンソールのエラーメッセージを確認

### 問題: 開発モードが有効にならない

**原因**:
- `entity.json`の`use_dev_mode`が`false`または未設定
- `dev_animation_config_path`が空または不正

**解決策**:
- `entity.json`の`display.use_dev_mode`を`true`に設定
- `display.dev_animation_config_path`が正しく設定されているか確認
- ファイルが実際に存在するか確認

### 問題: フレームが正しく表示されない

**原因**:
- `foot_offset_y`の設定が不適切
- フレームの`duration_ms`が0または負の値

**解決策**:
- `foot_offset_y`を調整（通常はスプライトの高さの一部）
- フレームの`duration_ms`が正の値であることを確認

## ベストプラクティス

### アセット管理

1. **開発用と本番用を分離**: `dev/`ディレクトリで開発用アセットを管理
2. **命名規則**: フレームファイルは`{clip_name}_{frame_index}.png`形式を推奨
3. **ディレクトリ構造**: 各状態ごとにディレクトリを分ける

### パフォーマンス

1. **遅延ロード**: テクスチャは必要時にロードされるため、大量のアニメーションでも効率的
2. **キャッシュ**: `FrameProviderFactory`で生成したProviderはキャッシュして再利用
3. **メモリ管理**: 不要になったProviderは適切に破棄（テクスチャがアンロードされる）

### 開発ワークフロー

1. **開発時**: `use_dev_mode: true`で分割スプライトを使用
2. **テスト**: 開発モードと本番モードの両方でテスト
3. **本番時**: `use_dev_mode: false`で本番用スプライトシートを使用

## 関連ドキュメント

- [キャラクター描画システム設計書](./CHARACTER_RENDERING_DESIGN.md)
- [IFrameProvider API仕様](../shared/include/Data/Graphics/IFrameProvider.h)
- [FrameRef API仕様](../shared/include/Data/Graphics/FrameRef.h)

## 変更履歴

- **v1.0** (2025-01-XX): 初版リリース

