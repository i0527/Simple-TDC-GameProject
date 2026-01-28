#include "LicenseOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include <sstream>
#include <vector>

namespace {
    std::vector<std::string> SplitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::stringstream stream(text);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        if (lines.empty()) {
            lines.push_back("");
        }
        return lines;
    }
}

namespace game {
namespace core {

LicenseOverlay::LicenseOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , scrollY_(0.0f)
    , totalContentHeight_(0.0f)
    , visibleContentHeight_(0.0f)
    , isDraggingScrollbar_(false)
    , dragStartY_(0.0f)
    , dragStartScrollY_(0.0f)
{
}

bool LicenseOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("LicenseOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("LicenseOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    scrollY_ = 0.0f;

    // コンチE��チE�E高さを計箁E
    totalContentHeight_ = CalculateTotalContentHeight();

    isInitialized_ = true;
    LOG_INFO("LicenseOverlay initialized");
    return true;
}

void LicenseOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }
    InputSystemAPI* inputAPI = ctx.inputAPI;

    // ESCキーで閉じめE
    if (inputAPI && inputAPI->IsEscapePressed()) {  // ESCキー
        requestClose_ = true;
    }
    
    // スクロール処琁E
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    const float contentAreaY = windowY + 100.0f;  // タイトル丁E
    const float contentAreaHeight = windowHeight - 180.0f;  // タイトルとボタンの閁E
    visibleContentHeight_ = contentAreaHeight;
    
    // マウスホイールでスクロール
    float wheelMove = inputAPI ? inputAPI->GetMouseWheelMove() : 0.0f;
    if (wheelMove != 0.0f) {
        auto mouse = inputAPI ? inputAPI->GetMousePositionInternal()
                               : Vec2{0.0f, 0.0f};
        // ウィンドウ冁E��マウスがある場合�Eみスクロール
        if (mouse.x >= windowX && mouse.x <= windowX + windowWidth &&
            mouse.y >= contentAreaY && mouse.y <= contentAreaY + contentAreaHeight) {
            scrollY_ -= wheelMove * 30.0f;  // スクロール速度調整
        }
    }
    
    // キーボ�Eドでスクロール
    if (inputAPI && inputAPI->IsUpPressed()) {
        scrollY_ -= 30.0f;
    }
    if (inputAPI && inputAPI->IsDownPressed()) {
        scrollY_ += 30.0f;
    }
    if (inputAPI && inputAPI->IsPageUpPressed()) {
        scrollY_ -= contentAreaHeight * 0.8f;
    }
    if (inputAPI && inputAPI->IsPageDownPressed()) {
        scrollY_ += contentAreaHeight * 0.8f;
    }
    
    // スクロールバ�Eのマウス操作�E琁E
    HandleScrollbarInteraction(inputAPI, windowX, windowY, windowWidth, windowHeight);
    
    // スクロール位置をクランチE
    float maxScroll = std::max(0.0f, totalContentHeight_ - visibleContentHeight_);
    scrollY_ = std::max(0.0f, std::min(scrollY_, maxScroll));
    
    // マウスクリチE��で閉じる�EタンをチェチE��
    if (inputAPI && inputAPI->IsLeftClickPressed()) {
        auto mouse = inputAPI->GetMousePositionInternal();
        
        const float buttonWidth = 150.0f;
        const float buttonHeight = 40.0f;
        float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
        float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
        
        // ボタン冁E��マウスがあるか判宁E
        if (mouse.x >= buttonX && mouse.x <= buttonX + buttonWidth &&
            mouse.y >= buttonY && mouse.y <= buttonY + buttonHeight) {
            requestClose_ = true;
            inputAPI->ConsumeLeftClick();
        }
    }
}

void LicenseOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // ウィンドウの位置とサイズ
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    
    Rectangle windowRect = {windowX, windowY, windowWidth, windowHeight};
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBackground,
                                         windowRect, 8, 8, 8, 8, WHITE);
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBorder, windowRect,
                                         8, 8, 8, 8, WHITE);
    
    // タイトル
    const char* titleText = "ライセンス";
    float titleFontSize = 36.0f;
    Vector2 titleSize =
        systemAPI_->Render().MeasureTextDefault(titleText, titleFontSize, 1.0f);
    float titleX = windowX + (windowWidth - titleSize.x) / 2.0f;
    float titleY = windowY + 20.0f;
    systemAPI_->Render().DrawTextDefault(titleText, titleX, titleY,
                                         titleFontSize,
                                         ui::OverlayColors::TEXT_DARK);
    
    // コンチE��チE��域
    const float contentAreaX = windowX + 40.0f;
    const float contentAreaY = windowY + 100.0f;
    const float contentAreaWidth = windowWidth - 100.0f;  // スクロールバ�E用の余白
    const float contentAreaHeight = windowHeight - 180.0f;  // タイトルとボタンの閁E
    
    // スクロール可能なコンチE��チE��描画�E�EcissorModeでクリチE��ング�E�E
    BeginScissorMode(
        static_cast<int>(contentAreaX),
        static_cast<int>(contentAreaY),
        static_cast<int>(contentAreaWidth),
        static_cast<int>(contentAreaHeight)
    );
    
    RenderLicenseText(contentAreaX, contentAreaY - scrollY_, contentAreaWidth, totalContentHeight_);
    
    EndScissorMode();
    
    // スクロールバ�Eを描画
    RenderScrollbar(windowX, windowY, windowWidth, windowHeight);
    
    // 閉じる�Eタン
    const float buttonWidth = 150.0f;
    const float buttonHeight = 40.0f;
    float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
    float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
    
    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal()
                              : Vec2{0.0f, 0.0f};
    bool isButtonHovered = (mouse.x >= buttonX && mouse.x <= buttonX + buttonWidth &&
                            mouse.y >= buttonY && mouse.y <= buttonY + buttonHeight);
    const char* buttonTexture = isButtonHovered
        ? ui::UiAssetKeys::ButtonPrimaryHover
        : ui::UiAssetKeys::ButtonPrimaryNormal;
    Rectangle buttonRect = {buttonX, buttonY, buttonWidth, buttonHeight};
    systemAPI_->Render().DrawUiNineSlice(buttonTexture, buttonRect, 8, 8, 8, 8,
                                         WHITE);
    
    // ボタンテキスト
    const char* buttonText = "閉じる";
    float buttonFontSize = 24.0f;
    Vector2 buttonTextSize = systemAPI_->Render().MeasureTextDefault(
        buttonText, buttonFontSize, 1.0f);
    float buttonTextX = buttonX + (buttonWidth - buttonTextSize.x) / 2.0f;
    float buttonTextY = buttonY + (buttonHeight - buttonFontSize) / 2.0f;
    systemAPI_->Render().DrawTextDefault(
        buttonText, buttonTextX, buttonTextY, buttonFontSize,
        systemAPI_->Render().GetReadableTextColor(buttonTexture));
}

void LicenseOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("LicenseOverlay shutdown");
}

bool LicenseOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool LicenseOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

float LicenseOverlay::CalculateTotalContentHeight() {
    float textFontSize = 20.0f;
    float lineHeight = textFontSize + 4.0f;
    float titleFontSize = 24.0f;
    float sectionSpacing = 40.0f;
    float currentHeight = 0.0f;
    
    // プロジェクトライセンス
    currentHeight += titleFontSize + 10.0f;  // タイトル
    currentHeight += 22 * lineHeight;  // ライセンスチE��スト！E1衁E+ 空行！E
    currentHeight += sectionSpacing;
    
    // raylib
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;
    
    // ImGui
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;
    
    // EnTT
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;
    
    // nlohmann/json
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;
    
    // spdlog
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;
    
    // rlImGui
    currentHeight += titleFontSize + 10.0f;
    currentHeight += 17 * lineHeight;
    currentHeight += sectionSpacing;

    // Kenney assets
    std::vector<AssetLicenseEntry> assetLicenses;
    if (systemAPI_) {
        assetLicenses = systemAPI_->Resource().GetAssetLicenses();
    }
    for (size_t i = 0; i < assetLicenses.size(); ++i) {
        currentHeight += titleFontSize + 10.0f;
        const auto lines = SplitLines(assetLicenses[i].licenseText);
        currentHeight += static_cast<float>(lines.size()) * lineHeight;
        if (i + 1 < assetLicenses.size()) {
            currentHeight += sectionSpacing;
        }
    }

    return currentHeight;
}

void LicenseOverlay::RenderLicenseText(float contentX, float contentY, float contentWidth, float contentHeight) {
    float textFontSize = 20.0f;
    float lineHeight = textFontSize + 4.0f;
    float sectionSpacing = 40.0f;
    float currentY = contentY;
    const Color titleColor = ui::OverlayColors::TEXT_DARK;
    const Color bodyColor = ui::OverlayColors::TEXT_DARK;
    
    // プロジェクトライセンス�E�EIT License�E�E
    const char* projectTitle = "=== tower of defense (MIT License) ===";
    float titleFontSize = 24.0f;
    Vector2 titleSize =
        systemAPI_->Render().MeasureTextDefault(projectTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(projectTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += titleSize.y + 10.0f;
    
    const char* projectLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2025 Simple-TDC-GameProject Contributors",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE",
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER",
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,",
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE",
        "SOFTWARE."
    };
    
    for (size_t i = 0; i < sizeof(projectLicense) / sizeof(projectLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            projectLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // raylib (zlib/libpng license)
    const char* raylibTitle = "=== raylib (zlib/libpng License) ===";
    Vector2 raylibTitleSize =
        systemAPI_->Render().MeasureTextDefault(raylibTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(raylibTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += raylibTitleSize.y + 10.0f;
    
    const char* raylibLicense[] = {
        "zlib/libpng License",
        "",
        "Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)",
        "",
        "This software is provided \"as-is\", without any express or implied warranty.",
        "In no event will the authors be held liable for any damages arising from",
        "the use of this software.",
        "",
        "Permission is granted to anyone to use this software for any purpose,",
        "including commercial applications, and to alter it and redistribute it",
        "freely, subject to the following restrictions:",
        "",
        "1. The origin of this software must not be misrepresented; you must not",
        "   claim that you wrote the original software.",
        "2. Altered source versions must be plainly marked as such, and must not",
        "   be misrepresented as being the original software.",
        "3. This notice may not be removed or altered from any source distribution."
    };
    
    for (size_t i = 0; i < sizeof(raylibLicense) / sizeof(raylibLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            raylibLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // ImGui (MIT License)
    const char* imguiTitle = "=== ImGui (MIT License) ===";
    Vector2 imguiTitleSize =
        systemAPI_->Render().MeasureTextDefault(imguiTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(imguiTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += imguiTitleSize.y + 10.0f;
    
    const char* imguiLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2014-2024 Omar Cornut",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
    };
    
    for (size_t i = 0; i < sizeof(imguiLicense) / sizeof(imguiLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            imguiLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // EnTT (MIT License)
    const char* enttTitle = "=== EnTT (MIT License) ===";
    Vector2 enttTitleSize =
        systemAPI_->Render().MeasureTextDefault(enttTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(enttTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += enttTitleSize.y + 10.0f;
    
    const char* enttLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2017-2024 Michele Caini",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
    };
    
    for (size_t i = 0; i < sizeof(enttLicense) / sizeof(enttLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            enttLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // nlohmann/json (MIT License)
    const char* jsonTitle = "=== nlohmann/json (MIT License) ===";
    Vector2 jsonTitleSize =
        systemAPI_->Render().MeasureTextDefault(jsonTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(jsonTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += jsonTitleSize.y + 10.0f;
    
    const char* jsonLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2013-2024 Niels Lohmann",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
    };
    
    for (size_t i = 0; i < sizeof(jsonLicense) / sizeof(jsonLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            jsonLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // spdlog (MIT License)
    const char* spdlogTitle = "=== spdlog (MIT License) ===";
    Vector2 spdlogTitleSize =
        systemAPI_->Render().MeasureTextDefault(spdlogTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(spdlogTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += spdlogTitleSize.y + 10.0f;
    
    const char* spdlogLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2016 Gabi Melman",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
    };
    
    for (size_t i = 0; i < sizeof(spdlogLicense) / sizeof(spdlogLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            spdlogLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // rlImGui (MIT License)
    const char* rlimguiTitle = "=== rlImGui (MIT License) ===";
    Vector2 rlimguiTitleSize =
        systemAPI_->Render().MeasureTextDefault(rlimguiTitle, titleFontSize, 1.0f);
    systemAPI_->Render().DrawTextDefault(rlimguiTitle, contentX, currentY,
                                         titleFontSize,
                                         titleColor);
    currentY += rlimguiTitleSize.y + 10.0f;
    
    const char* rlimguiLicense[] = {
        "MIT License",
        "",
        "Copyright (c) 2020-2024 raylib-extras contributors",
        "",
        "Permission is hereby granted, free of charge, to any person obtaining a copy",
        "of this software and associated documentation files (the \"Software\"), to deal",
        "in the Software without restriction, including without limitation the rights",
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
        "copies of the Software, and to permit persons to whom the Software is",
        "furnished to do so, subject to the following conditions:",
        "",
        "The above copyright notice and this permission notice shall be included in all",
        "copies or substantial portions of the Software.",
        "",
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,",
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
    };
    
    for (size_t i = 0; i < sizeof(rlimguiLicense) / sizeof(rlimguiLicense[0]); ++i) {
        systemAPI_->Render().DrawTextDefault(
            rlimguiLicense[i], contentX, currentY, textFontSize,
            bodyColor);
        currentY += lineHeight;
    }

    currentY += sectionSpacing;

    // Kenney assets
    std::vector<AssetLicenseEntry> assetLicenses;
    if (systemAPI_) {
        assetLicenses = systemAPI_->Resource().GetAssetLicenses();
    }
    for (size_t i = 0; i < assetLicenses.size(); ++i) {
        const std::string title = "=== Kenney: " + assetLicenses[i].packName + " ===";
        Vector2 kenneyTitleSize =
            systemAPI_->Render().MeasureTextDefault(title, titleFontSize, 1.0f);
        systemAPI_->Render().DrawTextDefault(title, contentX, currentY,
                                             titleFontSize,
                                             titleColor);
        currentY += kenneyTitleSize.y + 10.0f;

        const auto lines = SplitLines(assetLicenses[i].licenseText);
        for (const auto& line : lines) {
            systemAPI_->Render().DrawTextDefault(
                line, contentX, currentY, textFontSize,
                bodyColor);
            currentY += lineHeight;
        }

        if (i + 1 < assetLicenses.size()) {
            currentY += sectionSpacing;
        }
    }
}

void LicenseOverlay::HandleScrollbarInteraction(InputSystemAPI* inputAPI, float windowX, float windowY,
                                                float windowWidth, float windowHeight) {
    if (!inputAPI) {
        return;
    }
    if (totalContentHeight_ <= visibleContentHeight_) {
        // スクロール不要な場合�E何もしなぁE
        isDraggingScrollbar_ = false;
        return;
    }
    
    const float scrollbarWidth = 20.0f;
    const float scrollbarX = windowX + windowWidth - scrollbarWidth - 20.0f;
    const float scrollbarY = windowY + 100.0f;  // タイトル丁E
    const float scrollbarHeight = windowHeight - 180.0f;  // タイトルとボタンの閁E
    
    float scrollRatio = visibleContentHeight_ / totalContentHeight_;
    float thumbHeight = scrollbarHeight * scrollRatio;
    float thumbY = scrollbarY + (scrollY_ / totalContentHeight_) * scrollbarHeight;
    
    auto mouse = inputAPI->GetMousePositionInternal();
    bool mouseOverScrollbar = (mouse.x >= scrollbarX && mouse.x <= scrollbarX + scrollbarWidth &&
                               mouse.y >= scrollbarY && mouse.y <= scrollbarY + scrollbarHeight);
    bool mouseOverThumb = (mouse.x >= scrollbarX && mouse.x <= scrollbarX + scrollbarWidth &&
                           mouse.y >= thumbY && mouse.y <= thumbY + thumbHeight);
    
    // マウスボタンが押されたとぁE
    if (inputAPI->IsLeftClickPressed()) {
        if (mouseOverThumb) {
            // つまみをクリチE��した場合、ドラチE��開姁E
            isDraggingScrollbar_ = true;
            dragStartY_ = mouse.y;
            dragStartScrollY_ = scrollY_;
            inputAPI->ConsumeLeftClick();
        } else if (mouseOverScrollbar) {
            // スクロールバ�EのトラチE���E�つまみ以外）をクリチE��した場合、その位置にスクロール
            float maxScroll = totalContentHeight_ - visibleContentHeight_;
            float scrollRange = scrollbarHeight - thumbHeight;  // つまみが動ける篁E��
            
            if (scrollRange > 0.0f) {
                // クリチE��位置をスクロール篁E��にマッピング
                float clickY = mouse.y - scrollbarY;
                float clickRatio = (clickY - thumbHeight / 2.0f) / scrollRange;
                clickRatio = std::max(0.0f, std::min(1.0f, clickRatio));  // 0-1にクランチE
                scrollY_ = clickRatio * maxScroll;
            }
            inputAPI->ConsumeLeftClick();
        }
    }
    
    // ドラチE��中
    if (isDraggingScrollbar_ && inputAPI->IsLeftClickDown()) {
        float deltaY = mouse.y - dragStartY_;
        float maxScroll = totalContentHeight_ - visibleContentHeight_;
        float scrollRange = scrollbarHeight - thumbHeight;  // つまみが動ける篁E��
        
        if (scrollRange > 0.0f) {
            float scrollDelta = (deltaY / scrollRange) * maxScroll;
            scrollY_ = dragStartScrollY_ + scrollDelta;
            scrollY_ = std::max(0.0f, std::min(scrollY_, maxScroll));
        }
    }
    
    // マウスボタンが離されたとぁE
    if (inputAPI->IsLeftClickReleased()) {
        isDraggingScrollbar_ = false;
    }
}

void LicenseOverlay::RenderScrollbar(float windowX, float windowY, float windowWidth, float windowHeight) {
    const float scrollbarWidth = 20.0f;
    const float scrollbarX = windowX + windowWidth - scrollbarWidth - 20.0f;
    const float scrollbarY = windowY + 100.0f;  // タイトル丁E
    const float scrollbarHeight = windowHeight - 180.0f;  // タイトルとボタンの閁E
    
    Rectangle trackRect = {scrollbarX, scrollbarY, scrollbarWidth, scrollbarHeight};
    systemAPI_->Render().DrawUiTexture(ui::UiAssetKeys::ScrollTrackVertical,
                                       trackRect, WHITE);
    
    // スクロール可能な場合�Eみスクロールバ�Eを描画
    if (totalContentHeight_ > visibleContentHeight_) {
        float scrollRatio = visibleContentHeight_ / totalContentHeight_;
        float thumbHeight = scrollbarHeight * scrollRatio;
        float thumbY = scrollbarY + (scrollY_ / totalContentHeight_) * scrollbarHeight;
        Rectangle thumbRect = {scrollbarX, thumbY, scrollbarWidth, thumbHeight};
        systemAPI_->Render().DrawUiTexture(
            ui::UiAssetKeys::ScrollThumb, thumbRect,
            isDraggingScrollbar_ ? Color{220, 220, 255, 255} : WHITE);
    }
}

} // namespace core
} // namespace game
