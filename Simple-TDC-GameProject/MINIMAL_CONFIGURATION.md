# 最小構成仕様書

## 概要
このプロジェクトは、タイトル・ホーム・バトルの3シーン構成に簡略化されています。
その他のシーンは仮実装として最小限の機能のみを提供します。

## コアシーン（完全実装）

### 1. TitleScene（タイトルシーン）
- **ファイル**: `game/scenes/TitleScene.h`, `game/scenes/TitleScene.cpp`
- **機能**: ゲームの起動画面、メニュー選択
- **状態**: 完全実装

### 2. HomeScene（ホームシーン）
- **ファイル**: `game/scenes/HomeScene.h`, `game/scenes/HomeScene.cpp`
- **機能**: メインメニュー、各機能への遷移
- **状態**: 完全実装

### 3. TDGameScene（タワーディフェンスゲームシーン）
- **ファイル**: `game/scenes/TDGameScene.h`, `game/scenes/TDGameScene.cpp`
- **機能**: 実際のゲームプレイ（タワーディフェンス）
- **状態**: 完全実装
- **注意**: 文字化けエラーがある場合は、UTF-8エンコーディングで保存してください

## 仮実装シーン

### 1. StageSelectScene（ステージ選択シーン）
- **ファイル**: `game/scenes/StageSelectScene.h`, `game/scenes/StageSelectScene.cpp`
- **状態**: 仮実装
- **動作**:
  - 最初のステージを自動的に選択
  - 0.5秒後に自動的にゲーム開始
  - SPACE/ENTERで即座に開始
  - ESCで戻る

### 2. FormationScene（編成シーン）
- **ファイル**: `game/scenes/FormationScene.h`, `game/scenes/FormationScene.cpp`
- **状態**: 仮実装
- **動作**:
  - デフォルト編成を使用
  - 1秒後に自動的にホームに戻る
  - SPACE/ENTER/ESCで即座に戻る

### 3. LoadingScene（ローディングシーン）
- **ファイル**: `game/scenes/LoadingScene.h`, `game/scenes/LoadingScene.cpp`
- **状態**: 仮実装
- **動作**:
  - タスクを即座に実行
  - 最小表示時間（0.3秒）後に完了
  - シンプルなスピナー表示

### 4. GameScene（旧ゲームシーン）
- **ファイル**: `game/scenes/GameScene.h`, `game/scenes/GameScene.cpp`
- **状態**: 非推奨（仮実装）
- **動作**:
  - 非推奨メッセージを表示
  - TDGameSceneの使用を推奨
  - 2秒後に自動的に戻る
  - SPACE/ENTER/ESCで即座に戻る

## シーンフロー

```
TitleScene（タイトル）
    ↓
HomeScene（ホーム）
    ├→ StageSelectScene（仮実装）→ TDGameScene（バトル）
    ├→ FormationScene（仮実装）→ HomeScene
    └→ その他の機能（今後実装）
```

## 開発ガイドライン

### 仮実装シーンの拡張方法

1. **StageSelectScene**
   - ステージリスト表示
   - ステージ情報表示
   - スクロール機能
   - フィルタリング機能

2. **FormationScene**
   - キャラクター選択UI
   - ドラッグ&ドロップ編成
   - キャラクター詳細表示
   - 強化機能

3. **LoadingScene**
   - 進捗バー表示
   - ロード状況メッセージ
   - 非同期ロード

### 注意事項

- 仮実装シーンは、ゲームの基本フローを妨げない最小限の機能のみを提供します
- 完全実装への移行時は、対応するヘッダーファイルとソースファイルを拡張してください
- 文字化けエラーが発生した場合は、ファイルをUTF-8（BOM無し）で保存してください

## ビルド方法

```bash
# CMake設定
cmake --preset vs2022-debug

# ビルド
cmake --build build --config Debug
```

## 既知の問題

1. **文字化けエラー**
   - 一部のファイル（TDGameScene.cpp、SettingsPanel.cppなど）で文字化けが発生する可能性があります
   - 解決方法: Visual StudioでファイルをUTF-8（BOM無し）で保存し直してください

2. **インクルードパス**
   - 古いディレクトリ構造からのインクルードパスが残っている場合があります
   - 新しいパス構造を使用してください：
     - `Data/` → `managers/`
     - `Game/Systems/` → `ecs/systems/`
     - `Shared/Simulation/` → `utils/`

## 更新履歴

- 2025-01-XX: 最小構成への整理完了
  - StageSelectScene、FormationScene、LoadingScene、GameSceneを仮実装化
  - TitleScene、HomeScene、TDGameSceneをコアシーンとして維持
