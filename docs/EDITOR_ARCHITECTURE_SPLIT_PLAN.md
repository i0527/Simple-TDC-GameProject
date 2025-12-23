# エディタアーキテクチャ分割計画

## Overview

現在のモノリシックなエディタを機能別に分割し、にゃんこ大戦争風ゲームに必要な機能を段階的に追加する。

---

## 1. コアエディタ分割 (必須)

### 1.1 ユニットエディタ (Unit Editor)

**目的**: ユニットのメタデータと基本ステータスをJSONとして管理

**設定項目**:

- 基本情報: ID, 名前, 説明, レアリティ
- ステータス: HP, 攻撃力, 攻撃速度, 移動速度, 範囲, ノックバック
- 画像・アセット: アイコン, アトラステクスチャ, スプライトアクション
- ゲーム設定: コスト, クールタイム, タイプ(主力/サブ/敵), 敵フラグ

**出力**: `assets/definitions/entities/characters/{id}.json`

**技術仕様**:

- PropertyPanel で直接編集可能
- 変更はリアルタイムでメモリに反映
- Save ボタンで JSON に書き込み

---

### 1.2 描画エディタ (Rendering Editor)

**目的**: ユニットのアニメーション、ビジュアル効果、イベントタイミングを管理

**設定項目**:

- アニメーション制御
  - 再生速度倍率
  - フレーム単位のイベント配置
  - アニメーションループ設定
  
- イベントタイミング
  - 攻撃判定発生フレーム (attack_point: 0.0-1.0)
  - 攻撃フレーム固定値 (attack_frame: -1 = auto)
  
- 当たり判定 (Hitbox)
  - 幅, 高さ, オフセット (X, Y)
  - プレビューで赤枠表示, 中心を緑十字で表示
  
- ビジュアル効果
  - ヒット時エフェクト (effect_id 参照)
  - パーティクルシステム設定
  - スクリーンシェイク強度
  
- 状態異常表現
  - 毒, 凍結, スロウ, スタン等の色分け設定

**出力**: `assets/definitions/entities/characters/{id}.animation.json` (新規)

**技術仕様**:

- PreviewWindow でアニメーション再生 & リアルタイム確認
- Hitbox をドラッグで直感的に調整可能
- タイムラインUI で各フレームのイベント配置

---

### 1.3 ステージエディタ (Stage Editor)

**目的**: ステージの構成、敵の配置、難易度を管理

**設定項目**:

- ステージ基本情報
  - ID, 名前, 説明, ドメイン, サブドメイン
  - 難易度レベル
  - 基地初期 HP
  
- ウェーブ設定
  - ウェーブ数
  - 各ウェーブの敵構成 (エンティティ ID, 数, 出現間隔, 遅延)
  - ウェーブ間のポーズ時間
  
- 環境設定
  - 背景画像
  - BGM ID
  - スクロール速度
  
- 報酬設定
  - ゴールド報酬
  - 経験値報酬
  - ドロップアイテム

**出力**:

- `assets/definitions/stages/{id}.json`
- `assets/definitions/waves/{wave_id}.json`

---

## 2. 追加エディタ (高付加価値)

### 2.1 スキル/アビリティエディタ (Skill & Ability Editor)

**目的**: ユニットの個性付け、複雑な効果定義

**設定項目**:

- スキル定義
  - ID, 名前, 説明
  - 発動タイプ (受動/主動/トリガー型)
  - クールタイム, 発動確率
  - ターゲット種別 (自身/敵単体/敵範囲/全敵)
  
- 効果チェーン
  - ダメージ+範囲
  - バフ/デバフ (攻撃+20%, 防御-30% など)
  - 状態異常付与
  - 連鎖効果 (A 発動 → B 発動)

**出力**: `assets/definitions/skills/{id}.json`, `assets/definitions/abilities/{id}.json`

---

### 2.2 エフェクト/ビジュアルエディタ (Effect Editor)

**目的**: ゲーム内の視覚効果を統一管理

**設定項目**:

- エフェクトテンプレート
  - パーティクル画像/スプライト
  - 生存時間, フェードイン/アウト
  - 移動パターン (放射状/曲線/直進)
  
- 状態異常ビジュアル
  - 毒 = 緑オーラ + パーティクル
  - 凍結 = 青/白エフェクト + 移動速度低下アニメ
  - スタン = 星マーク回転
  
- ダメージ表現
  - ダメージ数字の色 (通常/クリティカル/状態異常)
  - フローティングテキスト効果

**出力**: `assets/definitions/effects/{id}.json`

---

### 2.3 デッキ/編成エディタ (Deck & Formation Editor)

**目的**: プレイヤーがゲーム内で編成するユニット構成を管理

**設定項目**:

- デッキテンプレート
  - デッキID, 名前
  - 最大ユニット数, 最大コスト
  - 利用可能ユニット制限
  
- ステージ別推奨編成
  - 各ステージに対して推奨ユニット
  - 推奨レベル

**出力**: `assets/definitions/decks/{id}.json`

**補足**: ゲーム内でも動的に生成可能なため、優先度は低め

---

### 2.4 難易度/バランスエディタ (Difficulty & Balance Editor)

**目的**: ステージのバランス調整、段階的な難易度上昇

**設定項目**:

- 敵強度倍率
  - HP 倍率, ダメージ倍率, 速度倍率
  - ボス専用オプション (体力回復, 特殊能力)
  
- 報酬倍率
  - ノーマル/ハード/チャレンジモード の報酬比率
  
- 制限条件
  - 使用可能ユニットの制限
  - マップギミック (敵の無敵時間, ダメージ制限)

**出力**: `assets/definitions/balancing/{id}.json`

---

## 3. ファイル構造案

```
assets/definitions/
├── entities/
│   ├── characters/
│   │   ├── unit_001.json               ← Unit Editor 出力
│   │   └── unit_001.animation.json     ← Rendering Editor 出力
│   └── enemies/
│       └── enemy_001.json
├── skills/
│   ├── skill_001.json                  ← Skill Editor 出力
│   └── ...
├── abilities/
│   ├── ability_001.json                ← Ability Editor 出力
│   └── ...
├── stages/
│   ├── stage_001.json                  ← Stage Editor 出力
│   └── ...
├── waves/
│   ├── wave_001.json
│   └── ...
├── effects/
│   ├── effect_hit.json                 ← Effect Editor 出力
│   └── ...
├── decks/
│   ├── deck_001.json                   ← Deck Editor 出力
│   └── ...
└── balancing/
    └── balance_001.json                ← Difficulty Editor 出力
```

---

## 4. 実装優先度

### Phase 1 (必須): コア分割

1. ユニットエディタ (既存機能の再構成)
2. 描画エディタ (アニメーションタイミング + Hitbox)
3. ステージエディタ (既存機能の再構成)

### Phase 2 (高優先): スキル強化

4. スキル/アビリティエディタ

### Phase 3 (中優先): ビジュアル管理

5. エフェクト/ビジュアルエディタ

### Phase 4 (低優先): 便利機能

6. デッキ/編成エディタ
2. 難易度/バランスエディタ

---

## 5. UI/UX 設計案

### メインメニュー

```
┌─ Simple TDC Editor ─────────────────┐
│  [ユニット] [描画] [ステージ]        │
│  [スキル] [エフェクト] [デッキ]       │
│  [バランス]                          │
│                                     │
│ 左: リスト/検索                      │
│ 中: プレビュー/編集エリア             │
│ 右: パラメータパネル                  │
└─────────────────────────────────────┘
```

### 描画エディタの詳細

```
┌─ Rendering Editor (Anim Timeline) ─┐
│  [Play] [Stop] [Reset] Speed: 1.0x │
├─────────────────────────────────────┤
│ | Frame 0 │ 10 │ 20 │ 30 │ Attack! │ ← Timeline
├─────────────────────────────────────┤
│ プレビュー (赤: Hitbox)              │
│ (Hitbox ドラッグ調整可能)             │
├─────────────────────────────────────┤
│ Attack Point: 0.5 ─────○─────       │
│ Frame: 15                            │
│ Hitbox: W=32, H=32                   │
│         Offset: X=0, Y=0             │
└─────────────────────────────────────┘
```

---

## 6. 技術実装メモ

### 新規ファイル

- `editor/include/Editor/Windows/UnitEditorWindow.h`
- `editor/include/Editor/Windows/RenderingEditorWindow.h`
- `editor/include/Editor/Windows/StageEditorWindow.h`
- `editor/include/Editor/Windows/SkillEditorWindow.h`
- `editor/include/Editor/Windows/EffectEditorWindow.h`
- `editor/include/Editor/Windows/DeckEditorWindow.h`
- `editor/include/Editor/Windows/BalanceEditorWindow.h`

### データ構造の拡張

- `EntityDef.h`: `animation_events`, `effects` フィールド追加
- 新規: `SkillDef.h`, `EffectDef.h`, `DeckDef.h`, `BalanceDef.h`

### ローダーの追加

- `SkillLoader`, `EffectLoader`, `DeckLoader`, `BalanceLoader` etc.

---

## 7. にゃんこ大戦争風ゲームとしての特有機能

- **ユニット進化**: 同一ユニットの複数レアリティ/段階管理
- **キャラパワー**: ユニット育成の深さ (レベル, 覚醒, アビリティ強化)
- **限定ステージ**: 期間限定イベント対応
- **デイリーチャレンジ**: 毎日のステージ変動

これらは **デッキ/編成** および **バランスエディタ** で対応可能。

---

## 8. 編集・改善項目 (To-Do)

- [ ] エディタ UI モックアップの作成
- [ ] アニメーションイベントシステムの詳細設計
- [ ] エフェクトシステムの実装方針決定
- [ ] Phase 1 実装スケジュール策定
- [ ] 各エディタのウィンドウクラス実装
