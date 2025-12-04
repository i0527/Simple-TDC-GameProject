/**
 * @file TDCompatibility.h
 * @brief TD名前空間の互換性エイリアス
 * 
 * 既存のコードとの互換性を保つため、TD名前空間へのエイリアスを提供。
 * 段階的な移行をサポートする。
 */
#pragma once

// TD名前空間の互換性エイリアス
namespace TD {
    namespace Components = Domain::TD::Components;
    namespace Systems = Domain::TD::Systems;
    namespace Managers = Domain::TD::Managers;
}

// 既存のincludeパスから新しい構造への移行を容易にするためのエイリアス
namespace TD::Components {
    using namespace Domain::TD::Components;
}

namespace TD::Systems {
    using namespace Domain::TD::Systems;
}

namespace TD::Managers {
    using namespace Domain::TD::Managers;
}

