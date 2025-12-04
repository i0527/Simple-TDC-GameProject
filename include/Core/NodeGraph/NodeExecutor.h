/**
 * @file NodeExecutor.h
 * @brief ノードグラフ実行エンジン
 */
#pragma once

#include "NodeGraph.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <set>

namespace Core::NodeGraph {

/**
 * @brief 実行ログエントリ
 */
struct ExecutionLogEntry {
    std::string nodeId;
    NodeStatus status;
    int64_t executionTimeMs;
    json outputData;
    
    json Serialize() const {
        return {
            {"node_id", nodeId},
            {"status", static_cast<int>(status)},
            {"execution_time_ms", executionTimeMs},
            {"output", outputData}
        };
    }
};

/**
 * @brief ノードグラフ実行エンジン
 * 
 * グラフの実行、デバッグ、ステップ実行を管理
 */
class NodeExecutor {
public:
    explicit NodeExecutor(NodeGraph& graph) : graph_(graph) {}
    
    /**
     * @brief グラフを実行
     * @param startNodeId 開始ノードID（nullptrの場合は最初のノード）
     * @param debugMode デバッグモード有効化
     */
    void Execute(const std::string& startNodeId = "", bool debugMode = false) {
        debugMode_ = debugMode;
        executionLog_.clear();
        visitedNodes_.clear();
        
        // 開始ノードを取得
        std::shared_ptr<Node> startNode;
        if (!startNodeId.empty()) {
            startNode = graph_.GetNode(startNodeId);
        } else {
            // 開始ノードが指定されていない場合、最初のノードを取得
            const auto& nodes = graph_.GetNodes();
            if (!nodes.empty()) {
                startNode = nodes.begin()->second;
            }
        }
        
        if (!startNode) {
            std::cerr << "NodeExecutor: Start node not found\n";
            return;
        }
        
        // 実行開始
        ExecuteNode(startNode, json());
    }
    
    /**
     * @brief ノードを実行（再帰的）
     */
    void ExecuteNode(std::shared_ptr<Node> node, const json& inputData) {
        if (!node) return;
        
        // 循環参照チェック
        if (visitedNodes_.count(node->GetId()) > 0) {
            std::cerr << "NodeExecutor: Circular reference detected at node " 
                     << node->GetId() << "\n";
            return;
        }
        visitedNodes_.insert(node->GetId());
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // ノード実行
        NodeStatus status = node->Execute(inputData);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime).count();
        
        // デバッグログ記録
        if (debugMode_) {
            json outputData = json::object();
            const auto& outputs = node->GetOutputs();
            if (!outputs.empty() && outputs[0].type == PortType::Data) {
                outputData = outputs[0].value;
            }
            
            executionLog_.push_back({
                node->GetId(),
                status,
                duration,
                outputData
            });
        }
        
        // 成功時は次のノードを実行
        if (status == NodeStatus::Completed) {
            auto connections = graph_.GetConnections(node->GetId());
            
            for (const auto& conn : connections) {
                auto nextNode = graph_.GetNode(conn.toNodeId);
                if (nextNode) {
                    // 出力データを次ノードに渡す
                    json outputData = json::object();
                    const auto& outputs = node->GetOutputs();
                    if (!outputs.empty()) {
                        outputData = outputs[0].value;
                    }
                    ExecuteNode(nextNode, outputData);
                }
            }
        }
        
        visitedNodes_.erase(node->GetId());
    }
    
    /**
     * @brief 実行を停止
     */
    void Stop() {
        // 実装は今後追加（バックグラウンド実行時用）
    }
    
    /**
     * @brief 実行ログを取得
     */
    std::vector<ExecutionLogEntry> GetExecutionLog() const {
        return executionLog_;
    }
    
    /**
     * @brief 実行ログをJSON化
     */
    json GetExecutionLogJson() const {
        json logs = json::array();
        for (const auto& entry : executionLog_) {
            logs.push_back(entry.Serialize());
        }
        return logs;
    }
    
private:
    NodeGraph& graph_;
    bool debugMode_ = false;
    std::vector<ExecutionLogEntry> executionLog_;
    std::set<std::string> visitedNodes_;  // 循環参照検出用
};

} // namespace Core::NodeGraph
