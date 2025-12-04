/**
 * @file RoguelikeCompatibility.h
 * @brief Roguelike名前空間の互換性エイリアス
 * 
 * 既存コードとの互換性を保つための名前空間エイリアス。
 * Phase 3: 段階的な移行をサポート
 */
#pragma once

// 既存のRoguelike名前空間へのエイリアス
namespace Roguelike {
    namespace Components {
        using namespace Domain::Roguelike::Components;
    }
    
    namespace Systems {
        using namespace Domain::Roguelike::Systems;
    }
    
    namespace Managers {
        using namespace Domain::Roguelike::Managers;
    }
}

