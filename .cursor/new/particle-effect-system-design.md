# パーティクルエフェクトシステム設計

> 目的: 斬撃・爆発・治癒などの軽量エフェクトを、JSON定義とコンポーネントで再利用しやすくし、横スクロールTDの演出を強化する。

## ゴール

- パーティクル定義を JSON で管理し、ゲーム/エディタで再利用。
- パフォーマンス: バッチングとプールで負荷を抑え、低コストで多発可能。
- DevMode で再生・プレビュー・ホットリロードに対応。

## データモデル

### JSON例

```json
{
  "presets": {
    "slash_small": {
      "texture": "assets/effects/slash.png",
      "maxParticles": 32,
      "emission": { "burst": 12 },
      "lifetime": { "min": 0.2, "max": 0.35 },
      "speed": { "min": 600, "max": 900 },
      "angle": { "min": -20, "max": 20 },
      "size": { "min": 0.8, "max": 1.1 },
      "fadeOut": true,
      "color": [255, 255, 255, 255],
      "gravity": 0,
      "blend": "add"
    },
    "heal_pulse": {
      "texture": "assets/effects/pulse.png",
      "maxParticles": 24,
      "emission": { "burst": 8 },
      "lifetime": { "min": 0.4, "max": 0.6 },
      "speed": { "min": 150, "max": 250 },
      "angle": { "min": 0, "max": 360 },
      "size": { "min": 0.5, "max": 0.9 },
      "fadeOut": true,
      "color": [120, 240, 255, 200],
      "gravity": -20,
      "blend": "alpha"
    }
  }
}
```

### ランタイム構造

- `ParticlePreset`
  - テクスチャ、`maxParticles`
  - `Emission`（burst, rate, duration）
  - `LifetimeRange`, `SpeedRange`, `AngleRange`, `SizeRange`
  - `gravity`, `drag`, `fadeOut`, `color`, `blendMode`
- `ParticleEmitterComponent`
  - `presetId`
  - `loop` or `oneShot`
  - `emitOnStart`
  - `position`, `direction`, `scale`, `tint`
- `ParticleInstance`
  - `pos`, `vel`, `life`, `maxLife`, `size`, `color`

## システム構成

### ParticleSystem

- 入力: `ParticleEmitterComponent`, `ParticlePreset`
- 処理:
  - OneShot: 生成時に `burst` を発射して終了。
  - Loop: `rate` と `duration` に従い継続発射。`loop=false` で停止指示。
  - 位置は `Transform` と合成（キャラクター追従用）。
  - `gravity` と `drag` を適用し、`life` 減算、`fadeOut` でアルファ減衰。
- 出力: `ParticleDrawCommand` を `ParticleRenderQueue` へ。

### ParticleRenderer

- `ParticleDrawCommand` をテクスチャ単位でバッチング。
- `blendMode` ごとに描画パスを切替（alpha / additive）。
- RenderTexture 上に描画し、ゲームビュー/エディタで共通利用。

### Pooling / パフォーマンス

- `ParticleInstance` を固定長プールで管理し、動的アロケーションを避ける。
- `maxParticles` で頭打ちをかけ、余剰は破棄。
- CPU削減:
  - 遠距離・非表示領域の emitter をスキップ
  - `life` の短いプリセットを推奨
- GPU:
  - 可能ならテクスチャアトラス化しドローコールを削減。

## DevMode / エディタ連携

- パーティクルプレビュー: F4 MotionEditor のビューポートでも再生できるよう、RenderQueue を共有。
- ホットリロード: `assets/effects/*.json` を監視し、`ParticlePreset` を再ロード。
- インスペクタ: `presetId`, `loop`, `scale`, `tint` を調整→即時反映。

## 代表的なプリセット案

- 斬撃: `slash_small`, `slash_big`（additive）
- 爆発: `explosion_small`, `explosion_fire`（additive）
- ヒール: `heal_pulse`, `heal_rise`（alpha）
- バフ: `buff_aura`（alpha, loop）
- デバフ: `curse_smoke`（alpha, loop, color暗）
- 被弾: `hit_spark`（burst短命）

## インターフェース

- `SpawnParticle(presetId, position, direction, scale=1.0, tint=WHITE, loop=false)`
- `AttachParticle(entity, presetId)`（Transform追従）
- `StopParticle(entity, presetId)`

## 実装ステップ（チェックリスト）

- [ ] `ParticlePreset` ローダと HotReload
- [ ] `ParticleEmitterComponent` / `ParticleSystem` / `ParticleRenderer` を実装
- [ ] プール管理を導入し、`maxParticles` を超過した場合の挙動を定義
- [ ] Additive/Alpha の描画パスを分離してドロー順を固定
- [ ] エディタでプリセットプレビューとパラメータ調整ができるようにする
- [ ] 代表プリセットを仮実装し、演出テストを行う
