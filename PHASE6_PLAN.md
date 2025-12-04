# Phase 6実装計画: 追加機能（スキル・エフェクト・サウンド統合）

## 📝 更新履歴

### 2025年12月4日 - CMake + Node.js 統合実装

初回セットアップの完全自動化を実現しました。

#### 実装内容

- **Node.js 自動ダウンロード** (`CMakeLists.txt` 行 41-155)
  - Windows (x64) / Mac (Intel/Apple Silicon) / Linux (x64) 対応
  - プロジェクトローカルに `build/.nodejs/` にセットアップ
  - システム Node.js インストール不要

- **WebUI npm install 自動化** (`CMakeLists.txt` 行 219-236)
  - `setup-webui` カスタムターゲット実装
  - SimpleTDCGame ビルド前に自動実行
  - ビルド環境に完全統合

- **ドキュメント作成** (`docs/CMAKE_NODEJS_INTEGRATION.md`)
  - セットアップフロー詳細
  - プラットフォーム対応表
  - トラブルシューティング

#### ユーザー体験の改善

**従来の手順**:

```bash
# 1. Node.js を手動インストール
# 2. VSCode で npm install 手動実行
# 3. npm run dev で WebUI サーバー起動
# 4. ゲームビルド
```

**改善後**:

```bash
git clone ...
cmake -B build
cmake --build build --config Release
# ✅ 完了！すべて自動
```

#### 関連ドキュメント

- 新規: `docs/CMAKE_NODEJS_INTEGRATION.md` - CMake + Node.js 統合ガイド
- 参考: `CMakeLists.txt` - 実装詳細
- 参考: `PHASE6_PLAN.md` (このファイル) - 全体計画

---

## 概要

Phase 6では、WebUIエディターにスキル・特殊能力、エフェクト、サウンドの管理機能を統合し、TD/Nethackの統一キャラクター定義システムを完成させます。

## 実装対象

### Phase 6A: WebUIエディター拡張（スキル・能力）

**目標**: 特殊能力とスキルをWebUIで設計・カスタマイズ

#### 新しいエディターコンポーネント

1. **SkillEditor** (`tools/webui-editor/src/components/Editors/SkillEditor.tsx`)
   - スキル定義の作成・編集
   - 発動条件設定（攻撃時、被ダメージ時、死亡時）
   - ターゲットタイプ設定（単体、範囲、全体）
   - スキル効果の追加・編集（ダメージ、回復、ステータス付与）
   - クールダウン・発動確率設定
   - アニメーション/エフェクト連携

2. **AbilityEditor** (`tools/webui-editor/src/components/Editors/AbilityEditor.tsx`)
   - キャラクター固有能力の定義
   - パッシブスキル設定
   - 能力の条件設定
   - 複合能力の組み合わせ

3. **StatusEffectEditor** (`tools/webui-editor/src/components/Editors/StatusEffectEditor.tsx`)
   - ステータス効果の定義（Slow, Stun, Poison等）
   - 効果値と継続時間設定
   - UI アイコン設定
   - 効果の重複ルール設定

**WebUIエディター構成**:

```
tools/webui-editor/src/
├── types/
│   ├── skill.ts          # スキル型定義
│   ├── ability.ts        # 能力型定義
│   ├── effect.ts         # エフェクト型定義
│   └── sound.ts          # サウンド型定義
├── components/Editors/
│   ├── SkillEditor.tsx
│   ├── AbilityEditor.tsx
│   ├── StatusEffectEditor.tsx
│   ├── EffectEditor.tsx
│   ├── SoundEditor.tsx
│   ├── Skill/
│   │   ├── SkillList.tsx
│   │   ├── SkillForm.tsx
│   │   └── SkillPreview.tsx
│   ├── Ability/
│   │   ├── AbilityList.tsx
│   │   ├── AbilityForm.tsx
│   │   └── AbilityPreview.tsx
│   ├── Effect/
│   │   ├── EffectTimeline.tsx      # タイムライン編集
│   │   ├── ParticleEmitterEditor.tsx
│   │   ├── ScreenEffectEditor.tsx
│   │   └── EffectPreview.tsx
│   └── Sound/
│       ├── SoundList.tsx
│       ├── SoundWaveform.tsx
│       └── SoundPreview.tsx
└── hooks/
    ├── useSkillAPI.ts
    ├── useAbilityAPI.ts
    ├── useEffectAPI.ts
    └── useSoundAPI.ts
```

### Phase 6B: WebUIエディター拡張（エフェクト）

**目標**: パーティクル、ビジュアル、スクリーンエフェクトをビジュアルに設計

#### 主要機能

1. **ParticleEmitterEditor**
   - エミッター形状選択（Point, Circle, Rectangle, Line, Ring, Cone）
   - 発生モード設定（Continuous, Burst, Distance）
   - 初期値設定（Lifetime, Speed, Angle, Scale, Color）
   - ライフタイム中の変化（Scale, Color, Alpha, Rotation）
   - 物理設定（Gravity, Drag, Damping）
   - リアルタイムプレビュー

2. **ScreenEffectEditor**
   - エフェクトタイプ選択（Shake, Flash, Fade, Zoom等）
   - パラメータ調整
   - イージング関数選択
   - プレビュー表示

3. **EffectTimeline**
   - 複合エフェクトのタイムライン編集
   - パーティクル、スプライト、スクリーンエフェクトの時間配置
   - サウンド同期

### Phase 6C: WebUIエディター拡張（サウンド）

**目標**: SFX、BGM、サウンドバンクをWebUIで管理

#### 主要機能

1. **SoundEditor**
   - SoundDef作成・編集
   - ファイル選択・バリエーション管理
   - 3D音源設定
   - ボリューム・ピッチ設定
   - グループ・タグ管理

2. **MusicEditor**
   - MusicDef作成・編集
   - ループ設定
   - イントロ/アウトロファイル設定
   - レイヤー管理（インタラクティブミュージック）

3. **SoundBankEditor**
   - バンク定義・管理
   - サウンドイベント設定
   - イベント再生モード（All, Random, Sequence）

## API拡張

### C++サーバー側の追加エンドポイント

```
# スキル
GET    /api/skills              # スキル一覧
POST   /api/skills              # 新規作成
PUT    /api/skills/{id}         # 更新
DELETE /api/skills/{id}         # 削除

# 能力
GET    /api/abilities           # 能力一覧
POST   /api/abilities           # 新規作成
PUT    /api/abilities/{id}      # 更新
DELETE /api/abilities/{id}      # 削除

# ステータス効果
GET    /api/status-effects      # ステータス効果一覧
POST   /api/status-effects      # 新規作成
PUT    /api/status-effects/{id} # 更新
DELETE /api/status-effects/{id} # 削除

# エフェクト
GET    /api/effects             # エフェクト一覧
POST   /api/effects             # 新規作成
PUT    /api/effects/{id}        # 更新
DELETE /api/effects/{id}        # 削除

# サウンド
GET    /api/sounds              # サウンド一覧
POST   /api/sounds              # 新規作成
PUT    /api/sounds/{id}         # 更新
DELETE /api/sounds/{id}         # 削除

GET    /api/music               # BGM一覧
POST   /api/music               # 新規作成
PUT    /api/music/{id}          # 更新
DELETE /api/music/{id}          # 削除

GET    /api/sound-banks         # サウンドバンク一覧
POST   /api/sound-banks         # 新規作成
PUT    /api/sound-banks/{id}    # 更新
DELETE /api/sound-banks/{id}    # 削除
```

## JSON定義ファイル構成

### assets/definitions/skills/

```json
{
  "id": "fireball",
  "name": "ファイアボール",
  "description": "敵に火炎ダメージを与える",
  "cooldown": 5.0,
  "activationChance": 1.0,
  "activateOnAttack": true,
  "targetType": "area",
  "effectArea": { "x": 0, "y": 0, "width": 100, "height": 100 },
  "effects": [
    {
      "type": "Damage",
      "value": 50,
      "isPercentage": false
    },
    {
      "type": "StatusApply",
      "statusEffectId": "burn"
    }
  ],
  "animationName": "cast_fireball",
  "effectSpritePath": "effects/fireball"
}
```

### assets/definitions/effects/

```json
{
  "id": "hit_explosion",
  "name": "ヒット時爆発",
  "emitters": [
    {
      "id": "particles",
      "shape": "circle",
      "radius": 50,
      "emissionRate": 50,
      "lifetime": [1.0, 2.0],
      "speed": [100, 200],
      "blendMode": "additive"
    }
  ],
  "duration": 2.0,
  "startSoundId": "explosion_sound"
}
```

### assets/definitions/sounds/

```json
{
  "id": "explosion",
  "name": "爆発音",
  "type": "sfx",
  "priority": "high",
  "variations": [
    {
      "filePath": "sounds/explosion_1.ogg",
      "weight": 1.0
    }
  ],
  "volume": 0.9,
  "maxInstances": 3,
  "is3D": true,
  "minDistance": 10.0,
  "maxDistance": 500.0
}
```

## キャラクター定義の統一化

### assets/definitions/characters/{id}.character.json

全キャラクター定義をJSON化：

```json
{
  "id": "cupslime_td",
  "name": "カップスライム（TD）",
  "gameMode": "TD",
  "visual": { /* ... */ },
  "stats": { /* ... */ },
  "combat": { /* ... */ },
  "skillIds": ["basic_attack", "slime_split"],
  "abilityIds": ["regeneration"],
  "td": { /* ... */ },
  "roguelike": null
}
```

## 実装優先度

### 高優先度

1. **SkillEditor** - キャラクター能力の基本
2. **EffectEditor（パーティクル）** - ビジュアル表現の中核
3. **SoundEditor** - サウンド統合の基本

### 中優先度

4. **StatusEffectEditor** - ゲームバランス調整
5. **AbilityEditor** - キャラクター固有の特性
6. **ScreenEffectEditor** - ビジュアル演出

### 低優先度

7. **MusicEditor** - BGM管理
8. **SoundBankEditor** - サウンド組織化

## 推定実装時間

- **WebUI拡張**: 1-2週間
  - SkillEditor: 2-3日
  - EffectEditor: 3-4日
  - SoundEditor: 2-3日

- **API実装**: 3-4日

- **JSON定義作成**: 2-3日

- **統合テスト**: 2-3日

**総計**: 2-3週間

## 次のマイルストーン

- Phase 6A完了時: スキルシステムのプロトタイプ完成
- Phase 6B完了時: ビジュアルエフェクト統合完成
- Phase 6完了時: 完全な統一キャラクター・スキル・エフェクト・サウンド定義システム完成

## リスク管理

1. **複雑な状態管理**: WebUIのフォーム状態が複雑になる可能性
   → Zustand ストアで状態一元管理

2. **パフォーマンス**: 複数のエディターの同時表示
   → 遅延ロード、仮想スクロール採用

3. **エフェクトプレビュー**: リアルタイムプレビューのパフォーマンス
   → WebGL キャンバスで実装、フレームレート制限

4. **ファイル監視**: 大量の定義ファイルの変更検知
   → ディバウンス、キャッシング機構強化
