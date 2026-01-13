#include "GachaOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <imgui.h>

namespace game {
namespace core {

GachaOverlay::GachaOverlay()
    : systemAPI_(nullptr), isInitialized_(false), requestClose_(false),
      hasTransitionRequest_(false), requestedNextState_(GameState::Title) {}

bool GachaOverlay::Initialize(BaseSystemAPI *systemAPI) {
  if (isInitialized_) {
    LOG_ERROR("GachaOverlay already initialized");
    return false;
  }

  if (!systemAPI) {
    LOG_ERROR("GachaOverlay: systemAPI is null");
    return false;
  }

  systemAPI_ = systemAPI;
  requestClose_ = false;
  hasTransitionRequest_ = false;

  // ホームスクリーン用のサイズ（ヘッダーとタブバーの間、左右に余白）
  const float marginLeft = 20.0f;
  const float marginRight = 20.0f;
  const float marginTop = 90.0f;
  const float marginBottom = 90.0f;
  const float contentWidth = 1920.0f - marginLeft - marginRight;
  const float contentHeight = 1080.0f - marginTop - marginBottom;

  panel_ = std::make_shared<ui::Panel>();
  panel_->SetId("gacha_panel");
  panel_->SetPosition(marginLeft, marginTop); // 画面内の絶対座標
  panel_->SetSize(contentWidth, contentHeight);
  panel_->SetRoot(true); // ルートパネルとして設定
  panel_->Initialize();

  gachaButton_ = std::make_shared<ui::Button>();
  gachaButton_->SetId("gacha_button");
  gachaButton_->SetPosition(contentWidth / 2.0f - 100.0f,
                            contentHeight / 2.0f - 30.0f); // 中央配置
  gachaButton_->SetSize(200.0f, 60.0f);
  gachaButton_->SetLabel("ガチャ!");
  gachaButton_->Initialize();

  gachaButton_->SetOnClickCallback([this, contentWidth, contentHeight]() {
    LOG_INFO("Gacha button clicked!");
    // ガチャ結果を表示（サンプル）
    resultCards_.clear();
    const float cardSpacing = 250.0f;
    const float cardStartX =
        (contentWidth - (3 * 200.0f + 2 * cardSpacing)) / 2.0f;
    const float cardY = contentHeight / 2.0f - 150.0f;
    for (int i = 0; i < 3; ++i) {
      auto card = std::make_shared<ui::Card>();
      card->SetId("gacha_result_" + std::to_string(i));
      card->SetPosition(cardStartX + i * (200.0f + cardSpacing), cardY);
      card->SetSize(200.0f, 300.0f);

      ui::CardContent content;
      content.title = "Character " + std::to_string(i + 1);
      content.description = "Rare character";
      content.imageId = "";
      card->SetContent(content);
      card->Initialize();

      resultCards_.push_back(card);
      panel_->AddChild(card);
    }
  });

  closeButton_ = std::make_shared<ui::Button>();
  closeButton_->SetId("close_button");
  closeButton_->SetPosition(contentWidth - 170.0f, 20.0f); // パネル内の相対座標
  closeButton_->SetSize(150.0f, 40.0f);
  closeButton_->SetLabel("Close");
  closeButton_->Initialize();

  closeButton_->SetOnClickCallback([this]() { requestClose_ = true; });

  panel_->AddChild(gachaButton_);
  panel_->AddChild(closeButton_);

  isInitialized_ = true;
  LOG_INFO("GachaOverlay initialized");
  return true;
}

void GachaOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }

  if (panel_) {
    panel_->Update(deltaTime);
  }

  if (systemAPI_->IsKeyPressed(256)) {
    requestClose_ = true;
  }
}

void GachaOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  if (panel_) {
    panel_->Render();
  }
}

void GachaOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  if (panel_) {
    panel_->Shutdown();
    panel_.reset();
  }

  resultCards_.clear();

  if (gachaButton_) {
    gachaButton_->Shutdown();
    gachaButton_.reset();
  }

  if (closeButton_) {
    closeButton_->Shutdown();
    closeButton_.reset();
  }

  isInitialized_ = false;
  systemAPI_ = nullptr;
  LOG_INFO("GachaOverlay shutdown");
}

bool GachaOverlay::RequestClose() const {
  if (requestClose_) {
    requestClose_ = false;
    return true;
  }
  return false;
}

bool GachaOverlay::RequestTransition(GameState &nextState) const {
  if (hasTransitionRequest_) {
    nextState = requestedNextState_;
    hasTransitionRequest_ = false;
    return true;
  }
  return false;
}

} // namespace core
} // namespace game
