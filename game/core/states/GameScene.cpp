#include "GameScene.hpp"
#include "../../utils/Log.h"
#include "../api/InputSystemAPI.hpp"
#include "../api/ECSystemAPI.hpp"
#include "../api/SceneOverlayControlAPI.hpp"
#include "../api/GameplayDataAPI.hpp"
#include "../ecs/defineComponents.hpp"
#include "../ui/OverlayColors.hpp"
#include "../config/RenderPrimitives.hpp"
#include <algorithm>

namespace game {
namespace core {
namespace states {

GameScene::GameScene()
    : systemAPI_(nullptr), sharedContext_(nullptr), inputAPI_(nullptr),
      battleProgressAPI_(nullptr), requestTransition_(false),
      requestQuit_(false) {
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

    // HUD�E�上部�E�下部バ�E�E�E
    battleHud_ = std::make_unique<::game::core::ui::BattleHUDRenderer>(systemAPI_);
    battleRenderer_ = std::make_unique<::game::core::game::BattleRenderer>(
        systemAPI_, sharedContext_ ? sharedContext_->ecsAPI : nullptr);

    LOG_INFO("GameScene initialized successfully");
    return true;
}

void GameScene::Update(float deltaTime) {
    const bool overlayActive =
        (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay());
    const bool pausedNow = (battleProgressAPI_ ? battleProgressAPI_->IsPaused() : false) || overlayActive;

    // ポ�Eズ中�E�また�Eオーバ�Eレイ表示中�E��EゲームロジチE��を更新しなぁE
    if (!pausedNow) {
        const float gameSpeed = battleProgressAPI_ ? battleProgressAPI_->GetGameSpeed() : 1.0f;
        if (battleProgressAPI_) {
            battleProgressAPI_->Update(deltaTime * gameSpeed);
        }
        if (battleRenderer_ && sharedContext_ && sharedContext_->ecsAPI) {
            battleRenderer_->UpdateAnimations(sharedContext_->ecsAPI, deltaTime * gameSpeed);
        }
        
        // ダメージホップアップ更新
        UpdateDamagePopups(deltaTime * gameSpeed);
    }

    // 入力�E琁E��常に更新�E�E
    ProcessInput();
}

void GameScene::Render() {
    RenderBattle();
}

void GameScene::RenderHUD() {
    if (!battleProgressAPI_ || !battleHud_) {
        return;
    }

    const auto& playerTower = battleProgressAPI_->GetPlayerTower();
    const auto& enemyTower = battleProgressAPI_->GetEnemyTower();
    SharedContext dummyCtx;
    const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
    const int curMaxGold = battleProgressAPI_->GetGoldMaxCurrent();
    const bool overlayActive =
        (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay());
    const bool pausedNow = battleProgressAPI_->IsPaused() || overlayActive;

    battleHud_->Render(ctx,
                       playerTower.currentHp, playerTower.maxHp,
                       enemyTower.currentHp, enemyTower.maxHp,
                       battleProgressAPI_->GetGold(), curMaxGold,
                       battleProgressAPI_->GetGameSpeed(), pausedNow,
                       battleProgressAPI_->GetBattleTime(),
                       battleProgressAPI_->GetUnitCooldownUntil());
}

void GameScene::Shutdown() {
    LOG_INFO("GameScene shutdown started");

    battleHud_.reset();
    battleRenderer_.reset();
    if (sharedContext_ && sharedContext_->ecsAPI) {
        sharedContext_->ecsAPI->ResetForScene();
    }

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

void GameScene::ProcessInput() {
    // 結果オーバ�Eレイ等が出てぁE��間�E、シーン側の入力を止める�E�オーバ�Eレイ側で処琁E��E
    if (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay()) {
        return;
    }

    // HUDクリチE���E�左クリチE���E�E
    if (inputAPI_ && inputAPI_->IsLeftClickPressed() && battleHud_ && battleProgressAPI_) {
        auto mousePos = inputAPI_->GetMousePosition();
        SharedContext dummyCtx;
        const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
        auto action = battleHud_->HandleClick(
            ctx, mousePos,
            battleProgressAPI_->GetGold(),
            battleProgressAPI_->GetBattleTime(),
            battleProgressAPI_->GetUnitCooldownUntil());
        HandleHUDAction(action);
    }

    // Escapeキーで戻めE
    if (inputAPI_ && inputAPI_->IsEscapePressed()) {
        LOG_INFO("Escape pressed, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }

    // スペ�Eスキーでポ�Eズ刁E��替ぁE
    if (inputAPI_ && inputAPI_->IsSpacePressed()) {
        if (sharedContext_ && sharedContext_->sceneOverlayAPI) {
            sharedContext_->sceneOverlayAPI->PushOverlay(OverlayState::Pause);
            LOG_INFO("Pause overlay opened (Space)");
        }
    }
}

void GameScene::HandleButtonClick(const std::string& buttonId) {
    LOG_INFO("Button clicked: {}", buttonId);

    if (buttonId == "speed_0.5") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(0.5f);
        }
    } else if (buttonId == "speed_1.0") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(1.0f);
        }
    } else if (buttonId == "speed_2.0") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(2.0f);
        }
    } else if (buttonId == "pause") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetPaused(!battleProgressAPI_->IsPaused());
        }
    } else if (buttonId == "exit") {
        LOG_INFO("Exit button clicked, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }
}

std::string GameScene::GetStageBackgroundPath(const std::string& stageId) const {
    if (stageId.empty()) {
        return "";
    }
    return "data/assets/textures/stage" + stageId + ".png";
}

void GameScene::RenderBattle() {
    if (!battleProgressAPI_) {
        return;
    }

    const auto& lane = battleProgressAPI_->GetLane();
    const auto& playerTower = battleProgressAPI_->GetPlayerTower();
    const auto& enemyTower = battleProgressAPI_->GetEnemyTower();

    // 背景�E�Eomeと同じ Tokyo Night風ダークチE�Eマ！E
    // 背景画像の取得と描画
    if (sharedContext_ && !sharedContext_->currentStageId.empty()) {
        std::string bgPath = GetStageBackgroundPath(sharedContext_->currentStageId);
        Texture2D* bgTexture = systemAPI_->Resource().GetTexturePtr(bgPath);
        
        if (bgTexture && bgTexture->id != 0) {
            // 背景画像を表示（全画面）
            Rect source = {0.0f, 0.0f, static_cast<float>(bgTexture->width), static_cast<float>(bgTexture->height)};
            Rect dest = {0.0f, 0.0f, 1920.0f, 1080.0f};
            Vec2 origin = {0.0f, 0.0f};
            systemAPI_->Render().DrawTexturePro(*bgTexture, source, dest, origin, 0.0f, ToCoreColor(WHITE));
        } else {
            // 画像がない場合は現状通り単色背景
            systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                               ToCoreColor(ui::OverlayColors::MAIN_BG));
        }
    } else {
        // ステージIDが空の場合は現状通り単色背景
        systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                           ToCoreColor(ui::OverlayColors::MAIN_BG));
    }

    // レーン�E�ライン�E�E
    ColorRGBA laneColor = ToCoreColor(ui::OverlayColors::ACCENT_GOLD);
    systemAPI_->Render().DrawLine(static_cast<int>(lane.startX),
                                  static_cast<int>(lane.y),
                                  static_cast<int>(lane.endX),
                                  static_cast<int>(lane.y), 4.0f, laneColor);

    // タワー�E�簡易矩形�E�E
    ColorRGBA towerEnemy = ToCoreColor(ui::OverlayColors::DANGER_RED);
    ColorRGBA towerPlayer = ToCoreColor(ui::OverlayColors::ACCENT_BLUE);

    Rect enemyRec = { enemyTower.x - enemyTower.width * 0.5f, enemyTower.y - enemyTower.height, enemyTower.width, enemyTower.height };
    Rect playerRec = { playerTower.x - playerTower.width * 0.5f, playerTower.y - playerTower.height, playerTower.width, playerTower.height };
    systemAPI_->Render().DrawRectangleRec(enemyRec, towerEnemy);
    systemAPI_->Render().DrawRectangleRec(playerRec, towerPlayer);
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(enemyRec.x), static_cast<int>(enemyRec.y),
        static_cast<int>(enemyRec.width), static_cast<int>(enemyRec.height), 2.0f,
        ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(playerRec.x), static_cast<int>(playerRec.y),
        static_cast<int>(playerRec.width), static_cast<int>(playerRec.height),
        2.0f, ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // 背景画像の存在を確認
    bool hasBackground = false;
    if (sharedContext_ && !sharedContext_->currentStageId.empty()) {
        std::string bgPath = GetStageBackgroundPath(sharedContext_->currentStageId);
        Texture2D* bgTexture = systemAPI_->Resource().GetTexturePtr(bgPath);
        hasBackground = (bgTexture && bgTexture->id != 0);
    }

    // 城HP�E��EチE��ではなく城の上に表示�E�E
    auto drawTowerHp = [&](const Rect& towerRect, int hp, int maxHp, ColorRGBA fillColor) {
        const float barH = 16.0f;
        const float padY = 10.0f;
        Rect barRect{ towerRect.x, towerRect.y - barH - padY, towerRect.width, barH };

        // 背景�E�少し透過�E�E
        ColorRGBA bg = ToCoreColor(ui::OverlayColors::PANEL_BG_PRIMARY);
        bg.a = 220;
        systemAPI_->Render().DrawRectangleRec(barRect, bg);

        const float pct = (maxHp > 0) ? std::clamp(static_cast<float>(hp) / static_cast<float>(maxHp), 0.0f, 1.0f) : 0.0f;
        Rect fill{ barRect.x, barRect.y, barRect.width * pct, barRect.height };
        systemAPI_->Render().DrawRectangleRec(fill, fillColor);

        systemAPI_->Render().DrawRectangleLines(
            static_cast<int>(barRect.x), static_cast<int>(barRect.y),
            static_cast<int>(barRect.width), static_cast<int>(barRect.height), 2.0f,
            ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

        // 数値（背景に応じて色を変更）
        // HPテキストをバーの上に配置（重ならないように）
        std::string text = "HP " + std::to_string(hp) + " / " + std::to_string(maxHp);
        Vec2 ts = systemAPI_->Render().MeasureTextDefaultCore(text, 48.0f, 1.0f);
        float tx = barRect.x + (barRect.width - ts.x) * 0.5f;
        float ty = barRect.y - ts.y - 10.0f;  // バーの上に余白を開けて配置
        ColorRGBA textColor = hasBackground 
            ? ToCoreColor(ui::OverlayColors::TEXT_DARK)  // 白背景→黒
            : ToCoreColor(ui::OverlayColors::TEXT_PRIMARY); // 暗背景→白
        systemAPI_->Render().DrawTextDefault(text, tx, ty, 48.0f, textColor);
    };
    drawTowerHp(enemyRec, enemyTower.currentHp, enemyTower.maxHp, ToCoreColor(ui::OverlayColors::DANGER_RED));
    drawTowerHp(playerRec, playerTower.currentHp, playerTower.maxHp, ToCoreColor(ui::OverlayColors::ACCENT_BLUE));

    // ユニット描画
    if (battleRenderer_ && sharedContext_ && sharedContext_->ecsAPI) {
        battleRenderer_->RenderEntities(sharedContext_->ecsAPI);
    }

    // ダメージホップアップ描画
    RenderDamagePopups();
    
    // クエスト表示
    RenderQuestPanel();
}

void GameScene::HandleHUDAction(const ::game::core::ui::BattleHUDAction& action) {
    using ::game::core::ui::BattleHUDActionType;

    switch (action.type) {
    case BattleHUDActionType::None:
        return;
    case BattleHUDActionType::TogglePause:
        if (battleProgressAPI_) {
            battleProgressAPI_->HandleHUDAction(action);
        }
        return;
    case BattleHUDActionType::SetSpeed:
    case BattleHUDActionType::SpawnUnit:
        if (battleProgressAPI_) {
            battleProgressAPI_->HandleHUDAction(action);
        }
        return;
    }
}

void GameScene::RenderQuestPanel() {
    if (!sharedContext_ || !sharedContext_->gameplayDataAPI || !battleProgressAPI_ || sharedContext_->currentStageId.empty()) {
        return;
    }

    // ステージデータを取得
    auto stage = sharedContext_->gameplayDataAPI->GetStageDataById(sharedContext_->currentStageId);
    if (!stage || stage->bonusConditions.empty()) {
        return;
    }

    // 現在のバトル統計を取得
    const auto stats = battleProgressAPI_->GetBattleStats();

    // パネル位置・サイズ
    const float panelX = 20.0f;
    const float panelY = 100.0f;
    const float lineH = 32.0f;
    const float padding = 16.0f;

    // クエスト文字列の最大幅を計算
    float maxTextWidth = 0.0f;
    for (const auto& condition : stage->bonusConditions) {
        std::string status = "✓ ";
        std::string text = status + condition.description + " +" + std::to_string(condition.rewardValue) + "G";
        Vec2 textSize = systemAPI_->Render().MeasureTextDefaultCore(text, 44.0f, 1.0f);
        maxTextWidth = std::max(maxTextWidth, textSize.x);
    }
    
    // タイトル幅も考慮
    Vec2 titleSize = systemAPI_->Render().MeasureTextDefaultCore("クエスト", 56.0f, 1.0f);
    maxTextWidth = std::max(maxTextWidth, titleSize.x);
    
    // パネル幅を文字列の長さに基づいて調整（最小400px、最大600px、余白を考慮）
    const float panelW = std::max(400.0f, std::min(600.0f, maxTextWidth + padding * 2.0f + 20.0f));

    // パネル背景
    ColorRGBA panelBg = ToCoreColor(ui::OverlayColors::PANEL_BG_SECONDARY);
    panelBg.a = 240;
    const float panelH = static_cast<float>(stage->bonusConditions.size()) * lineH + padding * 2.0f + 36.0f;
    Rect panelRect{ panelX, panelY, panelW, panelH };
    systemAPI_->Render().DrawRectangleRec(panelRect, panelBg);
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(panelRect.x), static_cast<int>(panelRect.y),
        static_cast<int>(panelRect.width), static_cast<int>(panelRect.height), 2.0f,
        ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // タイトル
    systemAPI_->Render().DrawTextDefault("クエスト", 
        static_cast<int>(panelX + padding), static_cast<int>(panelY + padding), 
        56.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // 各クエスト条件を表示
    float y = panelY + padding + 36.0f;
    for (const auto& condition : stage->bonusConditions) {
        // 条件達成判定
        bool conditionMet = false;
        if (condition.conditionType == "tower_hp_percent") {
            float hpPercent = static_cast<float>(stats.playerTowerHp) / stats.playerTowerMaxHp * 100.0f;
            if (condition.conditionOperator == "gte") {
                conditionMet = (hpPercent >= condition.conditionValue);
            } else if (condition.conditionOperator == "lte") {
                conditionMet = (hpPercent <= condition.conditionValue);
            } else if (condition.conditionOperator == "eq") {
                conditionMet = (std::abs(hpPercent - condition.conditionValue) < 1.0f);
            }
        } else if (condition.conditionType == "unit_count") {
            if (condition.conditionOperator == "lte") {
                conditionMet = (stats.spawnedUnitCount <= condition.conditionValue);
            } else if (condition.conditionOperator == "gte") {
                conditionMet = (stats.spawnedUnitCount >= condition.conditionValue);
            }
        } else if (condition.conditionType == "gold_spent") {
            if (condition.conditionOperator == "lte") {
                conditionMet = (stats.totalGoldSpent <= condition.conditionValue);
            } else if (condition.conditionOperator == "gte") {
                conditionMet = (stats.totalGoldSpent >= condition.conditionValue);
            }
        } else if (condition.conditionType == "clear_time") {
            if (condition.conditionOperator == "lte") {
                conditionMet = (stats.clearTime <= condition.conditionValue);
            } else if (condition.conditionOperator == "gte") {
                conditionMet = (stats.clearTime >= condition.conditionValue);
            }
        }

        // 表示色（達成：緑、未達成：グレー）
        ColorRGBA textColor = conditionMet 
            ? ToCoreColor(ui::OverlayColors::SUCCESS_GREEN)
            : ToCoreColor(ui::OverlayColors::TEXT_SECONDARY);
        
        // 達成マーク
        std::string status = conditionMet ? "✓ " : "○ ";
        std::string text = status + condition.description + " +" + std::to_string(condition.rewardValue) + "G";
        
        systemAPI_->Render().DrawTextDefault(text, 
            static_cast<int>(panelX + padding), static_cast<int>(y), 
            44.0f, textColor);
        
        y += lineH;
    }
}

void GameScene::UpdateDamagePopups(float deltaTime) {
    if (!battleProgressAPI_ || !sharedContext_ || !sharedContext_->ecsAPI) {
        return;
    }

    // 新しい攻撃ログエントリを検出
    const auto& attackLog = battleProgressAPI_->GetAttackLog();
    const size_t currentLogSize = attackLog.size();
    
    if (currentLogSize > lastAttackLogSize_) {
        // 新しいエントリを追加
        for (size_t i = lastAttackLogSize_; i < currentLogSize; ++i) {
            const auto& entry = attackLog[i];
            if (!entry.hit || entry.damage <= 0) {
                continue;
            }

            // エンティティの位置を取得（targetIdで検索）
            Vec2 entityPos = {0.0f, 0.0f};
            bool found = false;
            
            // CharacterIdコンポーネントで検索
            auto view = sharedContext_->ecsAPI->View<ecs::components::Position, ecs::components::CharacterId>();
            for (auto e : view) {
                const auto& charId = view.get<ecs::components::CharacterId>(e);
                if (charId.id == entry.targetId) {
                    const auto& pos = view.get<ecs::components::Position>(e);
                    entityPos = {pos.x, pos.y};
                    found = true;
                    break;
                }
            }

            if (!found) {
                // Positionコンポーネントのみで検索（最後のエンティティを使用）
                auto posView = sharedContext_->ecsAPI->View<ecs::components::Position>();
                if (!posView.empty()) {
                    // 簡易的に最後のエンティティの位置を使用
                    auto it = posView.rbegin();
                    const auto& pos = posView.get<ecs::components::Position>(*it);
                    entityPos = {pos.x, pos.y};
                    found = true;
                }
            }

            if (found) {
                DamagePopup popup;
                popup.position = entityPos;
                popup.damage = entry.damage;
                popup.lifetime = 1.0f;  // 1秒表示
                popup.maxLifetime = 1.0f;
                popup.color = ToCoreColor(ui::OverlayColors::DANGER_RED);
                damagePopups_.push_back(popup);
            }
        }
    }

    lastAttackLogSize_ = currentLogSize;

    // ポップアップの更新（上方向に移動、フェードアウト）
    for (auto& popup : damagePopups_) {
        popup.position.y -= deltaTime * 60.0f;  // 毎秒60ピクセル上に移動
        popup.lifetime -= deltaTime;
    }

    // 期限切れのポップアップを削除
    damagePopups_.erase(
        std::remove_if(damagePopups_.begin(), damagePopups_.end(),
            [](const DamagePopup& p) { return p.lifetime <= 0.0f; }),
        damagePopups_.end()
    );

    // 最大50個まで制限
    if (damagePopups_.size() > 50) {
        damagePopups_.erase(damagePopups_.begin(), damagePopups_.begin() + (damagePopups_.size() - 50));
    }
}

void GameScene::RenderDamagePopups() {
    for (const auto& popup : damagePopups_) {
        // アルファ値を計算
        float alpha = std::clamp(popup.lifetime / popup.maxLifetime, 0.0f, 1.0f);
        ColorRGBA color = popup.color;
        color.a = static_cast<unsigned char>(alpha * 255.0f);

        // ダメージ値を文字列化
        std::string text = "-" + std::to_string(popup.damage);

        // 描画
        systemAPI_->Render().DrawTextDefault(text, 
            static_cast<int>(popup.position.x), static_cast<int>(popup.position.y), 
            64.0f, color);
    }
}

} // namespace states
} // namespace core
} // namespace game
