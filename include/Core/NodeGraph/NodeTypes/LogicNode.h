/**
 * @file LogicNode.h
 * @brief ロジックノード（IF条件分岐）
 */
#pragma once

#include "../Node.h"
#include <iostream>

namespace Core::NodeGraph {

/**
 * @brief IF条件分岐ノード
 * 
 * 条件に基づいて実行フローを分岐する
 */
class LogicIfNode : public Node {
public:
    explicit LogicIfNode(const std::string& id)
        : Node(id, "logic_if") {
        
        // 入力ポート
        AddInputPort("trigger", PortType::Flow);
        AddInputPort("condition", PortType::Data);
        
        // 出力ポート
        AddOutputPort("true_flow", PortType::Flow);
        AddOutputPort("false_flow", PortType::Flow);
        
        // デフォルトプロパティ
        properties_ = {
            {"condition_type", "hp_below"},
            {"threshold_value", 50.0}
        };
    }
    
    NodeStatus Execute(const json& inputData) override {
        SetStatus(NodeStatus::Running);
        
        try {
            std::string conditionType = GetProperty<std::string>("condition_type", "hp_below");
            float threshold = GetProperty<float>("threshold_value", 50.0f);
            
            // 簡易的な条件評価（実際のゲーム状態と連携する）
            bool conditionMet = EvaluateCondition(conditionType, threshold, inputData);
            
            std::cout << "LogicIfNode[" << id_ << "]: Condition '" 
                     << conditionType << "' evaluated to " 
                     << (conditionMet ? "TRUE" : "FALSE") << "\n";
            
            // 条件に応じて異なる出力ポートをアクティブ化
            if (conditionMet) {
                outputs_[0].value = {{"result", true}};
            } else {
                outputs_[1].value = {{"result", false}};
            }
            
            SetStatus(NodeStatus::Completed);
            return NodeStatus::Completed;
            
        } catch (const std::exception& e) {
            std::cerr << "LogicIfNode Error: " << e.what() << "\n";
            SetStatus(NodeStatus::Error);
            return NodeStatus::Error;
        }
    }
    
    std::string GetDescription() const override {
        return "IF条件";
    }
    
    std::string GetCategory() const override {
        return "logic";
    }
    
    std::string GetColor() const override {
        return "#E2A04A";
    }
    
private:
    bool EvaluateCondition(const std::string& type, float threshold, const json& data) {
        // TODO: 実際のゲーム状態と連携
        // 現在は簡易実装
        if (type == "hp_below") {
            float hp = data.value("hp", 100.0f);
            return hp < threshold;
        } else if (type == "gold_above") {
            int gold = data.value("gold", 0);
            return gold > static_cast<int>(threshold);
        } else if (type == "wave_greater") {
            int wave = data.value("wave", 1);
            return wave > static_cast<int>(threshold);
        }
        
        return false;
    }
};

} // namespace Core::NodeGraph
