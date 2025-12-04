/**
 * @file SpawnNode.h
 * @brief 敵スポーンノード
 */
#pragma once

#include "../Node.h"
#include <iostream>

namespace Core::NodeGraph {

/**
 * @brief 敵スポーンノード
 * 
 * 指定したタイプの敵をスポーンする
 */
class EnemySpawnNode : public Node {
public:
    explicit EnemySpawnNode(const std::string& id)
        : Node(id, "enemy_spawn") {
        
        // 入力ポート
        AddInputPort("trigger", PortType::Flow);
        AddInputPort("enemy_type", PortType::Data);
        AddInputPort("count", PortType::Data);
        
        // 出力ポート
        AddOutputPort("flow", PortType::Flow);
        AddOutputPort("entities", PortType::Data);
        
        // デフォルトプロパティ
        properties_ = {
            {"enemy_type", "basic"},
            {"spawn_position", {{"x", 0}, {"y", 0}}},
            {"hp_multiplier", 1.0}
        };
    }
    
    NodeStatus Execute(const json& inputData) override {
        SetStatus(NodeStatus::Running);
        
        try {
            std::string enemyType = GetProperty<std::string>("enemy_type", "basic");
            float hpMultiplier = GetProperty<float>("hp_multiplier", 1.0f);
            
            // 入力からカウントを取得（デフォルト1）
            int count = 1;
            if (inputData.contains("enemy_count")) {
                count = inputData["enemy_count"];
            }
            
            std::cout << "EnemySpawnNode[" << id_ << "]: Spawning " 
                     << count << " enemies of type '" << enemyType << "'\n";
            
            // 出力データ準備
            outputs_[1].value = {
                {"enemy_type", enemyType},
                {"count", count},
                {"hp_multiplier", hpMultiplier}
            };
            
            SetStatus(NodeStatus::Completed);
            return NodeStatus::Completed;
            
        } catch (const std::exception& e) {
            std::cerr << "EnemySpawnNode Error: " << e.what() << "\n";
            SetStatus(NodeStatus::Error);
            return NodeStatus::Error;
        }
    }
    
    std::string GetDescription() const override {
        return "敵生成";
    }
    
    std::string GetCategory() const override {
        return "entity";
    }
    
    std::string GetColor() const override {
        return "#E24A4A";
    }
};

} // namespace Core::NodeGraph
