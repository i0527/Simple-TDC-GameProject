#include "LicenseOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <raylib.h>

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

bool LicenseOverlay::Initialize(BaseSystemAPI* systemAPI) {
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

    // コンテンツの高さを計算
    totalContentHeight_ = CalculateTotalContentHeight();

    isInitialized_ = true;
    LOG_INFO("LicenseOverlay initialized");
    return true;
}

void LicenseOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) {  // ESCキー
        requestClose_ = true;
    }
    
    // スクロール処理
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    const float contentAreaY = windowY + 100.0f;  // タイトル下
    const float contentAreaHeight = windowHeight - 180.0f;  // タイトルとボタンの間
    visibleContentHeight_ = contentAreaHeight;
    
    // マウスホイールでスクロール
    float wheelMove = systemAPI_->GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        Vector2 mouse = systemAPI_->GetMousePosition();
        // ウィンドウ内でマウスがある場合のみスクロール
        if (mouse.x >= windowX && mouse.x <= windowX + windowWidth &&
            mouse.y >= contentAreaY && mouse.y <= contentAreaY + contentAreaHeight) {
            scrollY_ -= wheelMove * 30.0f;  // スクロール速度調整
        }
    }
    
    // キーボードでスクロール
    if (systemAPI_->IsKeyPressed(KEY_UP)) {
        scrollY_ -= 30.0f;
    }
    if (systemAPI_->IsKeyPressed(KEY_DOWN)) {
        scrollY_ += 30.0f;
    }
    if (systemAPI_->IsKeyPressed(KEY_PAGE_UP)) {
        scrollY_ -= contentAreaHeight * 0.8f;
    }
    if (systemAPI_->IsKeyPressed(KEY_PAGE_DOWN)) {
        scrollY_ += contentAreaHeight * 0.8f;
    }
    
    // スクロールバーのマウス操作処理
    HandleScrollbarInteraction(windowX, windowY, windowWidth, windowHeight);
    
    // スクロール位置をクランプ
    float maxScroll = std::max(0.0f, totalContentHeight_ - visibleContentHeight_);
    scrollY_ = std::max(0.0f, std::min(scrollY_, maxScroll));
    
    // マウスクリックで閉じるボタンをチェック
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = systemAPI_->GetMousePosition();
        
        const float buttonWidth = 150.0f;
        const float buttonHeight = 40.0f;
        float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
        float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
        
        // ボタン内にマウスがあるか判定
        if (mouse.x >= buttonX && mouse.x <= buttonX + buttonWidth &&
            mouse.y >= buttonY && mouse.y <= buttonY + buttonHeight) {
            requestClose_ = true;
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
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
    
    // ウィンドウ背景（半透明の暗い背景）
    systemAPI_->DrawRectangle(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        Color{30, 30, 40, 240}
    );
    
    // ウィンドウ枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        2.0f,
        Color{100, 100, 120, 255}
    );
    
    // タイトル
    const char* titleText = "ライセンス";
    float titleFontSize = 36.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(titleText, titleFontSize, 1.0f);
    float titleX = windowX + (windowWidth - titleSize.x) / 2.0f;
    float titleY = windowY + 20.0f;
    systemAPI_->DrawTextDefault(titleText, titleX, titleY, titleFontSize, WHITE);
    
    // コンテンツ領域
    const float contentAreaX = windowX + 40.0f;
    const float contentAreaY = windowY + 100.0f;
    const float contentAreaWidth = windowWidth - 100.0f;  // スクロールバー用の余白
    const float contentAreaHeight = windowHeight - 180.0f;  // タイトルとボタンの間
    
    // スクロール可能なコンテンツを描画（ScissorModeでクリッピング）
    BeginScissorMode(
        static_cast<int>(contentAreaX),
        static_cast<int>(contentAreaY),
        static_cast<int>(contentAreaWidth),
        static_cast<int>(contentAreaHeight)
    );
    
    RenderLicenseText(contentAreaX, contentAreaY - scrollY_, contentAreaWidth, totalContentHeight_);
    
    EndScissorMode();
    
    // スクロールバーを描画
    RenderScrollbar(windowX, windowY, windowWidth, windowHeight);
    
    // 閉じるボタン
    const float buttonWidth = 150.0f;
    const float buttonHeight = 40.0f;
    float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
    float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
    
    // ボタン背景
    Color buttonColor = Color{80, 100, 150, 255};
    systemAPI_->DrawRectangle(
        static_cast<int>(buttonX),
        static_cast<int>(buttonY),
        static_cast<int>(buttonWidth),
        static_cast<int>(buttonHeight),
        buttonColor
    );
    
    // ボタン枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(buttonX),
        static_cast<int>(buttonY),
        static_cast<int>(buttonWidth),
        static_cast<int>(buttonHeight),
        2.0f,
        Color{120, 140, 180, 255}
    );
    
    // ボタンテキスト
    const char* buttonText = "閉じる";
    float buttonFontSize = 24.0f;
    Vector2 buttonTextSize = systemAPI_->MeasureTextDefault(buttonText, buttonFontSize, 1.0f);
    float buttonTextX = buttonX + (buttonWidth - buttonTextSize.x) / 2.0f;
    float buttonTextY = buttonY + (buttonHeight - buttonFontSize) / 2.0f;
    systemAPI_->DrawTextDefault(buttonText, buttonTextX, buttonTextY, buttonFontSize, WHITE);
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
    currentHeight += 22 * lineHeight;  // ライセンステキスト（21行 + 空行）
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
    
    return currentHeight;
}

void LicenseOverlay::RenderLicenseText(float contentX, float contentY, float contentWidth, float contentHeight) {
    float textFontSize = 20.0f;
    float lineHeight = textFontSize + 4.0f;
    float sectionSpacing = 40.0f;
    float currentY = contentY;
    
    // プロジェクトライセンス（MIT License）
    const char* projectTitle = "=== Cat Tower Defense (MIT License) ===";
    float titleFontSize = 24.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(projectTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(projectTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(projectLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // raylib (zlib/libpng license)
    const char* raylibTitle = "=== raylib (zlib/libpng License) ===";
    Vector2 raylibTitleSize = systemAPI_->MeasureTextDefault(raylibTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(raylibTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(raylibLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // ImGui (MIT License)
    const char* imguiTitle = "=== ImGui (MIT License) ===";
    Vector2 imguiTitleSize = systemAPI_->MeasureTextDefault(imguiTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(imguiTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(imguiLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // EnTT (MIT License)
    const char* enttTitle = "=== EnTT (MIT License) ===";
    Vector2 enttTitleSize = systemAPI_->MeasureTextDefault(enttTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(enttTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(enttLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // nlohmann/json (MIT License)
    const char* jsonTitle = "=== nlohmann/json (MIT License) ===";
    Vector2 jsonTitleSize = systemAPI_->MeasureTextDefault(jsonTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(jsonTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(jsonLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // spdlog (MIT License)
    const char* spdlogTitle = "=== spdlog (MIT License) ===";
    Vector2 spdlogTitleSize = systemAPI_->MeasureTextDefault(spdlogTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(spdlogTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(spdlogLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
    
    currentY += sectionSpacing;
    
    // rlImGui (MIT License)
    const char* rlimguiTitle = "=== rlImGui (MIT License) ===";
    Vector2 rlimguiTitleSize = systemAPI_->MeasureTextDefault(rlimguiTitle, titleFontSize, 1.0f);
    systemAPI_->DrawTextDefault(rlimguiTitle, contentX, currentY, titleFontSize, Color{255, 200, 100, 255});
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
        systemAPI_->DrawTextDefault(rlimguiLicense[i], contentX, currentY, textFontSize, Color{220, 220, 220, 255});
        currentY += lineHeight;
    }
}

void LicenseOverlay::HandleScrollbarInteraction(float windowX, float windowY, float windowWidth, float windowHeight) {
    if (totalContentHeight_ <= visibleContentHeight_) {
        // スクロール不要な場合は何もしない
        isDraggingScrollbar_ = false;
        return;
    }
    
    const float scrollbarWidth = 20.0f;
    const float scrollbarX = windowX + windowWidth - scrollbarWidth - 20.0f;
    const float scrollbarY = windowY + 100.0f;  // タイトル下
    const float scrollbarHeight = windowHeight - 180.0f;  // タイトルとボタンの間
    
    float scrollRatio = visibleContentHeight_ / totalContentHeight_;
    float thumbHeight = scrollbarHeight * scrollRatio;
    float thumbY = scrollbarY + (scrollY_ / totalContentHeight_) * scrollbarHeight;
    
    Vector2 mouse = systemAPI_->GetMousePosition();
    bool mouseOverScrollbar = (mouse.x >= scrollbarX && mouse.x <= scrollbarX + scrollbarWidth &&
                               mouse.y >= scrollbarY && mouse.y <= scrollbarY + scrollbarHeight);
    bool mouseOverThumb = (mouse.x >= scrollbarX && mouse.x <= scrollbarX + scrollbarWidth &&
                           mouse.y >= thumbY && mouse.y <= thumbY + thumbHeight);
    
    // マウスボタンが押されたとき
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (mouseOverThumb) {
            // つまみをクリックした場合、ドラッグ開始
            isDraggingScrollbar_ = true;
            dragStartY_ = mouse.y;
            dragStartScrollY_ = scrollY_;
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        } else if (mouseOverScrollbar) {
            // スクロールバーのトラック（つまみ以外）をクリックした場合、その位置にスクロール
            float maxScroll = totalContentHeight_ - visibleContentHeight_;
            float scrollRange = scrollbarHeight - thumbHeight;  // つまみが動ける範囲
            
            if (scrollRange > 0.0f) {
                // クリック位置をスクロール範囲にマッピング
                float clickY = mouse.y - scrollbarY;
                float clickRatio = (clickY - thumbHeight / 2.0f) / scrollRange;
                clickRatio = std::max(0.0f, std::min(1.0f, clickRatio));  // 0-1にクランプ
                scrollY_ = clickRatio * maxScroll;
            }
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        }
    }
    
    // ドラッグ中
    if (isDraggingScrollbar_ && systemAPI_->IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        float deltaY = mouse.y - dragStartY_;
        float maxScroll = totalContentHeight_ - visibleContentHeight_;
        float scrollRange = scrollbarHeight - thumbHeight;  // つまみが動ける範囲
        
        if (scrollRange > 0.0f) {
            float scrollDelta = (deltaY / scrollRange) * maxScroll;
            scrollY_ = dragStartScrollY_ + scrollDelta;
            scrollY_ = std::max(0.0f, std::min(scrollY_, maxScroll));
        }
    }
    
    // マウスボタンが離されたとき
    if (systemAPI_->IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDraggingScrollbar_ = false;
    }
}

void LicenseOverlay::RenderScrollbar(float windowX, float windowY, float windowWidth, float windowHeight) {
    const float scrollbarWidth = 20.0f;
    const float scrollbarX = windowX + windowWidth - scrollbarWidth - 20.0f;
    const float scrollbarY = windowY + 100.0f;  // タイトル下
    const float scrollbarHeight = windowHeight - 180.0f;  // タイトルとボタンの間
    
    // スクロールバー背景
    systemAPI_->DrawRectangle(
        static_cast<int>(scrollbarX),
        static_cast<int>(scrollbarY),
        static_cast<int>(scrollbarWidth),
        static_cast<int>(scrollbarHeight),
        Color{50, 50, 60, 255}
    );
    
    // スクロール可能な場合のみスクロールバーを描画
    if (totalContentHeight_ > visibleContentHeight_) {
        float scrollRatio = visibleContentHeight_ / totalContentHeight_;
        float thumbHeight = scrollbarHeight * scrollRatio;
        float thumbY = scrollbarY + (scrollY_ / totalContentHeight_) * scrollbarHeight;
        
        // つまみの色（ドラッグ中は明るく）
        Color thumbColor = isDraggingScrollbar_ ? 
            Color{150, 150, 170, 255} :  // ドラッグ中
            Color{120, 120, 140, 255};   // 通常
        
        // スクロールバーのつまみ
        systemAPI_->DrawRectangle(
            static_cast<int>(scrollbarX + 2),
            static_cast<int>(thumbY),
            static_cast<int>(scrollbarWidth - 4),
            static_cast<int>(thumbHeight),
            thumbColor
        );
        
        // つまみの枠線
        systemAPI_->DrawRectangleLines(
            static_cast<int>(scrollbarX + 2),
            static_cast<int>(thumbY),
            static_cast<int>(scrollbarWidth - 4),
            static_cast<int>(thumbHeight),
            1.0f,
            Color{160, 160, 180, 255}
        );
    }
}

} // namespace core
} // namespace game
