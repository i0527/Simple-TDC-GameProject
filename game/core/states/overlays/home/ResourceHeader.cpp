#include "ResourceHeader.hpp"
#include "TabBarManager.hpp"
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
    : gold_display_current_(0.0f)
    , current_tab_(HomeTab::StageSelect) {
  resources_ = {0, 0, 0, 100};
}

ResourceHeader::~ResourceHeader() {}

bool ResourceHeader::Initialize() {
  gold_display_current_ = static_cast<float>(resources_.gold);
  return true;
}

void ResourceHeader::SetResources(const PlayerResources &resources) {
  resources_ = resources;
  // アニメーション用の現在値を更新（必要に応じて）
}

void ResourceHeader::SetCurrentTab(HomeTab tab) {
  current_tab_ = tab;
}

std::string ResourceHeader::GetTabDisplayName(HomeTab tab) const {
  switch(tab) {
    case HomeTab::StageSelect: return "ステージ選択";
    case HomeTab::Formation: return "編成画面";
    case HomeTab::Unit: return "ユニット画面";
    case HomeTab::Enhancement: return "タワー強化";
    case HomeTab::Gacha: return "ガチャ";
    case HomeTab::Codex: return "図鑑";
    case HomeTab::Settings: return "設定";
    default: return "";
  }
}

void ResourceHeader::Update(float deltaTime) {
  // リソース変化アニメーション（オプション）
  // 金額が変わったとき、スムーズに数字がカウントアップするなど
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

  // ヘッダー下部の境界線
  systemAPI->Render().DrawLine(0, HEADER_HEIGHT, 1920, HEADER_HEIGHT, 2.0f,
                               ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

  ColorRGBA textColor = ToCoreColor(ui::OverlayColors::TEXT_PRIMARY);

  float fontSize = 32.0f;
  float yPos = HEADER_HEIGHT / 2.0f;

  // 左上: 現在のタブ名を表示
  std::string tabName = GetTabDisplayName(current_tab_);
  if (!tabName.empty()) {
    Vec2 tabNameSize =
        systemAPI->Render().MeasureTextDefaultCore(tabName, fontSize, 1.0f);
    float tabNameX = 40.0f;
    systemAPI->Render().DrawTextDefault(tabName, tabNameX,
                                        yPos - tabNameSize.y / 2.0f, fontSize,
                                        textColor);
  }

  // 右上: GoldとTicketsを右寄せで配置
  // まずTicketsを右端に配置
  std::string ticketText = "🎫 Tickets: " + std::to_string(resources_.tickets) +
                           " / " + std::to_string(resources_.max_tickets);
  Vec2 ticketSize =
      systemAPI->Render().MeasureTextDefaultCore(ticketText, fontSize, 1.0f);
  float ticketX = 1920.0f - ticketSize.x - 40.0f;
  systemAPI->Render().DrawTextDefault(ticketText, ticketX,
                                      yPos - ticketSize.y / 2.0f, fontSize,
                                      textColor);

  // GoldをTicketsの左側に配置（適切な間隔を空ける）
  std::string goldText =
      "Gold: " + std::to_string(static_cast<int>(gold_display_current_));
  Vec2 goldSize =
      systemAPI->Render().MeasureTextDefaultCore(goldText, fontSize, 1.0f);
  float spacing = 30.0f;  // GoldとTicketsの間隔
  float goldX = ticketX - goldSize.x - spacing;
  systemAPI->Render().DrawTextDefault(goldText, goldX,
                                      yPos - goldSize.y / 2.0f, fontSize,
                                      textColor);
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
