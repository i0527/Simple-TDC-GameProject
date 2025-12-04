/**
 * @file NodeGraph.h
 * @brief ノードグラフ管理クラス
 */
#pragma once

#include "Node.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace Core::NodeGraph {

/**
 * @brief ノード間の接続情報
 */
struct Connection {
    std::string id;
    std::string fromNodeId;
    std::string fromPortName;  // ← cppで使用
    std::string fromOutput;
    std::string toNodeId;
    std::string toPortName;    // ← cppで使用
    std::string toInput;
    
    json Serialize() const {
        return {
            {"id", id},
            {"from_node", fromNodeId},
            {"from_output", fromOutput},
            {"to_node", toNodeId},
            {"to_input", toInput}
        };
    }
};

/**
 * @brief ノードグラフ管理クラス
 * 
 * ノードの追加・削除・接続を管理
 */
class NodeGraph {
public:
    NodeGraph() = default;
    ~NodeGraph() = default;
    
    /**
     * @brief ノードを追加
     */
    void AddNode(std::shared_ptr<Node> node) {
        if (node) {
            nodes_[node->GetId()] = node;
        }
    }
    
    /**
     * @brief ノードを削除
     */
    bool RemoveNode(const std::string& nodeId) {
        // 接続も削除
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [&nodeId](const Connection& conn) {
                    return conn.fromNodeId == nodeId || conn.toNodeId == nodeId;
                }),
            connections_.end()
        );
        
        return nodes_.erase(nodeId) > 0;
    }
    
    /**
     * @brief ノードを取得
     */
    std::shared_ptr<Node> GetNode(const std::string& nodeId) {
        auto it = nodes_.find(nodeId);
        if (it != nodes_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief 全ノードを取得
     */
    const std::unordered_map<std::string, std::shared_ptr<Node>>& GetNodes() const {
        return nodes_;
    }
    
    /**
     * @brief ノード間を接続
     */
    std::string Connect(const std::string& fromNode, const std::string& fromOutput,
                       const std::string& toNode, const std::string& toInput) {
        std::string connId = "conn_" + std::to_string(nextConnectionId_++);
        
        connections_.push_back({
            connId,
            fromNode,
            fromOutput,
            toNode,
            toInput
        });
        
        return connId;
    }
    
    /**
     * @brief 接続を削除
     */
    bool RemoveConnection(const std::string& connectionId) {
        auto it = std::remove_if(connections_.begin(), connections_.end(),
            [&connectionId](const Connection& conn) {
                return conn.id == connectionId;
            });
        
        if (it != connections_.end()) {
            connections_.erase(it, connections_.end());
            return true;
        }
        return false;
    }
    
    /**
     * @brief 指定ノードからの接続を取得
     */
    std::vector<Connection> GetConnections(const std::string& fromNodeId) const {
        std::vector<Connection> result;
        for (const auto& conn : connections_) {
            if (conn.fromNodeId == fromNodeId) {
                result.push_back(conn);
            }
        }
        return result;
    }
    
    /**
     * @brief 全接続を取得
     */
    const std::vector<Connection>& GetAllConnections() const {
        return connections_;
    }
    
    /**
     * @brief グラフをクリア
     */
    void Clear() {
        nodes_.clear();
        connections_.clear();
        nextConnectionId_ = 0;
    }
    
    /**
     * @brief グラフをJSONにシリアライズ
     */
    json Serialize() const {
        json nodes = json::array();
        for (const auto& [id, node] : nodes_) {
            nodes.push_back(node->Serialize());
        }
        
        json connections = json::array();
        for (const auto& conn : connections_) {
            connections.push_back(conn.Serialize());
        }
        
        return {
            {"nodes", nodes},
            {"connections", connections}
        };
    }
    
private:
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes_;
    std::vector<Connection> connections_;
    int nextConnectionId_ = 0;
};

} // namespace Core::NodeGraph
