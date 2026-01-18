#pragma once

/// @brief コンポーネント定義情報の一元管理
/// 
/// すべてのコンポーネントヘッダーをここにインクルードします。
/// 最適化はコンパイラに任せます。

// コンポーネントインクルード
#include "components/Position.hpp"
#include "components/Health.hpp"
#include "components/Stats.hpp"
#include "components/Movement.hpp"
#include "components/Combat.hpp"
#include "components/Sprite.hpp"
#include "components/Animation.hpp"
#include "components/CharacterId.hpp"
#include "components/Team.hpp"
#include "components/Equipment.hpp"
#include "components/PassiveSkills.hpp"

namespace game {
    namespace core {
        namespace ecs {
            // コンポーネント定義は上記のインクルードで提供されます
        }
    }
}
