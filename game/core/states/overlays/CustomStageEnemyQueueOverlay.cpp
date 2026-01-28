#include "CustomStageEnemyQueueOverlay.hpp"

#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/StageManager.hpp"
#include <raylib.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>

namespace game {
namespace core {

CustomStageEnemyQueueOverlay::CustomStageEnemyQueueOverlay()
    : systemAPI_(nullptr), isInitialized_(false),
      requestClose_(false), hasTransitionRequest_(false),
      requestedNextState_(GameState::Home),
      targetStageId_(""), selectedCharacterIndex_(-1),
      selectedLevel_(1), selectedSpawnDelay_(1.0f),
      selectedQueueIndex_(-1), characterListScrollOffset_(0.0f),
      queueListScrollOffset_(0.0f) {
}

bool CustomStageEnemyQueueOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("CustomStageEnemyQueueOverlay already initialized");
        return false;
    }
    if (!systemAPI) {
        LOG_ERROR("CustomStageEnemyQueueOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    isInitialized_ = true;
    return true;
}

void CustomStageEnemyQueueOverlay::Shutdown() {
    if (!isInitialized_) return;
    isInitialized_ = false;
    systemAPI_ = nullptr;
    queue_.clear();
    availableCharacters_.clear();
    targetStageId_.clear();
}

void CustomStageEnemyQueueOverlay::SetTargetStageId(const std::string& stageId) {
    targetStageId_ = stageId;
    queue_.clear();
    selectedCharacterIndex_ = -1;
    selectedLevel_ = 1;
    selectedSpawnDelay_ = 1.0f;
    selectedQueueIndex_ = -1;
    characterListScrollOffset_ = 0.0f;
    queueListScrollOffset_ = 0.0f;
}

void CustomStageEnemyQueueOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) return;

    // 保有キャラクターは毎フレーム解放状態をループで更新（キャッシュずれ防止）
    if (ctx.gameplayDataAPI) {
        LoadAvailableCharacters(ctx);
    }
    if (queue_.empty() && !targetStageId_.empty() && ctx.gameplayDataAPI) {
        LoadQueueFromStageData(ctx);
    }

    HandleKeyboardInput(ctx);
    HandleMouseInput(ctx);
}

void CustomStageEnemyQueueOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) return;

    // 背景（半透明）
    ColorRGBA bgColor = ToCoreColor(ui::OverlayColors::MAIN_BG);
    bgColor.a = 200;
    systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080, bgColor);

    // メインウィンドウ
    const float windowW = 1600.0f;
    const float windowH = 900.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.5f;

    Rect windowRect{windowX, windowY, windowW, windowH};
    ColorRGBA whiteColor{255, 255, 255, 255};
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBackground,
                                         windowRect, 8, 8, 8, 8,
                                         whiteColor);
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBorder, windowRect,
                                         8, 8, 8, 8, whiteColor);

    // タイトル
    const char* title = "カスタムステージ 敵キュー設定";
    const float titleSize = 48.0f;
    Vec2 titleSize_ = systemAPI_->Render().MeasureTextDefaultCore(title, titleSize, 1.0f);
    float titleX = windowX + (windowW - titleSize_.x) * 0.5f;
    float titleY = windowY + 30.0f;
    systemAPI_->Render().DrawTextDefault(title, titleX, titleY, titleSize,
                                         ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // パネル描画
    RenderCharacterList(ctx);
    RenderSelectionPanel(ctx);
    RenderQueueList(ctx);

    // 保存・キャンセルボタン
    const float btnW = 200.0f;
    const float btnH = 50.0f;
    const float btnY = windowY + windowH - 80.0f;
    const float btnGap = 40.0f;
    const float btnX0 = windowX + (windowW - (btnW * 2 + btnGap)) * 0.5f;

    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};

    // 保存ボタン
    Rect saveRect{btnX0, btnY, btnW, btnH};
    bool saveHovered = mouse.x >= saveRect.x && mouse.x <= saveRect.x + saveRect.width &&
                       mouse.y >= saveRect.y && mouse.y <= saveRect.y + saveRect.height;
    const char* saveTexture = saveHovered
        ? ui::UiAssetKeys::ButtonPrimaryHover
        : ui::UiAssetKeys::ButtonPrimaryNormal;
    systemAPI_->Render().DrawUiNineSlice(saveTexture, saveRect, 8, 8, 8, 8,
                                         whiteColor);
    systemAPI_->Render().DrawTextDefault(
        "保存", saveRect.x + 70.0f, saveRect.y + 12.0f, 28.0f,
        ToCoreColor(ui::OverlayColors::TEXT_DARK));

    // キャンセルボタン
    Rect cancelRect{btnX0 + btnW + btnGap, btnY, btnW, btnH};
    bool cancelHovered = mouse.x >= cancelRect.x && mouse.x <= cancelRect.x + cancelRect.width &&
                         mouse.y >= cancelRect.y && mouse.y <= cancelRect.y + cancelRect.height;
    const char* cancelTexture = cancelHovered
        ? ui::UiAssetKeys::ButtonSecondaryHover
        : ui::UiAssetKeys::ButtonSecondaryNormal;
    systemAPI_->Render().DrawUiNineSlice(cancelTexture, cancelRect, 8, 8, 8, 8,
                                         whiteColor);
    systemAPI_->Render().DrawTextDefault(
        "キャンセル", cancelRect.x + 40.0f, cancelRect.y + 12.0f, 28.0f,
        ToCoreColor(ui::OverlayColors::TEXT_DARK));

    // ボタンクリック処理
    if (ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
        if (saveHovered) {
            SaveQueueToStageData(ctx);
            requestClose_ = true;
            ctx.inputAPI->ConsumeLeftClick();
        } else if (cancelHovered) {
            requestClose_ = true;
            ctx.inputAPI->ConsumeLeftClick();
        }
    }
}

bool CustomStageEnemyQueueOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool CustomStageEnemyQueueOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

void CustomStageEnemyQueueOverlay::LoadAvailableCharacters(SharedContext& ctx) {
    availableCharacters_.clear();
    if (!ctx.gameplayDataAPI) return;

    auto allCharacterIds = ctx.gameplayDataAPI->GetAllCharacterIds();
    for (const auto& charId : allCharacterIds) {
        auto charState = ctx.gameplayDataAPI->GetCharacterState(charId);
        if (charState.unlocked) {
            auto character = ctx.gameplayDataAPI->GetCharacterTemplate(charId);
            if (character) {
                availableCharacters_.push_back(character.get());
            }
        }
    }

    LOG_INFO("Loaded {} available characters for custom stage queue", availableCharacters_.size());
}

void CustomStageEnemyQueueOverlay::LoadQueueFromStageData(SharedContext& ctx) {
    queue_.clear();
    if (!ctx.gameplayDataAPI || targetStageId_.empty()) return;

    auto stageData = ctx.gameplayDataAPI->GetStageDataById(targetStageId_);
    if (!stageData) return;

    try {
        if (stageData->data.contains("customEnemyQueue") &&
            stageData->data["customEnemyQueue"].is_array()) {
            for (const auto& entryJson : stageData->data["customEnemyQueue"]) {
                CustomEnemyEntry entry;
                entry.enemyId = entryJson.value("enemyId", "");
                entry.level = entryJson.value("level", 1);
                entry.spawnDelay = entryJson.value("spawnDelay", 1.0f);
                if (!entry.enemyId.empty()) {
                    queue_.push_back(entry);
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARN("Failed to load custom queue from stage data: {}", e.what());
    }

    LOG_INFO("Loaded {} entries from custom queue", queue_.size());
}

void CustomStageEnemyQueueOverlay::SaveQueueToStageData(SharedContext& ctx) {
    if (!ctx.gameplayDataAPI || targetStageId_.empty()) return;

    auto stageData = ctx.gameplayDataAPI->GetStageDataById(targetStageId_);
    if (!stageData) return;

    try {
        nlohmann::json queueArray = nlohmann::json::array();
        for (const auto& entry : queue_) {
            nlohmann::json entryJson;
            entryJson["enemyId"] = entry.enemyId;
            entryJson["level"] = entry.level;
            entryJson["spawnDelay"] = entry.spawnDelay;
            queueArray.push_back(entryJson);
        }
        stageData->data["customEnemyQueue"] = queueArray;
        LOG_INFO("Saved {} entries to custom queue", queue_.size());
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save custom queue to stage data: {}", e.what());
    }
}

void CustomStageEnemyQueueOverlay::HandleKeyboardInput(SharedContext& ctx) {
    if (!ctx.inputAPI) return;

    if (ctx.inputAPI->IsEscapePressed()) {
        requestClose_ = true;
    }
}

void CustomStageEnemyQueueOverlay::HandleMouseInput(SharedContext& ctx) {
    // マウス入力処理はRender内で実装
    (void)ctx;
}

void CustomStageEnemyQueueOverlay::RenderCharacterList(SharedContext& ctx) {
    const float windowW = 1600.0f;
    const float windowH = 900.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.5f;

    const float panelX = windowX + 20.0f;
    const float panelY = windowY + 100.0f;
    const float panelW = 400.0f;
    const float panelH = windowH - 200.0f;

    // パネル背景
    Rect panelRect{panelX, panelY, panelW, panelH};
    ColorRGBA panelBg = ToCoreColor(ui::OverlayColors::PANEL_BG_SECONDARY);
    panelBg.a = 240;
    systemAPI_->Render().DrawRectangleRec(panelRect, panelBg);
    systemAPI_->Render().DrawRectangleLines(panelRect.x, panelRect.y,
                                            panelRect.width, panelRect.height, 2.0f,
                                            ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // タイトル
    systemAPI_->Render().DrawTextDefault("保有キャラクター", panelX + 10.0f, panelY + 10.0f,
                                         32.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // キャラクター一覧
    const float cardW = 180.0f;
    const float cardH = 100.0f;
    const float cardGapX = 10.0f;
    const float cardGapY = 10.0f;
    const int cols = 2;
    const float startY = panelY + 60.0f;
    const float visibleHeight = panelH - 70.0f;

    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};

    int index = 0;
    for (const auto* character : availableCharacters_) {
        const int row = index / cols;
        const int col = index % cols;
        const float cardX = panelX + 10.0f + col * (cardW + cardGapX);
        const float cardY = startY + row * (cardH + cardGapY) - characterListScrollOffset_;

        // 画面外の場合はスキップ
        if (cardY + cardH < panelY || cardY > panelY + panelH) {
            index++;
            continue;
        }

        Rect cardRect{cardX, cardY, cardW, cardH};
        bool isHovered = mouse.x >= cardRect.x && mouse.x <= cardRect.x + cardRect.width &&
                         mouse.y >= cardRect.y && mouse.y <= cardRect.y + cardRect.height;
        bool isSelected = (selectedCharacterIndex_ == index);

        ColorRGBA cardBg = isSelected
            ? ToCoreColor(ui::OverlayColors::CARD_BG_SELECTED)
            : (isHovered
                   ? ToCoreColor(ui::OverlayColors::SLOT_HOVER)
                   : ToCoreColor(ui::OverlayColors::CARD_BG_NORMAL));
        systemAPI_->Render().DrawRectangleRec(cardRect, cardBg);
        systemAPI_->Render().DrawRectangleLines(cardRect.x, cardRect.y,
                                                cardRect.width, cardRect.height, 2.0f,
                                                ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

        // キャラクター名
        std::string charName = character->name.empty() ? character->id : character->name;
        if (charName.length() > 12) {
            charName = charName.substr(0, 12) + "...";
        }
        systemAPI_->Render().DrawTextDefault(charName, cardX + 5.0f, cardY + 5.0f,
                                            20.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

        // レベル表示
        int maxLevel = GetCharacterMaxLevel(ctx, character->id);
        std::string levelText = "Lv." + std::to_string(maxLevel) + "まで";
        systemAPI_->Render().DrawTextDefault(levelText, cardX + 5.0f, cardY + 30.0f,
                                            18.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

        // クリック処理
        if (isHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            selectedCharacterIndex_ = index;
            selectedLevel_ = 1;
            ctx.inputAPI->ConsumeLeftClick();
        }

        index++;
    }

    // スクロール処理
    if (ctx.inputAPI) {
        float wheelMove = ctx.inputAPI->GetMouseWheelMove();
        if (wheelMove != 0.0f && mouse.x >= panelX && mouse.x <= panelX + panelW &&
            mouse.y >= panelY && mouse.y <= panelY + panelH) {
            characterListScrollOffset_ -= wheelMove * 30.0f;
            characterListScrollOffset_ = std::max(0.0f, characterListScrollOffset_);
        }
    }
}

void CustomStageEnemyQueueOverlay::RenderSelectionPanel(SharedContext& ctx) {
    const float windowW = 1600.0f;
    const float windowH = 900.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.5f;

    const float panelX = windowX + 440.0f;
    const float panelY = windowY + 100.0f;
    const float panelW = 400.0f;
    const float panelH = windowH - 200.0f;

    // パネル背景
    Rect panelRect{panelX, panelY, panelW, panelH};
    ColorRGBA panelBg = ToCoreColor(ui::OverlayColors::PANEL_BG_SECONDARY);
    panelBg.a = 240;
    systemAPI_->Render().DrawRectangleRec(panelRect, panelBg);
    systemAPI_->Render().DrawRectangleLines(panelRect.x, panelRect.y,
                                            panelRect.width, panelRect.height, 2.0f,
                                            ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // タイトル
    systemAPI_->Render().DrawTextDefault("設定", panelX + 10.0f, panelY + 10.0f,
                                         32.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    float y = panelY + 60.0f;

    // 選択中のキャラクター情報
    if (selectedCharacterIndex_ >= 0 && selectedCharacterIndex_ < static_cast<int>(availableCharacters_.size())) {
        const auto* character = availableCharacters_[selectedCharacterIndex_];
        std::string charName = character->name.empty() ? character->id : character->name;
        systemAPI_->Render().DrawTextDefault("キャラクター: " + charName, panelX + 10.0f, y,
                                             24.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        y += 40.0f;

        // レベル設定
        int maxLevel = GetCharacterMaxLevel(ctx, character->id);
        systemAPI_->Render().DrawTextDefault("レベル: " + std::to_string(selectedLevel_) + " / " + std::to_string(maxLevel),
                                             panelX + 10.0f, y, 22.0f,
                                             ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        y += 35.0f;

        // レベルスライダー（簡易実装：+/-ボタン）
        const float btnW = 50.0f;
        const float btnH = 40.0f;
        auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};

        // -ボタン
        Rect minusRect{panelX + 10.0f, y, btnW, btnH};
        bool minusHovered = mouse.x >= minusRect.x && mouse.x <= minusRect.x + minusRect.width &&
                             mouse.y >= minusRect.y && mouse.y <= minusRect.y + minusRect.height;
        ColorRGBA minusBg = minusHovered
            ? ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY)
            : ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY);
        systemAPI_->Render().DrawRectangleRec(minusRect, minusBg);
        systemAPI_->Render().DrawTextDefault("-", minusRect.x + 18.0f, minusRect.y + 8.0f,
                                             28.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
        if (minusHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            if (selectedLevel_ > 1) {
                selectedLevel_--;
                ctx.inputAPI->ConsumeLeftClick();
            }
        }

        // +ボタン
        Rect plusRect{panelX + 70.0f, y, btnW, btnH};
        bool plusHovered = mouse.x >= plusRect.x && mouse.x <= plusRect.x + plusRect.width &&
                           mouse.y >= plusRect.y && mouse.y <= plusRect.y + plusRect.height;
        ColorRGBA plusBg = plusHovered
            ? ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY)
            : ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY);
        systemAPI_->Render().DrawRectangleRec(plusRect, plusBg);
        systemAPI_->Render().DrawTextDefault("+", plusRect.x + 18.0f, plusRect.y + 8.0f,
                                             28.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
        if (plusHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            if (selectedLevel_ < maxLevel) {
                selectedLevel_++;
                ctx.inputAPI->ConsumeLeftClick();
            }
        }

        y += 60.0f;

        // スポーン遅延設定
        systemAPI_->Render().DrawTextDefault("スポーン遅延: " + std::to_string(selectedSpawnDelay_) + "秒",
                                             panelX + 10.0f, y, 22.0f,
                                             ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        y += 35.0f;

        // 遅延スライダー（簡易実装：+/-ボタン）
        Rect delayMinusRect{panelX + 10.0f, y, btnW, btnH};
        bool delayMinusHovered = mouse.x >= delayMinusRect.x && mouse.x <= delayMinusRect.x + delayMinusRect.width &&
                                 mouse.y >= delayMinusRect.y && mouse.y <= delayMinusRect.y + delayMinusRect.height;
        systemAPI_->Render().DrawRectangleRec(delayMinusRect, ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY));
        systemAPI_->Render().DrawTextDefault("-", delayMinusRect.x + 18.0f, delayMinusRect.y + 8.0f,
                                             28.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
        if (delayMinusHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            if (selectedSpawnDelay_ > 0.1f) {
                selectedSpawnDelay_ -= 0.1f;
                ctx.inputAPI->ConsumeLeftClick();
            }
        }

        Rect delayPlusRect{panelX + 70.0f, y, btnW, btnH};
        bool delayPlusHovered = mouse.x >= delayPlusRect.x && mouse.x <= delayPlusRect.x + delayPlusRect.width &&
                                mouse.y >= delayPlusRect.y && mouse.y <= delayPlusRect.y + delayPlusRect.height;
        systemAPI_->Render().DrawRectangleRec(delayPlusRect, ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY));
        systemAPI_->Render().DrawTextDefault("+", delayPlusRect.x + 18.0f, delayPlusRect.y + 8.0f,
                                             28.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
        if (delayPlusHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            selectedSpawnDelay_ += 0.1f;
            ctx.inputAPI->ConsumeLeftClick();
        }

        y += 60.0f;

        // キューに追加ボタン
        Rect addRect{panelX + 10.0f, y, panelW - 20.0f, 50.0f};
        bool addHovered = mouse.x >= addRect.x && mouse.x <= addRect.x + addRect.width &&
                          mouse.y >= addRect.y && mouse.y <= addRect.y + addRect.height;
        const char* addTexture = addHovered
            ? ui::UiAssetKeys::ButtonPrimaryHover
            : ui::UiAssetKeys::ButtonPrimaryNormal;
        ColorRGBA whiteColor2{255, 255, 255, 255};
        systemAPI_->Render().DrawUiNineSlice(addTexture, addRect, 8, 8, 8, 8,
                                             whiteColor2);
        systemAPI_->Render().DrawTextDefault("キューに追加", addRect.x + 100.0f, addRect.y + 12.0f,
                                             26.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
        if (addHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            CustomEnemyEntry entry;
            entry.enemyId = character->id;
            entry.level = selectedLevel_;
            entry.spawnDelay = selectedSpawnDelay_;
            queue_.push_back(entry);
            ctx.inputAPI->ConsumeLeftClick();
        }
    } else {
        systemAPI_->Render().DrawTextDefault("キャラクターを選択してください", panelX + 10.0f, y,
                                             22.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    }
}

void CustomStageEnemyQueueOverlay::RenderQueueList(SharedContext& ctx) {
    const float windowW = 1600.0f;
    const float windowH = 900.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.5f;

    const float panelX = windowX + 860.0f;
    const float panelY = windowY + 100.0f;
    const float panelW = 400.0f;
    const float panelH = windowH - 200.0f;

    // パネル背景
    Rect panelRect{panelX, panelY, panelW, panelH};
    ColorRGBA panelBg = ToCoreColor(ui::OverlayColors::PANEL_BG_SECONDARY);
    panelBg.a = 240;
    systemAPI_->Render().DrawRectangleRec(panelRect, panelBg);
    systemAPI_->Render().DrawRectangleLines(panelRect.x, panelRect.y,
                                            panelRect.width, panelRect.height, 2.0f,
                                            ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // タイトル
    std::string titleText = "キュー (" + std::to_string(queue_.size()) + ")";
    systemAPI_->Render().DrawTextDefault(titleText, panelX + 10.0f, panelY + 10.0f,
                                         32.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // キューリスト
    const float entryH = 80.0f;
    const float startY = panelY + 60.0f;
    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};

    for (size_t i = 0; i < queue_.size(); ++i) {
        const float entryY = startY + i * entryH - queueListScrollOffset_;

        // 画面外の場合はスキップ
        if (entryY + entryH < panelY || entryY > panelY + panelH) {
            continue;
        }

        const auto& entry = queue_[i];
        Rect entryRect{panelX + 10.0f, entryY, panelW - 20.0f, entryH - 5.0f};
        bool isHovered = mouse.x >= entryRect.x && mouse.x <= entryRect.x + entryRect.width &&
                         mouse.y >= entryRect.y && mouse.y <= entryRect.y + entryRect.height;
        bool isSelected = (selectedQueueIndex_ == static_cast<int>(i));

        ColorRGBA entryBg = isSelected
            ? ToCoreColor(ui::OverlayColors::CARD_BG_SELECTED)
            : (isHovered
                   ? ToCoreColor(ui::OverlayColors::SLOT_HOVER)
                   : ToCoreColor(ui::OverlayColors::CARD_BG_NORMAL));
        systemAPI_->Render().DrawRectangleRec(entryRect, entryBg);
        systemAPI_->Render().DrawRectangleLines(entryRect.x, entryRect.y,
                                                entryRect.width, entryRect.height, 2.0f,
                                                ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

        // キャラクター情報を取得
        std::string charName = entry.enemyId;
        if (ctx.gameplayDataAPI) {
            auto character = ctx.gameplayDataAPI->GetCharacterTemplate(entry.enemyId);
            if (character) {
                charName = character->name.empty() ? character->id : character->name;
            }
        }

        // エントリ情報表示
        std::string infoText = "#" + std::to_string(i + 1) + " " + charName + " Lv." + std::to_string(entry.level);
        systemAPI_->Render().DrawTextDefault(infoText, entryRect.x + 5.0f, entryRect.y + 5.0f,
                                             20.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        std::string delayText = "遅延: " + std::to_string(entry.spawnDelay) + "秒";
        systemAPI_->Render().DrawTextDefault(delayText, entryRect.x + 5.0f, entryRect.y + 30.0f,
                                             18.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

        // 削除ボタン
        const float delBtnW = 60.0f;
        const float delBtnH = 30.0f;
        Rect delRect{entryRect.x + entryRect.width - delBtnW - 5.0f, entryRect.y + 5.0f, delBtnW, delBtnH};
        bool delHovered = mouse.x >= delRect.x && mouse.x <= delRect.x + delRect.width &&
                          mouse.y >= delRect.y && mouse.y <= delRect.y + delRect.height;
        ColorRGBA delBg = delHovered
            ? ToCoreColor(ui::OverlayColors::DANGER_RED)
            : ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY);
        systemAPI_->Render().DrawRectangleRec(delRect, delBg);
        systemAPI_->Render().DrawTextDefault("削除", delRect.x + 12.0f, delRect.y + 5.0f,
                                             18.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        if (delHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
            queue_.erase(queue_.begin() + i);
            selectedQueueIndex_ = -1;
            ctx.inputAPI->ConsumeLeftClick();
            break;
        }

        // 上移動ボタン
        if (i > 0) {
            const float moveBtnW = 40.0f;
            const float moveBtnH = 25.0f;
            Rect upRect{entryRect.x + entryRect.width - delBtnW - moveBtnW - 10.0f, entryRect.y + 5.0f, moveBtnW, moveBtnH};
            bool upHovered = mouse.x >= upRect.x && mouse.x <= upRect.x + upRect.width &&
                             mouse.y >= upRect.y && mouse.y <= upRect.y + upRect.height;
            systemAPI_->Render().DrawRectangleRec(upRect, ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY));
            systemAPI_->Render().DrawTextDefault("↑", upRect.x + 12.0f, upRect.y + 2.0f,
                                                 18.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
            if (upHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
                std::swap(queue_[i], queue_[i - 1]);
                ctx.inputAPI->ConsumeLeftClick();
                break;
            }
        }

        // 下移動ボタン
        if (i < queue_.size() - 1) {
            const float moveBtnW = 40.0f;
            const float moveBtnH = 25.0f;
            Rect downRect{entryRect.x + entryRect.width - delBtnW - moveBtnW - 10.0f, entryRect.y + 30.0f, moveBtnW, moveBtnH};
            bool downHovered = mouse.x >= downRect.x && mouse.x <= downRect.x + downRect.width &&
                               mouse.y >= downRect.y && mouse.y <= downRect.y + downRect.height;
            systemAPI_->Render().DrawRectangleRec(downRect, ToCoreColor(ui::OverlayColors::BUTTON_SECONDARY));
            systemAPI_->Render().DrawTextDefault("↓", downRect.x + 12.0f, downRect.y + 2.0f,
                                                 18.0f, ToCoreColor(ui::OverlayColors::TEXT_DARK));
            if (downHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
                std::swap(queue_[i], queue_[i + 1]);
                ctx.inputAPI->ConsumeLeftClick();
                break;
            }
        }

        // クリックで選択
        if (isHovered && ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed() && !delHovered) {
            selectedQueueIndex_ = static_cast<int>(i);
            ctx.inputAPI->ConsumeLeftClick();
        }
    }

    // スクロール処理
    if (ctx.inputAPI) {
        float wheelMove = ctx.inputAPI->GetMouseWheelMove();
        if (wheelMove != 0.0f && mouse.x >= panelX && mouse.x <= panelX + panelW &&
            mouse.y >= panelY && mouse.y <= panelY + panelH) {
            queueListScrollOffset_ -= wheelMove * 30.0f;
            queueListScrollOffset_ = std::max(0.0f, queueListScrollOffset_);
        }
    }
}

int CustomStageEnemyQueueOverlay::GetCharacterMaxLevel(SharedContext& ctx, const std::string& characterId) const {
    if (!ctx.gameplayDataAPI) return 1;
    auto charState = ctx.gameplayDataAPI->GetCharacterState(characterId);
    return std::max(1, charState.level);
}

} // namespace core
} // namespace game
