#include "UIManager.h"

// raygui implementation (must be defined once)
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// rlImGui for Dear ImGui integration
#include <rlImGui.h>
#include <imgui.h>

#include <iostream>
#include <vector>

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
    // ファイル存在チェック（FileExists はRaylibの関数）
    if (!FileExists(fontPath.c_str())) {
        return false;
    }
    
    // 基本的なグリフのコードポイントを定義
    // 日本語フルセットは大きすぎるため、よく使う文字のみ含める
    // ASCII + ひらがな + カタカナ + 基本記号
    static std::vector<int> codepoints;
    if (codepoints.empty()) {
        // ASCII (0x0020-0x007F)
        for (int cp = 0x0020; cp <= 0x007F; ++cp) codepoints.push_back(cp);
        // 日本語句読点 (0x3000-0x303F)
        for (int cp = 0x3000; cp <= 0x303F; ++cp) codepoints.push_back(cp);
        // ひらがな (0x3040-0x309F)
        for (int cp = 0x3040; cp <= 0x309F; ++cp) codepoints.push_back(cp);
        // カタカナ (0x30A0-0x30FF)
        for (int cp = 0x30A0; cp <= 0x30FF; ++cp) codepoints.push_back(cp);
        // 全角ASCII、半角カナ (0xFF00-0xFFEF)
        for (int cp = 0xFF00; cp <= 0xFFEF; ++cp) codepoints.push_back(cp);
    }
    
    // フォントをロード（コードポイント配列を渡す）
    japaneseFont_ = LoadFontEx(
        fontPath.c_str(), 
        static_cast<int>(fontSize), 
        codepoints.data(), 
        static_cast<int>(codepoints.size())
    );
    
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
    
    // 日本語グリフ範囲を取得（CJK漢字は除外してメモリ使用量を抑制）
    // ひらがな、カタカナ、基本記号のみ含む
    static const ImWchar japaneseRanges[] = {
        0x0020, 0x007F,  // ASCII
        0x3000, 0x303F,  // 日本語句読点
        0x3040, 0x309F,  // ひらがな
        0x30A0, 0x30FF,  // カタカナ
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
    if (GuiButton({10, 500, 200, 40}, u8"日本語ボタン")) {
        std::cout << "raygui button clicked!" << std::endl;
    }
    
    // raygui ラベル
    GuiLabel({10, 550, 200, 20}, u8"raygui 日本語ラベル");
    
    // === ImGui サンプル描画 ===
    BeginImGui();
    
    // デバッグ情報ウィンドウ
    ImGui::Begin(u8"Debug Info / デバッグ情報");
    ImGui::Text("FPS: %d", GetFPS());
    //ImGui::Text("日本語デバッグ表示テスト.");
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
