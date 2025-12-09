#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace Shared::Core {

/// @brief タイプセーフなイベントシステム
/// @details 文字列ベースのイベント名で Subscribe/Emit を行う
class EventSystem {
public:
    EventSystem() = default;
    ~EventSystem() = default;

    /// @brief イベントを購読
    /// @param event_type イベント名（例: "entity_updated", "file_changed"）
    /// @param callback イベント発生時に呼ばれるコールバック
    void Subscribe(const std::string& event_type,
                   std::function<void(const nlohmann::json&)> callback);

    /// @brief イベントを発行
    /// @param event_type イベント名
    /// @param data イベントデータ（JSON形式）
    void Emit(const std::string& event_type, const nlohmann::json& data);

    /// @brief すべての購読を解除
    void ClearAll();

private:
    std::unordered_map<std::string, std::vector<std::function<void(const nlohmann::json&)>>> subscribers_;
};

} // namespace Shared::Core
