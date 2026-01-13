#include "GameScene.hpp"
#include "../../utils/Log.h"
#include "../entities/StageManager.hpp"
#include "../entities/CharacterManager.hpp"
#include "../ecs/defineComponents.hpp"
#include "../system/OverlayManager.hpp"
#include <raylib.h>
#include <algorithm>

namespace game {
namespace core {
namespace states {

GameScene::GameScene()
    : systemAPI_(nullptr), sharedContext_(nullptr),
      gold_(500), gameSpeed_(1.0f), isPaused_(false),
      gameStateText_("準備中..."),
      requestTransition_(false), requestQuit_(false) {
}

GameScene::~GameScene() {
    Shutdown();
}

bool GameScene::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("GameScene::Initialize: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    LOG_INFO("GameScene initialization started");

    // InputHandlerの初期化
    inputHandler_ = std::make_unique<gamescene::InputHandler>(systemAPI_);
    if (!inputHandler_->Initialize()) {
        LOG_ERROR("Failed to initialize InputHandler");
        return false;
    }

    // HUD（上部＋下部バー）
    battleHud_ = std::make_unique<::game::core::ui::BattleHUDRenderer>(systemAPI_);
    battleRenderer_ = std::make_unique<::game::core::game::BattleRenderer>(systemAPI_);

    // ゲーム速度とポーズ状態
    gameSpeed_ = 1.0f;
    isPaused_ = false;
    battleTime_ = 0.0f;
    battleResult_ = BattleResult::InProgress;

    InitializeBattleFromStage();

    LOG_INFO("GameScene initialized successfully");
    return true;
}

void GameScene::Update(float deltaTime) {
    // ポーズ中はゲームロジックを更新しない
    if (!isPaused_) {
        UpdateGameLogic(deltaTime * gameSpeed_);
    }

    // 入力処理（常に更新）
    inputHandler_->Update(deltaTime);
    ProcessInput();
}

void GameScene::Render() {
    RenderBattle();
}

void GameScene::Shutdown() {
    LOG_INFO("GameScene shutdown started");

    if (inputHandler_) {
        inputHandler_->Shutdown();
        inputHandler_.reset();
    }
    battleHud_.reset();
    battleRenderer_.reset();
    registry_.clear();

    LOG_INFO("GameScene shutdown completed");
}

bool GameScene::RequestTransition(GameState& nextState) {
    if (requestTransition_) {
        nextState = nextState_;
        requestTransition_ = false;
        return true;
    }
    return false;
}

bool GameScene::RequestQuit() {
    bool result = requestQuit_;
    requestQuit_ = false;
    return result;
}

void GameScene::UpdateGameLogic(float deltaTime) {
    battleTime_ += deltaTime;
    UpdateBattle(deltaTime);
    CheckBattleEnd();
}

void GameScene::ProcessInput() {
    // 結果オーバーレイ等が出ている間は、シーン側の入力を止める（オーバーレイ側で処理）
    if (sharedContext_ && sharedContext_->overlayManager && !sharedContext_->overlayManager->IsEmpty()) {
        return;
    }

    // HUDクリック（左クリック）
    if (inputHandler_->IsLeftClickPressed() && battleHud_) {
        Vector2 mousePos = inputHandler_->GetMousePosition();
        SharedContext dummyCtx;
        const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
        auto action = battleHud_->HandleClick(ctx, mousePos, gold_, battleTime_, unitCooldownUntil_);
        HandleHUDAction(action);
    }

    // Escapeキーで戻る
    if (inputHandler_->IsEscapePressed()) {
        LOG_INFO("Escape pressed, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }

    // スペースキーでポーズ切り替え
    if (inputHandler_->IsSpacePressed()) {
        isPaused_ = !isPaused_;
        LOG_INFO("Pause toggled: {}", isPaused_);
    }
}

void GameScene::HandleButtonClick(const std::string& buttonId) {
    LOG_INFO("Button clicked: {}", buttonId);

    if (buttonId == "speed_0.5") {
        gameSpeed_ = 0.5f;
    } else if (buttonId == "speed_1.0") {
        gameSpeed_ = 1.0f;
    } else if (buttonId == "speed_2.0") {
        gameSpeed_ = 2.0f;
    } else if (buttonId == "pause") {
        isPaused_ = !isPaused_;
    } else if (buttonId == "exit") {
        LOG_INFO("Exit button clicked, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }
}

void GameScene::InitializeBattleFromStage() {
    // デフォルト
    lane_ = LaneConfig{};
    playerTower_ = TowerState{};
    enemyTower_ = TowerState{};
    currentWave_ = 1;
    totalWaves_ = 1;
    gold_ = 500;
    goldMaxCap_ = 9999;
    goldMaxCurrent_ = static_cast<float>(goldMaxCap_);
    goldMaxGrowthPerSecond_ = 30.0f;
    goldRegenPerSecond_ = 10.0f;
    goldRegenAccumulator_ = 0.0f;

    // SharedContextからステージ情報を取得
    if (!sharedContext_ || !sharedContext_->stageManager || sharedContext_->currentStageId.empty()) {
        LOG_WARN("No stage selected, using default battle config");
        gameStateText_ = "Battle";
    } else {
        auto stageData = sharedContext_->stageManager->GetStageDataById(sharedContext_->currentStageId);
        if (!stageData) {
            LOG_WARN("Stage not found: {}, using default battle config", sharedContext_->currentStageId);
            gameStateText_ = "Battle";
        } else {
            gameStateText_ = stageData->stageName.empty() ? "Battle" : stageData->stageName;

            // lanes[0]
            try {
                if (stageData->data.contains("lanes") && stageData->data["lanes"].is_array() && !stageData->data["lanes"].empty()) {
                    const auto& lane0 = stageData->data["lanes"][0];
                    lane_.y = lane0.value("y", lane_.y);
                    lane_.startX = lane0.value("startX", lane_.startX);
                    lane_.endX = lane0.value("endX", lane_.endX);
                }
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read lanes from stage json: {}", e.what());
            }

            // minGap
            try {
                lane_.minGap = stageData->data.value("minGap", lane_.minGap);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read minGap from stage json: {}", e.what());
            }

            // wave count（後続TODOでWaveLoader接続）
            totalWaves_ = stageData->waveCount > 0 ? stageData->waveCount : 1;
            currentWave_ = 1;
            spawnSchedule_ = waveLoader_.LoadStageSpawnEvents(stageData->data);
            spawnCursor_ = 0;
            if (!spawnSchedule_.empty()) {
                LOG_INFO("Spawn schedule loaded: {} events", spawnSchedule_.size());
            } else {
                LOG_INFO("Spawn schedule is empty");
            }

            // gold
            try {
                gold_ = stageData->data.value("startingCost", gold_);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read startingCost from stage json: {}", e.what());
            }

            // お財布（最大値）/ 回復（任意キー。未指定ならデフォルト）
            try {
                goldMaxCap_ = stageData->data.value("maxCost", goldMaxCap_);
                goldMaxCap_ = stageData->data.value("maxGold", goldMaxCap_);

                // お財布開始値（無ければ cap の 1/4（最大1000））
                const int defaultStart = std::max(100, std::min(1000, goldMaxCap_ / 4));
                int startMax = stageData->data.value("walletMaxStart", defaultStart);
                startMax = stageData->data.value("startMaxGold", startMax);
                goldMaxCurrent_ = static_cast<float>(std::max(0, std::min(startMax, goldMaxCap_)));

                goldMaxGrowthPerSecond_ = stageData->data.value("walletGrowthPerSecond", goldMaxGrowthPerSecond_);
                goldMaxGrowthPerSecond_ = stageData->data.value("walletMaxGrowthPerSecond", goldMaxGrowthPerSecond_);

                goldRegenPerSecond_ = stageData->data.value("goldRegenPerSecond", goldRegenPerSecond_);
                goldRegenPerSecond_ = stageData->data.value("costRegenPerSecond", goldRegenPerSecond_);
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read gold regen config from stage json: {}", e.what());
            }
            const int curMax = std::max(0, static_cast<int>(goldMaxCurrent_));
            gold_ = std::max(0, std::min(gold_, curMax));

            // castle hp
            int playerHp = 1000;
            int enemyHp = 6000;
            try {
                if (stageData->data.contains("castle_hp") && stageData->data["castle_hp"].is_object()) {
                    const auto& chp = stageData->data["castle_hp"];
                    playerHp = chp.value("player_castle_hp", playerHp);
                    enemyHp = chp.value("enemy_castle_hp", enemyHp);
                } else {
                    // 互換フォールバック（旧 playerLife 等）
                    playerHp = stageData->data.value("playerLife", playerHp);
                    enemyHp = stageData->data.value("enemyLife", enemyHp);
                }
            } catch (const std::exception& e) {
                LOG_WARN("Failed to read castle_hp from stage json: {}", e.what());
            }

            playerTower_.maxHp = playerTower_.currentHp = playerHp;
            enemyTower_.maxHp = enemyTower_.currentHp = enemyHp;
        }
    }

    // タワー配置（lane start/endに配置）
    enemyTower_.x = lane_.startX;
    enemyTower_.y = lane_.y;
    playerTower_.x = lane_.endX;
    playerTower_.y = lane_.y;

    // 簡易マッピング（後でデータ駆動化可能）
    enemyToCharacterId_.clear();
    enemyToCharacterId_["enemy_slime"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_goblin"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_brute"] = "char_sub_orca_001";
    enemyToCharacterId_["enemy_slime_debug"] = "char_sub_hatslime_001";
    enemyToCharacterId_["enemy_ogre_debug"] = "char_sub_orca_001";
    // インラインwaves用（短いID）
    enemyToCharacterId_["goblin"] = "char_sub_orca_001";
    enemyToCharacterId_["goblin_boss"] = "char_sub_orca_001";
}

void GameScene::UpdateBattle(float deltaTime) {
    const float now = battleTime_;

    // ===== お財布（最大値）が時間で増える =====
    if (goldMaxGrowthPerSecond_ > 0.0f && goldMaxCurrent_ < static_cast<float>(goldMaxCap_)) {
        goldMaxCurrent_ += goldMaxGrowthPerSecond_ * deltaTime;
        if (goldMaxCurrent_ > static_cast<float>(goldMaxCap_)) {
            goldMaxCurrent_ = static_cast<float>(goldMaxCap_);
        }
    }

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

    auto spawnFromCharacter = [&](const entities::Character& character,
                                  ecs::components::Faction faction,
                                  float x, float y) {
        auto e = registry_.create();
        registry_.emplace<ecs::components::Position>(e, x, y);
        registry_.emplace<ecs::components::Health>(e, character.GetTotalHP(), character.GetTotalHP());
        registry_.emplace<ecs::components::Stats>(e, character.GetTotalAttack(), character.GetTotalDefense());
        registry_.emplace<ecs::components::Movement>(e, character.move_speed);
        registry_.emplace<ecs::components::Combat>(e, character.attack_type, character.attack_size, character.effect_type, character.attack_span);
        auto& combat = registry_.get<ecs::components::Combat>(e);
        combat.last_attack_time = -9999.0f;

        registry_.emplace<ecs::components::Sprite>(e,
                                                   character.move_sprite.sheet_path,
                                                   character.move_sprite.frame_width,
                                                   character.move_sprite.frame_height);
        registry_.emplace<ecs::components::Animation>(e,
                                                     character.move_sprite.frame_count,
                                                     character.move_sprite.frame_duration,
                                                     ecs::components::AnimationType::Move,
                                                     true);
        registry_.emplace<ecs::components::Team>(e, faction);
        return e;
    };

    // 敵スポーン（スケジュール）
    while (spawnCursor_ < spawnSchedule_.size() && spawnSchedule_[spawnCursor_].time <= battleTime_) {
        const std::string enemyId = spawnSchedule_[spawnCursor_].enemyId;
        spawnCursor_++;

        if (!sharedContext_ || !sharedContext_->characterManager) {
            continue;
        }

        std::string characterId;

        // 0) enemyIdがキャラIDならそのまま（敵味方のテクスチャを共通化）
        if (sharedContext_->characterManager->HasCharacter(enemyId)) {
            characterId = enemyId;
        }

        // 1) マッピング
        auto mapIt = enemyToCharacterId_.find(enemyId);
        if (mapIt != enemyToCharacterId_.end()) {
            characterId = mapIt->second;
        }

        // 2) フォールバック: 編成の最初
        if (characterId.empty() && !sharedContext_->formationData.IsEmpty()) {
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

        auto character = sharedContext_->characterManager->GetCharacterTemplate(characterId);
        if (!character) {
            LOG_WARN("Enemy spawn skipped (character not found): {} (enemyId={})", characterId, enemyId);
            continue;
        }

        const float y = lane_.y - static_cast<float>(character->move_sprite.frame_height);
        spawnFromCharacter(*character, ecs::components::Faction::Enemy, enemyTower_.x + 40.0f, y);
    }

    // ===== 戦闘更新（最小実装） =====
    // 1) 死亡削除
    {
        std::vector<entt::entity> toDestroy;
        auto hv = registry_.view<ecs::components::Health>();
        for (auto e : hv) {
            const auto& hp = hv.get<ecs::components::Health>(e);
            if (hp.current <= 0) {
                toDestroy.push_back(e);
            }
        }
        for (auto e : toDestroy) {
            registry_.destroy(e);
        }
    }

    // 2) 目標検索ヘルパ
    auto findNearestTarget = [&](entt::entity self, ecs::components::Faction selfFaction) -> entt::entity {
        const auto* selfPos = registry_.try_get<ecs::components::Position>(self);
        const auto* selfSprite = registry_.try_get<ecs::components::Sprite>(self);
        if (!selfPos || !selfSprite) return entt::null;

        const float selfCenter = selfPos->x + static_cast<float>(selfSprite->frame_width) * 0.5f;
        entt::entity best = entt::null;
        float bestDist = 1e9f;

        auto view = registry_.view<ecs::components::Position, ecs::components::Sprite, ecs::components::Team, ecs::components::Health>();
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
    auto units = registry_.view<ecs::components::Position, ecs::components::Sprite, ecs::components::Movement, ecs::components::Stats, ecs::components::Combat, ecs::components::Team>();
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

        // タワー接敵判定
        bool attackingTower = false;
        if (team.faction == ecs::components::Faction::Player) {
            if (centerX <= enemyTower_.x + enemyTower_.width * 0.5f + atkRange) {
                attackingTower = true;
                move.velocity = {0.0f, 0.0f};
                if (combat.CanAttack(now)) {
                    combat.last_attack_time = now;
                    enemyTower_.currentHp -= std::max(1, stats.attack);
                }
            }
        } else {
            if (centerX >= playerTower_.x - playerTower_.width * 0.5f - atkRange) {
                attackingTower = true;
                move.velocity = {0.0f, 0.0f};
                if (combat.CanAttack(now)) {
                    combat.last_attack_time = now;
                    playerTower_.currentHp -= std::max(1, stats.attack);
                }
            }
        }
        if (attackingTower) {
            continue;
        }

        // ユニット接敵
        entt::entity target = findNearestTarget(e, team.faction);
        bool attackingUnit = false;
        if (target != entt::null) {
            const auto& tp = registry_.get<ecs::components::Position>(target);
            const auto& ts = registry_.get<ecs::components::Sprite>(target);
            const float tCenter = tp.x + static_cast<float>(ts.frame_width) * 0.5f;
            const float dist = std::abs(tCenter - centerX);
            if (dist <= atkRange) {
                attackingUnit = true;
                move.velocity = {0.0f, 0.0f};
                if (combat.CanAttack(now)) {
                    combat.last_attack_time = now;
                    auto& th = registry_.get<ecs::components::Health>(target);
                    const auto* tstats = registry_.try_get<ecs::components::Stats>(target);
                    const int def = tstats ? tstats->defense : 0;
                    const int dmg = std::max(1, stats.attack - def);
                    th.current -= dmg;
                }
            }
        }

        if (!attackingUnit) {
            move.velocity = {dir * move.speed, 0.0f};
            pos.x += move.velocity.x * deltaTime;
        }
    }

    // 4) 同陣営の詰まり（minGap）
    auto enforceGap = [&](ecs::components::Faction faction) {
        struct UnitRef {
            entt::entity e;
            float centerX;
            int w;
        };
        std::vector<UnitRef> list;
        auto v = registry_.view<ecs::components::Position, ecs::components::Sprite, ecs::components::Team>();
        for (auto e : v) {
            const auto& t = v.get<ecs::components::Team>(e);
            if (t.faction != faction) continue;
            const auto& p = v.get<ecs::components::Position>(e);
            const auto& s = v.get<ecs::components::Sprite>(e);
            list.push_back(UnitRef{e, p.x + static_cast<float>(s.frame_width) * 0.5f, s.frame_width});
        }
        if (list.size() < 2) return;

        if (faction == ecs::components::Faction::Player) {
            std::sort(list.begin(), list.end(), [](const UnitRef& a, const UnitRef& b){ return a.centerX < b.centerX; });
            for (size_t i = 1; i < list.size(); ++i) {
                const float minCenter = list[i-1].centerX + lane_.minGap;
                if (list[i].centerX < minCenter) {
                    auto& p = registry_.get<ecs::components::Position>(list[i].e);
                    p.x += (minCenter - list[i].centerX);
                    list[i].centerX = minCenter;
                }
            }
        } else {
            std::sort(list.begin(), list.end(), [](const UnitRef& a, const UnitRef& b){ return a.centerX > b.centerX; });
            for (size_t i = 1; i < list.size(); ++i) {
                const float maxCenter = list[i-1].centerX - lane_.minGap;
                if (list[i].centerX > maxCenter) {
                    auto& p = registry_.get<ecs::components::Position>(list[i].e);
                    p.x -= (list[i].centerX - maxCenter);
                    list[i].centerX = maxCenter;
                }
            }
        }
    };
    enforceGap(ecs::components::Faction::Player);
    enforceGap(ecs::components::Faction::Enemy);

    // 5) アニメ更新
    if (battleRenderer_) {
        battleRenderer_->UpdateAnimations(registry_, deltaTime);
    }
}

void GameScene::RenderBattle() {
    // 背景（白）
    systemAPI_->DrawRectangle(0, 0, 1920, 1080, WHITE);

    // レーン（ライン）
    Color laneColor = { static_cast<unsigned char>(180), static_cast<unsigned char>(180), static_cast<unsigned char>(180), 255 };
    systemAPI_->DrawLine(static_cast<int>(lane_.startX), static_cast<int>(lane_.y),
                         static_cast<int>(lane_.endX), static_cast<int>(lane_.y),
                         4.0f, laneColor);

    // タワー（簡易矩形）
    Color towerEnemy = { static_cast<unsigned char>(220), static_cast<unsigned char>(120), static_cast<unsigned char>(120), 255 };
    Color towerPlayer = { static_cast<unsigned char>(120), static_cast<unsigned char>(160), static_cast<unsigned char>(220), 255 };

    Rectangle enemyRec = { enemyTower_.x - enemyTower_.width * 0.5f, enemyTower_.y - enemyTower_.height, enemyTower_.width, enemyTower_.height };
    Rectangle playerRec = { playerTower_.x - playerTower_.width * 0.5f, playerTower_.y - playerTower_.height, playerTower_.width, playerTower_.height };
    systemAPI_->DrawRectangleRec(enemyRec, towerEnemy);
    systemAPI_->DrawRectangleRec(playerRec, towerPlayer);

    // ユニット描画
    if (battleRenderer_) {
        battleRenderer_->RenderEntities(registry_);
    }

    // HUD（上部＋下部バー）
    if (battleHud_) {
        SharedContext dummyCtx;
        const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
        const int curMaxGold = std::max(0, static_cast<int>(goldMaxCurrent_));
        battleHud_->Render(ctx,
                           playerTower_.currentHp, playerTower_.maxHp,
                           enemyTower_.currentHp, enemyTower_.maxHp,
                           gold_, curMaxGold, gameSpeed_, isPaused_,
                           battleTime_, unitCooldownUntil_);
    }
}

void GameScene::HandleHUDAction(const ::game::core::ui::BattleHUDAction& action) {
    using ::game::core::ui::BattleHUDActionType;

    switch (action.type) {
    case BattleHUDActionType::None:
        return;
    case BattleHUDActionType::TogglePause:
        isPaused_ = !isPaused_;
        LOG_INFO("HUD: pause toggled: {}", isPaused_);
        return;
    case BattleHUDActionType::SetSpeed:
        gameSpeed_ = action.speed;
        LOG_INFO("HUD: speed set: {}", gameSpeed_);
        return;
    case BattleHUDActionType::SpawnUnit: {
        if (!sharedContext_ || !sharedContext_->characterManager) {
            LOG_WARN("HUD: SpawnUnit ignored (CharacterManager not available)");
            return;
        }
        auto character = sharedContext_->characterManager->GetCharacterTemplate(action.unitId);
        if (!character) {
            LOG_WARN("HUD: SpawnUnit ignored (character not found): {}", action.unitId);
            return;
        }
        if (gold_ < character->cost) {
            LOG_DEBUG("HUD: SpawnUnit blocked (not enough gold): {} cost={}", action.unitId, character->cost);
            return;
        }
        // 最小実装: ゴールド消費＋クールダウンのみ（ユニット実体は後続TODOでBattleRendererへ接続）
        gold_ -= character->cost;
        gold_ = std::max(0, gold_);
        constexpr float DEFAULT_SPAWN_COOLDOWN = 2.0f;
        unitCooldownUntil_[action.unitId] = battleTime_ + DEFAULT_SPAWN_COOLDOWN;
        LOG_INFO("HUD: SpawnUnit: {} (gold now {})", action.unitId, gold_);

        // 暫定: 即座に味方ユニットを生成（移動/戦闘は後続TODO）
        const float y = lane_.y - static_cast<float>(character->move_sprite.frame_height);
        auto e = registry_.create();
        registry_.emplace<ecs::components::Position>(e, playerTower_.x - 220.0f, y);
        registry_.emplace<ecs::components::Health>(e, character->GetTotalHP(), character->GetTotalHP());
        registry_.emplace<ecs::components::Stats>(e, character->GetTotalAttack(), character->GetTotalDefense());
        registry_.emplace<ecs::components::Movement>(e, character->move_speed);
        registry_.emplace<ecs::components::Combat>(e, character->attack_type, character->attack_size, character->effect_type, character->attack_span);
        auto& combat = registry_.get<ecs::components::Combat>(e);
        combat.last_attack_time = -9999.0f;
        registry_.emplace<ecs::components::Sprite>(e, character->move_sprite.sheet_path, character->move_sprite.frame_width, character->move_sprite.frame_height);
        registry_.emplace<ecs::components::Animation>(e, character->move_sprite.frame_count, character->move_sprite.frame_duration, ecs::components::AnimationType::Move, true);
        registry_.emplace<ecs::components::Team>(e, ecs::components::Faction::Player);
        return;
    }
    }
}

void GameScene::CheckBattleEnd() {
    if (battleResult_ != BattleResult::InProgress) {
        return;
    }

    if (enemyTower_.currentHp <= 0) {
        battleResult_ = BattleResult::Victory;
        gameStateText_ = "Victory";
        isPaused_ = true;
        LOG_INFO("Battle finished: Victory");
        if (sharedContext_ && sharedContext_->overlayManager) {
            sharedContext_->overlayManager->PushOverlay(OverlayState::BattleVictory, systemAPI_);
        }
        return;
    }

    if (playerTower_.currentHp <= 0) {
        battleResult_ = BattleResult::Defeat;
        gameStateText_ = "Defeat";
        isPaused_ = true;
        LOG_INFO("Battle finished: Defeat");
        if (sharedContext_ && sharedContext_->overlayManager) {
            sharedContext_->overlayManager->PushOverlay(OverlayState::BattleDefeat, systemAPI_);
        }
        return;
    }
}

} // namespace states
} // namespace core
} // namespace game
