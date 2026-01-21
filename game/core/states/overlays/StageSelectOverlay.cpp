#include "StageSelectOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
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
      targetScroll_(0.0f), animationTime_(0.0f), panelFadeAlpha_(0.0f),
      showDetailWindow_(false), detailWindowAlpha_(0.0f) {}

void StageSelectOverlay::LoadStageData(SharedContext &ctx) {
  stages_.clear();

  // StageManagerから全ステージデータを取得
  if (!ctx.gameplayDataAPI) {
    LOG_ERROR(
        "StageSelectOverlay: gameplayDataAPI is null, cannot load stage data");
    return;
  }

  stages_ = ctx.gameplayDataAPI->GetAllStageData();
  LOG_INFO("Loaded {} stages from GameplayDataAPI", stages_.size());
}

bool StageSelectOverlay::Initialize(BaseSystemAPI *systemAPI,
                                    UISystemAPI *uiAPI) {
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

  // 詳細ウィンドウのフェードイン
  if (showDetailWindow_) {
    detailWindowAlpha_ = std::min(1.0f, detailWindowAlpha_ + deltaTime * 3.0f);
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

void StageSelectOverlay::HandleMouseInput(SharedContext &ctx) {
  if (!systemAPI_)
    return;
  InputSystemAPI *inputAPI = ctx.inputAPI;
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
          for (const auto &stage : stages_) {
            if (stage.stageNumber == selectedStage_) {
              ctx.currentStageId = stage.id;
              LOG_INFO("Selected stage ID: {} (stageNumber: {})", stage.id,
                       selectedStage_);
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

      // 【詳細】ボタン
      if (mouseX >= PANEL_X + 240 && mouseX < PANEL_X + 390 &&
          mouseY >= BTN_Y && mouseY < BTN_Y + 50) {
        showDetailWindow_ = !showDetailWindow_;
        if (showDetailWindow_) {
          detailWindowAlpha_ = 0.0f;
        }
        LOG_INFO("Detail window toggled: {}", showDetailWindow_);
      }
    }
  } else if (inputAPI->IsRightClickPressed()) { // 右クリチE��
    requestClose_ = true;
  }
}

void StageSelectOverlay::HandleKeyboardInput(SharedContext &ctx) {
  if (!systemAPI_)
    return;
  InputSystemAPI *inputAPI = ctx.inputAPI;
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
      for (const auto &stage : stages_) {
        if (stage.stageNumber == selectedStage_) {
          ctx.currentStageId = stage.id;
          LOG_INFO("Selected stage ID: {} (stageNumber: {})", stage.id,
                   selectedStage_);
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

void StageSelectOverlay::HandleScrollInput(InputSystemAPI *inputAPI) {
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

void StageSelectOverlay::HandleCardSelection(int stageNumber,
                                             SharedContext &ctx) {
  selectedStage_ = stageNumber;
  panelFadeAlpha_ = 0.0f;

  // SharedContextに選択されたステージIDを設定
  for (const auto &stage : stages_) {
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
  systemAPI_->Render().DrawTextDefault("ステージ選択", MARGIN_X + 20,
                                       CONTENT_TOP + 15, 44.0f, panelTextColor);

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
    systemAPI_->Render().DrawRectangle(layout.screenX + offsetX + shadowSize,
                                       layout.screenY + offsetY + shadowSize,
                                       scaledW, scaledH, shadowColor);

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
    // ステージ番号（左上）
    std::string stageNumText = "Stage " + std::to_string(stage.stageNumber);
    systemAPI_->Render().DrawTextDefault(
        stageNumText, layout.screenX + offsetX + 15,
        layout.screenY + offsetY + 15, 28.0f, lightPanelTextColor);

    // 難易度（difficultyに基づいて★を表示）- 中央上部
    int difficultyStars = std::min(5, std::max(1, stage.difficulty));
    Color activeStarColor = Color{255, 215, 0, 255};     // ゴールド
    Color inactiveStarColor = Color{100, 100, 100, 100}; // グレー
    float starStartX =
        layout.screenX + offsetX + (layout.width - 5 * 22) * 0.5f;
    for (int s = 0; s < 5; ++s) {
      Color starColor =
          (s < difficultyStars) ? activeStarColor : inactiveStarColor;
      if (stage.isLocked) {
        starColor.a = static_cast<unsigned char>(starColor.a * 0.6f);
      }
      systemAPI_->Render().DrawTextDefault("★", starStartX + s * 22,
                                           layout.screenY + offsetY + 50, 24.0f,
                                           starColor);
    }

    // 獲得星�E�クリア済みのみ�E�E
    if (stage.isCleared) {
      for (int s = 0; s < 3; ++s) {
        Color earnedStarColor = Color{255, 215, 0, 255};
        systemAPI_->Render().DrawTextDefault(
            "★", layout.screenX + offsetX + 15 + s * 20,
            layout.screenY + offsetY + 45, 24.0f, earnedStarColor);
      }
    }

    // ステータス表示（下部中央）
    if (stage.isCleared) {
      Vector2 clearSize =
          systemAPI_->Render().MeasureTextDefault("CLEAR", 28.0f);
      systemAPI_->Render().DrawTextDefault(
          "CLEAR",
          layout.screenX + offsetX + (layout.width - clearSize.x) * 0.5f,
          layout.screenY + offsetY + layout.height - 35, 28.0f,
          lightPanelTextColor);
    } else if (stage.isLocked) {
      Vector2 lockedSize =
          systemAPI_->Render().MeasureTextDefault("LOCKED", 28.0f);
      systemAPI_->Render().DrawTextDefault(
          "LOCKED",
          layout.screenX + offsetX + (layout.width - lockedSize.x) * 0.5f,
          layout.screenY + offsetY + layout.height - 35, 28.0f,
          lightPanelTextColor);
    }

    // ボスステージ表示（中央）
    if (stage.isBoss) {
      // BOSS背景
      Rectangle bossRect{layout.screenX + offsetX + (layout.width - 120) * 0.5f,
                         layout.screenY + offsetY + 100, 120, 40};
      systemAPI_->Render().DrawRectangleRec(bossRect,
                                            ui::OverlayColors::BUTTON_PRIMARY);
      systemAPI_->Render().DrawRectangleLines(
          static_cast<int>(bossRect.x), static_cast<int>(bossRect.y),
          static_cast<int>(bossRect.width), static_cast<int>(bossRect.height),
          2.0f, ui::OverlayColors::BORDER_DEFAULT);
      Vector2 bossTextSize =
          systemAPI_->Render().MeasureTextDefault("BOSS", 32.0f);
      systemAPI_->Render().DrawTextDefault(
          "BOSS", bossRect.x + (bossRect.width - bossTextSize.x) * 0.5f,
          bossRect.y + (bossRect.height - bossTextSize.y) * 0.5f, 32.0f,
          ui::OverlayColors::TEXT_DARK);
    }

    // チャプター表示�E�小さく！E
    // チャプター表示（右上）
    std::string chapterText = "Ch." + std::to_string(stage.chapter);
    Vector2 chapterSize =
        systemAPI_->Render().MeasureTextDefault(chapterText, 20.0f);
    systemAPI_->Render().DrawTextDefault(
        chapterText,
        layout.screenX + offsetX + layout.width - chapterSize.x - 15,
        layout.screenY + offsetY + 15, 20.0f, lightPanelTextColor);
  }

  // ヒント表示�E�下部�E�E
  systemAPI_->Render().DrawTextDefault(
      "マウスホイールでスクロール | ESCで閉じる | Enterで開始", MARGIN_X + 30,
      CONTENT_BOTTOM - 35, 30.0f, panelTextColor);
}

void StageSelectOverlay::RenderDetailPanel(SharedContext &ctx) {
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
                                          3.0f, ui::OverlayColors::BORDER_GOLD);

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

  float textAlpha = panelFadeAlpha_;

  // ステージ画像プレースホルダー（枠線付き）
  const int IMAGE_W = PANEL_W - 20;
  const int IMAGE_H = 320;
  systemAPI_->Render().DrawRectangle(PANEL_X + 10, PANEL_Y + 10, IMAGE_W,
                                     IMAGE_H, ui::OverlayColors::PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleLines(
      PANEL_X + 10, PANEL_Y + 10, IMAGE_W, IMAGE_H, 2.0f,
      Color{200, 170, 100, static_cast<unsigned char>(150 * textAlpha)});
  Color detailTextColor = ui::OverlayColors::TEXT_PRIMARY;
  Color previewTextColor = ui::OverlayColors::TEXT_PRIMARY;
  systemAPI_->Render().DrawTextDefault("[Stage Preview]",
                                       PANEL_X + PANEL_W / 2 - 60,
                                       PANEL_Y + 160, 22.0f, previewTextColor);

  // ステージ詳細テキスト
  int textY = PANEL_Y + 340;

  // Chapter番号:チャプター名
  std::string chapterInfo = "Chapter " +
                            std::to_string(selectedStageData->chapter) + ": " +
                            selectedStageData->chapterName;
  systemAPI_->Render().DrawTextDefault(chapterInfo, PANEL_X + 20, textY, 32.0f,
                                       detailTextColor);
  textY += 45;

  // Stage番号:ステージ名
  std::string stageTitle = "Stage " +
                           std::to_string(selectedStageData->stageNumber) +
                           ": " + selectedStageData->stageName;
  systemAPI_->Render().DrawTextDefault(stageTitle, PANEL_X + 20, textY, 32.0f,
                                       detailTextColor);
  textY += 45;

  // 難易度:☆~☆☆☆☆☆
  systemAPI_->Render().DrawTextDefault("難易度: ", PANEL_X + 20, textY, 30.0f,
                                       detailTextColor);
  int difficultyStars = std::min(5, std::max(1, selectedStageData->difficulty));
  Color activeStarColor = Color{
      255, 215, 0, static_cast<unsigned char>(255 * textAlpha)}; // ゴールド
  Color inactiveStarColor = Color{
      100, 100, 100, static_cast<unsigned char>(100 * textAlpha)}; // グレー
  for (int i = 0; i < 5; ++i) {
    Color starColor =
        (i < difficultyStars) ? activeStarColor : inactiveStarColor;
    systemAPI_->Render().DrawTextDefault("★", PANEL_X + 120 + i * 25, textY,
                                         30.0f, starColor);
  }
  textY += 40;

  // 推奨レベル
  std::string recLevelText =
      "推奨レベル: Lv." + std::to_string(selectedStageData->recommendedLevel) +
      "+";
  systemAPI_->Render().DrawTextDefault(recLevelText, PANEL_X + 20, textY, 30.0f,
                                       detailTextColor);
  textY += 35;

  // Wave数
  std::string waveText =
      "敵数: " + std::to_string(selectedStageData->waveCount) + " Wave";
  // Wave数は詳細ウィンドウに移動（削除）
  textY += 50;

  // クリア基本報酬
  std::string rewardText =
      "クリア基本報酬: " + std::to_string(selectedStageData->rewardGold) + " G";
  systemAPI_->Render().DrawTextDefault(rewardText, PANEL_X + 20, textY, 30.0f,
                                       detailTextColor);
  textY += 40;

  // 報酬ボーナス条件
  if (!selectedStageData->bonusConditions.empty()) {
    systemAPI_->Render().DrawTextDefault("報酬ボーナス条件", PANEL_X + 20,
                                         textY, 30.0f, detailTextColor);
    textY += 40;

    for (const auto &bonus : selectedStageData->bonusConditions) {
      std::string bonusText = "★ " + bonus.description;
      if (bonus.rewardType == "gold" && bonus.rewardValue > 0) {
        bonusText += " (+" + std::to_string(bonus.rewardValue) + "%ゴールド)";
      } else if (bonus.rewardType == "item") {
        bonusText += " (アイテム獲得)";
      }
      systemAPI_->Render().DrawTextDefault(bonusText, PANEL_X + 30, textY,
                                           26.0f, detailTextColor);
      textY += 32;
    }
    textY += 10;
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
  auto mousePos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  bool startBtnHover =
      (mousePos.x >= startBtnX && mousePos.x < startBtnX + startBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  Rectangle startRect{startBtnX, startBtnY, startBtnW, startBtnH};
  const char *startTexture = (startBtnHover && !isLocked)
                                 ? UiAssetKeys::ButtonPrimaryHover
                                 : UiAssetKeys::ButtonPrimaryNormal;
  if (isLocked) {
    startTexture = UiAssetKeys::ButtonSecondaryNormal;
  }
  systemAPI_->Render().DrawUiNineSlice(startTexture, startRect, 8, 8, 8, 8,
                                       WHITE);

  systemAPI_->Render().DrawTextDefault(
      "【開始】", startBtnX + 45, startBtnY + 10, 36.0f,
      systemAPI_->Render().GetReadableTextColor(startTexture));

  // 【詳細惁E��】�Eタン�E�影付き�E�E
  const float detailBtnX = PANEL_X + 240.0f;
  const float detailBtnW = 150.0f;
  bool detailBtnHover =
      (mousePos.x >= detailBtnX && mousePos.x < detailBtnX + detailBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  Rectangle detailRect{detailBtnX, startBtnY, detailBtnW, startBtnH};
  const char *detailTexture = detailBtnHover
                                  ? UiAssetKeys::ButtonSecondaryHover
                                  : UiAssetKeys::ButtonSecondaryNormal;
  systemAPI_->Render().DrawUiNineSlice(detailTexture, detailRect, 8, 8, 8, 8,
                                       WHITE);

  systemAPI_->Render().DrawTextDefault(
      "【詳細】", detailBtnX + 30, startBtnY + 10, 36.0f,
      systemAPI_->Render().GetReadableTextColor(detailTexture));

  // ロチE��表示
  if (selectedStageData->isLocked) {
    textY += 70;
    systemAPI_->Render().DrawTextDefault("このステージはまだプレイできません",
                                         PANEL_X + 50, textY, 26.0f,
                                         detailTextColor);
  }
}

void StageSelectOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  RenderCards();
  RenderDetailPanel(ctx);

  // 詳細ウィンドウの描画
  if (showDetailWindow_) {
    RenderDetailWindow(ctx);
  }
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

void StageSelectOverlay::RenderDetailWindow(SharedContext &ctx) {
  if (selectedStage_ < 0) {
    return;
  }

  const StageData *selectedStageData = nullptr;
  for (const auto &stage : stages_) {
    if (stage.stageNumber == selectedStage_) {
      selectedStageData = &stage;
      break;
    }
  }

  if (!selectedStageData) {
    return;
  }

  using namespace ui;
  const int SCREEN_W = 1920;
  const int SCREEN_H = 1080;
  const float WINDOW_W = 800.0f;
  const float WINDOW_H = 700.0f;
  const float WINDOW_X = (SCREEN_W - WINDOW_W) * 0.5f;
  const float WINDOW_Y = (SCREEN_H - WINDOW_H) * 0.5f;

  // 背景オーバーレイ（半透明）
  Color overlayColor =
      Color{0, 0, 0, static_cast<unsigned char>(200 * detailWindowAlpha_)};
  systemAPI_->Render().DrawRectangle(0, 0, SCREEN_W, SCREEN_H, overlayColor);

  // ウィンドウ背景
  Color windowBg = OverlayColors::PANEL_BG_DARK;
  windowBg.a = static_cast<unsigned char>(255 * detailWindowAlpha_);
  Rectangle windowRect{WINDOW_X, WINDOW_Y, WINDOW_W, WINDOW_H};
  systemAPI_->Render().DrawRectangleRec(windowRect, windowBg);
  systemAPI_->Render().DrawRectangleLines(
      static_cast<int>(WINDOW_X), static_cast<int>(WINDOW_Y),
      static_cast<int>(WINDOW_W), static_cast<int>(WINDOW_H), 3.0f,
      OverlayColors::BORDER_GOLD);

  // タイトル
  Color titleColor = OverlayColors::TEXT_PRIMARY;
  titleColor.a = static_cast<unsigned char>(255 * detailWindowAlpha_);
  systemAPI_->Render().DrawTextDefault("詳細情報", WINDOW_X + 30, WINDOW_Y + 30,
                                       40.0f, titleColor);

  // 閉じるボタン
  auto mousePos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  const float CLOSE_BTN_SIZE = 40.0f;
  const float CLOSE_BTN_X = WINDOW_X + WINDOW_W - CLOSE_BTN_SIZE - 20;
  const float CLOSE_BTN_Y = WINDOW_Y + 20;
  bool closeBtnHover =
      (mousePos.x >= CLOSE_BTN_X && mousePos.x < CLOSE_BTN_X + CLOSE_BTN_SIZE &&
       mousePos.y >= CLOSE_BTN_Y && mousePos.y < CLOSE_BTN_Y + CLOSE_BTN_SIZE);

  Color closeBtnColor = closeBtnHover ? OverlayColors::BUTTON_PRIMARY
                                      : OverlayColors::BUTTON_SECONDARY;
  closeBtnColor.a = static_cast<unsigned char>(255 * detailWindowAlpha_);
  Rectangle closeRect{CLOSE_BTN_X, CLOSE_BTN_Y, CLOSE_BTN_SIZE, CLOSE_BTN_SIZE};
  systemAPI_->Render().DrawRectangleRec(closeRect, closeBtnColor);
  systemAPI_->Render().DrawTextDefault("×", CLOSE_BTN_X + 12, CLOSE_BTN_Y + 8,
                                       32.0f, titleColor);

  // 詳細情報の描画
  float textY = WINDOW_Y + 100;
  Color textColor = OverlayColors::TEXT_PRIMARY;
  textColor.a = static_cast<unsigned char>(255 * detailWindowAlpha_);

  // Wave数
  std::string waveText =
      "敵数: " + std::to_string(selectedStageData->waveCount) + " Wave";
  systemAPI_->Render().DrawTextDefault(waveText, WINDOW_X + 30, textY, 28.0f,
                                       textColor);
  textY += 40;

  // 獲得モンスター
  if (!selectedStageData->rewardMonsters.empty()) {
    systemAPI_->Render().DrawTextDefault("獲得モンスター", WINDOW_X + 30, textY,
                                         28.0f, textColor);
    textY += 40;

    for (const auto &monster : selectedStageData->rewardMonsters) {
      std::string monsterText =
          "・" + monster.monsterId + " Lv." + std::to_string(monster.level);
      systemAPI_->Render().DrawTextDefault(monsterText, WINDOW_X + 50, textY,
                                           24.0f, textColor);
      textY += 30;
    }
    textY += 10;
  }

  // 出現モンスター詳細
  if (!selectedStageData->enemySpawns.empty()) {
    systemAPI_->Render().DrawTextDefault("出現モンスター", WINDOW_X + 30, textY,
                                         28.0f, textColor);
    textY += 40;

    for (const auto &spawn : selectedStageData->enemySpawns) {
      if (spawn.minLevel == spawn.maxLevel) {
        std::string spawnText = "・" + spawn.monsterId + " Lv." +
                                std::to_string(spawn.minLevel) + " × " +
                                std::to_string(spawn.count) + "体";
        systemAPI_->Render().DrawTextDefault(spawnText, WINDOW_X + 50, textY,
                                             24.0f, textColor);
      } else {
        std::string spawnText = "・" + spawn.monsterId + " Lv." +
                                std::to_string(spawn.minLevel) + "-" +
                                std::to_string(spawn.maxLevel) + " × " +
                                std::to_string(spawn.count) + "体";
        systemAPI_->Render().DrawTextDefault(spawnText, WINDOW_X + 50, textY,
                                             24.0f, textColor);
      }
      textY += 30;
    }
    textY += 10;
  }

  // ボス戦フェーズ情報
  if (selectedStageData->isBoss && !selectedStageData->bossPhases.empty()) {
    systemAPI_->Render().DrawTextDefault("ボス戦フェーズ", WINDOW_X + 30, textY,
                                         28.0f, textColor);
    textY += 40;

    for (const auto &phase : selectedStageData->bossPhases) {
      std::string phaseText = phase.description;
      systemAPI_->Render().DrawTextDefault(phaseText, WINDOW_X + 50, textY,
                                           24.0f, textColor);
      textY += 30;

      for (const auto &action : phase.actions) {
        std::string actionText = "  - " + action;
        systemAPI_->Render().DrawTextDefault(actionText, WINDOW_X + 70, textY,
                                             22.0f, textColor);
        textY += 28;
      }
      textY += 10;
    }
    textY += 10;
  }

  // クリア実績 - クリア済みのみ
  if (selectedStageData->isCleared) {
    std::string clearText =
        "クリア状況: " + std::to_string(selectedStageData->starsEarned) +
        "/3 ★";
    systemAPI_->Render().DrawTextDefault(clearText, WINDOW_X + 30, textY, 28.0f,
                                         textColor);
    textY += 40;
  }

  // 閉じるボタンのクリック処理
  if (ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
    if (closeBtnHover) {
      showDetailWindow_ = false;
      LOG_INFO("Detail window closed");
    }
  }
}

} // namespace core
} // namespace game
