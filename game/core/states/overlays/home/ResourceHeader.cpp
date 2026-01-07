#include "ResourceHeader.hpp"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../../utils/Log.h"
#include <raylib.h>

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

ResourceHeader::ResourceHeader()
    : gold_display_current_(0.0f)
    , gems_display_current_(0.0f)
{
    resources_ = {0, 0, 0, 100};
}

ResourceHeader::~ResourceHeader() {
}

bool ResourceHeader::Initialize() {
    gold_display_current_ = static_cast<float>(resources_.gold);
    gems_display_current_ = static_cast<float>(resources_.gems);
    return true;
}

void ResourceHeader::SetResources(const PlayerResources& resources) {
    resources_ = resources;
    // アニメーション用の現在値を更新（必要に応じて）
}

void ResourceHeader::Update(float deltaTime) {
    // リソース変化アニメーション（オプション）
    // 金額が変わったとき、スムーズに数字がカウントアップするなど
    const float animationSpeed = 5.0f;  // アニメーション速度
    
    // Gold アニメーション
    float targetGold = static_cast<float>(resources_.gold);
    if (gold_display_current_ < targetGold) {
        gold_display_current_ += (targetGold - gold_display_current_) * animationSpeed * deltaTime;
        if (gold_display_current_ > targetGold) gold_display_current_ = targetGold;
    } else if (gold_display_current_ > targetGold) {
        gold_display_current_ -= (gold_display_current_ - targetGold) * animationSpeed * deltaTime;
        if (gold_display_current_ < targetGold) gold_display_current_ = targetGold;
    }
    
    // Gems アニメーション
    float targetGems = static_cast<float>(resources_.gems);
    if (gems_display_current_ < targetGems) {
        gems_display_current_ += (targetGems - gems_display_current_) * animationSpeed * deltaTime;
        if (gems_display_current_ > targetGems) gems_display_current_ = targetGems;
    } else if (gems_display_current_ > targetGems) {
        gems_display_current_ -= (gems_display_current_ - targetGems) * animationSpeed * deltaTime;
        if (gems_display_current_ < targetGems) gems_display_current_ = targetGems;
    }
}

void ResourceHeader::Render(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        return;
    }
    
    // ヘッダー背景
    systemAPI->DrawRectangle(0, 0, 1920, HEADER_HEIGHT, Color{30, 30, 30, 255});
    
    // ヘッダー下部の境界線
    systemAPI->DrawLine(0, HEADER_HEIGHT, 1920, HEADER_HEIGHT, 2.0f, Color{80, 80, 80, 255});
    
    float fontSize = 28.0f;
    
    // 左側: Gold, Gems
    float xPos = 40.0f;
    float yPos = HEADER_HEIGHT / 2.0f;
    
    // Gold表示
    std::string goldText = "💰 Gold: " + std::to_string(static_cast<int>(gold_display_current_));
    Vector2 goldSize = systemAPI->MeasureTextDefault(goldText, fontSize, 1.0f);
    systemAPI->DrawTextDefault(goldText, xPos, yPos - goldSize.y / 2.0f, fontSize, Color{255, 215, 0, 255});
    
    // Gems表示
    xPos += goldSize.x + 60.0f;
    std::string gemsText = "💎 Gems: " + std::to_string(static_cast<int>(gems_display_current_));
    Vector2 gemsSize = systemAPI->MeasureTextDefault(gemsText, fontSize, 1.0f);
    systemAPI->DrawTextDefault(gemsText, xPos, yPos - gemsSize.y / 2.0f, fontSize, Color{255, 20, 147, 255});
    
    // 右側: Tickets
    std::string ticketText = "🎫 Tickets: " + std::to_string(resources_.tickets) + " / " + std::to_string(resources_.max_tickets);
    Vector2 ticketSize = systemAPI->MeasureTextDefault(ticketText, fontSize, 1.0f);
    float ticketX = 1920.0f - ticketSize.x - 40.0f;
    systemAPI->DrawTextDefault(ticketText, ticketX, yPos - ticketSize.y / 2.0f, fontSize, Color{144, 238, 144, 255});
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
