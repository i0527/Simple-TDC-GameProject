# 共通イベント設計（簡易版）

## 目的

- ゲームイベントとDevイベントの通知ルートを整理し、優先度と通知手段（ログ/トースト）を共有する。

## イベント分類と通知

- ゲーム系:  
  - `wave_started`, `wave_finished`（INFO, トースト）  
  - `boss_spawned`（WARN, トースト＋SE＋振動小）  
  - `player_hit`, `player_down`（INFO, ログ。UIトーストは任意）  
  - `victory`, `defeat`（INFO, トースト＋結果画面）  
  - `timescale_changed`（DEBUG/INFO, ログのみ）
- システム/データ系:  
  - `hotreload_ok`, `hotreload_partial`, `hotreload_error`（INFO/WARN/ERROR, Devトースト＋ログ）  
  - `schema_validation_failed`（ERROR, Devトースト＋ログ）
- Dev系:  
  - `dev_command_executed`（DEBUG, ログのみ）※Dev/Debugビルド限定。

## 優先度とキュー

- 優先度高: boss_spawned, hotreload_error, schema_validation_failed  
- 中: wave_started/finished, victory/defeat  
- 低: player_hit/down, timescale_changed  
- 同一イベントの連続発火はクールダウンで集約。

## 通知手段

- ログ: `[HH:MM:SS.mmm][LEVEL][Event] name payload`  
- UIトースト: 2.5s目安、優先度で並び替え、重複は集約。  
- 振動: 設定がONかつイベントが要求する場合のみ（boss_spawned=小）。

## 非対応

- リプレイ/ネット送信はスコープ外（オフライン想定）。
