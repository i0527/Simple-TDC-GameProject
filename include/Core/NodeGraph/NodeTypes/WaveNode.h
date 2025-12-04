/**
 * @file WaveNode.h
 * @brief Wave開始ノード
 */
#pragma once

#include "../Node.h"
#include <iostream>

namespace Core::NodeGraph {

/**
 * @brief Wave開始ノード
 * 
 * ゲームのWaveを開始し、敵数やスポーン間隔を設定する
 */
class WaveStartNode : public Node {
public:
    explicit WaveStartNode(const std::string& id)
        : Node(id, "wave_start") {
        
        // 入力ポート
        AddInputPort("trigger", PortType::Flow);
        AddInputPort("previous_wave", PortType::Data);
        
        // 出力ポート
        AddOutputPort("flow", PortType::Flow);
        AddOutputPort("wave_data", PortType::Data);
        
        // デフォルトプロパティ
        properties_ = {
            {"wave_number", 1},
            {"enemy_count", 10},
            {"spawn_interval", 2.0}
        };
    }
    
    NodeStatus Execute(const json& inputData) override {
        SetStatus(NodeStatus::Running);
        
        try {
            int waveNumber = GetProperty<int>("wave_number", 1);
            int enemyCount = GetProperty<int>("enemy_count", 10);
            float spawnInterval = GetProperty<float>("spawn_interval", 2.0f);
            
            std::cout << "WaveStartNode[" << id_ << "]: Starting Wave " 
                     << waveNumber << " with " << enemyCount << " enemies\n";
            
            // 出力データ準備
            outputs_[1].value = {
                {"wave_number", waveNumber},
                {"enemy_count", enemyCount},
                {"spawn_interval", spawnInterval}
            };
            
            SetStatus(NodeStatus::Completed);
            return NodeStatus::Completed;
            
        } catch (const std::exception& e) {
            std::cerr << "WaveStartNode Error: " << e.what() << "\n";
            SetStatus(NodeStatus::Error);
            return NodeStatus::Error;
        }
    }
    
    std::string GetDescription() const override {
        return "Wave開始";
    }
    
    std::string GetCategory() const override {
        return "game_flow";
    }
    
    std::string GetColor() const override {
        return "#4A90E2";
    }
};

} // namespace Core::NodeGraph
