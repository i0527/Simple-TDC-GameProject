#include "StageSelectOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <cmath>
#include <raylib.h>

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
  if (!ctx.stageManager) {
    LOG_ERROR("StageSelectOverlay: stageManager is null, cannot load stage data");
    return;
  }

  stages_ = ctx.stageManager->GetAllStageData();
  LOG_INFO("Loaded {} stages from StageManager", stages_.size());
}

bool StageSelectOverlay::Initialize(BaseSystemAPI *systemAPI) {
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
  // Initialize()時点ではSharedContextが利用できないため

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

  // カードスケールをリセット
  for (auto &[stageId, scale] : cardScales_) {
    scale = 1.0f;
  }

  // ホバー中のカードのみスケール（選択中の拡大は廃止）
  if (hoveredStage_ >= 0) {
    float progress = std::min(1.0f, animationTime_ / 0.15f);
    cardScales_[hoveredStage_] = 1.0f + 0.1f * progress;
  }
}

void StageSelectOverlay::HandleMouseInput(SharedContext& ctx) {
  if (!systemAPI_)
    return;

  Vector2 mousePos = systemAPI_->GetMousePosition();
  int mouseX = static_cast<int>(mousePos.x);
  int mouseY = static_cast<int>(mousePos.y);

  // ホバー検出
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

  // ホバー対象が変わった時だけアニメーション時間をリセット
  if (hoveredStage_ != lastHovered) {
    animationTime_ = 0.0f;
  }

  // クリック検出
  if (systemAPI_->IsMouseButtonPressed(0)) { // 左クリック
    // カードクリック
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

    // 【開始】ボタンクリック
    if (selectedStage_ >= 0) {
      const int HEADER_HEIGHT = 90;
      const int TAB_HEIGHT = 90;
      const int MARGIN_X = 100;
      const int PANEL_X = MARGIN_X + 1140;
      const int PANEL_H = 1080 - HEADER_HEIGHT - TAB_HEIGHT;
      const int BTN_Y = HEADER_HEIGHT + PANEL_H - 120;

      if (mouseX >= PANEL_X + 20 && mouseX < PANEL_X + 220 && mouseY >= BTN_Y &&
          mouseY < BTN_Y + 50) {
        // ロック中でないことを確認
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
          
          // サウンドエフェクト再生（決定音）
          // systemAPI_->PlaySound("ui_confirm");

          LOG_INFO("Starting stage {}, transitioning to Game scene",
                   selectedStage_);
          hasTransitionRequest_ = true;
          requestedNextState_ = GameState::Game;
        } else {
          // ロック中の場合はエラー音を再生
          // systemAPI_->PlaySound("ui_error");
          LOG_WARN("Stage {} is locked, cannot start", selectedStage_);
        }
      }
    }
  } else if (systemAPI_->IsMouseButtonPressed(1)) { // 右クリック
    requestClose_ = true;
  }
}

void StageSelectOverlay::HandleKeyboardInput(SharedContext& ctx) {
  if (!systemAPI_)
    return;

  // ESCキーで閉じる
  if (systemAPI_->IsKeyPressed(KEY_ESCAPE)) {
    requestClose_ = true;
  }

  // Enterキーで開始
  if (systemAPI_->IsKeyPressed(KEY_ENTER) && selectedStage_ >= 0) {
    // ロック中でないことを確認
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
      
      // サウンドエフェクト再生（決定音）
      // systemAPI_->PlaySound("ui_confirm");

      LOG_INFO("Starting stage {} (Enter key), transitioning to Game scene",
               selectedStage_);
      hasTransitionRequest_ = true;
      requestedNextState_ = GameState::Game;
    } else {
      // ロック中の場合はエラー音を再生
      // systemAPI_->PlaySound("ui_error");
      LOG_WARN("Stage {} is locked, cannot start", selectedStage_);
    }
  }
}

void StageSelectOverlay::HandleScrollInput() {
  if (!systemAPI_)
    return;

  float wheelMove = systemAPI_->GetMouseWheelMove();
  if (wheelMove != 0.0f) {
    targetScroll_ -= wheelMove * 80.0f;
    targetScroll_ = std::max(0.0f, targetScroll_);

    // 最大スクロール位置を計算
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

  // サウンドエフェクト再生（選択音）
  if (systemAPI_) {
    // サウンドが存在する場合のみ再生
    // systemAPI_->PlaySound("ui_select");
  }

  LOG_INFO("Stage {} selected (ID: {})", stageNumber, ctx.currentStageId);
}

void StageSelectOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }

  // 初回のみステージデータを読み込む
  if (stages_.empty() && ctx.stageManager) {
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

  // レイアウトを再計算（スクロール反映）
  CalculateCardLayouts();

  // アニメーション更新
  UpdateAnimations(deltaTime);

  // 入力処理
  HandleMouseInput(ctx);
  HandleKeyboardInput(ctx);
  HandleScrollInput();
}

void StageSelectOverlay::RenderCards() {
  if (!systemAPI_)
    return;

  // ヘッダーとタブバーを避ける (正確な値を定数から取得)
  const int HEADER_HEIGHT = 90;
  const int TAB_HEIGHT = 90;
  const int MARGIN_X = 100; // 左右マージン

  const int CONTENT_TOP = HEADER_HEIGHT;
  const int CONTENT_BOTTOM = 1080 - TAB_HEIGHT;
  const int CONTENT_HEIGHT = CONTENT_BOTTOM - CONTENT_TOP;
  const int CONTENT_WIDTH = 1920 - MARGIN_X * 2;

  const int LEFT_PANEL_WIDTH = 1140;

  const int VIEWPORT_TOP = CONTENT_TOP;
  const int VIEWPORT_BOTTOM = CONTENT_BOTTOM;

  // 背景パネル（半透明）- 茶色系に統一
  systemAPI_->DrawRectangle(MARGIN_X, CONTENT_TOP, CONTENT_WIDTH,
                            CONTENT_HEIGHT, ui::OverlayColors::OVERLAY_BG);

  // 左側カードエリア背景（グラデーション風）- 茶色系に統一
  systemAPI_->DrawRectangle(MARGIN_X, CONTENT_TOP, LEFT_PANEL_WIDTH,
                            CONTENT_HEIGHT, ui::OverlayColors::PANEL_BG_DARK);
  systemAPI_->DrawRectangleGradientV(MARGIN_X, CONTENT_TOP, LEFT_PANEL_WIDTH,
                                     100, ui::OverlayColors::PANEL_BG,
                                     ui::OverlayColors::PANEL_BG_DARK);

  // タイトル
  Font *font = static_cast<Font *>(systemAPI_->GetDefaultFont());
  if (font) {
    // タイトルに影を追加
    systemAPI_->DrawTextWithFont(font, "ステージ選択", MARGIN_X + 22,
                                 CONTENT_TOP + 17, 36, Color{0, 0, 0, 100});
    systemAPI_->DrawTextWithFont(font, "ステージ選択", MARGIN_X + 20,
                                 CONTENT_TOP + 15, 36,
                                 ui::OverlayColors::TEXT_PRIMARY);
  }

  // 区切り線 - ゴールド系
  systemAPI_->DrawLine(MARGIN_X + 20, CONTENT_TOP + 60,
                       MARGIN_X + LEFT_PANEL_WIDTH - 20, CONTENT_TOP + 60, 2.0f,
                       Color{200, 170, 100, 100});

  // スクロールインジケーター
  const int CARD_H = 280;
  const int SPACING_V = 20;
  const int ROWS = (stages_.size() + 3) / 4;
  const int SCROLL_AREA_HEIGHT = CONTENT_HEIGHT - 80;
  const float maxScroll =
      std::max(0.0f, ROWS * (CARD_H + SPACING_V) -
                         static_cast<float>(SCROLL_AREA_HEIGHT));

  if (maxScroll > 0.0f) {
    // スクロールバー背景
    const int SCROLLBAR_X = MARGIN_X + LEFT_PANEL_WIDTH - 10;
    const int SCROLLBAR_TOP = CONTENT_TOP + 70;
    const int SCROLLBAR_HEIGHT = SCROLL_AREA_HEIGHT - 10;
    systemAPI_->DrawRectangle(SCROLLBAR_X, SCROLLBAR_TOP, 5, SCROLLBAR_HEIGHT,
                              ui::OverlayColors::SLOT_EMPTY);

    // スクロールバー - ゴールド系
    float barHeight = static_cast<float>(SCROLLBAR_HEIGHT) *
                      (static_cast<float>(SCROLL_AREA_HEIGHT) /
                       (ROWS * (CARD_H + SPACING_V)));
    float barY = SCROLLBAR_TOP +
                 (scrollPosition_ / maxScroll) * (SCROLLBAR_HEIGHT - barHeight);
    systemAPI_->DrawRectangle(SCROLLBAR_X, barY, 5, barHeight,
                              ui::OverlayColors::BORDER_GOLD);
  }

  // カード描画
  for (size_t i = 0; i < stages_.size(); ++i) {
    const auto &stage = stages_[i];
    const auto &layout = cardLayouts_[i];

    // カリング（画面外は描画しない）
    if (layout.screenY + layout.height < VIEWPORT_TOP ||
        layout.screenY > VIEWPORT_BOTTOM) {
      continue;
    }

    // アニメーション状態取得
    float scale = cardScales_.count(i) ? cardScales_[i] : 1.0f;
    float alpha = cardAlphas_.count(i) ? cardAlphas_[i] : 1.0f;

    // カード背景色 - 茶色系に統一
    Color cardColor = ui::OverlayColors::PANEL_BG_DARK;
    if (stage.isLocked) {
      cardColor =
          Color{80, 65, 50, static_cast<unsigned char>(255 * 0.6f * alpha)};
    } else {
      cardColor.a = static_cast<unsigned char>(255 * alpha);
    }

    // カード本体描画
    float scaledW = layout.width * scale;
    float scaledH = layout.height * scale;
    float offsetX = (layout.width - scaledW) * 0.5f;
    float offsetY = (layout.height - scaledH) * 0.5f;

    // 影を描画（ホバー時は大きく）
    float shadowSize = (hoveredStage_ == static_cast<int>(i)) ? 8.0f : 4.0f;
    Color shadowColor =
        Color{0, 0, 0,
              static_cast<unsigned char>(
                  50 + (hoveredStage_ == static_cast<int>(i) ? 50 : 0))};
    systemAPI_->DrawRectangle(layout.screenX + offsetX + shadowSize,
                              layout.screenY + offsetY + shadowSize, scaledW,
                              scaledH, shadowColor);

    // カード本体
    systemAPI_->DrawRectangle(layout.screenX + offsetX,
                              layout.screenY + offsetY, scaledW, scaledH,
                              cardColor);

    // ホバー時の光エフェクト
    if (hoveredStage_ == static_cast<int>(i) && !stage.isLocked) {
      Color lightColor =
          Color{200, 170, 100, static_cast<unsigned char>(20 * scale)};
      systemAPI_->DrawRectangle(layout.screenX + offsetX,
                                layout.screenY + offsetY, scaledW, scaledH,
                                lightColor);
    }

    // ボーダー - 茶色/ゴールド系に統一
    Color borderColor = ui::OverlayColors::BORDER_DEFAULT;
    float borderThickness = 2.0f;
    if (hoveredStage_ == static_cast<int>(i)) {
      borderColor = ui::OverlayColors::BORDER_GOLD;
      borderThickness = 2.0f;
    }
    if (stage.stageNumber == selectedStage_) {
      borderColor = ui::OverlayColors::BORDER_GOLD;
      borderThickness = 3.0f;
    }

    systemAPI_->DrawRectangleLines(layout.screenX + offsetX,
                                   layout.screenY + offsetY, scaledW, scaledH,
                                   borderThickness, borderColor);

    // テキスト描画（カード内 - オフセットを適用）
    if (font) {
      // ステージ番号
      std::string stageNumText = "Stage " + std::to_string(stage.stageNumber);
      systemAPI_->DrawTextWithFont(
          font, stageNumText, layout.screenX + offsetX + 20,
          layout.screenY + offsetY + 20, 24, ui::OverlayColors::TEXT_PRIMARY);

      // 難易度星
      for (int s = 0; s < 5; ++s) {
        Color starColor = s < stage.difficulty ? Color{255, 215, 0, 255}
                                               : Color{76, 76, 76, 255};
        systemAPI_->DrawTextWithFont(
            font, "★", layout.screenX + offsetX + 20 + s * 22,
            layout.screenY + offsetY + 55, 20, starColor);
      }

      // 獲得星（クリア済みのみ）
      if (stage.isCleared) {
        for (int s = 0; s < 3; ++s) {
          Color earnedStarColor = s < stage.starsEarned
                                      ? Color{255, 215, 0, 255}
                                      : Color{76, 76, 76, 255};
          systemAPI_->DrawTextWithFont(
              font, "★", layout.screenX + offsetX + 140 + s * 22,
              layout.screenY + offsetY + 20, 24, earnedStarColor);
        }
      }

      // ステータス表示
      if (stage.isCleared) {
        systemAPI_->DrawTextWithFont(
            font, "✓ CLEAR", layout.screenX + offsetX + 60,
            layout.screenY + offsetY + 240, 24, Color{100, 200, 100, 255});
      } else if (stage.isLocked) {
        systemAPI_->DrawTextWithFont(
            font, "🔒 LOCKED", layout.screenX + offsetX + 50,
            layout.screenY + offsetY + 240, 24, Color{128, 128, 128, 255});
      }

      // ボスステージ表示
      if (stage.isBoss) {
        // BOSS背景
        systemAPI_->DrawRectangle(layout.screenX + offsetX + 60,
                                  layout.screenY + offsetY + 110, 120, 45,
                                  Color{255, 80, 80, 200});
        systemAPI_->DrawRectangleLines(layout.screenX + offsetX + 60,
                                       layout.screenY + offsetY + 110, 120, 45,
                                       2.0f, Color{255, 120, 120, 255});
        systemAPI_->DrawTextWithFont(
            font, "BOSS", layout.screenX + offsetX + 75,
            layout.screenY + offsetY + 120, 28, Color{255, 255, 255, 255});
      }

      // チャプター表示（小さく）
      if (font) {
        std::string chapterText = "Ch." + std::to_string(stage.chapter);
        systemAPI_->DrawTextWithFont(
            font, chapterText, layout.screenX + offsetX + 180,
            layout.screenY + offsetY + 20, 14, Color{180, 180, 180, 255});
      }
    }
  }

  // ヒント表示（下部）
  if (font) {
    systemAPI_->DrawTextWithFont(
        font, "マウスホイールでスクロール | ESCで閉じる | Enterで開始",
        MARGIN_X + 30, CONTENT_BOTTOM - 35, 22, Color{180, 180, 180, 200});
  }
}

void StageSelectOverlay::RenderDetailPanel() {
  if (!systemAPI_ || selectedStage_ < 0 || panelFadeAlpha_ < 0.01f)
    return;

  const int HEADER_HEIGHT = 90;
  const int TAB_HEIGHT = 90;
  const int MARGIN_X = 100;
  const int PANEL_X = MARGIN_X + 1140;
  const int PANEL_Y = HEADER_HEIGHT;
  const int PANEL_W = 1920 - MARGIN_X * 2 - 1140;
  const int PANEL_H = 1080 - HEADER_HEIGHT - TAB_HEIGHT;

  // パネル背景（グラデーション）- 茶色系に統一
  systemAPI_->DrawRectangleGradientV(
      PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
      Color{140, 110, 80, static_cast<unsigned char>(220 * panelFadeAlpha_)},
      Color{100, 80, 60, static_cast<unsigned char>(200 * panelFadeAlpha_)});

  // ボーダー（左側を強調）- ゴールド系
  systemAPI_->DrawRectangleLines(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, 3.0f,
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

  Font *font = static_cast<Font *>(systemAPI_->GetDefaultFont());
  if (!font)
    return;

  float textAlpha = panelFadeAlpha_;

  // ステージ画像プレースホルダー（枠線付き）
  const int IMAGE_W = PANEL_W - 20;
  const int IMAGE_H = 320;
  systemAPI_->DrawRectangle(PANEL_X + 10, PANEL_Y + 10, IMAGE_W, IMAGE_H,
                            ui::OverlayColors::PANEL_BG_DARK);
  systemAPI_->DrawRectangleLines(
      PANEL_X + 10, PANEL_Y + 10, IMAGE_W, IMAGE_H, 2.0f,
      Color{200, 170, 100, static_cast<unsigned char>(150 * textAlpha)});
  systemAPI_->DrawTextWithFont(
      font, "[Stage Preview]", PANEL_X + PANEL_W / 2 - 60, PANEL_Y + 160, 14,
      Color{128, 128, 128, static_cast<unsigned char>(255 * textAlpha)});

  // ステージ情報テキスト
  int textY = PANEL_Y + 340;

  // チャプター名
  systemAPI_->DrawTextWithFont(
      font, selectedStageData->chapterName, PANEL_X + 20, textY, 28,
      Color{240, 170, 60, static_cast<unsigned char>(255 * textAlpha)});
  textY += 45;

  // ステージ名
  std::string stageTitle = "Stage " +
                           std::to_string(selectedStageData->stageNumber) +
                           ": " + selectedStageData->stageName;
  systemAPI_->DrawTextWithFont(
      font, stageTitle, PANEL_X + 20, textY, 26,
      Color{255, 255, 255, static_cast<unsigned char>(255 * textAlpha)});
  textY += 45;

  // 難易度
  systemAPI_->DrawTextWithFont(
      font, "難易度: ", PANEL_X + 20, textY, 24,
      Color{180, 180, 180, static_cast<unsigned char>(255 * textAlpha)});
  for (int i = 0; i < 5; ++i) {
    Color starColor =
        i < selectedStageData->difficulty
            ? Color{240, 170, 60, static_cast<unsigned char>(255 * textAlpha)}
            : Color{76, 76, 76, static_cast<unsigned char>(255 * textAlpha)};
    systemAPI_->DrawTextWithFont(font, "★", PANEL_X + 130 + i * 25, textY, 24,
                                 starColor);
  }
  textY += 40;

  // 推奨レベル
  std::string recLevelText =
      "推奨レベル: Lv." + std::to_string(selectedStageData->recommendedLevel) +
      "+";
  systemAPI_->DrawTextWithFont(
      font, recLevelText, PANEL_X + 20, textY, 24,
      Color{255, 255, 255, static_cast<unsigned char>(255 * textAlpha)});
  textY += 35;

  // Wave数
  std::string waveText =
      "敵数: " + std::to_string(selectedStageData->waveCount) + " Wave";
  systemAPI_->DrawTextWithFont(
      font, waveText, PANEL_X + 20, textY, 24,
      Color{255, 255, 255, static_cast<unsigned char>(255 * textAlpha)});
  textY += 50;

  // 報酬セクション
  systemAPI_->DrawTextWithFont(
      font, "── 報酬 ──────────────", PANEL_X + 20, textY, 24,
      Color{240, 170, 60, static_cast<unsigned char>(200 * textAlpha)});
  textY += 40;

  std::string rewardText =
      "クリア報酬: " + std::to_string(selectedStageData->rewardGold) + " G";
  systemAPI_->DrawTextWithFont(
      font, rewardText, PANEL_X + 20, textY, 24,
      Color{255, 215, 0, static_cast<unsigned char>(255 * textAlpha)});
  textY += 35;

  systemAPI_->DrawTextWithFont(
      font, "☆1クリア: スケジュール宝箱 ×1", PANEL_X + 20, textY, 24,
      Color{200, 200, 200, static_cast<unsigned char>(255 * textAlpha)});
  textY += 35;

  if (selectedStageData->isBoss) {
    systemAPI_->DrawTextWithFont(
        font, "☆3クリア: プレミアム宝箱 ×1 + 50G", PANEL_X + 20, textY, 24,
        Color{200, 200, 200, static_cast<unsigned char>(255 * textAlpha)});
    textY += 35;
  }
  textY += 25;

  // クリア実績（クリア済みのみ）
  if (selectedStageData->isCleared) {
    std::string clearText =
        "クリア状況: " + std::to_string(selectedStageData->starsEarned) +
        "/3 ★";
    systemAPI_->DrawTextWithFont(
        font, clearText, PANEL_X + 20, textY, 24,
        Color{100, 200, 100, static_cast<unsigned char>(255 * textAlpha)});
    textY += 40;
  }

  // ボタン（パネルの下部から計算）
  textY = PANEL_Y + PANEL_H - 120;

  // 【開始】ボタン（影付き）
  using namespace ui;
  const float startBtnX = PANEL_X + 20.0f;
  const float startBtnY = textY;
  const float startBtnW = 200.0f;
  const float startBtnH = 50.0f;

  bool isLocked = selectedStageData->isLocked;
  Vector2 mousePos = systemAPI_->GetMousePosition();
  bool startBtnHover =
      (mousePos.x >= startBtnX && mousePos.x < startBtnX + startBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  UIEffects::DrawModernButton(systemAPI_, startBtnX, startBtnY, startBtnW,
                              startBtnH, OverlayColors::BUTTON_PRIMARY_DARK,
                              OverlayColors::BUTTON_PRIMARY_BRIGHT,
                              startBtnHover && !isLocked, isLocked);

  systemAPI_->DrawTextWithFont(
      font, "【開始】", startBtnX + 45, startBtnY + 10, 28,
      isLocked ? OverlayColors::TEXT_DISABLED : OverlayColors::TEXT_DARK);

  // 【詳細情報】ボタン（影付き）
  const float detailBtnX = PANEL_X + 240.0f;
  const float detailBtnW = 150.0f;
  bool detailBtnHover =
      (mousePos.x >= detailBtnX && mousePos.x < detailBtnX + detailBtnW &&
       mousePos.y >= startBtnY && mousePos.y < startBtnY + startBtnH);

  UIEffects::DrawModernButton(systemAPI_, detailBtnX, startBtnY, detailBtnW,
                              startBtnH, OverlayColors::BUTTON_SECONDARY_DARK,
                              OverlayColors::BUTTON_SECONDARY_BRIGHT,
                              detailBtnHover, false);

  systemAPI_->DrawTextWithFont(font, "【詳細】", detailBtnX + 30,
                               startBtnY + 10, 28, OverlayColors::TEXT_DARK);

  // ロック表示
  if (selectedStageData->isLocked) {
    textY += 70;
    systemAPI_->DrawTextWithFont(
        font, "このステージはまだプレイできません", PANEL_X + 50, textY, 18,
        Color{255, 80, 80, static_cast<unsigned char>(255 * textAlpha)});
  }
}

void StageSelectOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  RenderCards();
  RenderDetailPanel();
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
