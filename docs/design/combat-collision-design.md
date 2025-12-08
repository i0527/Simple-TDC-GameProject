# 戦闘・判定システム設計（横スクロールTD特化）

> 目的: にゃんこ／アクナイツ風の横スクロールTD向けに、ダメージ計算・衝突判定・範囲攻撃・デバッグ可視化を標準化する。

## ゴール

- ユニット/敵/弾丸の衝突判定を統一し、JSON定義と紐づくコンポーネントで管理する。
- ダメージ計算（基礎攻撃力、属性、クリティカル、バフ・デバフ）をシステムで一元化。
- 範囲攻撃（扇形/矩形/円）と貫通・多段ヒットを扱える。
- デバッグ表示で当たり判定・攻撃範囲を可視化。

## コンポーネント設計

- `HealthComponent`
  - `hp`, `maxHp`, `shield`, `def`, `resist`, `isDead`
- `AttackComponent`
  - `attack`, `critChance`, `critMultiplier`, `attackInterval`, `lastAttackTime`
  - `rangeType`（Melee, Ranged）
  - `targetMask`（Enemy, Ally, Neutral）
- `DamageModifierComponent`（バフ/デバフ）
  - `flatAttack`, `pctAttack`, `flatDef`, `pctDef`, `flatResist`, `pctResist`
  - `dot`（継続ダメージ）: `dps`, `duration`, `tickInterval`
  - `hot`（回復）: `hps`, `duration`, `tickInterval`
- `CollisionShapeComponent`
  - `shapeType`（Circle, AABB）
  - `radius` / `halfExtents`
  - `offset`
- `HitboxComponent`（受け）
  - `CollisionShapeComponent` を参照
  - `team`（Ally/Enemy/Neutral）
  - `hurtbox` フラグ（被弾可否）
- `HitEmitterComponent`（攻撃側の当たり生成）
  - `shape`（Circle/Rect/Sector）
  - `pierceCount`（貫通回数: -1 = 無制限）
  - `maxHitsPerTarget`（多段制限）
  - `lifetime`
  - `spawnInterval`（多段HIT間隔）
  - `factionMask`（対象チーム）
  - `damageProfileId`（JSON参照）
- `BulletComponent`（飛翔体）
  - `velocity`, `gravity`, `maxDistance`, `pierceCount`
  - `hitEmitterRef`（衝突時に使用する HitEmitter）
  - `onHitClip`（アニメ/SE 触発用 ID）

## JSON定義（例）

```json
{
  "damageProfiles": {
    "basic_slash": {
      "baseAttack": 120,
      "critChance": 0.1,
      "critMultiplier": 1.5,
      "attribute": "physical",
      "defPenetration": 0.1,
      "resistPenetration": 0.0,
      "status": { "bleed": { "dps": 10, "duration": 3.0 } }
    }
  },
  "hitEmitters": {
    "sword_melee": {
      "shape": { "type": "Rect", "halfExtents": [90, 60], "offset": [80, 0] },
      "pierceCount": 1,
      "maxHitsPerTarget": 1,
      "lifetime": 0.08,
      "damageProfileId": "basic_slash",
      "factionMask": "Enemy"
    }
  }
}
```

## システム

### CollisionDetectionSystem

- 入力: `HitEmitterComponent`（発生中の攻撃判定）、`HitboxComponent`
- 処理:
  - ブロードフェーズ: レーン / セル単位での簡易フィルタ（横スクロールでは x 範囲とレーンID）
  - ナローフェーズ: Circle vs Circle / AABB vs AABB / Circle vs AABB 交差判定
  - 1フレームで `maxHitsPerTarget` を超えないよう管理
- 出力: `HitEvent { attacker, target, damageProfileId, hitPos }`

### DamageSystem

- 入力: `HitEvent`, `HealthComponent`, `DamageModifierComponent`
- 計算:
  - `effectiveAtk = (baseAttack + flatAttack) * (1 + pctAttack)`
  - 防御減算: `damage = max(1, effectiveAtk - def*(1 - defPenetration))`
  - 属性/耐性: `damage *= (1 - resist*(1 - resistPenetration))`
  - クリティカル: `if rand < critChance -> damage *= critMultiplier`
  - バフ/デバフ: DOT/HOT の付与・更新
- 出力: HP更新、`DeathEvent` 発火（hp <= 0）

### BulletSystem

- 入力: `BulletComponent`, `HitEmitterComponent`
- 処理:
  - 位置更新（速度＋重力）
  - `maxDistance` 超過で消滅
  - 衝突した場合は `HitEmitter` を起動し、自身を削除（または pierceCount--）

### AoESystem（範囲攻撃）

- `HitEmitterComponent` の `shape` が Circle/Rect/Sector の場合、位置基準で範囲判定を実施
- 多段HIT (`spawnInterval > 0`) に対応するため、`lifetime` 内で一定間隔で HitEvent を生成

### StatusEffectSystem

- DOT/HOT を tick 単位で適用（`tickInterval`）
- デバフ: 攻撃力/防御力/移動速度/攻撃速度などを一時的に変更
- 解除: duration 経過または `Cleanse` イベント

## デバッグ可視化

- `DebugCollisionRenderer`:
  - `HitEmitter` の形状を色付きで描画（攻撃=赤、回復=緑、バフ=青）。
  - `Hitbox` をワイヤーフレームで表示。
  - フラグ: `showCollision`, `showAoE`, `showBulletPath` を DevMode メニューに配置。
- `EventLog`:
  - `HitEvent`, `DeathEvent`, `StatusApplyEvent` をコンソール/オンスクリーンに表示（DevMode限定）。

## TD（横スクロール）特有の考慮

- レーンID: 同一yレーン内のみ当たり判定を行う（省コスト）。空中/地上を別レーン扱いにする。
- 押し合い: 近接同士が衝突した際の停止判定（Position制御で行い、Hitboxは攻撃のみ）。
- 画面外処理: 画面外に出た敵は Hitbox を無効化しつつ、到達判定（拠点ダメージ）を発火。

## インターフェース（イベント）

- `HitEvent(attacker, target, damageProfileId, pos)`
- `DeathEvent(entity)`
- `StatusApplyEvent(entity, statusId)`

## 実装ステップ（チェックリスト）

- [ ] `CollisionShapeComponent` / `HitboxComponent` / `HitEmitterComponent` / `BulletComponent` を定義
- [ ] `CollisionDetectionSystem`（ブロード＋ナロー）を実装
- [ ] `DamageSystem` でダメージ計算とステータス反映
- [ ] `BulletSystem` / `AoESystem` を実装し、範囲・貫通・多段に対応
- [ ] `StatusEffectSystem` で DOT/HOT/バフ・デバフを適用
- [ ] デバッグ可視化を DevMode メニューに統合
- [ ] JSONロードと HotReload で `damageProfiles` / `hitEmitters` を再読み込み可能にする
