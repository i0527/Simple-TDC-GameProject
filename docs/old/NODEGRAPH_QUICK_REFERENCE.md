# NodeGraph システム クイックリファレンス

**最終更新**: 2025年12月4日

## 🚀 クイックスタート（5分）

### 1. 最小限のコード

```cpp
#include "Core/NodeGraph/NodeRegistry.h"
#include "Core/NodeGraph/NodeGraph.h"
#include "Core/NodeGraph/NodeExecutor.h"

int main() {
    // レジストリ初期化（アプリ起動時に1回）
    auto& registry = NodeRegistry::GetInstance();
    registry.RegisterStandardNodes();
    
    // グラフ作成
    NodeGraph graph("my_first_graph");
    
    // ノード作成・追加
    auto waveNode = registry.CreateNode("wave_start", "wave_1");
    waveNode->SetProperty("wave_number", 1);
    graph.AddNode(std::move(waveNode));
    
    // 実行
    NodeExecutor executor;
    executor.Execute(&graph, "wave_1");
    
    return 0;
}
```

### 2. ビルド確認

```bash
cmake --build build --config Release
.\build\bin\Release\SimpleTDCGame.exe
```

---

## 📦 利用可能なノード

| ノードタイプ | ID | 用途 | 入力 | 出力 |
|------------|----|----|------|------|
| Wave開始 | `wave_start` | Wave開始 | `trigger`, `previous_wave` | `flow`, `wave_data` |
| 敵スポーン | `enemy_spawn` | 敵生成 | `trigger`, `enemy_type`, `count` | `flow`, `entities` |
| IF条件 | `logic_if` | 条件分岐 | `trigger`, `condition` | `true_flow`, `false_flow` |

### プロパティ一覧

#### WaveStartNode

```cpp
SetProperty("wave_number", 1);        // Wave番号 (int)
SetProperty("enemy_count", 10);       // 敵数 (int)
SetProperty("spawn_interval", 2.0f);  // スポーン間隔秒 (float)
```

#### EnemySpawnNode

```cpp
SetProperty("enemy_type", "basic");             // 敵タイプ (string)
SetProperty("hp_multiplier", 1.0f);             // HP倍率 (float)
SetProperty("spawn_position", {{"x", 0}, {"y", 0}});  // 座標 (json)
```

#### LogicIfNode

```cpp
SetProperty("condition_type", "hp_below");  // 条件タイプ (string)
SetProperty("threshold_value", 50.0f);      // 閾値 (float)
```

**条件タイプ**:

- `"hp_below"` - HP < threshold
- `"gold_above"` - Gold > threshold
- `"wave_greater"` - Wave > threshold

---

## 🔗 接続パターン

### パターン1: 線形（Wave → Spawn）

```cpp
graph.Connect("wave_1", "flow", "spawn_1", "trigger");
```

### パターン2: 分岐（IF → True/False）

```cpp
graph.Connect("if_1", "true_flow", "boss_spawn", "trigger");
graph.Connect("if_1", "false_flow", "normal_spawn", "trigger");
```

### パターン3: データ渡し

```cpp
// wave_1のwave_dataをspawn_1のenemy_typeに接続
graph.Connect("wave_1", "wave_data", "spawn_1", "enemy_type");
```

---

## 💾 保存・読み込み

### JSON保存

```cpp
NodeGraph graph("my_graph");
// ... ノード追加 ...

json data = graph.Serialize();
std::ofstream file("graphs/my_graph.json");
file << data.dump(2);  // インデント2でフォーマット
file.close();
```

### JSON読み込み

```cpp
std::ifstream file("graphs/my_graph.json");
json data = json::parse(file);

NodeGraph graph("loaded");
graph.Deserialize(data);

// 実行
NodeExecutor executor;
executor.Execute(&graph, "start_node_id");
```

---

## 🧪 デバッグ

### 実行ログ確認

```cpp
NodeExecutor executor;
executor.Execute(&graph, "start_node");

auto log = executor.GetExecutionLog();
for (const auto& entry : log) {
    std::cout << entry.nodeId << ": "
              << (entry.status == NodeStatus::Completed ? "OK" : "FAIL")
              << " (" << entry.executionTimeMs << "ms)\n";
}
```

### ノード状態確認

```cpp
auto* node = graph.GetNode("wave_1");
std::cout << "Status: " << static_cast<int>(node->GetStatus()) << "\n";
std::cout << "Type: " << node->GetType() << "\n";
std::cout << "Props: " << node->GetProperties().dump(2) << "\n";
```

---

## ⚡ パフォーマンス

- **小規模グラフ** (<10ノード): <10ms
- **中規模グラフ** (10-50ノード): <50ms
- **大規模グラフ** (50+ノード): 未検証

---

## ❗ よくあるエラー

### エラー1: "Unknown node type"

```
原因: NodeRegistryに登録されていないノードタイプ
解決: RegisterStandardNodes()を呼び出す
```

### エラー2: "Node with id 'xxx' already exists"

```
原因: 同じIDのノードを2回追加
解決: ノードIDをユニークにする（例: "wave_1", "wave_2"）
```

### エラー3: "Circular reference detected"

```
原因: ノードが自分自身に戻る接続（無限ループ）
解決: グラフ構造を確認し、循環を削除
```

### エラー4: 型変換エラー

```cpp
// ❌ 悪い例
int value = GetProperty("count", 10);  // json型を返す

// ✅ 良い例
int value = GetProperty<int>("count", 10);  // テンプレート版を使用
```

---

## 🎯 ベストプラクティス

### 1. ノードIDは説明的に

```cpp
// ❌ 悪い例
auto node = registry.CreateNode("wave_start", "n1");

// ✅ 良い例
auto node = registry.CreateNode("wave_start", "wave_boss_intro");
```

### 2. プロパティ設定は型を明示

```cpp
// ✅ 良い例
node->SetProperty("count", 10);          // int
node->SetProperty("multiplier", 1.5f);   // float
node->SetProperty("name", "enemy");      // string
```

### 3. エラーハンドリング

```cpp
auto node = registry.CreateNode("wave_start", "w1");
if (!node) {
    std::cerr << "Failed to create node\n";
    return;
}
```

### 4. スマートポインタ活用

```cpp
// グラフに追加後はnodeはnullptr
graph.AddNode(std::move(node));

// グラフから取得
auto* rawNode = graph.GetNode("w1");  // 生ポインタ
if (rawNode) {
    rawNode->SetProperty("key", "value");
}
```

---

## 📊 JSONフォーマット例

### ノード単体

```json
{
    "id": "wave_1",
    "type": "wave_start",
    "category": "game_flow",
    "color": "#4A90E2",
    "properties": {
        "wave_number": 1,
        "enemy_count": 10
    },
    "status": 4,
    "inputs": [
        {"name": "trigger", "type": 0, "is_output": false}
    ],
    "outputs": [
        {"name": "flow", "type": 0, "is_output": true},
        {"name": "wave_data", "type": 1, "is_output": true}
    ]
}
```

### グラフ全体

```json
{
    "id": "wave_graph_1",
    "name": "Wave 1 設計",
    "nodes": [
        { /* WaveStartNode */ },
        { /* EnemySpawnNode */ }
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

## 🔧 カスタマイズ

### 新しいノードタイプ追加

1. `include/Core/NodeGraph/NodeTypes/MyNode.h` 作成
2. `Node`クラスを継承
3. `Execute()`メソッド実装
4. `NodeRegistry::RegisterStandardNodes()`に登録

詳細は `.cursor/NODEGRAPH_HANDOVER.md` 参照。

---

## 📚 関連ドキュメント

- **詳細設計**: `.cursor/NODEGRAPH_HANDOVER.md` (包括的ガイド)
- **TD設計書**: `.cursor/TD_PHASE1_DESIGNER_WORKBENCH.md` (2200行)
- **開発マニュアル**: `docs/DEVELOPER_MANUAL.md`

---

**次の一歩**: `.cursor/NODEGRAPH_HANDOVER.md` を読み、WebSocket統合を始める。
