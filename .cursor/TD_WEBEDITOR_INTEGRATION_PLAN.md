# TD開発集中フェーズ - Webエディタ連携計画

**開始日**: 2025年12月4日  
**優先度**: 🔴 最優先  
**方針**: TDゲーム開発に集中 + Webエディタでリアルタイム連携  
**開発環境**: 開発者モードで全て実施

---

## 🎯 目標

WebエディタからTDゲームの以下をリアルタイムで開発・調整できるようにする：

1. ✅ **ステージ定義** - ウェーブ、敵の種類、出現パターン
2. ✅ **ユニット定義** - 敵ユニット、味方ユニット、スキル
3. ✅ **ゲーム状態管理** - HP、金、ウェーブ進行
4. ✅ **リアルタイムプレビュー** - WebUIで即座に確認
5. ✅ **デバッグコンソール** - ゲーム内の状態確認・操作

---

## 📋 実装計画

### Phase TD-1: WebエディタTD統合API設計

**目標**: WebエディタからTDゲームを完全制御できるAPIを設計

**API エンドポイント**:

```
GET    /api/td/entities              # TD全エンティティ取得
GET    /api/td/entities/{id}         # エンティティ詳細
POST   /api/td/entities              # 敵/味方作成
PUT    /api/td/entities/{id}         # エンティティ更新
DELETE /api/td/entities/{id}         # エンティティ削除

GET    /api/td/stage                 # ステージ情報取得
POST   /api/td/stage/spawn           # ユニット即座スポーン
POST   /api/td/stage/wave            # ウェーブ進行制御

GET    /api/td/gamestate             # ゲーム状態（HP、金等）
POST   /api/td/gamestate/hp          # HP変更
POST   /api/td/gamestate/gold        # 金変更

GET    /api/td/preview               # リアルタイム状態プレビュー
WS     /ws/td/preview                # WebSocketリアルタイムストリーム
```

**実装ファイル**: `src/Core/HTTPServer.cpp`

**チェックリスト**:
- [ ] TD専用APIエンドポイント実装（15個以上）
- [ ] エンティティ操作API（生成・削除・修正）
- [ ] ゲーム状態制御API
- [ ] WebSocketリアルタイムストリーム

---

### Phase TD-2: Webエディタ TDコントローラー実装

**目標**: WebUIからTDゲームを操作するUIコンポーネント実装

**作成コンポーネント**:

```
tools/webui-editor/src/components/DevTools/
├── TDController.tsx                # TD操作コンポーネント（メイン）
├── TDEntityManager.tsx             # エンティティ管理
├── TDStageManager.tsx              # ステージ管理
├── TDGameStateManager.tsx          # ゲーム状態管理
└── TDRealtimePreview.tsx           # リアルタイムプレビュー
```

**機能**:

1. **TDController** - 統合制御画面
   - ゲーム操作ボタン（開始・停止・リセット）
   - ウェーブ制御（次ウェーブ・ジャンプ）
   - デバッグ情報表示

2. **TDEntityManager** - ユニット管理
   - 敵ユニット一覧表示
   - 味方ユニット管理
   - クイック生成ボタン

3. **TDStageManager** - ステージ編集
   - ステージ選択・作成
   - 敵出現パターン定義
   - JSON エディタ

4. **TDGameStateManager** - 状態管理
   - HP表示・操作
   - 金表示・操作
   - ウェーブ情報

5. **TDRealtimePreview** - プレビュー
   - ゲーム画面ライブ表示
   - 状態更新監視

**チェックリスト**:
- [ ] TDController メインコンポーネント実装
- [ ] TDEntityManager 実装
- [ ] TDStageManager 実装
- [ ] TDGameStateManager 実装
- [ ] TDRealtimePreview 実装
- [ ] 親App.tsx に統合

---

### Phase TD-3: リアルタイム同期機構

**目標**: WebエディタとTDゲームのリアルタイム双方向同期

**実装内容**:

1. **WebSocketストリーム**
   - ゲーム状態の自動送信（毎フレーム更新）
   - エンティティの追加・削除・修正イベント
   - HP・金・ウェーブ情報の更新

2. **自動リロード**
   - JSONステージ定義の変更を自動検出
   - ホットリロード機能

3. **デバッグログストリーミング**
   - ゲーム内ログをWebUIに表示
   - 重要度別フィルタ

**実装ファイル**:
- `src/Core/HTTPServer.cpp` - WebSocketルート追加
- `src/Application/TDGameScene.cpp` - イベント通知追加
- `tools/webui-editor/src/api/client.ts` - WebSocket ハンドラ

**チェックリスト**:
- [ ] WebSocketエンドポイント実装
- [ ] ゲーム側イベント通知システム
- [ ] クライアント側リスナー実装
- [ ] 双方向通信テスト

---

### Phase TD-4: 開発者デバッグ ツール

**目標**: TDゲーム開発に必要なデバッグ機能を実装

**機能**:

1. **コンソール** - ゲームコマンド実行
   ```
   spawn enemy 0 10 10        # 敵生成
   kill enemy 5               # 敵削除
   set_hp 100                 # HP設定
   set_gold 1000              # 金設定
   next_wave                  # 次ウェーブ
   ```

2. **エンティティインスペクター**
   - 全エンティティ表示
   - コンポーネント表示・編集
   - プロパティ動的変更

3. **パフォーマンスモニター**
   - FPS表示
   - メモリ使用量
   - システム処理時間

4. **ゲーム内デバッグ UI**
   - コマンドパレット（Ctrl+K）
   - デバッグメニュー（F1）

**実装ファイル**:
- `tools/webui-editor/src/components/DevTools/TDDebugConsole.tsx`
- `src/Application/TDGameScene.cpp` - コマンド処理

**チェックリスト**:
- [ ] WebUIコンソール実装
- [ ] ゲーム側コマンドパーサー実装
- [ ] デバッグコマンド 10個以上
- [ ] パフォーマンスモニター UI

---

## 🔄 開発ワークフロー

### 典型的な開発サイクル

```
1. WebUIで敵ユニット定義作成
   ↓
2. "Spawn" ボタンで即座にゲームに表示
   ↓
3. リアルタイムプレビューで動作確認
   ↓
4. パラメータ調整（WebUIで即座に反映）
   ↓
5. デバッグコンソールでテスト
   ↓
6. 完成！ JSON定義を保存
```

### 必要なUI構成

```
WebUI Main (App.tsx)
├─ Navigation Sidebar
│  └─ 「開発者モード」セクション
│     ├─ TD コントローラー ← NEW
│     ├─ TD エンティティ管理 ← NEW
│     ├─ TD ステージ管理 ← NEW
│     ├─ TD ゲーム状態 ← NEW
│     ├─ エンティティインスペクター (既存)
│     ├─ デバッグコンソール (既存)
│     └─ リアルタイムプレビュー (既存)
```

---

## 📊 実装スケジュール

| フェーズ | タスク | 予想時間 | 優先度 |
|---------|--------|---------|--------|
| TD-1 | APIデザイン + 実装 | 4時間 | 🔴 最高 |
| TD-2 | Webコンポーネント実装 | 6時間 | 🔴 最高 |
| TD-3 | リアルタイム同期 | 4時間 | 🟡 高 |
| TD-4 | デバッグツール | 3時間 | 🟡 高 |
| **合計** | | **17時間** | |

---

## 🚀 実装開始

### 次のアクション（優先順）

1. **Phase TD-1 開始** - APIエンドポイント設計・実装
   - HTTPServer.cpp に TD専用ルート追加
   - UnifiedGame.cpp に TD操作メソッド追加

2. **Phase TD-2 開始** - Webコンポーネント実装
   - TDController.tsx メインコンポーネント
   - 子コンポーネント群

3. **統合テスト** - Webからゲーム操作

---

## 📁 ファイル一覧

**修正ファイル**:
- `src/Core/HTTPServer.cpp` - TD APIエンドポイント追加
- `include/Application/UnifiedGame.h` - メソッド追加
- `src/Application/UnifiedGame.cpp` - メソッド実装
- `src/Application/TDGameScene.cpp` - イベント通知追加

**新規ファイル**:
- `tools/webui-editor/src/components/DevTools/TDController.tsx`
- `tools/webui-editor/src/components/DevTools/TDEntityManager.tsx`
- `tools/webui-editor/src/components/DevTools/TDStageManager.tsx`
- `tools/webui-editor/src/components/DevTools/TDGameStateManager.tsx`
- `tools/webui-editor/src/components/DevTools/TDRealtimePreview.tsx`
- `tools/webui-editor/src/components/DevTools/TDDebugConsole.tsx`

---

## ✅ 完了条件

- [x] Roguelike クラッシュ修正 ✅ 完了
- [ ] Phase TD-1: API実装完了
- [ ] Phase TD-2: Webコンポーネント実装完了
- [ ] Phase TD-3: リアルタイム同期完了
- [ ] Phase TD-4: デバッグツール完了
- [ ] 統合テスト合格
- [ ] WebエディタからTD全機能制御可能

---

**計画作成日**: 2025年12月4日  
**ステータス**: 🟡 計画完成、実装準備中  
**推奨開始日**: 本日（2025年12月4日）

