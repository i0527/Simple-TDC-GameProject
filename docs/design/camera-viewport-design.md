# カメラ・ビューポート設計

> 目的: 仮想FHDレンダリング上でのカメラ制御（スクロール/ズーム）を標準化し、横スクロールTDやマップ型TDに対応する。

## ゴール

- 仮想FHD (1920x1080) を基準に、レーン型（横スクロール）とマップ型双方でカメラ操作を統一。
- パララックス/ズーム/スクロールをサポートし、UIと座標変換を一貫化。
- DevMode のエディタ（F1〜F4）から同じカメラ計算を利用。

## コンポーネント

- `CameraComponent`
  - `position` (world space, center)
  - `zoom` (1.0 = 1:1 to virtual FHD)
  - `minZoom`, `maxZoom`
  - `bounds`（表示可能領域: AABB）
  - `followTarget`（entityId, optional）
  - `lerp`（位置補間速度）
- `ParallaxLayerComponent`
  - `layerId`, `parallax`（例: 0.5, 0.8, 1.0）
  - `texture` / `speed`（自動スクロール用）

## システム

### CameraSystem

- 入力: `CameraComponent`, `Input`（DevMode: middle-drag, wheel）
- 処理:
  - スクロール: ドラッグで `position` を移動（ズームに応じて速度を調整）。
  - ズーム: ホイールで `zoom` を変更し、`minZoom`〜`maxZoom` にクランプ。
  - フォロー: `followTarget` があれば補間で追従。
  - 境界: `bounds` にクランプして背景抜けを防止。
- 出力: `ViewMatrix`/`ProjMatrix` を VirtualRenderer に渡す。

### ParallaxSystem

- `ParallaxLayerComponent` を持つ背景を、`camera.position * parallax` でスクロール。
- ループタイル式で背景を繰り返し配置。
- スクロール速度を持つ場合は `speed` 分を加算。

### CoordinateConversion

- `WorldToScreen(worldPos)`: 仮想FHD座標への変換。
- `ScreenToWorld(screenPos)`: 入力座標をワールドへ逆変換。
- UI との整合: カメラ矩形を UI に通知してヒットテストを一致させる。

## 横スクロールTDでの使い方

- カメラは基本的に x 軸スクロールのみ。y はレーン中央に固定または僅かに追従。
- 進軍方向に合わせて緩やかに前方を映す（先読み）。
- ボス登場時にズームアウトして全体を見せる演出を用意。

## マップ型TDでの使い方

- 2Dグリッド全体を移動・ズーム。選択セルは `ScreenToWorld` で取得。
- ビルドモード中はズーム/ドラッグ有効、プレイ中は自動パン＋軽いズームアウト。
- 道路/タイルに合わせたスナップを提供。

## DevMode（F1〜F4）連携

- エディタビュー（Viewport）もゲームと同じ `CameraSystem` を利用し、入力だけ差し替え。
- カメラパラメータを HUD に表示（pos/zoom/bounds）。
- リセットボタンで `position` を初期値（ステージ中心）へ戻す。

## 実装ステップ（チェックリスト）

- [ ] `CameraComponent` / `CameraSystem` を実装し、スクロール・ズーム・フォロー・境界クランプを対応
- [ ] `ParallaxLayerComponent` / `ParallaxSystem` を実装
- [ ] `WorldToScreen` / `ScreenToWorld` をユーティリティ化し、UIと共通化
- [ ] 横スクロールTD用のカメラデフォルト挙動（xのみスクロール）を設定
- [ ] マップ型TD用のドラッグ/ズーム/スナップを設定
- [ ] DevModeのViewport入力をカメラへ橋渡しする（リセット・HUD含む）
