#include "StageSelectOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include <algorithm>
#include <cmath>

namespace game {
namespace core {

StageSelectOverlay::StageSelectOverlay()
    : systemAPI_(nullptr), isInitialized_(false), requestClose_(false),
      hasTransitionRequest_(false), requestedNextState_(GameState::Title),
      selectedStage_(-1), hoveredStage_(-1), scrollPosition_(0.0f),
      targetScroll_(0.0f), animationTime_(0.0f), panelFadeAlpha_(0.0f) {}

void StageSelectOverlay::LoadStageData(SharedContext& ctx) {
  stages_.clear();

  // StageManagerから全ステージデータを取得
  if (!ctx.gameplayDataAPI) {
    LOG_ERROR("StageSelectOverlay: gameplayDataAPI is null, cannot load stage data");
    return;
  }

  stages_ = ctx.gameplayDataAPI->GetAllStageData();
  LOG_INFO("Loaded {} stages from GameplayDataAPI", stages_.size());
}

bool StageSelectOverlay::Initialize(BaseSystemAPI *systemAPI, UISystemAPI* uiAPI) {
  if (isInitialized_) {
    LOG_ERROR("StageSelectOverlay already initialized");
    return false;
  }

  if (!systemAPI) {
    LOG_ERROR("StageSelectOverlay: systemAPI is null");
    return false;
  }

  systemAPI_ = systemAPI;
  requestClose_ = false;
  hasTransitionRequest_ = false;

  // ステージデータはUpdate()でSharedContext経由で読み込む
  // Initialize()時点ではSharedContextが利用できなぁE��めE

  isInitialized_ = true;
  LOG_INFO("StageSelectOverlay initialized");
  return true;
}

void StageSelectOverlay::CalculateCardLayouts() {
  cardLayouts_.clear();
  const int COLS = 4;
  const int CARD_W = 240;
  const int CARD_H = 280;
  const int SPACING_H = 20;
  const int SPACING_V = 20;
  const int MARGIN_X = 100;
  const int MARGIN_LEFT = MARGIN_X + 20;
  const int HEADER_HEIGHT = 90;
  const int MARGIN_TOP = HEADER_HEIGHT + 80; // ヘッダ + タイトル領域

  for (size_t i = 0; i < stages_.size(); ++i) {
    CardLayout layout;
    layout.gridX = i % COLS;
    layout.gridY = i / COLS;
    layout.screenX = MARGIN_LEFT + layout.gridX * (CARD_W + SPACING_H);
    layout.screenY =
        MARGIN_TOP + layout.gridY * (CARD_H + SPACING_V) - scrollPosition_;
    layout.width = CARD_W;
    layout.height = CARD_H;

    cardLayouts_.push_back(layout);
  }
}

void StageSelectOverlay::UpdateAnimations(float deltaTime) {
  animationTime_ += deltaTime;

  // パネルフェードイン
  if (selectedStage_ >= 0) {
    panelFadeAlpha_ = std::min(1.0f, panelFadeAlpha_ + deltaTime * 3.0f);
  }

  // カードスケールをリセチE��
  for (auto &[stageId, scale] : cardScales_) {
    scale = 1.0f;
  }

  // ホバー中のカード�Eみスケール�E�選択中の拡大は廁E���E�E
  if (hoveredStage_ >= 0) {
    float progress = std::min(1.0f, animationTime_ / 0.15f);
    cardScales_[hoveredStage_] = 1.0f + 0.1f * progress;
  }
}

void StageSelectOverlay::HandleMouseInput(SharedContext& ctx) {
  if (!systemAPI_)
    return;
  InputSystemAPI* inputAPI = ctx.inputAPI;
  if (!inputAPI)
    return;

  auto mousePos = inputAPI->GetMousePosition();
  int mouseX = static_cast<int>(mousePos.x);
  int mouseY = static_cast<int>(mousePos.y);

  // ホバー検�E
  int lastHovered = hoveredStage_;
  hoveredStage_ = -1;
  for (size_t i = 0; i < cardLayouts_.size(); ++i) {
    const auto &layout = cardLayouts_[i];
    if (mouseX >= layout.screenX && mouseX < layout.screenX + layout.width &&
        mouseY >= layout.screenY && mouseY < layout.screenY + layout.height) {
      hoveredStage_ = static_cast<int>(i);
      break;
    }
  }

  // ホバー対象が変わった時だけアニメーション時間をリセチE��
  if (hoveredStage_ != lastHovered) {
    animationTime_ = 0.0f;
  }

  // クリチE��検�E
  if (inputAPI->IsLeftClickPressed()) { // 左クリチE��
    // カードクリチE��
    for (size_t i = 0; i < cardLayouts_.size(); ++i) {
      const auto &layout = cardLayouts_[i];
      if (mouseX >= layout.screenX && mouseX < layout.screenX + layout.width &&
          mouseY >= layout.screenY && mouseY < layout.screenY + layout.height) {
        if (!stages_[i].isLocked) {
          HandleCardSelection(stages_[i].stageNumber, ctx);
        }
        return;
      }
    }

    // 【開始】�EタンクリチE��
    if (selectedStage_ >= 0) {
      const int HEADER_HEIGHT = 90;
      const int TAB_HEIGHT = 90;
      const int MARGIN_X = 100;
      const int PANEL_X = MARGIN_X + 1140;
      const int PANEL_H = 1080 - HEADER_HEIGHT - TAB_HEIGHT;
      const int BTN_Y = HEADER_HEIGHT + PANEL_H - 120;

      if (mouseX >= PANEL_X + 20 && mouseX < PANEL_X + 220 && mouseY >= BTN_Y &&
          mouseY < BTN_Y + 50) {
        // ロチE��中でなぁE��とを確誁E
        bool isLocked = false;
        for (const auto &stage : stages_) {
          if (stage.stageNumber == selectedStage_) {
            isLocked = stage.isLocked;
            break;
          }
        }

        if (!isLocked) {
          // SharedContextに選択されたステージIDを設定
          for (const auto& stage : stages_) {
            if (stage.stageNumber == selectedStage_) {
              ctx.currentStageId = stage.id;
              LOG_INFO("Selected stage ID: {} (stageNumber: {})", stage.id, selectedStage_);
              break;
            }
          }
          
          // サウンドエフェクト�E生（決定音�E�E
          // systemAPI_->PlaySound("ui_confirm");

          LOG_INFO("Starting stage {}, transitioning to Game scene",
                   selectedStage_);
          hasTransitionRequest_ = true;
          requestedNextState_ = GameState::Game;
        } else {
          // ロチE��中の場合�Eエラー音を�E甁E
          // systemAPI_->PlaySound("ui_error");
          LOG_WARN("Stage {} is locked, cannot start", selectedStage_);
        }
      }
    }
  } else if (inputAPI->IsRightClickPressed()) { // 右クリチE��
    requestClose_ = true;
  }
}

void StageSelectOverlay::HandleKeyboardInput(SharedContext& ctx) {
  if (!systemAPI_)
    return;
  InputSystemAPI* inputAPI = ctx.inputAPI;
  if (!inputAPI)
    return;

  // ESCキーで閉じる
  if (inputAPI->IsEscapePressed()) {
    requestClose_ = true;
  }

  // Enterキーで開始
  if (inputAPI->IsEnterPressed() && selectedStage_ >= 0) {
    // ロチE��中でなぁE��とを確誁E
    bool isLocked = false;
    for (const auto &stage : stages_) {
      if (stage.stageNumber == selectedStage_) {
        isLocked = stage.isLocked;
        break;
      }
    }

    if (!isLocked) {
      // SharedContextに選択されたステージIDを設定
      for (const auto& stage : stages_) {
        if (stage.stageNumber == selectedStage_) {
          ctx.currentStageId = stage.id;
          LOG_INFO("Selected stage ID: {} (stageNumber: {})", stage.id, selectedStage_);
          break;
        }
      }
      
      // サウンドエフェクト�E生（決定音�E�E
      // systemAPI_->PlaySound("ui_confirm");

      LOG_INFO("Starting stage {} (Enter key), transitioning to Game scene",
               selectedStage_);
      hasTransitionRequest_ = true;
      requestedNextState_ = GameState::Game;
    } else {
      // ロチE��中の場合�Eエラー音を�E甁E
      // systemAPI_->PlaySound("ui_error");
      LOG_WARN("Stage {} is locked, cannot start", selectedStage_);
    }
  }
}

void StageSelectOverlay::HandleScrollInput(InputSystemAPI* inputAPI) {
  if (!systemAPI_ || !inputAPI)
    return;

  float wheelMove = inputAPI->GetMouseWheelMove();
  if (wheelMove != 0.0f) {
    targetScroll_ -= wheelMove * 80.0f;
    targetScroll_ = std::max(0.0f, targetScroll_);

    // 最大スクロール位置を計箁E
    const int HEADER_HEIGHT = 90;
    const int TAB_HEIGHT = 90;
    const int CARD_H = 280;
    const int SPACING_V = 20;
    const int ROWS = (stages_.size() + 3) / 4;
    const int SCROLL_AREA_HEIGHT = (1080 - HEADER_HEIGHT - TAB_HEIGHT) - 70;
    const float maxScroll =
        std::max(0.0f, ROWS * (CARD_H + SPACING_V) -
                           static_cast<float>(SCROLL_AREA_HEIGHT));
    targetScroll_ = std::min(maxScroll, targetScroll_);
  }
}

void StageSelectOverlay::HandleCardSelection(int stageNumber, SharedContext& ctx) {
  selectedStage_ = stageNumber;
  panelFadeAlpha_ = 0.0f;

  // SharedContextに選択されたステージIDを設定
  for (const auto& stage : stages_) {
    if (stage.stageNumber == stageNumber) {
      ctx.currentStageId = stage.id;
      break;
    }
  }

  // サウンドエフェクト�E生（選択音�E�E
  if (systemAPI_) {
    // サウンドが存在する場合�Eみ再生
    // systemAPI_->PlaySound("ui_select");
  }

  LOG_INFO("Stage {} selected (ID: {})", stageNumber, ctx.currentStageId);
}

void StageSelectOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }

  // 初回のみステージデータを読み込む
  if (stages_.empty() && ctx.gameplayDataAPI) {
    LoadStageData(ctx);
    CalculateCardLayouts();
    
    // 最初のステージを自動選択
    if (!stages_.empty() && !stages_[0].isLocked) {
      selectedStage_ = stages_[0].stageNumber;
      panelFadeAlpha_ = 0.0f;
    }
  }

  // スムーススクロール
  if (std::abs(targetScroll_ - scrollPosition_) > 1.0f) {
    scrollPosition_ += (targetScroll_ - scrollPosition_) * 0.1f;
  } else {
    scrollPosition_ = targetScroll_;
  }

  // レイアウトを再計算（スクロール反映�E�E
  CalculateCardLayouts();

  // アニメーション更新
  UpdateAnimations(deltaTime);

  // 入力�E琁E
  HandleMouseInput(ctx);
  HandleKeyboardInput(ctx);
  HandleScrollInput(ctx.inputAPI);
}

void StageSelectOverlay::RenderCards() {
  if (!systemAPI_)
    return;

  // ヘッダーとタブバーを避ける (正確な値を定数から取征E
  const int HEADER_HEIGHT = 90;
  const int TAB_HEIGHT = 90;
  const int MARGIN_X = 100; // 左右マ�Eジン

  const int CONTENT_TOP = HEADER_HEIGHT;
  const int CONTENT_BOTTOM = 1080 - TAB_HEIGHT;
  const int CONTENT_HEIGHT = CONTENT_BOTTOM - CONTENT_TOP;
  const int CONTENT_WIDTH = 1920 - MARGIN_X * 2;

  const int LEFT_PANEL_WIDTH = 1140;

  const int VIEWPORT_TOP = CONTENT_TOP;
  const int VIEWPORT_BOTTOM = CONTENT_BOTTOM;

  systemAPI_->Render().DrawRectangle(MARGIN_X, CONTENT_TOP, CONTENT_WIDTH,
                                     CONTENT_HEIGHT,
                                     ui::OverlayColors::OVERLAY_BG);

  systemAPI_->Render().DrawRectangle(MARGIN_X, CONTENT_TOP, LEFT_PANEL_WIDTH,
                                     CONTENT_HEIGHT,
                                     ui::OverlayColors::PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleGradientV(
      MARGIN_X, CONTENT_TOP, LEFT_PANEL_WIDTH, 100, ui::OverlayColors::PANEL_BG,
      ui::OverlayColors::PANEL_BG_DARK);

  Color panelTextColor = ui::OverlayColors::TEXT_PRIMARY;
  Color lightPanelTextColor = ui::OverlayColors::TEXT_PRIMARY;

  // タイトル
  Font *font = static_cast<Font *>(systemAPI_->Resource().GetDefaultFont());
  if (font) {
    systemAPI_->Render().DrawTextWithFont(font, "ステージ選択", MARGIN_X + 20,
                                          CONTENT_TOP + 15, 36, panelTextColor);
  }

  // 区刁E��緁E
  systemAPI_->Render().DrawLine(
      MARGIN_X + 20, CONTENT_TOP + 60, MARGIN_X + LEFT_PANEL_WIDTH - 20,
      CONTENT_TOP + 60, 2.0f, Color{200, 170, 100, 100});

  // スクロールインジケーター
  const int CARD_H = 280;
  const int SPACING_V = 20;
  const int ROWS = (stages_.size() + 3) / 4;
  const int SCROLL_AREA_HEIGHT = CONTENT_HEIGHT - 80;
  const float maxScroll =
      std::max(0.0f, ROWS * (CARD_H + SPACING_V) -
                         static_cast<float>(SCROLL_AREA_HEIGHT));

  if (maxScroll > 0.0f) {
    // スクロールバ�E背景
    const int SCROLLBAR_X = MARGIN_X + LEFT_PANEL_WIDTH - 10;
    const int SCROLLBAR_TOP = CONTENT_TOP + 70;
    const int SCROLLBAR_HEIGHT = SCROLL_AREA_HEIGHT - 10;
    systemAPI_->Render().DrawRectangle(SCROLLBAR_X, SCROLLBAR_TOP, 5,
                                       SCROLLBAR_HEIGHT,
                                       ui::OverlayColors::SLOT_EMPTY);

    // スクロールバ�E - ゴールド系
    float barHeight = static_cast<float>(SCROLLBAR_HEIGHT) *
                      (static_cast<float>(SCROLL_AREA_HEIGHT) /
                       (ROWS * (CARD_H + SPACING_V)));
    float barY = SCROLLBAR_TOP +
                 (scrollPosition_ / maxScroll) * (SCROLLBAR_HEIGHT - barHeight);
    systemAPI_->Render().DrawRectangle(SCROLLBAR_X, barY, 5, barHeight,
                                       ui::OverlayColors::BORDER_GOLD);
  }

  // カード描画
  for (size_t i = 0; i < stages_.size(); ++i) {
    const auto &stage = stages_[i];
    const auto &layout = cardLayouts_[i];

    // カリング�E�画面外�E描画しなぁE��E
    if (layout.screenY + layout.height < VIEWPORT_TOP ||
        layout.screenY > VIEWPORT_BOTTOM) {
      continue;
    }

    // アニメーション状態取征E
    float scale = cardScales_.count(i) ? cardScales_[i] : 1.0f;
    float alpha = cardAlphas_.count(i) ? cardAlphas_[i] : 1.0f;

    // カード本体描画
    float scaledW = layout.width * scale;
    float scaledH = layout.height * scale;
    float offsetX = (layout.width - scaledW) * 0.5f;
    float offsetY = (layout.height - scaledH) * 0.5f;

    // 影を描画�E��Eバ�E時�E大きく�E�E
    float shadowSize = (hoveredStage_ == static_cast<int>(i)) ? 8.0f : 4.0f;
    Color shadowColor =
        Color{0, 0, 0,
              static_cast<unsigned char>(
                  50 + (hoveredStage_ == static_cast<int>(i) ? 50 : 0))};
    systemAPI_->Render().DrawRectangle(
        layout.screenX + offsetX + shadowSize,
        layout.screenY + offsetY + shadowSize, scaledW, scaledH, shadowColor);

    Color cardColor = ui::OverlayColors::PANEL_BG_DARK;
    if (stage.isLocked) {
      cardColor =
          Color{80, 65, 50, static_cast<unsigned char>(255 * 0.6f * alpha)};
    } else {
      cardColor.a = static_cast<unsigned char>(255 * alpha);
    }
    systemAPI_->Render().DrawRectangle(layout.screenX + offsetX,
                                       layout.screenY + offsetY, scaledW,
                                       scaledH, cardColor);

    Color borderColor = ui::OverlayColors::BORDER_DEFAULT;
    float borderThickness = 2.0f;
    if (hoveredStage_ == static_cast<int>(i)) {
      borderColor = ui::OverlayColors::BORDER_GOLD;
    }
    if (stage.stageNumber == selectedStage_) {
      borderColor = ui::OverlayColors::BORDER_GOLD;
      borderThickness = 3.0f;
    }
    systemAPI_->Render().DrawRectangleLines(
        layout.screenX + offsetX, layout.screenY + offsetY, scaledW, scaledH,
        borderThickness, borderColor);

    // チE��スト描画�E�カード�E - オフセチE��を適用�E�E
    if (font) {
      // ステージ番号
      std::string stageNumText = "Stage " + std::to_string(stage.stageNumber);
      systemAPI_->Render().DrawTextWithFont(
          font, stageNumText, layout.screenX + offsetX + 20,
          layout.screenY + offsetY + 20, 24, lightPanelTextColor);

      // 難易度昁E
      for (int s = 0; s < 5; ++s) {
        Color starColor = lightPanelTextColor;
      systemAPI_->Render().DrawTextWithFont(
          font, "★", layout.screenX + offsetX + 20 + s * 22,
          layout.screenY + offsetY + 55, 20, starColor);
      }

      // 獲得星�E�クリア済みのみ�E�E
      if (stage.isCleared) {
        for (int s = 0; s < 3; ++s) {
          Color earnedStarColor = lightPanelTextColor;
          systemAPI_->Render().DrawTextWithFont(
              font, "★", layout.screenX + offsetX + 140 + s * 22,
              layout.screenY + offsetY + 20, 24, earnedStarColor);
        }
      }

      // ステータス表示
      if (stage.isCleared) {
        systemAPI_->Render().DrawTextWithFont(
            font, "CLEAR", layout.screenX + offsetX + 60,
            layout.screenY + offsetY + 240, 24, lightPanelTextColor);
      } else if (stage.isLocked) {
        systemAPI_->Render().DrawTextWithFont(
            font, "LOCKED", layout.screenX + offsetX + 50,
            layout.screenY + offsetY + 240, 24, lightPanelTextColor);
      }

      // ボスステージ表示
      if (stage.isBoss) {
        // BOSS背景
        Rectangle bossRect{layout.screenX + offsetX + 60,
                           layout.screenY + offsetY + 110, 120, 45};
        systemAPI_->Render().DrawRectangleRec(bossRect,
                                              ui::OverlayColors::BUTTON_PRIMARY);
        systemAPI_->Render().DrawRectangleLines(
            static_cast<int>(bossRect.x), static_cast<int>(bossRect.y),
            static_cast<int>(bossRect.width),
            static_cast<int>(bossRect.height), 2.0f,
            ui::OverlayColors::BORDER_DEFAULT);
        systemAPI_->Render().DrawTextWithFont(
            font, "BOSS", layout.screenX + offsetX + 75,
            layout.screenY + offsetY + 120, 28,
            ui::OverlayColors::TEXT_DARK);
      }

      // チャプター表示�E�小さく！E
      if (font) {
        std::string chapterText = "Ch." + std::to_string(stage.chapter);
        systemAPI_->Render().DrawTextWithFont(
            font, chapterText, layout.screenX + offsetX + 180,
            layout.screenY + offsetY + 20, 14, lightPanelTextColor);
      }
    }
  }

  // ヒント表示�E�下部�E�E
  if (font) {
    systemAPI_->Render().DrawTextWithFont(
        font, "マウスホイールでスクロール | ESCで閉じる | Enterで開始",
        MARGIN_X + 30, CONTENT_BOTTOM - 35, 22, panelTextColor);
  }
}

void StageSelectOverlay::RenderDetailPanel(SharedContext& ctx) {
  if (!systemAPI_ || selectedStage_ < 0 || panelFadeAlpha_ < 0.01f)
    return;

  const int HEADER_HEIGHT = 90;
  const int TAB_HEIGHT = 90;
  const int MARGIN_X = 100;
  const int PANEL_X = MARGIN_X + 1140;
  const int PANEL_Y = HEADER_HEIGHT;
  const int PANEL_W = 1920 - MARGIN_X * 2 - 1140;
  const int PANEL_H = 1080 - HEADER_HEIGHT - TAB_HEIGHT;

  systemAPI_->Render().DrawRectangleGradientV(
      PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
      Color{140, 110, 80, static_cast<unsigned char>(220 * panelFadeAlpha_)},
      Color{100, 80, 60, static_cast<unsigned char>(200 * panelFadeAlpha_)});
  systemAPI_->Render().DrawRectangleLines(PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
                                          3.0f,
                                          ui::OverlayColors::BORDER_GOLD);

  // 選択中のステージデータを取得
  StageData *selectedStageData = nullptr;
  for (auto &stage : stages_) {
    if (stage.stageNumber == selectedStage_) {
      selectedStageData = &stage;
      break;
    }
  }

  if (!selectedStageData)
    return;

  Font *font = static_cast<Font *>(systemAPI_->Resource().GetDefaultFont());
  if (!font)
    return;

  float textAlpha = panelFadeAlpha_;

  // ステージ画像プレースホルダー（枠線付き）
  const int IMAGE_W = PANEL_W - 20;
  const int IMAGE_H = 320;
  systemAPI_->Render().DrawRectangle(PANEL_X + 10, PANEL_Y + 10, IMAGE_W,
                                     IMAGE_H,
                                     ui::OverlayColors::PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleLines(
      PANEL_X + 10, PANEL_Y + 10, IMAGE_W, IMAGE_H, 2.0f,
      Color{200, 170, 100, static_cast<unsigned char>(150 * textAlpha)});
  Color detailTextColor = ui::OverlayColors::TEXT_PRIMARY;
  Color previewTextColor = ui::OverlayColors::TEXT_PRIMARY;
  systemAPI_->Render().DrawTextWithFont(
      font, "[Stage Preview]", PANEL_X + PANEL_W / 2 - 60, PANEL_Y + 160, 14,
      previewTextColor);

  // ステージ詳細テキスト
  int textY = PANEL_Y + 340;

  // チャプター吁E
  systemAPI_->Render().DrawTextWithFont(
      font, selectedStageData->chapterName, PANEL_X + 20, textY, 28,
      detailTextColor);
  textY += 45;

  // ステージ名
  std::string stageTitle = "Stage " +
                           std::to_string(selectedStageData->stageNumber) +
                           ": " + selectedStageData->stageName;
  systemAPI_->Render().DrawTextWithFont(
      font, stageTitle, PANEL_X + 20, textY, 26,
      detailTextColor);
  textY += 45;

  // 難易度
  systemAPI_->Render().DrawTextWithFont(
      font, "難易度: ", PANEL_X + 20, textY, 24,
      detailTextColor);
  for (int i = 0; i < 5; ++i) {
    Color starColor = detailTextColor;
    systemAPI_->Render().DrawTextWithFont(
        font, "★", PANEL_X + 130 + i * 25, textY, 24, starColor);
  }
  textY += 40;

  // 推奨レベル
  std::string recLevelText =
      "推奨レベル: Lv." + std::to_string(selectedStageData->recommendedLevel) +
      "+";
  systemAPI_->Render().DrawTextWithFont(
      font, recLevelText, PANEL_X + 20, textY, 24,
      detailTextColor);
  textY += 35;

  // Wave数
  std::string waveText =
      "敵数: " + std::to_string(selectedStageData->waveCount) + " Wave";
  systemAPI_->Render().DrawTextWithFont(
      font, waveText, PANEL_X + 20, textY, 24,
      detailTextColor);
  textY += 50;

  // 報酬セクション
  systemAPI_->Render().DrawTextWithFont(
      font, "── 報酬 ──────────────", PANEL_X + 20, textY, 24,
      detailTextColor);
  textY += 40;

  std::string rewardText =
      "クリア報酬: " + std::to_string(selectedStageData->rewardGold) + " G";
  systemAPI_->Render().DrawTextWithFont(
      font, rewardText, PANEL_X + 20, textY, 24,
      detailTextColor);
  textY += 35;

    systemAPI_->Render().DrawTextWithFont(
        font, "★クリア: スケジュール宝箱 A", PANEL_X + 20, textY, 24,
        detailTextColor);
  textY += 35;

  if (selectedStageData->isBoss) {
    systemAPI_->Render().DrawTextWithFont(
        font, "★クリア: プレミアム宝箱 A + 50G", PANEL_X + 20, textY, 24,
        detailTextColor);
    textY += 35;
  }
  textY += 25;

  // クリア実績�E�クリア済みのみ�E�E
  if (selectedStageData->isCleared) {
    std::string clearText =
        "クリア状況: " + std::to_string(selectedStageData->starsEarned) + "/3 ★";
    systemAPI_->Render().DrawTextWithFont(
        font, clearText, PANEL_X + 20, textY, 24,
        detailTextColor);
    textY += 40;
  }

  // ボタン�E�パネルの下部から計算！E
  textY = PANEL_Y + PANEL_H - 120;

  // 【開始】�Eタン�E�影付き�E�E
  using namespace ui;
  const float startBtnX = PANEL_X + 20.0f;
  const float startBtnY = textY;
  const float startBtnW = 200.0f;
  const float startBtnH = 50.0f;

  bool isLocked = selectedStageData->isLocked;
  auto mousePos = ctx.inputAPI ? ctx.inputAPI->GetMousePosition()
                               : Vec2{0.0f, 0.0f};
  bool startBtnHover =
      (mousePos.x >= startBtnX && mousePos.x < startBtnX + startBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  Rectangle startRect{startBtnX, startBtnY, startBtnW, startBtnH};
  const char* startTexture = (startBtnHover && !isLocked)
      ? UiAssetKeys::ButtonPrimaryHover
      : UiAssetKeys::ButtonPrimaryNormal;
  if (isLocked) {
      startTexture = UiAssetKeys::ButtonSecondaryNormal;
  }
  systemAPI_->Render().DrawUiNineSlice(startTexture, startRect, 8, 8, 8, 8,
                                       WHITE);

  systemAPI_->Render().DrawTextWithFont(
      font, "【開始】", startBtnX + 45, startBtnY + 10, 28,
      systemAPI_->Render().GetReadableTextColor(startTexture));

  // 【詳細惁E��】�Eタン�E�影付き�E�E
  const float detailBtnX = PANEL_X + 240.0f;
  const float detailBtnW = 150.0f;
  bool detailBtnHover =
      (mousePos.x >= detailBtnX && mousePos.x < detailBtnX + detailBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  Rectangle detailRect{detailBtnX, startBtnY, detailBtnW, startBtnH};
  const char* detailTexture = detailBtnHover
      ? UiAssetKeys::ButtonSecondaryHover
      : UiAssetKeys::ButtonSecondaryNormal;
  systemAPI_->Render().DrawUiNineSlice(detailTexture, detailRect, 8, 8, 8, 8,
                                       WHITE);

  systemAPI_->Render().DrawTextWithFont(
      font, "【詳細】", detailBtnX + 30, startBtnY + 10, 28,
      systemAPI_->Render().GetReadableTextColor(detailTexture));

  // ロチE��表示
  if (selectedStageData->isLocked) {
    textY += 70;
    systemAPI_->Render().DrawTextWithFont(
        font, "このステージはまだプレイできません", PANEL_X + 50, textY, 18,
        detailTextColor);
  }
}

void StageSelectOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  RenderCards();
  RenderDetailPanel(ctx);
}

void StageSelectOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  stages_.clear();
  cardLayouts_.clear();
  cardScales_.clear();
  cardAlphas_.clear();

  isInitialized_ = false;
  systemAPI_ = nullptr;
  LOG_INFO("StageSelectOverlay shutdown");
}

bool StageSelectOverlay::RequestClose() const {
  if (requestClose_) {
    requestClose_ = false;
    return true;
  }
  return false;
}

bool StageSelectOverlay::RequestTransition(GameState &nextState) const {
  if (hasTransitionRequest_) {
    nextState = requestedNextState_;
    hasTransitionRequest_ = false;
    return true;
  }
  return false;
}

} // namespace core
} // namespace game
