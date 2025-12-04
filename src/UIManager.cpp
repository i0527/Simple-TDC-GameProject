#include "UIManager.h"
#include "Components.h"
#include "ResourceManager.h"

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

void UIManager::Initialize(const std::string& fontPath, float baseFontSize) {
    if (initialized_) {
        std::cerr << "UIManager already initialized. Skipping." << std::endl;
        return;
    }
    
    if (fontPath.empty()) {
        std::cerr << "Font path is empty. UIManager initialization failed." << std::endl;
        return;
    }
    
    fontPath_ = fontPath;
    baseFontSize_ = static_cast<int>(baseFontSize);
    
    // rlImGui初期化
    rlImGuiSetup(true);
    
    // ベースフォントをロード
    std::cout << "Loading base font: " << fontPath_ << " (size: " << baseFontSize_ << ")" << std::endl;
    baseFont_ = LoadFontWithGlyphs(fontPath_, baseFontSize_);
    
    if (baseFont_.texture.id == 0) {
        std::cerr << "Failed to load base font: " << fontPath_ << std::endl;
        rlImGuiShutdown();
        return;
    }
    
    // ライブラリにフォントを設定
    SetupRayguiFont();
    SetupImGuiFont(fontPath_);
    
    initialized_ = true;
    std::cout << "UIManager initialized successfully" << std::endl;
    std::cout << "  - Font: " << fontPath_ << std::endl;
    std::cout << "  - Base size: " << baseFontSize_ << std::endl;
    std::cout << "  - Texture ID: " << baseFont_.texture.id << std::endl;
}

void UIManager::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // キャッシュされたフォントをアンロード
    for (auto& [size, font] : fontCache_) {
        if (font.texture.id != 0) {
            UnloadFont(font);
        }
    }
    fontCache_.clear();
    
    // ベースフォントをアンロード
    if (baseFont_.texture.id != 0) {
        UnloadFont(baseFont_);
        baseFont_ = {};
    }
    
    // rlImGui終了
    rlImGuiShutdown();
    
    initialized_ = false;
    std::cout << "UIManager shutdown complete" << std::endl;
}

const Font& UIManager::GetFont(int fontSize) {
    // ベースサイズの場合はベースフォントを返す
    if (fontSize == baseFontSize_) {
        return baseFont_;
    }
    
    // キャッシュを確認
    auto it = fontCache_.find(fontSize);
    if (it != fontCache_.end()) {
        return it->second;
    }
    
    // 新しいサイズのフォントをロードしてキャッシュ
    std::cout << "Loading font size: " << fontSize << std::endl;
    Font newFont = LoadFontWithGlyphs(fontPath_, fontSize);
    
    if (newFont.texture.id == 0) {
        std::cerr << "Failed to load font size " << fontSize << ", using base font" << std::endl;
        return baseFont_;
    }
    
    fontCache_[fontSize] = newFont;
    return fontCache_[fontSize];
}

Font UIManager::LoadFontWithGlyphs(const std::string& fontPath, int fontSize) {
    // ファイル存在チェック
    if (!FileExists(fontPath.c_str())) {
        std::cerr << "Font file not found: " << fontPath << std::endl;
        return Font{};
    }
    
    // グリフのコードポイント範囲を定義（静的キャッシュ）
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

        // ----- 以下を追加: JIS 第一水準・第二水準 (主要な漢字集合) -----
        // JIS第一・二水準の漢字は主に CJK Unified Ideographs 範囲に含まれるため、
        // 代表的な範囲を追加します（U+4E00 - U+9FFF）。
        // 必要に応じて Extension A (U+3400 - U+4DBF) も含めます。
        // 注意: 大量のグリフをロードするとメモリ使用量とフォント生成時間が増えます。

        // CJK Unified Ideographs (一般的な漢字)
        for (int cp = 0x4E00; cp <= 0x9FFF; ++cp) codepoints.push_back(cp);
        // CJK Unified Ideographs Extension A (追加漢字)
        for (int cp = 0x3400; cp <= 0x4DBF; ++cp) codepoints.push_back(cp);
    }
    
    // フォントをロード
    Font font = LoadFontEx(
        fontPath.c_str(),
        fontSize,
        codepoints.data(),
        static_cast<int>(codepoints.size())
    );
    
    if (font.texture.id != 0) {
        std::cout << "Font loaded: size=" << fontSize << ", baseSize=" << font.baseSize << 
                     ", glyphs=" << codepoints.size() << std::endl;
    }
    
    return font;
}

void UIManager::SetupRayguiFont() {
    if (baseFont_.texture.id != 0) {
        GuiSetFont(baseFont_);
        GuiSetStyle(DEFAULT, TEXT_SIZE, baseFont_.baseSize);
        std::cout << "raygui font configured" << std::endl;
    }
}

void UIManager::SetupImGuiFont(const std::string& fontPath) {
    ImGuiIO& io = ImGui::GetIO();
    
    // 日本語グリフ範囲を定義
    static const ImWchar japaneseRanges[] = {
        0x0020, 0x007F,  // ASCII
        0x3000, 0x303F,  // 日本語句読点
        0x3040, 0x309F,  // ひらがな
        0x30A0, 0x30FF,  // カタカナ
        0xFF00, 0xFFEF,  // 全角ASCII、半角カナ
        0x3400, 0x4DBF,  // CJK Ext A
        0x4E00, 0x9FFF,  // CJK Unified Ideographs
        0
    };
    
    // ImGuiにフォントを追加し、戻り値のImFont*をデフォルトフォントに設定する
    ImFont* imFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), static_cast<float>(baseFontSize_), nullptr, japaneseRanges);
    if (imFont == nullptr) {
        std::cerr << "ImGui: failed to load font from " << fontPath << std::endl;
        return;
    }
    
    // 明示的にフォントアトラスをビルドしてデフォルトフォントを設定
    io.Fonts->Build();
    io.FontDefault = imFont;
    
    // rlImGui 用のフォントテクスチャを再構築
    rlImGuiReloadFonts();
    
    std::cout << "ImGui font configured and set as default" << std::endl;
}

void UIManager::BeginImGui() {
    rlImGuiBegin();
}

void UIManager::EndImGui() {
    rlImGuiEnd();
}

void UIManager::DrawSampleUI() {
    // === raygui サンプル描画 ===
    if (GuiButton({10, 500, 200, 40}, u8"日本語ボタン")) {
        std::cout << "raygui button clicked!" << std::endl;
    }
    
    GuiLabel({10, 550, 200, 20}, u8"raygui 日本語ラベル");
}

void UIManager::DrawDebugWindow(entt::registry& registry) {
    // ImGuiウィンドウは既にGame::Render()でBeginImGui/EndImGuiで囲まれているため、
    // ここでは直接ウィンドウを描画するだけ
    
    // === ImGui サンプル描画 ===
    BeginImGui();
    
    ImGui::Begin(u8"Debug Info / デバッグ情報");
    ImGui::Text("FPS: %d", GetFPS());
    ImGui::Separator();
    ImGui::Text("Frame Time: %.3f ms", GetFrameTime() * 1000.0f);
    
    static bool showDemo = false;
    ImGui::Checkbox("Show Demo Window", &showDemo);
    if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
    }
    
    ImGui::End();
    
    // --- Entity Debug Window ---
    ImGui::Begin(u8"Entity Debug / エンティティデバッグ", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    // 利用可能なスプライト一覧を取得
    Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
    auto& imageMgr = rm.GetImageManager();
    std::vector<std::string> availableSprites = imageMgr.GetAllSpriteSheetNames();
    
    // --- Player (矢印キー操作) ---
    ImGui::SeparatorText(u8"Player (Arrow Keys / 矢印キー)");
    
    auto playerView = registry.view<Components::Player, Components::Position, Components::Velocity, Components::Scale, Components::SpriteAnimation>();
    for (auto entity : playerView) {
        auto& pos = playerView.get<Components::Position>(entity);
        auto& vel = playerView.get<Components::Velocity>(entity);
        auto& scale = playerView.get<Components::Scale>(entity);
        auto& anim = playerView.get<Components::SpriteAnimation>(entity);
        
        ImGui::PushID(static_cast<int>(entity));
        
        ImGui::Text(u8"Entity ID: %d", static_cast<int>(entity));
        
        // キャラクター切り替えコンボボックス
        static int currentSpriteIndex = 0;
        if (ImGui::BeginCombo(u8"Character / キャラクター", anim.spriteName.c_str())) {
            for (int i = 0; i < availableSprites.size(); ++i) {
                const bool isSelected = (anim.spriteName == availableSprites[i]);
                if (ImGui::Selectable(availableSprites[i].c_str(), isSelected)) {
                    // キャラクター変更
                    std::string newSprite = availableSprites[i];
                    std::vector<std::string> newFrames = imageMgr.GetAllFrameNames(newSprite);
                    
                    if (!newFrames.empty()) {
                        anim.spriteName = newSprite;
                        anim.frames = newFrames;
                        anim.currentFrameIndex = 0;
                        anim.elapsedTime = 0.0f;
                        
                        // SpriteFrameとSpriteTextureも更新
                        auto& spriteFrame = registry.get<Components::SpriteFrame>(entity);
                        auto& spriteTexture = registry.get<Components::SpriteTexture>(entity);
                        
                        auto frameInfo = imageMgr.GetFrameInfo(newFrames[0]);
                        spriteFrame.frameName = newFrames[0];
                        spriteFrame.sourceRect = frameInfo.rect;
                        spriteTexture.textureName = newSprite;
                        
                        std::cout << "Character changed to: " << newSprite << std::endl;
                    }
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        // 位置
        float position[2] = {pos.x, pos.y};
        if (ImGui::DragFloat2(u8"Position / 位置", position, 1.0f)) {
            pos.x = position[0];
            pos.y = position[1];
        }
        
        // 速度（ボタンで増減）
        ImGui::Text(u8"Velocity / 速度: (%.1f, %.1f)", vel.dx, vel.dy);
        ImGui::SameLine();
        if (ImGui::Button(u8"+X##vel_player")) vel.dx += 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"-X##vel_player")) vel.dx -= 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"+Y##vel_player")) vel.dy += 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"-Y##vel_player")) vel.dy -= 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"Reset##vel_player")) {
            vel.dx = 0.0f;
            vel.dy = 0.0f;
        }
        
        // スケール
        float scaleVal[2] = {scale.x, scale.y};
        if (ImGui::DragFloat2(u8"Scale / スケール", scaleVal, 0.1f, 0.1f, 5.0f)) {
            scale.x = scaleVal[0];
            scale.y = scaleVal[1];
        }
        
        ImGui::PopID();
        ImGui::Spacing();
    }
    
    ImGui::Separator();
    
    // --- WASD Player ---
    ImGui::SeparatorText(u8"WASD Player");
    
    auto wasdView = registry.view<Components::WASDPlayer, Components::Position, Components::Velocity, Components::Scale, Components::SpriteAnimation>();
    for (auto entity : wasdView) {
        auto& pos = wasdView.get<Components::Position>(entity);
        auto& vel = wasdView.get<Components::Velocity>(entity);
        auto& scale = wasdView.get<Components::Scale>(entity);
        auto& anim = wasdView.get<Components::SpriteAnimation>(entity);
        
        ImGui::PushID(static_cast<int>(entity));
        
        ImGui::Text(u8"Entity ID: %d", static_cast<int>(entity));
        
        // キャラクター切り替えコンボボックス
        if (ImGui::BeginCombo(u8"Character / キャラクター", anim.spriteName.c_str())) {
            for (int i = 0; i < availableSprites.size(); ++i) {
                const bool isSelected = (anim.spriteName == availableSprites[i]);
                if (ImGui::Selectable(availableSprites[i].c_str(), isSelected)) {
                    // キャラクター変更
                    std::string newSprite = availableSprites[i];
                    std::vector<std::string> newFrames = imageMgr.GetAllFrameNames(newSprite);
                    
                    if (!newFrames.empty()) {
                        anim.spriteName = newSprite;
                        anim.frames = newFrames;
                        anim.currentFrameIndex = 0;
                        anim.elapsedTime = 0.0f;
                        
                        // SpriteFrameとSpriteTextureも更新
                        auto& spriteFrame = registry.get<Components::SpriteFrame>(entity);
                        auto& spriteTexture = registry.get<Components::SpriteTexture>(entity);
                        
                        auto frameInfo = imageMgr.GetFrameInfo(newFrames[0]);
                        spriteFrame.frameName = newFrames[0];
                        spriteFrame.sourceRect = frameInfo.rect;
                        spriteTexture.textureName = newSprite;
                        
                        std::cout << "Character changed to: " << newSprite << std::endl;
                    }
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        // 位置
        float position[2] = {pos.x, pos.y};
        if (ImGui::DragFloat2(u8"Position / 位置", position, 1.0f)) {
            pos.x = position[0];
            pos.y = position[1];
        }
        
        // 速度（ボタンで増減）
        ImGui::Text(u8"Velocity / 速度: (%.1f, %.1f)", vel.dx, vel.dy);
        ImGui::SameLine();
        if (ImGui::Button(u8"+X##vel_wasd")) vel.dx += 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"-X##vel_wasd")) vel.dx -= 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"+Y##vel_wasd")) vel.dy += 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"-Y##vel_wasd")) vel.dy -= 10.0f;
        ImGui::SameLine();
        if (ImGui::Button(u8"Reset##vel_wasd")) {
            vel.dx = 0.0f;
            vel.dy = 0.0f;
        }
        
        // スケール
        float scaleVal[2] = {scale.x, scale.y};
        if (ImGui::DragFloat2(u8"Scale / スケール", scaleVal, 0.1f, 0.1f, 5.0f)) {
            scale.x = scaleVal[0];
            scale.y = scaleVal[1];
        }
        
        ImGui::PopID();
        ImGui::Spacing();
    }
    
    ImGui::End();
}

} // namespace UI
