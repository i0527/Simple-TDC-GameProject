#include "HUDRenderer.hpp"
#include "../../utils/Log.h"
#include <sstream>
#include <iomanip>

namespace game {
namespace core {
namespace ui {

HUDRenderer::HUDRenderer(BaseSystemAPI* sysAPI)
    : sysAPI_(sysAPI) {
}

void HUDRenderer::RenderTopHUD(int wave, int totalWaves, int hp, int maxHp,
                               int gold, float gameSpeed, bool isPaused,
                               const std::string& gameStateText) {
    // 閭梧勹謠冗判
    ColorRGBA bgColor = {
        static_cast<unsigned char>(30),
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawRectangle(0, 0, 1920, 50, bgColor);
    
    // 繝懊・繝繝ｼ邱・
    ColorRGBA borderColor = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(110),
        static_cast<unsigned char>(120),
        static_cast<unsigned char>(50)
    };
    sysAPI_->Render().DrawLine(0, 50, 1920, 50, 2.0f, borderColor);
    
    // 繝・く繧ｹ繝郁牡
    ColorRGBA textWhite = ToCoreColor(WHITE);
    ColorRGBA textGray = {
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(255)
    };
    ColorRGBA textGold = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    ColorRGBA textRed = {
        static_cast<unsigned char>(255),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(255)
    };
    
    // Wave陦ｨ遉ｺ・亥ｷｦ・・
    std::ostringstream waveText;
    waveText << "Wave: " << wave << "/" << totalWaves;
    DrawText(10, 15, waveText.str(), 18, textWhite);
    
    // HP陦ｨ遉ｺ・井ｸｭ螟ｮ蟾ｦ・・
    std::ostringstream hpText;
    hpText << "HP: " << hp << "/" << maxHp;
    DrawText(160, 18, "HP:", 14, textGray);
    
    // HP繝舌・
    ColorRGBA hpColorGreen = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    ColorRGBA hpColor = (hp < maxHp * 0.3f) ? textRed : hpColorGreen;
    ColorRGBA hpBgColor = {
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(255)
    };
    DrawBar(200, 18, 120, 14, static_cast<float>(hp), static_cast<float>(maxHp), 
            hpColor, hpBgColor);
    
    // HP謨ｰ蛟､
    DrawText(330, 18, hpText.str(), 12, textGray);
    
    // Gold陦ｨ遉ｺ・井ｸｭ螟ｮ・・
    std::ostringstream goldText;
    goldText << "Gold: " << gold << " G";
    DrawText(450, 15, goldText.str(), 18, textGold);
    
    // 繧ｲ繝ｼ繝迥ｶ諷九ユ繧ｭ繧ｹ繝茨ｼ井ｸｭ螟ｮ蜿ｳ・・
    DrawText(650, 18, gameStateText, 14, textGray);
    
    // 繧ｹ繝斐・繝峨・繧ｿ繝ｳ・亥承荳ｭ螟ｮ・・
    topHudButtons_.clear();
    
    float speedBtnY = 8;
    float speedBtnWidth = 80;
    float speedBtnHeight = 34;
    float speedBtnSpacing = 10;
    float speedBtnStartX = 900;
    
    // ﾃ・.5
    bool isSpeed05 = (std::abs(gameSpeed - 0.5f) < 0.01f);
    ColorRGBA speedBtn1Color = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX, speedBtnY, speedBtnWidth, speedBtnHeight,
               "x0.5", isSpeed05, speedBtn1Color);
    topHudButtons_.push_back({speedBtnStartX, speedBtnY, speedBtnWidth, speedBtnHeight, "speed_0.5"});
    
    // ﾃ・.0
    bool isSpeed10 = (std::abs(gameSpeed - 1.0f) < 0.01f);
    ColorRGBA speedBtn2Color = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX + speedBtnWidth + speedBtnSpacing, speedBtnY, 
               speedBtnWidth, speedBtnHeight, "x1.0", isSpeed10, speedBtn2Color);
    topHudButtons_.push_back({speedBtnStartX + speedBtnWidth + speedBtnSpacing, 
                             speedBtnY, speedBtnWidth, speedBtnHeight, "speed_1.0"});
    
    // ﾃ・.0
    bool isSpeed20 = (std::abs(gameSpeed - 2.0f) < 0.01f);
    ColorRGBA speedBtn3Color = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX + (speedBtnWidth + speedBtnSpacing) * 2, speedBtnY,
               speedBtnWidth, speedBtnHeight, "x2.0", isSpeed20, speedBtn3Color);
    topHudButtons_.push_back({speedBtnStartX + (speedBtnWidth + speedBtnSpacing) * 2,
                             speedBtnY, speedBtnWidth, speedBtnHeight, "speed_2.0"});
    
    // Pause繝懊ち繝ｳ・亥承・・
    float pauseBtnX = 1660;
    ColorRGBA pauseBtnColor = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    DrawButton(pauseBtnX, speedBtnY, 120, speedBtnHeight,
               isPaused ? "Resume" : "Pause", isPaused, pauseBtnColor);
    topHudButtons_.push_back({pauseBtnX, speedBtnY, 120, speedBtnHeight, "pause"});
    
    // Exit繝懊ち繝ｳ・亥承遶ｯ・・
    float exitBtnX = 1790;
    ColorRGBA exitBtnColor = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    DrawButton(exitBtnX, speedBtnY, 120, speedBtnHeight,
               "Exit", false, exitBtnColor);
    topHudButtons_.push_back({exitBtnX, speedBtnY, 120, speedBtnHeight, "exit"});
}

void HUDRenderer::RenderLeftPanel(float fieldOriginX, float fieldOriginY,
                                  float fieldWidth, float fieldHeight) {
    // 閭梧勹謠冗判
    ColorRGBA bgColor = {
        static_cast<unsigned char>(25),
        static_cast<unsigned char>(30),
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawRectangle(0, 50, 640, 1030, bgColor);
    
    // 繝懊・繝繝ｼ邱・
    ColorRGBA borderColor = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(90),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawLine(640, 50, 640, 1080, 2.0f, borderColor);
    
    // 繧ｿ繧､繝医Ν
    ColorRGBA textWhite = ToCoreColor(WHITE);
    DrawText(20, 60, "Field Overview", 16, textWhite);
    
    // 繝溘ル繝槭ャ繝玲棧・育ｰ｡譏楢｡ｨ遉ｺ・・
    float miniMapX = 20;
    float miniMapY = 100;
    float miniMapWidth = 600;
    float miniMapHeight = 900;
    
    ColorRGBA miniMapBorderColor = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(110),
        static_cast<unsigned char>(120),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawRectangleLines(miniMapX, miniMapY, miniMapWidth,
                                         miniMapHeight, 2.0f, miniMapBorderColor);
    
    // 繝溘ル繝槭ャ繝苓ｪｬ譏弱ユ繧ｭ繧ｹ繝・
    ColorRGBA textGray = {
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(255)
    };
    DrawText(miniMapX + 10, miniMapY + 10, 
            "Mini Map (Future Implementation)", 12, textGray);
}

void HUDRenderer::RenderRightPanel(const std::vector<UnitListItem>& units,
                                   const std::string& selectedUnitId) {
    // 閭梧勹謠冗判
    ColorRGBA bgColor = {
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(45),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawRectangle(1600, 50, 320, 1030, bgColor);
    
    // 繝懊・繝繝ｼ邱・
    ColorRGBA borderColor = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(90),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawLine(1600, 50, 1600, 1080, 2.0f, borderColor);
    
    // 繧ｿ繧､繝医Ν
    ColorRGBA textGold = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    ColorRGBA textWhite = ToCoreColor(WHITE);
    ColorRGBA textGray = {
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(255)
    };
    
    DrawText(1620, 60, "Unit Selection", 16, textGold);
    
    // 繝ｦ繝九ャ繝医Μ繧ｹ繝・
    float listY = 100;
    float itemHeight = 120;
    float listX = 1610;
    float listWidth = 300;
    
    for (size_t i = 0; i < units.size(); ++i) {
        const auto& unit = units[i];
        float itemY = listY + i * itemHeight;
        
        // 繧｢繧､繝・Β閭梧勹
        ColorRGBA itemBgSelected = {
            static_cast<unsigned char>(60),
            static_cast<unsigned char>(80),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        ColorRGBA itemBgNormal = {
            static_cast<unsigned char>(45),
            static_cast<unsigned char>(50),
            static_cast<unsigned char>(55),
            static_cast<unsigned char>(255)
        };
        ColorRGBA itemBg = unit.isSelected ? itemBgSelected : itemBgNormal;
        sysAPI_->Render().DrawRectangle(listX, itemY, listWidth, itemHeight - 10,
                                        itemBg);
        
        // 繝ｦ繝九ャ繝亥錐
        DrawText(listX + 10, itemY + 10, unit.displayName, 14, textWhite);
        
        // 驟咲ｽｮ謨ｰ
        std::ostringstream countText;
        countText << "Placed: " << unit.currentCount << "/" << unit.maxCount;
        DrawText(listX + 10, itemY + 35, countText.str(), 12, textGray);
        
        // 繧ｳ繧ｹ繝・
        std::ostringstream costText;
        costText << "Cost: " << unit.costGold << " G";
        DrawText(listX + 10, itemY + 55, costText.str(), 12, textGold);
        
        // 驕ｸ謚槭・繧ｿ繝ｳ
        if (unit.isSelected) {
            ColorRGBA selectedBtnColor = {
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(100),
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(255)
            };
            DrawButton(listX + 10, itemY + 75, 100, 25, "Selected", true, selectedBtnColor);
        } else {
            ColorRGBA selectBtnColor = {
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(80),
                static_cast<unsigned char>(100),
                static_cast<unsigned char>(255)
            };
            DrawButton(listX + 10, itemY + 75, 100, 25, "Select", false, selectBtnColor);
        }
    }
    
    // 謫堺ｽ懊ぎ繧､繝会ｼ井ｸ矩Κ・・
    float guideY = 950;
    DrawText(1620, guideY, "Operation Guide", 14, textGold);
    DrawText(1620, guideY + 25, "- Left Click: Place/Select", 12, textGray);
    DrawText(1620, guideY + 45, "- Right Click: Remove/Cancel", 12, textGray);
}

void HUDRenderer::RenderFieldUI(int hoverGx, int hoverGy, int selectGx, int selectGy,
                                bool isPlaceable, int cellSize,
                                float fieldOriginX, float fieldOriginY) {
    // 繝帙ヰ繝ｼ陦ｨ遉ｺ
    if (hoverGx >= 0 && hoverGy >= 0) {
        float hoverX = fieldOriginX + hoverGx * cellSize;
        float hoverY = fieldOriginY + hoverGy * cellSize;
        
        ColorRGBA hoverColorPlaceable = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(80)
        };
        ColorRGBA hoverColorBlocked = {
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(80)
        };
        ColorRGBA hoverColor = isPlaceable ? hoverColorPlaceable : hoverColorBlocked;
        
        sysAPI_->Render().DrawRectangle(hoverX, hoverY, cellSize, cellSize, hoverColor);
        
        // 譫邱・
        ColorRGBA borderColorPlaceable = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        ColorRGBA borderColorBlocked = {
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        ColorRGBA borderColor = isPlaceable ? borderColorPlaceable : borderColorBlocked;
        sysAPI_->Render().DrawRectangleLines(hoverX, hoverY, cellSize, cellSize,
                                             2.0f, borderColor);
    }
    
    // 驕ｸ謚櫁｡ｨ遉ｺ
    if (selectGx >= 0 && selectGy >= 0) {
        float selectX = fieldOriginX + selectGx * cellSize;
        float selectY = fieldOriginY + selectGy * cellSize;
        
    ColorRGBA selectColor = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(150),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(255)
        };
        sysAPI_->Render().DrawRectangleLines(selectX, selectY, cellSize, cellSize,
                                             3.0f, selectColor);
    }
}

std::string HUDRenderer::CheckTopHUDButtonClick(float mouseX, float mouseY) {
    for (const auto& btn : topHudButtons_) {
        if (IsMouseInRect(mouseX, mouseY, btn.x, btn.y, btn.width, btn.height)) {
            return btn.id;
        }
    }
    return "";
}

void HUDRenderer::DrawBar(float x, float y, float width, float height,
                         float current, float max, ColorRGBA fillColor, ColorRGBA bgColor) {
    // 閭梧勹
    sysAPI_->Render().DrawRectangle(x, y, width, height, bgColor);
    
    // 繝輔ぅ繝ｫ
    float fillWidth = (current / max) * width;
    if (fillWidth > 0) {
        sysAPI_->Render().DrawRectangle(x, y, fillWidth, height, fillColor);
    }
    
    // 譫邱・
    ColorRGBA barBorderColor = {
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(255)
    };
    sysAPI_->Render().DrawRectangleLines(x, y, width, height, 1.0f, barBorderColor);
}

void HUDRenderer::DrawText(float x, float y, const std::string& text,
                          int fontSize, ColorRGBA color) {
    sysAPI_->Render().DrawTextDefault(text, x, y, static_cast<float>(fontSize), color);
}

void HUDRenderer::DrawTextCentered(float centerX, float y, const std::string& text,
                                  int fontSize, ColorRGBA color) {
    Vec2 textSize =
        sysAPI_->Render().MeasureTextDefaultCore(text, static_cast<float>(fontSize));
    float x = centerX - textSize.x / 2.0f;
    DrawText(x, y, text, fontSize, color);
}

void HUDRenderer::DrawButton(float x, float y, float width, float height,
                            const std::string& label, bool isActive, ColorRGBA baseColor) {
    // 繝懊ち繝ｳ閭梧勹
    ColorRGBA btnColor = isActive ?
        ColorRGBA{
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.r) + 30)),
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.g) + 30)),
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.b) + 30)),
            static_cast<unsigned char>(255)
        } : baseColor;
    sysAPI_->Render().DrawRectangle(x, y, width, height, btnColor);
    
    // 譫邱・
    ColorRGBA borderColorActive = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    ColorRGBA borderColorInactive = {
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(255)
    };
    ColorRGBA borderColor = isActive ? borderColorActive : borderColorInactive;
    sysAPI_->Render().DrawRectangleLines(x, y, width, height, 2.0f, borderColor);
    
    // 繝ｩ繝吶Ν・井ｸｭ螟ｮ謠・∴・・
    ColorRGBA textColor = ToCoreColor(WHITE);
    DrawTextCentered(x + width / 2, y + height / 2 - 7, label, 14, textColor);
}

bool HUDRenderer::IsMouseInRect(float mouseX, float mouseY,
                               float rectX, float rectY, float rectW, float rectH) {
    return mouseX >= rectX && mouseX <= rectX + rectW &&
           mouseY >= rectY && mouseY <= rectY + rectH;
}

} // namespace ui
} // namespace core
} // namespace game
