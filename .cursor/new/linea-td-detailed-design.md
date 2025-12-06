# 直線型TD 詳細設計（横スクロール特化）

> 目的: にゃんこ／アクナイツ風の横スクロールTDにおけるレーン管理、ウェーブ進行、ユニット配置/コスト、敵進軍、UI連携を詳細化する。

## ゴール

- 複数レーンを持つ横スクロールTDのゲームループを定義。
- JSONでウェーブ/敵/ユニット/コストを定義し、エディタで編集可能にする。
- UI（ユニット選択、コスト表示、ウェーブ表示）とゲームロジックを疎結合に統合。

## コア概念

- **レーン**: y座標またはレーンIDで識別。地上/空中を別レーンにできる。
- **拠点ライン**: 敵が到達するとライフを減らす終端X座標。
- **前線位置**: 味方/敵が押し合った結果の境界。近接攻撃は前線を基準に行う。
- **コスト**: 時間で回復するリソース。ユニット配置に消費。
- **ウェーブ**: スポーンイベントの集合。中ボス/ボスを含む。

## データモデル（JSON例）

```json
{
  "lanes": [
    { "id": 0, "y": 540, "type": "ground" },
    { "id": 1, "y": 420, "type": "air" }
  ],
  "cost": { "max": 50, "start": 10, "regenPerSec": 2.5 },
  "waves": [
    {
      "id": 1,
      "startTime": 0.0,
      "spawns": [
        { "time": 0.0, "lane": 0, "enemyId": "slime_s", "count": 3, "interval": 1.0 },
        { "time": 5.0, "lane": 0, "enemyId": "slime_m", "count": 1 },
        { "time": 8.0, "lane": 1, "enemyId": "bat_s", "count": 2, "interval": 0.5 }
      ],
      "boss": false
    },
    {
      "id": 2,
      "startTime": 25.0,
      "spawns": [
        { "time": 0.0, "lane": 0, "enemyId": "slime_boss", "count": 1 }
      ],
      "boss": true
    }
  ],
  "unitDeck": [
    { "slot": 0, "unitId": "soldier_s", "cost": 8, "cooldown": 1.5 },
    { "slot": 1, "unitId": "archer_s", "cost": 10, "cooldown": 2.0 }
  ]
}
```

## コンポーネント

- `LaneComponent`（全エンティティに付与）
  - `laneId`, `laneType`（ground/air）
- `PositionComponent`
  - `x`, `y`, `direction`（1 = 右→左, -1 = 左→右）
- `MoveComponent`
  - `speed`, `stopAtFront`（前線で停止）、`pushPriority`
- `CostComponent`（ゲーム全体で1つのリソース保持エンティティ）
  - `current`, `max`, `regenPerSec`
- `SpawnRequestComponent`
  - `unitId/enemyId`, `laneId`, `count`, `interval`, `direction`
- `WaveStateComponent`
  - `currentWaveId`, `elapsed`, `isBossWave`, `waves[]`
- `PlacementCooldownComponent`
  - `slot`, `remain`

## システム

### LaneManagerSystem

- `lanes[]` をロードし、`laneId -> y座標` をテーブル化。
- レーンタイプに応じた移動・衝突フィルタを提供（地上 vs 空中）。

### WaveSchedulerSystem

- `WaveStateComponent` を持つゲームエンティティで運用。
- `startTime` と `spawns` を時間経過でキューイングし、`SpawnRequestComponent` を発行。
- `boss` フラグを UI に通知（ボスアイコン表示用イベント）。

### SpawnSystem

- `SpawnRequestComponent` を消化し、敵/ユニットを生成。
- 生成時に `LaneComponent` と `PositionComponent` を付与。
- 進行方向: 敵は `direction = -1`（右→左）、味方は `direction = 1`（左→右）。

### FrontlineSystem（押し合い判定）

- 各レーンで味方/敵の最前線x位置を計算。
- 近接ユニットは前線より先に進まないよう `MoveComponent` を停止。
- 前線の差で攻撃レンジ内かを AttackSystem に提供。

### MoveSystem

- `speed * direction` で x を更新。停止状態のユニットは移動しない。
- 画面外に出た敵: ライフ減算イベント発火（拠点ダメージ）→削除。

### AttackSystem

- 近接: 前線を基準に `range` 内なら攻撃。
- 遠隔: `projectileSpawnOffset` から `Bullet` を生成。
- `combat-collision-design.md` の HitEmitter/Bullet と連携。

### CostSystem

- `regenPerSec` で `current` を回復（`max` まで）。
- ユニット配置時にコストを消費、足りない場合は失敗イベントを UI に送る。
- `PlacementCooldownComponent` を更新し、スロットごとの再配置待ちを管理。

### UI連携（イベント）

- `CostChanged(current, max)`
- `WaveProgress(currentWave, total, isBoss)`
- `UnitPlacementResult(success, reason)`
- `BossIncoming(waveId)`
- DevMode: ウェーブタイムラインを編集し保存する場合は `.cursor/new/` の新規エディタ要件に追加で対応。

## レーンと衝突の簡易フィルタ

- `LaneComponent.laneId` が一致するもののみ衝突候補。
- 空中/地上で別レーンに分けることで衝突計算を削減。
- 近接判定: 前線を超えないよう `FrontlineSystem` で x をクランプ。

## UI要素（推奨）

- コストバー: `current / max`、回復アニメーション。
- デッキスロット: 各ユニットのコストとクールダウン表示。配置不可時はグレイアウト。
- ウェーブ表示: 現在ウェーブ番号、残り敵数、次のボス予告。
- 速度・ポーズ: テスト用に 0.5x/1x/2x/ポーズをトグル。

## エディタ連携

- Wave/Spawn JSON は StageEditor(F3) で編集できるようにする（リスト＋タイムライン）。
- レーン設定（y, type）とコストパラメータも StageEditor で編集。
- ホットリロード: JSON更新→`WaveSchedulerSystem` と `LaneManager` が再ロード。

## 実装ステップ（チェックリスト）

- [ ] `lanes` `waves` `cost` JSON をロードするレジストリを実装
- [ ] `LaneManagerSystem` と `WaveSchedulerSystem` を追加
- [ ] `SpawnSystem`（ユニット/敵生成）を実装
- [ ] `FrontlineSystem` と `MoveSystem` を実装し、押し合いと到達判定を制御
- [ ] `AttackSystem` を `combat-collision-design.md` の判定ロジックと接続
- [ ] `CostSystem` と `PlacementCooldown` 管理を実装
- [ ] UIイベントを発火し、UI側と疎結合に連携
- [ ] StageEditor(F3) でウェーブ/レーン/コストを編集・保存できるよう拡張
