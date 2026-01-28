#include "../BattleProgressAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cmath>
#include <unordered_map>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../ECSystemAPI.hpp"
#include "../GameplayDataAPI.hpp"
#include "../SceneOverlayControlAPI.hpp"
#include "../SetupAPI.hpp"
#include "../../config/GameState.hpp"
#include "../../config/SharedContext.hpp"
#include "../../ecs/defineComponents.hpp"
#include "../../ecs/entities/CharacterStatCalculator.hpp"
#include "../../ecs/entities/EntityCreationData.hpp"
#include "../../system/TowerEnhancementEffects.hpp"
#include "../../ui/BattleHUDRenderer.hpp"

namespace game {
namespace core {

void BattleProgressAPI::Update(float deltaTime) {
    battleTime_ += deltaTime;
    UpdateBattle(deltaTime);
    CheckBattleEnd();
}

void BattleProgressAPI::HandleHUDAction(const ui::BattleHUDAction& action) {
    using ::game::core::ui::BattleHUDActionType;

    switch (action.type) {
    case BattleHUDActionType::None:
        return;
    case BattleHUDActionType::TogglePause:
        if (sceneOverlayAPI_) {
            sceneOverlayAPI_->PushOverlay(OverlayState::Pause);
            LOG_INFO("HUD: Pause overlay opened");
        }
        return;
    case BattleHUDActionType::SetSpeed:
        gameSpeed_ = action.speed;
        LOG_INFO("HUD: speed set: {}", gameSpeed_);
        return;
    case BattleHUDActionType::SpawnUnit: {
        if (!gameplayDataAPI_) {
            LOG_WARN("HUD: SpawnUnit ignored (GameplayDataAPI not available)");
            return;
        }
        auto character = gameplayDataAPI_->GetCharacterTemplate(action.unitId);
        if (!character) {
            LOG_WARN("HUD: SpawnUnit ignored (character not found): {}", action.unitId);
            return;
        }
        if (gold_ < character->cost) {
            LOG_DEBUG("HUD: SpawnUnit blocked (not enough gold): {} cost={}", action.unitId, character->cost);
            return;
        }
        gold_ -= character->cost;
        gold_ = std::max(0, gold_);
        
        // 統計情報を更新
        spawnedUnitCount_++;
        totalGoldSpent_ += character->cost;
        
        constexpr float DEFAULT_SPAWN_COOLDOWN = 2.0f;
        unitCooldownUntil_[action.unitId] = battleTime_ + DEFAULT_SPAWN_COOLDOWN;
        LOG_INFO("HUD: SpawnUnit: {} (gold now {}, units spawned: {}, total gold spent: {})", 
                 action.unitId, gold_, spawnedUnitCount_, totalGoldSpent_);

        // 暫定: 即座に味方ユニットを生成（移動/戦闘は後続TODO）
        const float y = lane_.y - static_cast<float>(character->move_sprite.frame_height);
        int maxHp = character->GetTotalHP();
        int atk = character->GetTotalAttack();
        int def = character->GetTotalDefense();
        float moveSpeed = character->move_speed;
        Vector2 attackSize = character->attack_size;
        float attackSpan = character->attack_span;

        // セーブロードアウトを適用
        if (gameplayDataAPI_) {
            const auto st = gameplayDataAPI_->GetCharacterState(action.unitId);
            const auto* itemPassiveManager = gameplayDataAPI_->GetItemPassiveManager();
            if (itemPassiveManager) {
                const auto calc = ::game::core::entities::CharacterStatCalculator::Calculate(*character, st, *itemPassiveManager);
                maxHp = calc.hp.final;
                atk = calc.attack.final;
                def = calc.defense.final;
                moveSpeed = calc.moveSpeed.final;
                attackSize.x = calc.range.final;
                attackSpan = calc.attackSpan.final;
            }
        }

        // タワー強化による味方バフを後乗せ（UIのユニット強化計算とは分離）
        if (gameplayDataAPI_) {
            const auto te = gameplayDataAPI_->GetTowerEnhancements();
            const auto attachments = gameplayDataAPI_->GetTowerAttachments();
            const auto& masters = gameplayDataAPI_->GetAllTowerAttachmentMasters();
            const auto mul = system::CalculateTowerEnhancementMultipliers(te, attachments, masters);
            maxHp = std::max(1, static_cast<int>(std::round(static_cast<float>(maxHp) * mul.allyHpMul)));
            atk = std::max(0, static_cast<int>(std::round(static_cast<float>(atk) * mul.allyAttackMul)));
        }

        if (!setupAPI_ || !ecsAPI_) {
            return;
        }
        entities::EntityCreationData creationData;
        creationData.character_id = character->id;
        creationData.position = {playerTower_.x - 220.0f, y};
        creationData.level = 1;

        SpawnOverrides overrides;
        overrides.maxHp = maxHp;
        overrides.attack = atk;
        overrides.defense = def;
        overrides.moveSpeed = moveSpeed;
        overrides.attackSize = attackSize;
        overrides.attackSpan = attackSpan;

        auto e = setupAPI_->CreateBattleEntityFromCharacter(
            *character, creationData, ecs::components::Faction::Player, &overrides);
        if (e == entt::null) {
            return;
        }
        return;
    }
    }
}

void BattleProgressAPI::UpdateBattle(float deltaTime) {
    const float now = battleTime_;
    if (!ecsAPI_ || !setupAPI_) {
        return;
    }
    
    // 無限ステージの処理
    if (isInfinite_) {
        survivalTime_ += deltaTime;
        UpdateInfiniteDifficulty(deltaTime);
    }

    // ===== お財布（最大値）が時間で増える =====
    // 上限が上がらないように無効化
    // if (goldMaxGrowthPerSecond_ > 0.0f && goldMaxCurrent_ < static_cast<float>(goldMaxCap_)) {
    //     goldMaxCurrent_ += goldMaxGrowthPerSecond_ * deltaTime;
    //     if (goldMaxCurrent_ > static_cast<float>(goldMaxCap_)) {
    //         goldMaxCurrent_ = static_cast<float>(goldMaxCap_);
    //     }
    // }

    const int curMaxGold = std::max(0, static_cast<int>(goldMaxCurrent_));
    if (gold_ > curMaxGold) {
        gold_ = curMaxGold;
    }

    // ===== ゴールド回復（現在上限まで）=====
    if (gold_ < curMaxGold && goldRegenPerSecond_ > 0.0f) {
        goldRegenAccumulator_ += goldRegenPerSecond_ * deltaTime;
        const int add = static_cast<int>(goldRegenAccumulator_);
        if (add > 0) {
            gold_ = std::min(curMaxGold, gold_ + add);
            goldRegenAccumulator_ -= static_cast<float>(add);
        }
    }

    system::TowerEnhancementMultipliers towerMul;
    if (gameplayDataAPI_) {
        const auto te = gameplayDataAPI_->GetTowerEnhancements();
        const auto attachments = gameplayDataAPI_->GetTowerAttachments();
        const auto& masters = gameplayDataAPI_->GetAllTowerAttachmentMasters();
        towerMul = system::CalculateTowerEnhancementMultipliers(te, attachments, masters);
    }

    auto spawnFromCharacter = [&](const entities::Character& character,
                                  ecs::components::Faction faction,
                                  float x, float y, int level) {
        entities::EntityCreationData creationData;
        creationData.character_id = character.id;
        creationData.position = {x, y};
        creationData.level = std::max(1, level);
        if (faction != ecs::components::Faction::Enemy) {
            return setupAPI_->CreateBattleEntityFromCharacter(character, creationData, faction, nullptr);
        }

        SpawnOverrides overrides;
        overrides.maxHp = std::max(
            1, static_cast<int>(std::round(static_cast<float>(character.GetTotalHP()) * towerMul.enemyHpMul)));
        overrides.attack = std::max(
            0, static_cast<int>(std::round(static_cast<float>(character.GetTotalAttack()) * towerMul.enemyAttackMul)));
        overrides.defense = character.GetTotalDefense();
        overrides.moveSpeed = std::max(0.0f, character.move_speed * towerMul.enemyMoveSpeedMul);
        overrides.attackSize = character.attack_size;
        overrides.attackSpan = character.attack_span;
        return setupAPI_->CreateBattleEntityFromCharacter(character, creationData, faction, &overrides);
    };

    // 敵スポーン（スケジュール）
    while (spawnCursor_ < spawnSchedule_.size() && spawnSchedule_[spawnCursor_].time <= battleTime_) {
        const std::string enemyId = spawnSchedule_[spawnCursor_].enemyId;
        const int enemyLevel = spawnSchedule_[spawnCursor_].level;
        spawnCursor_++;

        if (!gameplayDataAPI_) {
            continue;
        }

        std::string characterId;

        // 0) enemyIdがキャラIDならそのまま（敵味方のテクスチャを共通化）
        if (gameplayDataAPI_->HasCharacter(enemyId)) {
            characterId = enemyId;
        }

        // 1) マッピング
        auto mapIt = enemyToCharacterId_.find(enemyId);
        if (mapIt != enemyToCharacterId_.end()) {
            characterId = mapIt->second;
        }

        // 2) フォールバック: 編成の最初
        if (characterId.empty() && sharedContext_ && !sharedContext_->formationData.IsEmpty()) {
            for (const auto& s : sharedContext_->formationData.slots) {
                if (!s.second.empty()) {
                    characterId = s.second;
                    break;
                }
            }
        }
        if (characterId.empty()) {
            LOG_WARN("Enemy spawn skipped (no character mapping/fallback): {}", enemyId);
            continue;
        }

        auto character = gameplayDataAPI_->GetCharacterTemplate(characterId);
        if (!character) {
            LOG_WARN("Enemy spawn skipped (character not found): {} (enemyId={})", characterId, enemyId);
            continue;
        }

        const float y = lane_.y - static_cast<float>(character->move_sprite.frame_height);
        spawnFromCharacter(*character, ecs::components::Faction::Enemy,
                           enemyTower_.x + 40.0f, y, enemyLevel);
    }

    // ===== 戦闘更新（最小実装） =====
    // 1) 死亡削除
    ecsAPI_->DestroyDeadEntities();

    // 2) 目標検索ヘルパ
    auto findNearestTarget = [&](entt::entity self, ecs::components::Faction selfFaction) -> entt::entity {
        const auto* selfPos = ecsAPI_->Try<ecs::components::Position>(self);
        const auto* selfSprite = ecsAPI_->Try<ecs::components::Sprite>(self);
        if (!selfPos || !selfSprite) return entt::null;

        const float selfCenter = selfPos->x + static_cast<float>(selfSprite->frame_width) * 0.5f;
        entt::entity best = entt::null;
        float bestDist = 1e9f;

        auto view = ecsAPI_->View<ecs::components::Position, ecs::components::Sprite, ecs::components::Team, ecs::components::Health>();
        for (auto other : view) {
            if (other == self) continue;
            const auto& team = view.get<ecs::components::Team>(other);
            if (team.faction == selfFaction) continue;
            const auto& op = view.get<ecs::components::Position>(other);
            const auto& os = view.get<ecs::components::Sprite>(other);
            const auto& oh = view.get<ecs::components::Health>(other);
            if (oh.current <= 0) continue;

            const float otherCenter = op.x + static_cast<float>(os.frame_width) * 0.5f;
            const float dist = std::abs(otherCenter - selfCenter);
            if (dist < bestDist) {
                bestDist = dist;
                best = other;
            }
        }
        return best;
    };

    // 3) 移動/攻撃
    static const std::unordered_map<std::string, entities::Character> kEmptyCharacters;
    const auto& characterMasters =
        gameplayDataAPI_ ? gameplayDataAPI_->GetAllCharacterMasters() : kEmptyCharacters;
    auto findCharacter = [&](const std::string& id) -> const entities::Character* {
        auto it = characterMasters.find(id);
        if (it != characterMasters.end()) {
            return &it->second;
        }
        return nullptr;
    };
    auto setAnimation = [&](entt::entity entity, const entities::Character& ch, bool isAttack) {
        auto* sprite = ecsAPI_->Try<ecs::components::Sprite>(entity);
        auto* anim = ecsAPI_->Try<ecs::components::Animation>(entity);
        if (!sprite || !anim) {
            return;
        }
        const auto& info = isAttack ? ch.attack_sprite : ch.move_sprite;
        sprite->sheet_path = info.sheet_path;
        sprite->frame_width = info.frame_width;
        sprite->frame_height = info.frame_height;
        anim->frame_count = std::max(1, info.frame_count);
        anim->frame_duration = std::max(0.01f, info.frame_duration);
        anim->type = isAttack ? ecs::components::AnimationType::Attack
                              : ecs::components::AnimationType::Move;
        anim->is_looping = !isAttack;
        anim->Reset();
    };

    auto getCharacterId = [&](entt::entity entity) -> std::string {
        const auto* cid = ecsAPI_->Try<ecs::components::CharacterId>(entity);
        return cid ? cid->id : "unknown";
    };

    auto pushAttackLog = [&](const std::string& attackerId,
                             const std::string& targetId,
                             int damage,
                             bool hit) {
        if (!attackLogEnabled_) {
            return;
        }
        if (attackLog_.size() >= 200) {
            attackLog_.erase(attackLog_.begin());
        }
        AttackLogEntry entry;
        entry.time = battleTime_;
        entry.attackerId = attackerId;
        entry.targetId = targetId;
        entry.damage = damage;
        entry.hit = hit;
        attackLog_.push_back(entry);
    };

    auto units = ecsAPI_->View<ecs::components::Position, ecs::components::Sprite,
                               ecs::components::Movement, ecs::components::Stats,
                               ecs::components::Combat, ecs::components::Team>();
    for (auto e : units) {
        auto& pos = units.get<ecs::components::Position>(e);
        const auto& sprite = units.get<ecs::components::Sprite>(e);
        auto& move = units.get<ecs::components::Movement>(e);
        const auto& stats = units.get<ecs::components::Stats>(e);
        auto& combat = units.get<ecs::components::Combat>(e);
        const auto& team = units.get<ecs::components::Team>(e);

        const float centerX = pos.x + static_cast<float>(sprite.frame_width) * 0.5f;
        const float atkRange = std::max(10.0f, combat.attack_size.x);
        const float dir = (team.faction == ecs::components::Faction::Player) ? -1.0f : 1.0f;

        const entities::Character* character = nullptr;
        if (auto* chId = ecsAPI_->Try<ecs::components::CharacterId>(e)) {
            character = findCharacter(chId->id);
        }

        // タワー接敵判定
        const bool towerInRange =
            (team.faction == ecs::components::Faction::Player)
                ? (centerX <= enemyTower_.x + enemyTower_.width * 0.5f + atkRange)
                : (centerX >= playerTower_.x - playerTower_.width * 0.5f - atkRange);

        // ユニット接敵
        entt::entity target = findNearestTarget(e, team.faction);
        float targetDist = 99999.0f;
        if (target != entt::null) {
            const auto& tp = ecsAPI_->Get<ecs::components::Position>(target);
            const auto& ts = ecsAPI_->Get<ecs::components::Sprite>(target);
            const float tCenter = tp.x + static_cast<float>(ts.frame_width) * 0.5f;
            targetDist = std::abs(tCenter - centerX);
        }

        const std::string attackerId = getCharacterId(e);
        auto startAttack = [&]() {
            combat.is_attacking = true;
            combat.attack_start_time = now;
            combat.attack_hit_fired = false;
            combat.last_attack_time = now;
            if (character) {
                setAnimation(e, *character, true);
            }
        };

        auto updateAttack = [&]() {
            if (!combat.is_attacking) {
                return;
            }
            const float elapsed = now - combat.attack_start_time;
            const float hitTime = std::min(
                std::max(0.0f, combat.attack_hit_time), combat.attack_duration);
            if (!combat.attack_hit_fired && elapsed >= hitTime) {
                combat.attack_hit_fired = true;
                if (towerInRange) {
                    if (team.faction == ecs::components::Faction::Player) {
                        const int damage = std::max(1, stats.attack);
                        enemyTower_.currentHp -= damage;
                        pushAttackLog(attackerId, "tower_enemy", damage, true);
                    } else {
                        const int damage = std::max(1, stats.attack);
                        playerTower_.currentHp -= damage;
                        pushAttackLog(attackerId, "tower_player", damage, true);
                    }
                } else if (target != entt::null && targetDist <= atkRange) {
                    auto& th = ecsAPI_->Get<ecs::components::Health>(target);
                    const auto* tstats = ecsAPI_->Try<ecs::components::Stats>(target);
                    const int def = tstats ? tstats->defense : 0;
                    const int dmg = std::max(1, stats.attack - def);
                    th.current -= dmg;
                    pushAttackLog(attackerId, getCharacterId(target), dmg, true);
                } else {
                    pushAttackLog(attackerId, "none", 0, false);
                }
            }
            if (elapsed >= combat.attack_duration) {
                combat.is_attacking = false;
                combat.attack_hit_fired = false;
                if (character) {
                    setAnimation(e, *character, false);
                }
            }
        };

        if (combat.is_attacking) {
            move.velocity = {0.0f, 0.0f};
            updateAttack();
            if (combat.is_attacking) {
                continue;
            }
        }

        if (towerInRange) {
            move.velocity = {0.0f, 0.0f};
            if (combat.CanAttack(now)) {
                startAttack();
            }
            updateAttack();
            continue;
        }

        if (target != entt::null && targetDist <= atkRange) {
            move.velocity = {0.0f, 0.0f};
            if (combat.CanAttack(now)) {
                startAttack();
            }
            updateAttack();
            continue;
        }

        move.velocity = {dir * move.speed, 0.0f};
        pos.x += move.velocity.x * deltaTime;
    }

    // 4) 同陣営の詰まり（minGap）
    // 要件: 味方同士・敵同士の重複を許可するため、同陣営の押し戻しは行わない。

    // 破棄タイミングを統一（フレーム終端で一括破棄）
    ecsAPI_->FlushDestroyQueue();
}

void BattleProgressAPI::UpdateInfiniteDifficulty(float deltaTime) {
    // 30秒ごとに難易度を5%増加
    constexpr float DIFFICULTY_UPDATE_INTERVAL = 30.0f;
    constexpr float DIFFICULTY_INCREASE_RATE = 0.05f;
    
    if (survivalTime_ - lastDifficultyUpdateTime_ >= DIFFICULTY_UPDATE_INTERVAL) {
        enemyStatMultiplier_ += DIFFICULTY_INCREASE_RATE;
        enemySpawnRateMultiplier_ += DIFFICULTY_INCREASE_RATE * 0.5f; // スポーン率は半分の増加率
        lastDifficultyUpdateTime_ = survivalTime_;
        currentWaveNumber_++;
        
        LOG_DEBUG("Infinite stage difficulty updated: multiplier={:.2f}, spawnRate={:.2f}, wave={}", 
                 enemyStatMultiplier_, enemySpawnRateMultiplier_, currentWaveNumber_);
    }
    
    // 無限ステージでは定期的に敵をスポーン
    // 基本スポーン間隔を難易度に応じて調整
    const float baseSpawnInterval = difficultyLevel_ == 0 ? 3.0f : 1.5f;
    const float adjustedInterval = baseSpawnInterval / enemySpawnRateMultiplier_;
    
    waveTimer_ += deltaTime;
    if (waveTimer_ >= adjustedInterval && !spawnSchedule_.empty()) {
        waveTimer_ = 0.0f;
        
        // 最初のウェーブから敵をスポーン（レベルを難易度倍率で調整）
        for (const auto& event : spawnSchedule_) {
            if (event.time <= 1.0f) { // 最初の1秒以内のイベントのみ
                const int adjustedLevel = std::max(1, static_cast<int>(std::round(
                    static_cast<float>(event.level) * enemyStatMultiplier_)));
                
                // 敵をスポーン（既存のスポーンロジックを再利用）
                if (!gameplayDataAPI_) {
                    continue;
                }
                
                std::string characterId;
                if (gameplayDataAPI_->HasCharacter(event.enemyId)) {
                    characterId = event.enemyId;
                } else {
                    auto mapIt = enemyToCharacterId_.find(event.enemyId);
                    if (mapIt != enemyToCharacterId_.end()) {
                        characterId = mapIt->second;
                    }
                }
                
                if (characterId.empty()) {
                    continue;
                }
                
                auto character = gameplayDataAPI_->GetCharacterTemplate(characterId);
                if (!character) {
                    continue;
                }
                
                const float y = lane_.y - static_cast<float>(character->move_sprite.frame_height);
                
                system::TowerEnhancementMultipliers towerMul;
                if (gameplayDataAPI_) {
                    const auto te = gameplayDataAPI_->GetTowerEnhancements();
                    const auto attachments = gameplayDataAPI_->GetTowerAttachments();
                    const auto& masters = gameplayDataAPI_->GetAllTowerAttachmentMasters();
                    towerMul = system::CalculateTowerEnhancementMultipliers(te, attachments, masters);
                }
                
                entities::EntityCreationData creationData;
                creationData.character_id = character->id;
                creationData.position = {enemyTower_.x + 40.0f, y};
                creationData.level = adjustedLevel;
                
                SpawnOverrides overrides;
                overrides.maxHp = std::max(1, static_cast<int>(std::round(
                    static_cast<float>(character->GetTotalHP()) * towerMul.enemyHpMul * enemyStatMultiplier_)));
                overrides.attack = std::max(0, static_cast<int>(std::round(
                    static_cast<float>(character->GetTotalAttack()) * towerMul.enemyAttackMul * enemyStatMultiplier_)));
                overrides.defense = character->GetTotalDefense();
                overrides.moveSpeed = std::max(0.0f, character->move_speed * towerMul.enemyMoveSpeedMul);
                overrides.attackSize = character->attack_size;
                overrides.attackSpan = character->attack_span;
                
                setupAPI_->CreateBattleEntityFromCharacter(*character, creationData, 
                    ecs::components::Faction::Enemy, &overrides);
            }
        }
    }
}

int BattleProgressAPI::CalculateInfiniteReward(float survivalTime, int difficultyLevel) const {
    // 基本報酬: 生存時間（秒） * 難易度係数
    // 難易度0（易）: 1G/秒
    // 難易度1（難）: 2G/秒
    const float baseRewardPerSecond = difficultyLevel == 0 ? 1.0f : 2.0f;
    int baseReward = static_cast<int>(survivalTime * baseRewardPerSecond);
    
    // ボーナス: 5分ごとに10%追加
    const int bonusMinutes = static_cast<int>(survivalTime / 300.0f);
    int bonusReward = static_cast<int>(baseReward * 0.1f * bonusMinutes);
    
    return baseReward + bonusReward;
}

void BattleProgressAPI::CheckBattleEnd() {
    if (battleResult_ != BattleResult::InProgress) {
        return;
    }
    
    // 無限ステージのギブアップ処理
    if (isInfinite_ && giveUpRequested_) {
        battleResult_ = BattleResult::Victory;
        gameStateText_ = "Give Up";
        isPaused_ = true;
        LOG_INFO("Battle finished: Give Up (survival time: {:.1f}s)", survivalTime_);
        if (gameplayDataAPI_ && sharedContext_ && !sharedContext_->currentStageId.empty()) {
            // ギブアップ時の報酬計算
            int rewardGold = CalculateInfiniteReward(survivalTime_, difficultyLevel_);
            BattleStats stats = GetBattleStats();
            stats.clearTime = survivalTime_;
            
            // ステージデータを取得して報酬を一時的に設定
            auto stage = gameplayDataAPI_->GetStageDataById(sharedContext_->currentStageId);
            if (stage) {
                // 一時的に報酬を設定（MarkStageClearedで使用される）
                stage->rewardGold = rewardGold;
            }
            
            gameplayDataAPI_->MarkStageCleared(sharedContext_->currentStageId, 1, &stats);
            LOG_INFO("Give up reward: {} gold", rewardGold);
        }
        if (sceneOverlayAPI_) {
            sceneOverlayAPI_->PushOverlay(OverlayState::BattleVictory);
        }
        return;
    }

    if (enemyTower_.currentHp <= 0) {
        battleResult_ = BattleResult::Victory;
        gameStateText_ = "Victory";
        isPaused_ = true;
        LOG_INFO("Battle finished: Victory");
        if (gameplayDataAPI_ && sharedContext_ && !sharedContext_->currentStageId.empty()) {
            // 戦闘統計情報を取得して渡す
            BattleStats stats = GetBattleStats();
            gameplayDataAPI_->MarkStageCleared(sharedContext_->currentStageId, 3, &stats);
        }
        if (sceneOverlayAPI_) {
            sceneOverlayAPI_->PushOverlay(OverlayState::BattleVictory);
        }
        return;
    }

    if (playerTower_.currentHp <= 0) {
        battleResult_ = BattleResult::Defeat;
        gameStateText_ = "Defeat";
        isPaused_ = true;
        LOG_INFO("Battle finished: Defeat");
        if (sceneOverlayAPI_) {
            sceneOverlayAPI_->PushOverlay(OverlayState::BattleDefeat);
        }
        return;
    }
}

} // namespace core
} // namespace game
