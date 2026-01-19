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
    : gold_display_current_(0.0f), gems_display_current_(0.0f) {
  resources_ = {0, 0, 0, 100};
}

ResourceHeader::~ResourceHeader() {}

bool ResourceHeader::Initialize() {
  gold_display_current_ = static_cast<float>(resources_.gold);
  gems_display_current_ = static_cast<float>(resources_.gems);
  return true;
}

void ResourceHeader::SetResources(const PlayerResources &resources) {
  resources_ = resources;
  // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ逕ｨ縺ｮ迴ｾ蝨ｨ蛟､繧呈峩譁ｰ・亥ｿ・ｦ√↓蠢懊§縺ｦ・・
}

void ResourceHeader::Update(float deltaTime) {
  // 繝ｪ繧ｽ繝ｼ繧ｹ螟牙喧繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ・医が繝励す繝ｧ繝ｳ・・
  // 驥鷹｡阪′螟峨ｏ縺｣縺溘→縺阪√せ繝繝ｼ繧ｺ縺ｫ謨ｰ蟄励′繧ｫ繧ｦ繝ｳ繝医い繝・・縺吶ｋ縺ｪ縺ｩ
  const float animationSpeed = 5.0f; // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ騾溷ｺｦ

  // Gold 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ
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

  // Gems 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ
  float targetGems = static_cast<float>(resources_.gems);
  if (gems_display_current_ < targetGems) {
    gems_display_current_ +=
        (targetGems - gems_display_current_) * animationSpeed * deltaTime;
    if (gems_display_current_ > targetGems)
      gems_display_current_ = targetGems;
  } else if (gems_display_current_ > targetGems) {
    gems_display_current_ -=
        (gems_display_current_ - targetGems) * animationSpeed * deltaTime;
    if (gems_display_current_ < targetGems)
      gems_display_current_ = targetGems;
  }
}

void ResourceHeader::Render(BaseSystemAPI *systemAPI) {
  if (!systemAPI) {
    return;
  }

  // 繝倥ャ繝繝ｼ閭梧勹
  systemAPI->Render().DrawRectangle(0, 0, 1920, HEADER_HEIGHT,
                                    ToCoreColor(ui::OverlayColors::HEADER_BG));

  // 繝倥ャ繝繝ｼ荳矩Κ縺ｮ蠅・阜邱・
  systemAPI->Render().DrawLine(0, HEADER_HEIGHT, 1920, HEADER_HEIGHT, 2.0f,
                               ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

  ColorRGBA textColor = ToCoreColor(ui::OverlayColors::TEXT_PRIMARY);

  float fontSize = 32.0f;

  // 蟾ｦ蛛ｴ: Gold, Gems
  float xPos = 40.0f;
  float yPos = HEADER_HEIGHT / 2.0f;

  // Gold陦ｨ遉ｺ・育ｵｵ譁・ｭ励・繝輔か繝ｳ繝域ｬ縺代・蜿ｯ閭ｽ諤ｧ縺後≠繧九◆繧∽ｽｿ逕ｨ縺励↑縺・ｼ・
  std::string goldText =
      "Gold: " + std::to_string(static_cast<int>(gold_display_current_));
  Vec2 goldSize =
      systemAPI->Render().MeasureTextDefaultCore(goldText, fontSize, 1.0f);
  systemAPI->Render().DrawTextDefault(goldText, xPos, yPos - goldSize.y / 2.0f,
                                      fontSize, textColor);

  // Gems陦ｨ遉ｺ
  xPos += goldSize.x + 80.0f;
  std::string gemsText =
      "虫 Gems: " + std::to_string(static_cast<int>(gems_display_current_));
  Vec2 gemsSize =
      systemAPI->Render().MeasureTextDefaultCore(gemsText, fontSize, 1.0f);
  systemAPI->Render().DrawTextDefault(gemsText, xPos, yPos - gemsSize.y / 2.0f,
                                      fontSize, textColor);

  // 蜿ｳ蛛ｴ: Tickets
  std::string ticketText = "辞 Tickets: " + std::to_string(resources_.tickets) +
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
