/**
 * @file NodeRegistry.h
 * @brief ノード登録・生成システム
 */
#pragma once

#include "Node.h"
#include "NodeTypes/WaveNode.h"
#include "NodeTypes/SpawnNode.h"
#include "NodeTypes/LogicNode.h"
#include <memory>
#include <unordered_map>
#include <functional>

namespace Core::NodeGraph {

/**
 * @brief ノードファクトリ型定義
 */
using NodeFactory = std::function<std::unique_ptr<Node>(const std::string&)>;

/**
 * @brief ノードレジストリ
 * 
 * 利用可能なノードタイプを登録・管理し、動的に生成する
 */
class NodeRegistry {
public:
    static NodeRegistry& GetInstance() {
        static NodeRegistry instance;
        return instance;
    }
    
    /**
     * @brief ノードタイプを登録
     * 
     * @param type ノードタイプID
     * @param factory ノード生成関数
     */
    void RegisterNodeType(const std::string& type, NodeFactory factory) {
        factories_[type] = factory;
        std::cout << "Registered node type: " << type << "\n";
    }
    
    /**
     * @brief ノードを生成
     * 
     * @param type ノードタイプID
     * @param id ノードID
     * @return 生成されたノード
     */
    std::unique_ptr<Node> CreateNode(const std::string& type, const std::string& id) {
        auto it = factories_.find(type);
        if (it == factories_.end()) {
            std::cerr << "Unknown node type: " << type << "\n";
            return nullptr;
        }
        
        return it->second(id);
    }
    
    /**
     * @brief 登録済みノードタイプ一覧を取得
     */
    std::vector<std::string> GetRegisteredTypes() const {
        std::vector<std::string> types;
        types.reserve(factories_.size());
        for (const auto& [type, _] : factories_) {
            types.push_back(type);
        }
        return types;
    }
    
    /**
     * @brief 標準ノードを登録
     */
    void RegisterStandardNodes() {
        RegisterNodeType("wave_start", [](const std::string& id) {
            return std::make_unique<WaveStartNode>(id);
        });
        
        RegisterNodeType("enemy_spawn", [](const std::string& id) {
            return std::make_unique<EnemySpawnNode>(id);
        });
        
        RegisterNodeType("logic_if", [](const std::string& id) {
            return std::make_unique<LogicIfNode>(id);
        });
    }
    
private:
    NodeRegistry() = default;
    ~NodeRegistry() = default;
    NodeRegistry(const NodeRegistry&) = delete;
    NodeRegistry& operator=(const NodeRegistry&) = delete;
    
    std::unordered_map<std::string, NodeFactory> factories_;
};

} // namespace Core::NodeGraph
