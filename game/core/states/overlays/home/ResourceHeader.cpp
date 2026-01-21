#include "ResourceHeader.hpp"
#include "../../../../utils/Log.h"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../ui/OverlayColors.hpp"
#include "../../../ui/UiAssetKeys.hpp"
#include "../../../config/RenderPrimitives.hpp"


namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

ResourceHeader::ResourceHeader()
    : gold_display_current_(0.0f) {
  resources_ = {0, 0, 0, 100};
}

ResourceHeader::~ResourceHeader() {}

bool ResourceHeader::Initialize() {
  gold_display_current_ = static_cast<float>(resources_.gold);
  return true;
}

void ResourceHeader::SetResources(const PlayerResources &resources) {
  resources_ = resources;
  // アニメーション用の現在値を更新�E�忁E��に応じて�E�E
}

void ResourceHeader::Update(float deltaTime) {
  // リソース変化アニメーション�E�オプション�E�E
  // 金額が変わったとき、スムーズに数字がカウントアチE�Eするなど
  const float animationSpeed = 5.0f; // アニメーション速度

  // Gold アニメーション
  float targetGold = static_cast<float>(resources_.gold);
  if (gold_display_current_ < targetGold) {
    gold_display_current_ +=
        (targetGold - gold_display_current_) * animationSpeed * deltaTime;
    if (gold_display_current_ > targetGold)
      gold_display_current_ = targetGold;
  } else if (gold_display_current_ > targetGold) {
    gold_display_current_ -=
        (gold_display_current_ - targetGold) * animationSpeed * deltaTime;
    if (gold_display_current_ < targetGold)
      gold_display_current_ = targetGold;
  }
}

void ResourceHeader::Render(BaseSystemAPI *systemAPI) {
  if (!systemAPI) {
    return;
  }

  // ヘッダー背景
  systemAPI->Render().DrawRectangle(0, 0, 1920, HEADER_HEIGHT,
                                    ToCoreColor(ui::OverlayColors::HEADER_BG));

  // ヘッダー下部の墁E��緁E
  systemAPI->Render().DrawLine(0, HEADER_HEIGHT, 1920, HEADER_HEIGHT, 2.0f,
                               ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

  ColorRGBA textColor = ToCoreColor(ui::OverlayColors::TEXT_PRIMARY);

  float fontSize = 32.0f;

  // 左側: Gold
  float xPos = 40.0f;
  float yPos = HEADER_HEIGHT / 2.0f;

  // Gold表示�E�絵斁E���Eフォント欠け�E可能性があるため使用しなぁE��E
  std::string goldText =
      "Gold: " + std::to_string(static_cast<int>(gold_display_current_));
  Vec2 goldSize =
      systemAPI->Render().MeasureTextDefaultCore(goldText, fontSize, 1.0f);
  systemAPI->Render().DrawTextDefault(goldText, xPos, yPos - goldSize.y / 2.0f,
                                      fontSize, textColor);

  // 右側: Tickets
  std::string ticketText = "🎫 Tickets: " + std::to_string(resources_.tickets) +
                           " / " + std::to_string(resources_.max_tickets);
  Vec2 ticketSize =
      systemAPI->Render().MeasureTextDefaultCore(ticketText, fontSize, 1.0f);
  float ticketX = 1920.0f - ticketSize.x - 40.0f;
  systemAPI->Render().DrawTextDefault(ticketText, ticketX,
                                      yPos - ticketSize.y / 2.0f, fontSize,
                                      textColor);
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
