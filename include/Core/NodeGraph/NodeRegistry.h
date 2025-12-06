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
#include <iostream>

namespace Core::NodeGraph {

using NodeFactory = std::function<std::unique_ptr<Node>(const std::string&)>;

class NodeRegistry {
public:
    NodeRegistry() = default;
    ~NodeRegistry() = default;

    void RegisterNodeType(const std::string& type, NodeFactory factory) {
        factories_[type] = factory;
        std::cout << "Registered node type: " << type << "\n";
    }

    std::unique_ptr<Node> CreateNode(const std::string& type, const std::string& id) {
        auto it = factories_.find(type);
        if (it == factories_.end()) {
            std::cerr << "Unknown node type: " << type << "\n";
            return nullptr;
        }

        return it->second(id);
    }

    std::vector<std::string> GetRegisteredTypes() const {
        std::vector<std::string> types;
        types.reserve(factories_.size());
        for (const auto& [type, _] : factories_) {
            types.push_back(type);
        }
        return types;
    }

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
    std::unordered_map<std::string, NodeFactory> factories_;
};

} // namespace Core::NodeGraph
