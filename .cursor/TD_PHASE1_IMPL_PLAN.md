# TD Webエディタ統合 - 実装優先順序

**作成日**: 2025年12月4日  
**戦略**: 段階的実装（Phase TD-1 → TD-2 → ...）  
**優先度**: 🔴 最優先

---

## 🎯 Phase TD-1: 基本APIとWebUIコンポーネント（本フェーズ）

### 1.1 C++側 修正（HTTPServer + UnifiedGame）

**ファイル**: `src/Core/HTTPServer.cpp` + `src/Application/UnifiedGame.cpp`

**追加API エンドポイント** (7個):

```
GET    /api/td/entities              # TD全エンティティ
GET    /api/td/gamestate             # ゲーム状態（HP、金、Wave）
POST   /api/td/spawn                 # 敵/味方スポーン
POST   /api/td/wave/next             # 次Wave進行
POST   /api/td/gamestate/hp          # HP設定
POST   /api/td/gamestate/gold        # 金設定
GET    /api/td/entities/:type        # 敵/味方別フィルタ
```

**実装内容**:
- [ ] TDGameScene メンバー変数公開（entities, gameState など）
- [ ] UnifiedGame に TD操作メソッド追加（GetTDEntities, SetTDHP など）
- [ ] HTTPServer に /api/td/* ルート追加

---

### 1.2 React コンポーネント実装

**新規ファイル**: `tools/webui-editor/src/components/DevTools/TD/`

```
├── TDController.tsx              # メインコンポーネント（統合ダッシュボード）
├── TDEntitySpawner.tsx           # ユニット生成UI
├── TDGameStatePanel.tsx          # ゲーム状態表示・操作
└── TDStatusMonitor.tsx           # リアルタイム状態監視
```

**機能**:
- TDゲーム基本操作ボタン（開始、停止、リセット）
- ウェーブ手動進行
- 敵ユニット一覧表示
- 味方ユニット管理
- HP・金リアルタイム表示・操作
- ゲーム状態の即座確認

---

### 1.3 WebUI統合

**ファイル**: `tools/webui-editor/src/App.tsx`

```tsx
// 開発者モード セクションに追加
<NavItem label="TDコントローラー" onClick={() => setCurrentMode('tdController')} />
```

---

## ✅ 実装チェックリスト

### C++側
- [ ] UnifiedGame::GetTDEntities()
- [ ] UnifiedGame::GetTDGameState()
- [ ] UnifiedGame::SpawnTDEntity()
- [ ] UnifiedGame::SetTDHP()
- [ ] UnifiedGame::SetTDGold()
- [ ] UnifiedGame::NextTDWave()
- [ ] HTTPServer /api/td/* ルート実装

### React側
- [ ] TDController コンポーネント
- [ ] TDEntitySpawner コンポーネント
- [ ] TDGameStatePanel コンポーネント
- [ ] TDStatusMonitor コンポーネント
- [ ] App.tsx 統合
- [ ] API呼び出し実装

### テスト
- [ ] Webからゲーム状態取得できる
- [ ] Webからゲーム操作できる
- [ ] リアルタイム状態更新される

---

## 📝 実装の進め方

### ステップ1: C++実装（30分）
1. UnifiedGame にメソッド追加
2. HTTPServer に /api/td/* ルート追加
3. ビルド・テスト

### ステップ2: React実装（1時間）
1. TDController.tsx 基本構造
2. 子コンポーネント実装
3. API呼び出し実装

### ステップ3: 統合テスト（30分）
1. Webからゲーム操作確認
2. リアルタイム状態更新確認
3. デバッグ・修正

---

**推奨実装時間**: 2時間  
**難易度**: 🟡 中（Webエディタ連携が初めて）  
**次フェーズ**: リアルタイム双方向同期 (WebSocket統合)


