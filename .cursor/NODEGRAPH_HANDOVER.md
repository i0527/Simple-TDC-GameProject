# NodeGraph システム 引き継ぎ資料

**作成日**: 2025年12月4日  
**バージョン**: 1.0  
**対象**: Cursor AI RAD開発

---

## 📋 概要

Simple-TDC-GameProjectに**ComfyUI/NodeRED風のビジュアルノードグラフシステム**を実装しました。  
TDゲームのWave設計、敵スポーン、条件分岐をビジュアルエディタで構築可能にする基盤です。

### 🎯 目的

- **WebUIデザイナーワークベンチ**の基盤実装
- C++側でノードグラフ実行エンジンを提供
- React側でビジュアルエディタUI（未実装）
- WebSocketでリアルタイム同期（未実装）

---

## 🏗️ アーキテクチャ

```
┌──────────────────────────────────────────┐
│         React ノードエディタUI           │ ← 未実装
│  (ReactFlow + WebSocket)                 │
└────────────────┬─────────────────────────┘
                 │ WebSocket
                 │ /ws/designer
┌────────────────▼─────────────────────────┐
│       HTTPServer (cpp-httplib)           │ ← 未実装
│  REST API: /api/nodes/*                  │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│      NodeGraph C++ Engine                │ ✅ 実装済み
│  ┌──────────────────────────────────┐   │
│  │ NodeGraph: グラフ管理            │   │
│  │ NodeExecutor: 実行エンジン        │   │
│  │ NodeRegistry: ノード登録          │   │
│  └──────────────────────────────────┘   │
│  ┌──────────────────────────────────┐   │
│  │ 基本ノード (3種類)                │   │
│  │ - WaveStartNode                   │   │
│  │ - EnemySpawnNode                  │   │
│  │ - LogicIfNode                     │   │
│  └──────────────────────────────────┘   │
└──────────────────────────────────────────┘
```

---

## 📁 ファイル構成

### ✅ 実装済みファイル

```
include/Core/NodeGraph/
├── Node.h                      # 基底クラス（ヘッダーオンリー）
├── NodeGraph.h                 # グラフ管理クラス（ヘッダーオンリー）
├── NodeExecutor.h              # 実行エンジン（ヘッダーオンリー）
├── NodeRegistry.h              # ノード登録システム（ヘッダーオンリー）
├── NodeTestHelper.h            # テストヘルパー（ヘッダーオンリー）
└── NodeTypes/
    ├── WaveNode.h              # Wave開始ノード
    ├── SpawnNode.h             # 敵スポーンノード
    └── LogicNode.h             # IF条件分岐ノード
```

**重要**: すべてヘッダーオンリー実装（.cppファイル不要）

### ❌ 未実装ファイル

```
src/Core/HTTPServer.cpp         # WebSocket統合（タスク5）
tools/webui-editor/
├── src/
│   ├── components/
│   │   ├── NodeCanvas.tsx      # ReactFlowキャンバス（タスク6）
│   │   ├── nodes/              # カスタムノード（タスク7）
│   │   └── MapEditor.tsx       # マップエディタ（タスク9）
│   └── hooks/
│       └── useDesignerWebSocket.ts  # WebSocket Hook（タスク8）
```

---

## 🔧 C++ API リファレンス

### 1. Node 基底クラス

```cpp
namespace Core::NodeGraph {

class Node {
public:
    // コンストラクタ
    Node(const std::string& id, const std::string& type);
    
    // 純粋仮想関数: サブクラスで実装必須
    virtual NodeStatus Execute(const json& inputData) = 0;
    
    // プロパティ操作
    void SetProperty(const std::string& key, const json& value);
    json GetProperty(const std::string& key, const json& defaultValue = json()) const;
    json GetProperties() const;
    
    // ポート管理
    void AddInputPort(const std::string& name, PortType type);
    void AddOutputPort(const std::string& name, PortType type);
    const std::vector<Port>& GetInputs() const;
    const std::vector<Port>& GetOutputs() const;
    
    // メタデータ
    virtual std::string GetDescription() const;
    virtual std::string GetCategory() const;
    virtual std::string GetColor() const;  // HEXカラー
    
    // ステータス
    NodeStatus GetStatus() const;
    void SetStatus(NodeStatus status);
    
    // シリアライゼーション
    virtual json Serialize() const;
};

} // namespace Core::NodeGraph
```

### 2. NodeGraph クラス

```cpp
class NodeGraph {
public:
    NodeGraph(const std::string& id);
    
    // ノード管理
    bool AddNode(std::unique_ptr<Node> node);
    bool RemoveNode(const std::string& nodeId);
    std::shared_ptr<Node> GetNode(const std::string& nodeId);
    std::vector<std::string> GetNodeIds() const;
    
    // 接続管理
    std::string Connect(const std::string& fromNodeId, 
                       const std::string& fromPort,
                       const std::string& toNodeId, 
                       const std::string& toPort);
    bool Disconnect(const std::string& connectionId);
    
    // メタデータ
    void SetName(const std::string& name);
    std::string GetName() const;
    
    // シリアライゼーション
    json Serialize() const;
    bool Deserialize(const json& data);
};
```

### 3. NodeExecutor クラス

```cpp
class NodeExecutor {
public:
    // グラフ実行
    bool Execute(NodeGraph* graph, const std::string& startNodeId);
    
    // 実行ログ取得
    std::vector<ExecutionLogEntry> GetExecutionLog() const;
    
    // デバッグ情報
    void PrintExecutionLog() const;
};

struct ExecutionLogEntry {
    std::string nodeId;
    NodeStatus status;
    int64_t executionTimeMs;
};
```

### 4. NodeRegistry クラス

```cpp
class NodeRegistry {
public:
    // シングルトン
    static NodeRegistry& GetInstance();
    
    // ノード登録
    void RegisterNodeType(const std::string& type, NodeFactory factory);
    void RegisterStandardNodes();  // 標準ノードを一括登録
    
    // ノード生成
    std::unique_ptr<Node> CreateNode(const std::string& type, 
                                     const std::string& id);
    
    // 登録済みタイプ一覧
    std::vector<std::string> GetRegisteredTypes() const;
};
```

---

## 💡 使用例

### 例1: シンプルなWave実行

```cpp
#include "Core/NodeGraph/NodeRegistry.h"
#include "Core/NodeGraph/NodeGraph.h"
#include "Core/NodeGraph/NodeExecutor.h"

void RunSimpleWave() {
    // 1. レジストリ初期化
    auto& registry = NodeRegistry::GetInstance();
    registry.RegisterStandardNodes();
    
    // 2. グラフ作成
    NodeGraph graph("wave_graph_1");
    graph.SetName("Wave 1 設計");
    
    // 3. Wave開始ノード作成
    auto waveNode = registry.CreateNode("wave_start", "wave_1");
    waveNode->SetProperty("wave_number", 1);
    waveNode->SetProperty("enemy_count", 10);
    waveNode->SetProperty("spawn_interval", 2.0);
    
    // 4. 敵スポーンノード作成
    auto spawnNode = registry.CreateNode("enemy_spawn", "spawn_1");
    spawnNode->SetProperty("enemy_type", "basic");
    spawnNode->SetProperty("hp_multiplier", 1.0);
    
    // 5. グラフに追加
    std::string waveId = waveNode->GetId();
    std::string spawnId = spawnNode->GetId();
    
    graph.AddNode(std::move(waveNode));
    graph.AddNode(std::move(spawnNode));
    
    // 6. 接続
    graph.Connect(waveId, "flow", spawnId, "trigger");
    
    // 7. 実行
    NodeExecutor executor;
    executor.Execute(&graph, waveId);
    
    // 8. 結果確認
    auto log = executor.GetExecutionLog();
    for (const auto& entry : log) {
        std::cout << entry.nodeId << ": " 
                  << entry.executionTimeMs << "ms\n";
    }
}
```

### 例2: 条件分岐（ボス戦）

```cpp
void RunBossWave() {
    auto& registry = NodeRegistry::GetInstance();
    registry.RegisterStandardNodes();
    
    NodeGraph graph("boss_wave");
    
    // HP判定ノード
    auto ifNode = registry.CreateNode("logic_if", "hp_check");
    ifNode->SetProperty("condition_type", "hp_below");
    ifNode->SetProperty("threshold_value", 50.0);
    
    // 通常敵スポーン
    auto normalSpawn = registry.CreateNode("enemy_spawn", "normal_spawn");
    normalSpawn->SetProperty("enemy_type", "basic");
    
    // ボススポーン
    auto bossSpawn = registry.CreateNode("enemy_spawn", "boss_spawn");
    bossSpawn->SetProperty("enemy_type", "boss");
    bossSpawn->SetProperty("hp_multiplier", 5.0);
    
    // グラフ構築
    graph.AddNode(std::move(ifNode));
    graph.AddNode(std::move(normalSpawn));
    graph.AddNode(std::move(bossSpawn));
    
    // 接続: HP < 50 → ボス、それ以外 → 通常
    graph.Connect("hp_check", "true_flow", "boss_spawn", "trigger");
    graph.Connect("hp_check", "false_flow", "normal_spawn", "trigger");
    
    // 実行（HP=30でテスト）
    NodeExecutor executor;
    executor.Execute(&graph, "hp_check");
}
```

### 例3: JSONシリアライゼーション

```cpp
void SaveLoadGraph() {
    // グラフ作成
    NodeGraph graph("save_test");
    auto waveNode = registry.CreateNode("wave_start", "w1");
    waveNode->SetProperty("wave_number", 5);
    graph.AddNode(std::move(waveNode));
    
    // 保存
    json serialized = graph.Serialize();
    std::ofstream file("graphs/wave_5.json");
    file << serialized.dump(2);
    file.close();
    
    // 読み込み
    std::ifstream loadFile("graphs/wave_5.json");
    json loadedData = json::parse(loadFile);
    
    NodeGraph loadedGraph("loaded");
    loadedGraph.Deserialize(loadedData);
    
    // 確認
    auto* node = loadedGraph.GetNode("w1");
    int waveNum = node->GetProperty("wave_number", 0);
    std::cout << "Loaded Wave: " << waveNum << "\n";
}
```

---

## 🎨 カスタムノード作成ガイド

### ステップ1: ヘッダーファイル作成

`include/Core/NodeGraph/NodeTypes/YourNode.h`:

```cpp
#pragma once

#include "../Node.h"
#include <iostream>

namespace Core::NodeGraph {

class YourCustomNode : public Node {
public:
    explicit YourCustomNode(const std::string& id)
        : Node(id, "your_custom") {
        
        // 入力ポート定義
        AddInputPort("trigger", PortType::Flow);
        AddInputPort("param1", PortType::Data);
        
        // 出力ポート定義
        AddOutputPort("flow", PortType::Flow);
        AddOutputPort("result", PortType::Data);
        
        // デフォルトプロパティ
        properties_ = {
            {"your_property", "default_value"}
        };
    }
    
    NodeStatus Execute(const json& inputData) override {
        SetStatus(NodeStatus::Running);
        
        try {
            // プロパティ取得
            std::string prop = GetProperty("your_property", "default");
            
            // ロジック実装
            std::cout << "Executing YourCustomNode: " << prop << "\n";
            
            // 出力データ設定
            outputs_[1].value = {
                {"output_key", "output_value"}
            };
            
            SetStatus(NodeStatus::Completed);
            return NodeStatus::Completed;
            
        } catch (const std::exception& e) {
            std::cerr << "YourCustomNode Error: " << e.what() << "\n";
            SetStatus(NodeStatus::Error);
            return NodeStatus::Error;
        }
    }
    
    std::string GetDescription() const override {
        return "カスタムノードの説明";
    }
    
    std::string GetCategory() const override {
        return "custom";  // カテゴリ
    }
    
    std::string GetColor() const override {
        return "#FF6B6B";  // 表示カラー（HEX）
    }
};

} // namespace Core::NodeGraph
```

### ステップ2: NodeRegistryに登録

```cpp
// NodeRegistry.hの RegisterStandardNodes() に追加
void RegisterStandardNodes() {
    RegisterNodeType("wave_start", [](const std::string& id) {
        return std::make_unique<WaveStartNode>(id);
    });
    
    // ... 既存ノード ...
    
    // 新規ノード追加
    RegisterNodeType("your_custom", [](const std::string& id) {
        return std::make_unique<YourCustomNode>(id);
    });
}
```

---

## 🧪 テスト実行

### テストヘルパー使用

```cpp
#include "Core/NodeGraph/NodeTestHelper.h"

int main() {
    // 全テスト実行
    Core::NodeGraph::NodeTestHelper::RunAllTests();
    
    // 個別テスト
    // Core::NodeGraph::NodeTestHelper::TestSimpleGraph();
    // Core::NodeGraph::NodeTestHelper::TestCircularReference();
    
    return 0;
}
```

### 期待される出力

```
=== NodeGraph System Tests ===

WaveStartNode[wave_1]: Starting Wave 1 with 5 enemies
EnemySpawnNode[spawn_1]: Spawning 5 enemies of type 'basic'
✓ SimpleGraph test passed
Execution log (2 entries):
  - wave_1: 4 (12ms)
  - spawn_1: 4 (8ms)

LogicIfNode[if_1]: Condition 'hp_below' evaluated to TRUE
✓ ConditionalGraph test passed

✓ Serialization test passed

NodeExecutor: Circular reference detected at node 'n1'
✓ CircularReference detection passed

=== Test Results: 4/4 passed ===
```

---

## 🔮 次のステップ（Cursorで実装）

### タスク5: HTTPServer WebSocket統合

**ファイル**: `src/Core/HTTPServer.cpp`

```cpp
// 実装すべきエンドポイント

// WebSocket
httpServer.Get("/ws/designer", [](const Request& req, Response& res) {
    // WebSocket接続確立
    // ノードグラフリアルタイム同期
});

// REST API
httpServer.Get("/api/nodes/types", [](const Request& req, Response& res) {
    auto& registry = NodeRegistry::GetInstance();
    auto types = registry.GetRegisteredTypes();
    
    json response;
    for (const auto& type : types) {
        auto node = registry.CreateNode(type, "temp");
        response.push_back({
            {"type", type},
            {"description", node->GetDescription()},
            {"category", node->GetCategory()},
            {"color", node->GetColor()}
        });
    }
    
    res.set_content(response.dump(), "application/json");
});

httpServer.Post("/api/graphs/:id/execute", [](const Request& req, Response& res) {
    // グラフ実行APIの実装
});
```

### タスク6-8: React ノードエディタ

**必要なライブラリ**:

```bash
cd tools/webui-editor
npm install reactflow
npm install @types/reactflow
```

**NodeCanvas.tsx** (基本構造):

```tsx
import ReactFlow, { 
    Node, 
    Edge, 
    Controls, 
    Background 
} from 'reactflow';
import 'reactflow/dist/style.css';

export const NodeCanvas = () => {
    const [nodes, setNodes] = useState<Node[]>([]);
    const [edges, setEdges] = useState<Edge[]>([]);
    
    // WebSocket接続
    const ws = useDesignerWebSocket('ws://localhost:8080/ws/designer');
    
    // ノードタイプ定義
    const nodeTypes = {
        wave_start: WaveStartNodeComponent,
        enemy_spawn: EnemySpawnNodeComponent,
        logic_if: LogicIfNodeComponent,
    };
    
    return (
        <ReactFlow
            nodes={nodes}
            edges={edges}
            nodeTypes={nodeTypes}
            onNodesChange={onNodesChange}
            onEdgesChange={onEdgesChange}
            onConnect={onConnect}
        >
            <Controls />
            <Background />
        </ReactFlow>
    );
};
```

---

## 📊 データフォーマット

### ノードJSON形式

```json
{
    "id": "wave_1",
    "type": "wave_start",
    "category": "game_flow",
    "color": "#4A90E2",
    "description": "Wave開始",
    "properties": {
        "wave_number": 1,
        "enemy_count": 10,
        "spawn_interval": 2.0
    },
    "status": 4,
    "inputs": [
        {
            "name": "trigger",
            "type": 0,
            "is_output": false
        }
    ],
    "outputs": [
        {
            "name": "flow",
            "type": 0,
            "is_output": true
        },
        {
            "name": "wave_data",
            "type": 1,
            "is_output": true
        }
    ]
}
```

### グラフJSON形式

```json
{
    "id": "wave_graph_1",
    "name": "Wave 1 設計",
    "nodes": [
        { /* ノードオブジェクト */ },
        { /* ノードオブジェクト */ }
    ],
    "connections": [
        {
            "id": "wave_1:flow->spawn_1:trigger",
            "from_node": "wave_1",
            "from_output": "flow",
            "to_node": "spawn_1",
            "to_input": "trigger"
        }
    ]
}
```

---

## ⚠️ 既知の制限事項

1. **WebSocket未実装**: C++側とReact側の通信が未完成
2. **GameContext統合**: ノード実行がゲーム状態に反映されない
3. **ノード種類**: 現在3種類のみ（Wave/Spawn/Logic）
4. **エラーハンドリング**: try-catch基本実装のみ
5. **パフォーマンス**: 大規模グラフ未検証

---

## 🎯 推奨実装順序（Cursor作業）

### Phase 1: API統合（推定2時間）

1. HTTPServer.cppにREST API実装
2. /api/nodes/typesエンドポイント
3. /api/graphs/:id/executeエンドポイント

### Phase 2: WebSocket基盤（推定3時間）

4. WebSocketライブラリ統合（websocketpp or Beast）
5. /ws/designerエンドポイント実装
6. リアルタイムメッセージングプロトコル定義

### Phase 3: React UI（推定5時間）

7. ReactFlow統合
8. NodeCanvasコンポーネント作成
9. カスタムノードコンポーネント10種類
10. useDesignerWebSocket Hook実装

### Phase 4: マップエディタ（推定4時間）

11. MapEditor.tsxコンポーネント
12. グリッドキャンバス実装
13. パス描画ツール（ベジェ曲線）

### Phase 5: テスト・統合（推定2時間）

14. E2Eテストシナリオ実行
15. パフォーマンステスト（<500msレイテンシー）
16. ドキュメント更新

**合計推定**: 16時間

---

## 📚 参考リソース

### 既存ドキュメント

- `.cursor/TD_PHASE1_DESIGNER_WORKBENCH.md` - 詳細設計書（2200+行）
- `docs/DEVELOPER_MANUAL.md` - 開発者マニュアル
- `.github/copilot-instructions.md` - コーディング規約

### ライブラリドキュメント

- [ReactFlow公式](https://reactflow.dev/)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [nlohmann/json](https://github.com/nlohmann/json)

### 類似プロジェクト参考

- [ComfyUI](https://github.com/comfyanonymous/ComfyUI) - ノードベースUI
- [NodeRED](https://nodered.org/) - フロープログラミング

---

## 💬 質問・トラブルシューティング

### Q1: ノード追加後にビルドエラー

**A**: すべてヘッダーオンリー実装。.cppファイルを作成しないこと。

### Q2: GetProperty()で型エラー

**A**: テンプレート版を使用:

```cpp
int value = GetProperty<int>("key", 0);
float value = GetProperty<float>("key", 0.0f);
```

### Q3: WebSocket接続失敗

**A**: HTTPServer.cppで`/ws/designer`エンドポイント実装を確認。

### Q4: ReactFlowでノードが表示されない

**A**: `nodeTypes`に正しくコンポーネント登録されているか確認。

---

## ✅ チェックリスト（Cursor作業前）

- [ ] CMakeビルド成功確認 (`cmake --build build --config Release`)
- [ ] NodeTestHelper全テスト成功
- [ ] HTTPServer.cpp現状確認
- [ ] tools/webui-editor/ディレクトリ確認
- [ ] TD_PHASE1_DESIGNER_WORKBENCH.md熟読

---

**作成者**: GitHub Copilot + VSCode  
**最終更新**: 2025年12月4日  
**ライセンス**: プロジェクトライセンスに準拠

---

## 🚀 即座に試せるコマンド

```bash
# プロジェクトルートで実行

# ビルド
cmake --build build --config Release

# テスト実行（main_unified.cppにテストコード追加後）
.\build\bin\Release\SimpleTDCGame.exe

# WebUI起動（HTTPServer実装後）
.\build\bin\Release\SimpleTDCGame.exe --webui

# React開発サーバー（UI実装後）
cd tools/webui-editor
npm run dev
```

---

**次回Cursor起動時のプロンプト例**:

> "NodeGraph システムのHTTPServer WebSocket統合を実装してください。  
> `.cursor/NODEGRAPH_HANDOVER.md`のタスク5を参照。  
> `/ws/designer`エンドポイントと`/api/nodes/*` REST APIを実装します。"

---

End of Document.
