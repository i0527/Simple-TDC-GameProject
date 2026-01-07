#pragma once

/// @brief モジュール情報の一元管理
/// 
/// 各モジュールへのパスを一元管理します。
/// 最適化はコンパイラに任せます。
/// 
/// 使用例:
/// #include "Modules/MovementModule.hpp"
/// #include "Modules/RenderModule.hpp"
/// #include "Modules/InputModule.hpp"

namespace game {
    namespace core {
        namespace ecs {
            // モジュール定義は将来的にここに追加
            // 現在は空（モジュール実装時に追加）
        }
    }
}
