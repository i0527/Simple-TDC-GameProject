#pragma once

#include <string>

namespace game {
namespace core {
namespace ui {

/// @brief UIイベントの種類
enum class UIEventType {
    None,   ///< イベントなし
    Click,  ///< マウスクリック
    Hover,  ///< マウスホバー
    Key,    ///< キー入力
};

/// @brief UIイベント構造体
///
/// UIコンポーネントに渡されるイベント情報を表します。
struct UIEvent {
    UIEventType type = UIEventType::None;  ///< イベントの種類
    float x = 0.0f;   ///< Click / Hover 時の座標（内部座標系）
    float y = 0.0f;   ///< Click / Hover 時の座標（内部座標系）
    int key = 0;      ///< Key イベント時のキーコード
};

/// @brief UIイベント処理結果
///
/// UIコンポーネントがイベントを処理した結果を返します。
struct UIEventResult {
    bool handled = false;         ///< このコンポーネント（または子）がイベントを処理したか
    std::string componentId;      ///< イベント発生元の UI コンポーネント ID
    std::string actionId;         ///< ビジネスロジック側で解釈するためのアクション ID（例: "start_battle"）
};

} // namespace ui
} // namespace core
} // namespace game
