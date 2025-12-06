# オーディオシステム設計

> 目的: BGM/SE を JSON 定義で管理し、ゲーム状態・演出と同期できる軽量なサウンドシステムを整備する。

## ゴール

- BGM: ステージ開始/ボス/勝利/敗北でトラックを切替、クロスフェード対応。
- SE: 攻撃・被弾・UI操作など多数同時再生をプールで効率化。
- DevMode: 音量・ミュート・テスト再生を即時操作。

## データモデル（JSON例）

```json
{
  "bgm": {
    "stage_normal": { "path": "assets/audio/bgm/stage_normal.ogg", "volume": 0.8, "loop": true },
    "stage_boss":   { "path": "assets/audio/bgm/stage_boss.ogg",   "volume": 0.9, "loop": true },
    "result_win":   { "path": "assets/audio/bgm/result_win.ogg",   "volume": 0.9, "loop": false },
    "result_lose":  { "path": "assets/audio/bgm/result_lose.ogg",  "volume": 0.8, "loop": false }
  },
  "se": {
    "attack_slash": { "path": "assets/audio/se/attack_slash.wav", "volume": 0.9 },
    "hit_body":     { "path": "assets/audio/se/hit_body.wav",     "volume": 0.9 },
    "ui_click":     { "path": "assets/audio/se/ui_click.wav",     "volume": 0.6 }
  }
}
```

## コンポーネント

- `AudioListenerComponent`
  - 原点を決める（2Dなので1つで十分）。距離減衰を使う場合に参照。
- `AudioSourceComponent`（SE向け）
  - `clipId`, `volume`, `loop`, `spatial`（2D/3D）、`position`
  - `autoDestroy`（再生完了で削除）
- `BgmStateComponent`
  - `currentTrackId`, `targetTrackId`, `fadeTime`, `state`（Playing/Fading）

## システム

### AudioManager (サービス)

- JSON をロードし、`AudioClip` をキャッシュ。
- `PlayBgm(trackId, fadeTime)` / `StopBgm(fadeTime)`
- `PlaySe(clipId, volumeScale=1.0, position?, spatial?)`
- グローバル音量: `master`, `bgm`, `se`。
- HotReload: JSON 更新を監視し、クリップ設定を再読込。

### BgmSystem

- `BgmStateComponent` を監視し、`targetTrackId` が変わればクロスフェード。
- クロスフェード: `fadeTime` で current を減衰し、target を増加。
- 状態遷移フック:
  - `Playing` へ入る: `stage_normal`
  - `BossIncoming`: `stage_boss`
  - `Victory`: `result_win`
  - `Defeat`: `result_lose`

### SeSystem

- `AudioSourceComponent` をキューし、プールからプレーヤを取得して再生。
- 同一 SE の多重再生を許可（プール上限で打ち切り）。
- 距離減衰（必要なら）: 2D で簡易的に距離に応じて音量低減。

## DevMode / UI

- スライダー: `master/bgm/se` 音量。
- トグル: ミュート、ループ。
- テスト再生ボタン: 任意の `clipId` を鳴らす。
- ログ: 再生失敗時にコンソール表示。

## インターフェース

- `AudioManager::PlayBgm(id, fadeSec=0.5f)`
- `AudioManager::StopBgm(fadeSec=0.5f)`
- `AudioManager::PlaySe(id, volumeScale=1.0f)`
- `AudioManager::SetVolume(master, bgm, se)`
- `AudioManager::ReloadDefinitions()`（HotReload用）

## 実装ステップ（チェックリスト）

- [ ] JSONローダとクリップキャッシュを実装（BGM/SEを区分）
- [ ] `AudioManager` API を提供し、DI で必要システムに注入
- [ ] `BgmSystem` を実装し、状態イベント（BossIncoming/Victory/Defeat）でトラック切替
- [ ] `SeSystem` を実装し、プール再生＋上限処理を行う
- [ ] DevMode UI に音量・テスト再生・リロードを追加
- [ ] HotReload で定義変更を即時反映
