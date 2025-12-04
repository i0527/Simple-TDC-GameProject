/**
 * @file Node.h
 * @brief ノードグラフシステムの基底ノードクラス
 * 
 * ComfyUI/NodeRED風のビジュアルプログラミング用ノード定義
 */
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Core::NodeGraph {

/**
 * @brief ポート種別
 */
enum class PortType {
    Flow,    // 実行フロー
    Data,    // データ
    Event    // イベント
};

/**
 * @brief ノード実行状態
 */
enum class NodeStatus {
    Idle,       // 待機中
    Running,    // 実行中
    Completed,  // 完了
    Error,      // エラー
    Skipped     // スキップ
};

/**
 * @brief 入出力ポート
 */
struct Port {
    std::string name;
    PortType type;
    bool isOutput;
    json value;  // 現在の値

    json Serialize() const {
        return {
            {"name", name},
            {"type", static_cast<int>(type)},
            {"is_output", isOutput}
        };
    }
};

/**
 * @brief ノード基底クラス
 * 
 * すべてのノードはこのクラスを継承して実装する
 */
class Node {
public:
    Node(const std::string& id, const std::string& type)
        : id_(id), type_(type), status_(NodeStatus::Idle) {}
    
    virtual ~Node() = default;
    
    /**
     * @brief ノード実行（サブクラスで実装）
     * @param inputData 入力データ
     * @return 実行結果ステータス
     */
    virtual NodeStatus Execute(const json& inputData) = 0;
    
    /**
     * @brief ノードの説明を取得
     */
    virtual std::string GetDescription() const {
        return "Base Node";
    }
    
    /**
     * @brief ノードのカテゴリを取得
     */
    virtual std::string GetCategory() const {
        return "general";
    }
    
    /**
     * @brief ノードの表示カラーを取得
     */
    virtual std::string GetColor() const {
        return "#808080";
    }
    
    // プロパティ取得・設定
    virtual json GetProperties() const { return properties_; }
    
    virtual void SetProperty(const std::string& key, const json& value) {
        properties_[key] = value;
    }
    
    json GetProperty(const std::string& key, const json& defaultValue = json()) const {
        if (properties_.contains(key)) {
            return properties_[key];
        }
        return defaultValue;
    }
    
    /**
     * @brief 型安全なプロパティ取得
     * @tparam T 取得する型
     * @param key プロパティキー
     * @param defaultValue デフォルト値
     * @return T型の値
     */
    template<typename T>
    T GetProperty(const std::string& key, const T& defaultValue) const {
        try {
            if (properties_.contains(key)) {
                return properties_[key].get<T>();
            }
        } catch (const json::exception& e) {
            std::cerr << "GetProperty type conversion error [" << key 
                     << "]: " << e.what() << "\n";
        }
        return defaultValue;
    }
    
    // 入出力ポート管理
    void AddInputPort(const std::string& name, PortType type) {
        inputs_.push_back({name, type, false, json()});
    }
    
    void AddOutputPort(const std::string& name, PortType type) {
        outputs_.push_back({name, type, true, json()});
    }
    
    const std::vector<Port>& GetInputs() const { return inputs_; }
    const std::vector<Port>& GetOutputs() const { return outputs_; }
    
    std::vector<Port>& GetMutableOutputs() { return outputs_; }
    
    // ステータス管理
    NodeStatus GetStatus() const { return status_; }
    void SetStatus(NodeStatus status) { status_ = status; }
    
    // ID・型取得
    std::string GetId() const { return id_; }
    std::string GetType() const { return type_; }
    
    // シリアライゼーション
    virtual json Serialize() const {
        json inputs = json::array();
        for (const auto& port : inputs_) {
            inputs.push_back(port.Serialize());
        }
        
        json outputs = json::array();
        for (const auto& port : outputs_) {
            outputs.push_back(port.Serialize());
        }
        
        return {
            {"id", id_},
            {"type", type_},
            {"category", GetCategory()},
            {"color", GetColor()},
            {"description", GetDescription()},
            {"properties", properties_},
            {"status", static_cast<int>(status_)},
            {"inputs", inputs},
            {"outputs", outputs}
        };
    }
    
protected:
    std::string id_;
    std::string type_;
    NodeStatus status_;
    json properties_;
    std::vector<Port> inputs_;
    std::vector<Port> outputs_;
};

} // namespace Core::NodeGraph
