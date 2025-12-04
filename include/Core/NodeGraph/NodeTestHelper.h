/**
 * @file NodeTestHelper.h
 * @brief ノードグラフテスト用ヘルパークラス
 */
#pragma once

#include "NodeGraph.h"
#include "NodeExecutor.h"
#include "NodeRegistry.h"
#include <cassert>

namespace Core::NodeGraph {

/**
 * @brief ノードグラフテストヘルパー
 * 
 * 開発・デバッグ用のテストユーティリティ
 */
class NodeTestHelper {
public:
    /**
     * @brief 簡単なグラフを作成してテスト
     */
    static bool TestSimpleGraph() {
        // レジストリ初期化
        auto& registry = NodeRegistry::GetInstance();
        registry.RegisterStandardNodes();
        
        // グラフ作成
        NodeGraph graph("test_graph");
        
        // Wave開始ノード作成
        auto waveNode = registry.CreateNode("wave_start", "wave_1");
        if (!waveNode) {
            std::cerr << "Failed to create wave_start node\n";
            return false;
        }
        waveNode->SetProperty("wave_number", 1);
        waveNode->SetProperty("enemy_count", 5);
        
        // Spawn ノード作成
        auto spawnNode = registry.CreateNode("enemy_spawn", "spawn_1");
        if (!spawnNode) {
            std::cerr << "Failed to create enemy_spawn node\n";
            return false;
        }
        
        // グラフに追加
        std::string waveId = waveNode->GetId();
        std::string spawnId = spawnNode->GetId();
        
        graph.AddNode(std::move(waveNode));
        graph.AddNode(std::move(spawnNode));
        
        // 接続
        std::string connId = graph.Connect(waveId, "flow", spawnId, "trigger");
        if (connId.empty()) {
            std::cerr << "Failed to connect nodes\n";
            return false;
        }
        
        // 実行
        NodeExecutor executor;
        bool result = executor.Execute(&graph, waveId);
        
        if (result) {
            std::cout << "✓ SimpleGraph test passed\n";
            
            // ログ表示
            auto log = executor.GetExecutionLog();
            std::cout << "Execution log (" << log.size() << " entries):\n";
            for (const auto& entry : log) {
                std::cout << "  - " << entry.nodeId << ": " 
                         << static_cast<int>(entry.status) 
                         << " (" << entry.executionTimeMs << "ms)\n";
            }
        } else {
            std::cerr << "✗ SimpleGraph test failed\n";
        }
        
        return result;
    }
    
    /**
     * @brief 条件分岐テスト
     */
    static bool TestConditionalGraph() {
        auto& registry = NodeRegistry::GetInstance();
        registry.RegisterStandardNodes();
        
        NodeGraph graph("conditional_test");
        
        // ロジックノード作成
        auto ifNode = registry.CreateNode("logic_if", "if_1");
        if (!ifNode) return false;
        
        ifNode->SetProperty("condition_type", "hp_below");
        ifNode->SetProperty("threshold_value", 50.0);
        
        std::string ifId = ifNode->GetId();
        graph.AddNode(std::move(ifNode));
        
        // 実行（HP=30でテスト）
        NodeExecutor executor;
        json testData = {{"hp", 30.0}};
        
        auto* node = graph.GetNode(ifId);
        if (!node) return false;
        
        NodeStatus status = node->Execute(testData);
        
        if (status == NodeStatus::Completed) {
            std::cout << "✓ ConditionalGraph test passed\n";
            return true;
        } else {
            std::cerr << "✗ ConditionalGraph test failed\n";
            return false;
        }
    }
    
    /**
     * @brief シリアライゼーションテスト
     */
    static bool TestSerialization() {
        auto& registry = NodeRegistry::GetInstance();
        registry.RegisterStandardNodes();
        
        NodeGraph graph("serialize_test");
        graph.SetName("Test Graph");
        
        auto waveNode = registry.CreateNode("wave_start", "wave_s1");
        waveNode->SetProperty("wave_number", 3);
        graph.AddNode(std::move(waveNode));
        
        // シリアライズ
        json serialized = graph.Serialize();
        
        // デシリアライズ
        NodeGraph loadedGraph("loaded");
        bool success = loadedGraph.Deserialize(serialized);
        
        if (success) {
            auto* node = loadedGraph.GetNode("wave_s1");
            if (node) {
                int waveNum = node->GetProperty("wave_number", 0);
                if (waveNum == 3) {
                    std::cout << "✓ Serialization test passed\n";
                    return true;
                }
            }
        }
        
        std::cerr << "✗ Serialization test failed\n";
        return false;
    }
    
    /**
     * @brief 循環参照検出テスト
     */
    static bool TestCircularReference() {
        auto& registry = NodeRegistry::GetInstance();
        registry.RegisterStandardNodes();
        
        NodeGraph graph("circular_test");
        
        auto node1 = registry.CreateNode("wave_start", "n1");
        auto node2 = registry.CreateNode("enemy_spawn", "n2");
        auto node3 = registry.CreateNode("wave_start", "n3");
        
        graph.AddNode(std::move(node1));
        graph.AddNode(std::move(node2));
        graph.AddNode(std::move(node3));
        
        // 循環参照作成: n1 -> n2 -> n3 -> n1
        graph.Connect("n1", "flow", "n2", "trigger");
        graph.Connect("n2", "flow", "n3", "trigger");
        graph.Connect("n3", "flow", "n1", "trigger");
        
        NodeExecutor executor;
        bool result = executor.Execute(&graph, "n1");
        
        // 循環参照検出で失敗することを期待
        if (!result) {
            std::cout << "✓ CircularReference detection passed\n";
            return true;
        } else {
            std::cerr << "✗ CircularReference detection failed (should detect cycle)\n";
            return false;
        }
    }
    
    /**
     * @brief 全テスト実行
     */
    static void RunAllTests() {
        std::cout << "\n=== NodeGraph System Tests ===\n\n";
        
        int passed = 0;
        int total = 4;
        
        if (TestSimpleGraph()) passed++;
        if (TestConditionalGraph()) passed++;
        if (TestSerialization()) passed++;
        if (TestCircularReference()) passed++;
        
        std::cout << "\n=== Test Results: " << passed << "/" << total << " passed ===\n";
    }
};

} // namespace Core::NodeGraph
