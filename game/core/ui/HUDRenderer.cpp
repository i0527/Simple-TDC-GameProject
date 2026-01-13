#include "HUDRenderer.hpp"
#include "../../utils/Log.h"
#include <raylib.h>
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
    // 背景描画
    Color bgColor = {
        static_cast<unsigned char>(30),
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawRectangle(0, 0, 1920, 50, bgColor);
    
    // ボーダー線
    Color borderColor = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(110),
        static_cast<unsigned char>(120),
        static_cast<unsigned char>(50)
    };
    sysAPI_->DrawLine(0, 50, 1920, 50, 2.0f, borderColor);
    
    // テキスト色
    Color textWhite = WHITE;
    Color textGray = {
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(255)
    };
    Color textGold = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    Color textRed = {
        static_cast<unsigned char>(255),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(255)
    };
    
    // Wave表示（左）
    std::ostringstream waveText;
    waveText << "Wave: " << wave << "/" << totalWaves;
    DrawText(10, 15, waveText.str(), 18, textWhite);
    
    // HP表示（中央左）
    std::ostringstream hpText;
    hpText << "HP: " << hp << "/" << maxHp;
    DrawText(160, 18, "HP:", 14, textGray);
    
    // HPバー
    Color hpColorGreen = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    Color hpColor = (hp < maxHp * 0.3f) ? textRed : hpColorGreen;
    Color hpBgColor = {
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(255)
    };
    DrawBar(200, 18, 120, 14, static_cast<float>(hp), static_cast<float>(maxHp), 
            hpColor, hpBgColor);
    
    // HP数値
    DrawText(330, 18, hpText.str(), 12, textGray);
    
    // Gold表示（中央）
    std::ostringstream goldText;
    goldText << "Gold: " << gold << " G";
    DrawText(450, 15, goldText.str(), 18, textGold);
    
    // ゲーム状態テキスト（中央右）
    DrawText(650, 18, gameStateText, 14, textGray);
    
    // スピードボタン（右中央）
    topHudButtons_.clear();
    
    float speedBtnY = 8;
    float speedBtnWidth = 80;
    float speedBtnHeight = 34;
    float speedBtnSpacing = 10;
    float speedBtnStartX = 900;
    
    // ×0.5
    bool isSpeed05 = (std::abs(gameSpeed - 0.5f) < 0.01f);
    Color speedBtn1Color = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX, speedBtnY, speedBtnWidth, speedBtnHeight,
               "x0.5", isSpeed05, speedBtn1Color);
    topHudButtons_.push_back({speedBtnStartX, speedBtnY, speedBtnWidth, speedBtnHeight, "speed_0.5"});
    
    // ×1.0
    bool isSpeed10 = (std::abs(gameSpeed - 1.0f) < 0.01f);
    Color speedBtn2Color = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX + speedBtnWidth + speedBtnSpacing, speedBtnY, 
               speedBtnWidth, speedBtnHeight, "x1.0", isSpeed10, speedBtn2Color);
    topHudButtons_.push_back({speedBtnStartX + speedBtnWidth + speedBtnSpacing, 
                             speedBtnY, speedBtnWidth, speedBtnHeight, "speed_1.0"});
    
    // ×2.0
    bool isSpeed20 = (std::abs(gameSpeed - 2.0f) < 0.01f);
    Color speedBtn3Color = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    DrawButton(speedBtnStartX + (speedBtnWidth + speedBtnSpacing) * 2, speedBtnY,
               speedBtnWidth, speedBtnHeight, "x2.0", isSpeed20, speedBtn3Color);
    topHudButtons_.push_back({speedBtnStartX + (speedBtnWidth + speedBtnSpacing) * 2,
                             speedBtnY, speedBtnWidth, speedBtnHeight, "speed_2.0"});
    
    // Pauseボタン（右）
    float pauseBtnX = 1660;
    Color pauseBtnColor = {
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    DrawButton(pauseBtnX, speedBtnY, 120, speedBtnHeight,
               isPaused ? "Resume" : "Pause", isPaused, pauseBtnColor);
    topHudButtons_.push_back({pauseBtnX, speedBtnY, 120, speedBtnHeight, "pause"});
    
    // Exitボタン（右端）
    float exitBtnX = 1790;
    Color exitBtnColor = {
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
    // 背景描画
    Color bgColor = {
        static_cast<unsigned char>(25),
        static_cast<unsigned char>(30),
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawRectangle(0, 50, 640, 1030, bgColor);
    
    // ボーダー線
    Color borderColor = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(90),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawLine(640, 50, 640, 1080, 2.0f, borderColor);
    
    // タイトル
    Color textWhite = WHITE;
    DrawText(20, 60, "Field Overview", 16, textWhite);
    
    // ミニマップ枠（簡易表示）
    float miniMapX = 20;
    float miniMapY = 100;
    float miniMapWidth = 600;
    float miniMapHeight = 900;
    
    Color miniMapBorderColor = {
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(110),
        static_cast<unsigned char>(120),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawRectangleLines(miniMapX, miniMapY, miniMapWidth, miniMapHeight,
                               2.0f, miniMapBorderColor);
    
    // ミニマップ説明テキスト
    Color textGray = {
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
    // 背景描画
    Color bgColor = {
        static_cast<unsigned char>(35),
        static_cast<unsigned char>(40),
        static_cast<unsigned char>(45),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawRectangle(1600, 50, 320, 1030, bgColor);
    
    // ボーダー線
    Color borderColor = {
        static_cast<unsigned char>(80),
        static_cast<unsigned char>(90),
        static_cast<unsigned char>(100),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawLine(1600, 50, 1600, 1080, 2.0f, borderColor);
    
    // タイトル
    Color textGold = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    Color textWhite = WHITE;
    Color textGray = {
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(180),
        static_cast<unsigned char>(255)
    };
    
    DrawText(1620, 60, "Unit Selection", 16, textGold);
    
    // ユニットリスト
    float listY = 100;
    float itemHeight = 120;
    float listX = 1610;
    float listWidth = 300;
    
    for (size_t i = 0; i < units.size(); ++i) {
        const auto& unit = units[i];
        float itemY = listY + i * itemHeight;
        
        // アイテム背景
        Color itemBgSelected = {
            static_cast<unsigned char>(60),
            static_cast<unsigned char>(80),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        Color itemBgNormal = {
            static_cast<unsigned char>(45),
            static_cast<unsigned char>(50),
            static_cast<unsigned char>(55),
            static_cast<unsigned char>(255)
        };
        Color itemBg = unit.isSelected ? itemBgSelected : itemBgNormal;
        sysAPI_->DrawRectangle(listX, itemY, listWidth, itemHeight - 10, itemBg);
        
        // ユニット名
        DrawText(listX + 10, itemY + 10, unit.displayName, 14, textWhite);
        
        // 配置数
        std::ostringstream countText;
        countText << "Placed: " << unit.currentCount << "/" << unit.maxCount;
        DrawText(listX + 10, itemY + 35, countText.str(), 12, textGray);
        
        // コスト
        std::ostringstream costText;
        costText << "Cost: " << unit.costGold << " G";
        DrawText(listX + 10, itemY + 55, costText.str(), 12, textGold);
        
        // 選択ボタン
        if (unit.isSelected) {
            Color selectedBtnColor = {
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(100),
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(255)
            };
            DrawButton(listX + 10, itemY + 75, 100, 25, "Selected", true, selectedBtnColor);
        } else {
            Color selectBtnColor = {
                static_cast<unsigned char>(60),
                static_cast<unsigned char>(80),
                static_cast<unsigned char>(100),
                static_cast<unsigned char>(255)
            };
            DrawButton(listX + 10, itemY + 75, 100, 25, "Select", false, selectBtnColor);
        }
    }
    
    // 操作ガイド（下部）
    float guideY = 950;
    DrawText(1620, guideY, "Operation Guide", 14, textGold);
    DrawText(1620, guideY + 25, "- Left Click: Place/Select", 12, textGray);
    DrawText(1620, guideY + 45, "- Right Click: Remove/Cancel", 12, textGray);
}

void HUDRenderer::RenderFieldUI(int hoverGx, int hoverGy, int selectGx, int selectGy,
                                bool isPlaceable, int cellSize,
                                float fieldOriginX, float fieldOriginY) {
    // ホバー表示
    if (hoverGx >= 0 && hoverGy >= 0) {
        float hoverX = fieldOriginX + hoverGx * cellSize;
        float hoverY = fieldOriginY + hoverGy * cellSize;
        
        Color hoverColorPlaceable = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(80)
        };
        Color hoverColorBlocked = {
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(80)
        };
        Color hoverColor = isPlaceable ? hoverColorPlaceable : hoverColorBlocked;
        
        sysAPI_->DrawRectangle(hoverX, hoverY, cellSize, cellSize, hoverColor);
        
        // 枠線
        Color borderColorPlaceable = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        Color borderColorBlocked = {
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(255)
        };
        Color borderColor = isPlaceable ? borderColorPlaceable : borderColorBlocked;
        sysAPI_->DrawRectangleLines(hoverX, hoverY, cellSize, cellSize,
                                   2.0f, borderColor);
    }
    
    // 選択表示
    if (selectGx >= 0 && selectGy >= 0) {
        float selectX = fieldOriginX + selectGx * cellSize;
        float selectY = fieldOriginY + selectGy * cellSize;
        
        Color selectColor = {
            static_cast<unsigned char>(100),
            static_cast<unsigned char>(150),
            static_cast<unsigned char>(200),
            static_cast<unsigned char>(255)
        };
        sysAPI_->DrawRectangleLines(selectX, selectY, cellSize, cellSize,
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
                         float current, float max, Color fillColor, Color bgColor) {
    // 背景
    sysAPI_->DrawRectangle(x, y, width, height, bgColor);
    
    // フィル
    float fillWidth = (current / max) * width;
    if (fillWidth > 0) {
        sysAPI_->DrawRectangle(x, y, fillWidth, height, fillColor);
    }
    
    // 枠線
    Color barBorderColor = {
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(200),
        static_cast<unsigned char>(255)
    };
    sysAPI_->DrawRectangleLines(x, y, width, height, 1.0f, barBorderColor);
}

void HUDRenderer::DrawText(float x, float y, const std::string& text,
                          int fontSize, Color color) {
    sysAPI_->DrawTextDefault(text, x, y, static_cast<float>(fontSize), color);
}

void HUDRenderer::DrawTextCentered(float centerX, float y, const std::string& text,
                                  int fontSize, Color color) {
    Vector2 textSize = sysAPI_->MeasureTextDefault(text, static_cast<float>(fontSize));
    float x = centerX - textSize.x / 2.0f;
    DrawText(x, y, text, fontSize, color);
}

void HUDRenderer::DrawButton(float x, float y, float width, float height,
                            const std::string& label, bool isActive, Color baseColor) {
    // ボタン背景
    Color btnColor = isActive ?
        Color{
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.r) + 30)),
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.g) + 30)),
            static_cast<unsigned char>(std::min(255, static_cast<int>(baseColor.b) + 30)),
            static_cast<unsigned char>(255)
        } : baseColor;
    sysAPI_->DrawRectangle(x, y, width, height, btnColor);
    
    // 枠線
    Color borderColorActive = {
        static_cast<unsigned char>(240),
        static_cast<unsigned char>(170),
        static_cast<unsigned char>(60),
        static_cast<unsigned char>(255)
    };
    Color borderColorInactive = {
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(150),
        static_cast<unsigned char>(255)
    };
    Color borderColor = isActive ? borderColorActive : borderColorInactive;
    sysAPI_->DrawRectangleLines(x, y, width, height, 2.0f, borderColor);
    
    // ラベル（中央揃え）
    Color textColor = WHITE;
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
