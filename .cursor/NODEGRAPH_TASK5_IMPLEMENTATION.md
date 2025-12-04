# NodeGraph システム HTTPServer WebSocket統合 - 実装レポート

**実装日**: 2025年12月4日  
**対象タスク**: タスク5（HTTPServer WebSocket統合）  
**引き継ぎ資料**: `.cursor/NODEGRAPH_HANDOVER.md`  
**ステータス**: 🚀 実装開始

---

## 📋 実装概要

NodeGraph C++ エンジン（ノード実装済み）をHTTPサーバー経由でWebUIから制御できるようにする。

### 実装対象API

1. **REST API**:
   - `GET /api/nodes/types` - 登録済みノードタイプ一覧
   - `POST /api/graphs` - グラフ作成
   - `GET /api/graphs/:id` - グラフ取得
   - `POST /api/graphs/:id/execute` - グラフ実行
   - `DELETE /api/graphs/:id` - グラフ削除

2. **WebSocket**:
   - `/ws/designer` - リアルタイムノードグラフ同期

---

## 🔧 実装仕様

### 1. REST API: `/api/nodes/types` (GET)

**レスポンス** (200 OK):
```json
{
  "success": true,
  "types": [
    {
      "type": "wave_start",
      "description": "Wave開始",
      "category": "game_flow",
      "color": "#4A90E2",
      "inputs": [
        {
          "name": "trigger",
          "type": 0,
          "description": "開始トリガー"
        }
      ],
      "outputs": [
        {
          "name": "flow",
          "type": 0,
          "description": "実行フロー"
        },
        {
          "name": "wave_data",
          "type": 1,
          "description": "Wave情報"
        }
      ]
    },
    // enemy_spawn, logic_if も同様...
  ]
}
```

---

### 2. REST API: `/api/graphs` (POST)

**リクエスト**:
```json
{
  "id": "wave_graph_1",
  "name": "Wave 1 設計"
}
```

**レスポンス** (201 Created):
```json
{
  "success": true,
  "graph": {
    "id": "wave_graph_1",
    "name": "Wave 1 設計",
    "nodes": [],
    "connections": []
  }
}
```

---

### 3. REST API: `/api/graphs/:id/execute` (POST)

**リクエスト**:
```json
{
  "start_node_id": "wave_1"
}
```

**レスポンス** (200 OK):
```json
{
  "success": true,
  "execution_log": [
    {
      "node_id": "wave_1",
      "status": 4,
      "execution_time_ms": 12
    },
    {
      "node_id": "spawn_1",
      "status": 4,
      "execution_time_ms": 8
    }
  ],
  "total_time_ms": 20
}
```

---

### 4. WebSocket: `/ws/designer`

**接続後のメッセージプロトコル**:

```json
// クライアント → サーバー: ノード追加
{
  "type": "node_add",
  "graph_id": "wave_graph_1",
  "node": {
    "id": "wave_1",
    "node_type": "wave_start",
    "properties": {
      "wave_number": 1,
      "enemy_count": 5
    }
  }
}

// サーバー → クライアント: 確認
{
  "type": "node_added",
  "graph_id": "wave_graph_1",
  "node_id": "wave_1"
}

// クライアント → サーバー: 接続作成
{
  "type": "connection_add",
  "graph_id": "wave_graph_1",
  "from_node": "wave_1",
  "from_output": "flow",
  "to_node": "spawn_1",
  "to_input": "trigger"
}

// クライアント → サーバー: グラフ実行
{
  "type": "execute",
  "graph_id": "wave_graph_1",
  "start_node_id": "wave_1"
}

// サーバー → クライアント: 実行完了通知
{
  "type": "execution_complete",
  "graph_id": "wave_graph_1",
  "log": [ /* 実行ログ */ ]
}
```

---

## 📁 実装対象ファイル

### 修正ファイル

1. **`src/Core/HTTPServer.cpp`**
   - `SetupNodeGraphRoutes()` メソッド追加
   - `/api/nodes/*` エンドポイント実装
   - `/ws/designer` WebSocket実装
   - `SetupRoutes()` に統合

2. **`include/Core/HTTPServer.h`**
   - ノードグラフ関連メンバー変数追加
   - メソッド宣言追加

---

## 🚀 実装アプローチ

### ステップ1: メモリ管理
```cpp
// HTTPServer内でグラフを管理
std::map<std::string, std::unique_ptr<Core::NodeGraph::NodeGraph>> graphs_;
std::mutex graphsMutex_;
```

### ステップ2: REST APIルート追加
```cpp
// /api/nodes/types
impl_->server->Get("/api/nodes/types", 
    [this](const httplib::Request& req, httplib::Response& res) {
        // NodeRegistry::GetInstance().GetRegisteredTypes()から
        // 全ノードタイプを列挙
    });

// /api/graphs/:id/execute
impl_->server->Post("/api/graphs/:id/execute",
    [this](const httplib::Request& req, httplib::Response& res) {
        // グラフ取得 → NodeExecutor::Execute() → ログ返却
    });
```

### ステップ3: WebSocket実装
```cpp
// cpp-httplibはWebSocket非対応のため、以下の選択肢がある：
// 1. websocketpp ライブラリを別途統合
// 2. SSE (Server-Sent Events) で代用
// 3. httplib v0.13以降の試験的WebSocket対応を利用

// 当面は HTTP ポーリング + イベント通知で実装
impl_->server->Get("/api/graphs/:id/events",
    [this](const httplib::Request& req, httplib::Response& res) {
        // グラフの変更イベントをJSON配列で返却
    });
```

---

## ✅ 完了チェックリスト

### 実装前チェック
- [ ] CMakeビルド成功確認
- [ ] NodeTestHelper全テスト成功
- [ ] HTTPServer現状確認

### API実装
- [ ] `/api/nodes/types` 実装・テスト
- [ ] `/api/graphs` (POST) 実装・テスト
- [ ] `/api/graphs/:id` (GET) 実装・テスト
- [ ] `/api/graphs/:id/execute` 実装・テスト
- [ ] `/api/graphs/:id` (DELETE) 実装・テスト

### WebSocket/イベント
- [ ] WebSocket実装方式決定
- [ ] イベント通知機構実装
- [ ] エラーハンドリング実装

### テスト
- [ ] cURLで各APIテスト
- [ ] ログ記録・出力確認
- [ ] エッジケース対応（重複ID、存在しないグラフ等）

---

## 📚 関連資料

- **引き継ぎ資料**: `.cursor/NODEGRAPH_HANDOVER.md` (タスク5参照)
- **クイックリファレンス**: `docs/NODEGRAPH_QUICK_REFERENCE.md`
- **APIドキュメント**: 本レポート上記セクション

---

**推奨実装時間**: 2-3時間  
**難易度**: 🟡 中（HTTPライブラリの制限理解が鍵）


