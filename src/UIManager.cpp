#include "UIManager.h"

// raygui implementation (must be defined once)
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// rlImGui for Dear ImGui integration
#include <rlImGui.h>
#include <imgui.h>

#include <iostream>
#include <fstream>

namespace UI {

UIManager& UIManager::GetInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::Initialize(const std::string& fontPath, float fontSize) {
    if (initialized_) {
        return;
    }
    
    // rlImGui初期化
    rlImGuiSetup(true);
    
    // 日本語フォントのロード
    if (!fontPath.empty()) {
        if (LoadJapaneseFont(fontPath, fontSize)) {
            SetupRayguiFont();
            SetupImGuiFont(fontPath, fontSize);
            std::cout << "Japanese font loaded: " << fontPath << std::endl;
        } else {
            std::cerr << "Failed to load Japanese font: " << fontPath << std::endl;
        }
    }
    
    initialized_ = true;
    std::cout << "UIManager initialized" << std::endl;
}

void UIManager::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // フォントのアンロード
    if (japaneseFont_.texture.id != 0) {
        UnloadFont(japaneseFont_);
        japaneseFont_ = {};
    }
    
    // rlImGui終了
    rlImGuiShutdown();
    
    initialized_ = false;
    std::cout << "UIManager shutdown" << std::endl;
}

void UIManager::BeginFrame() {
    // フレーム開始時の処理（必要に応じて拡張）
}

void UIManager::BeginImGui() {
    rlImGuiBegin();
}

void UIManager::EndImGui() {
    rlImGuiEnd();
}

void UIManager::EndFrame() {
    // フレーム終了時の処理（必要に応じて拡張）
}

bool UIManager::LoadJapaneseFont(const std::string& fontPath, float fontSize) {
    // ファイル存在チェック
    std::ifstream file(fontPath);
    if (!file.good()) {
        return false;
    }
    file.close();
    
    // 日本語グリフ範囲を定義（基本的な日本語文字）
    // ひらがな、カタカナ、基本漢字、記号
    static const int glyphRanges[] = {
        0x0020, 0x007F,  // ASCII
        0x3000, 0x303F,  // 日本語句読点
        0x3040, 0x309F,  // ひらがな
        0x30A0, 0x30FF,  // カタカナ
        0x4E00, 0x9FFF,  // CJK統合漢字
        0xFF00, 0xFFEF,  // 全角ASCII、半角カナ
        0
    };
    
    // フォントをロード（グリフ数を指定）
    int glyphCount = 0;
    for (int i = 0; glyphRanges[i] != 0; i += 2) {
        glyphCount += glyphRanges[i + 1] - glyphRanges[i] + 1;
    }
    
    // フォントをロード
    japaneseFont_ = LoadFontEx(fontPath.c_str(), static_cast<int>(fontSize), nullptr, glyphCount);
    
    return japaneseFont_.texture.id != 0;
}

void UIManager::SetupRayguiFont() {
    if (japaneseFont_.texture.id != 0) {
        GuiSetFont(japaneseFont_);
        GuiSetStyle(DEFAULT, TEXT_SIZE, japaneseFont_.baseSize);
    }
}

void UIManager::SetupImGuiFont(const std::string& fontPath, float fontSize) {
    ImGuiIO& io = ImGui::GetIO();
    
    // 日本語グリフ範囲を取得
    static const ImWchar japaneseRanges[] = {
        0x0020, 0x007F,  // ASCII
        0x3000, 0x303F,  // 日本語句読点
        0x3040, 0x309F,  // ひらがな
        0x30A0, 0x30FF,  // カタカナ
        0x4E00, 0x9FFF,  // CJK統合漢字
        0xFF00, 0xFFEF,  // 全角ASCII、半角カナ
        0
    };
    
    // ImGuiにフォントを追加
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, nullptr, japaneseRanges);
    
    // フォントテクスチャを再構築
    rlImGuiReloadFonts();
    
    std::cout << "ImGui Japanese font configured" << std::endl;
}

void UIManager::DrawSampleUI() {
    // === raygui サンプル描画 ===
    // raygui ボタン（HUD/操作パネル用）
    if (GuiButton((Rectangle){10, 500, 200, 40}, "日本語ボタン")) {
        std::cout << "raygui button clicked!" << std::endl;
    }
    
    // raygui ラベル
    GuiLabel((Rectangle){10, 550, 200, 20}, "raygui 日本語ラベル");
    
    // === ImGui サンプル描画 ===
    BeginImGui();
    
    // デバッグ情報ウィンドウ
    ImGui::Begin("Debug Info / デバッグ情報");
    ImGui::Text("FPS: %d", GetFPS());
    ImGui::Text("日本語デバッグ表示テスト");
    ImGui::Separator();
    ImGui::Text("Frame Time: %.3f ms", GetFrameTime() * 1000.0f);
    
    static bool showDemo = false;
    ImGui::Checkbox("Show Demo Window", &showDemo);
    if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
    }
    
    ImGui::End();
    
    EndImGui();
}

} // namespace UI
