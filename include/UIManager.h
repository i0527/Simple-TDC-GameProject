#pragma once

#include <raylib.h>
#include <string>

namespace UI {
    // UIマネージャークラス（Singleton）
    // raygui と Dear ImGui (rlImGui) を共存させて管理するクラス
    class UIManager {
    public:
        static UIManager& GetInstance();
        
        // インスタンスのコピーを禁止
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;
        
        // 初期化（日本語フォント対応含む）
        // fontPath: 日本語フォントのパス（例: "assets/fonts/NotoSansCJKjp-Regular.otf"）
        // fontSize: フォントサイズ
        void Initialize(const std::string& fontPath = "", float fontSize = 18.0f);
        
        // 終了処理
        void Shutdown();
        
        // フレーム開始処理（メインループの BeginDrawing() 後に呼び出す）
        void BeginFrame();
        
        // ImGui描画開始（raygui描画後に呼び出す）
        void BeginImGui();
        
        // ImGui描画終了
        void EndImGui();
        
        // フレーム終了処理（メインループの EndDrawing() 前に呼び出す）
        void EndFrame();
        
        // 日本語フォントがロードされているか
        bool HasJapaneseFont() const { return japaneseFont_.texture.id != 0; }
        
        // rayguiフォントを取得
        Font GetRayguiFont() const { return japaneseFont_; }
        
        // サンプルUIを描画（デモ用）
        void DrawSampleUI();
        
    private:
        UIManager() = default;
        ~UIManager() = default;
        
        // 日本語フォントをロード
        bool LoadJapaneseFont(const std::string& fontPath, float fontSize);
        
        // rayguiにフォントを設定
        void SetupRayguiFont();
        
        // ImGuiにフォントを設定
        void SetupImGuiFont(const std::string& fontPath, float fontSize);
        
        Font japaneseFont_{};       // 日本語フォント（raygui用）
        bool initialized_ = false;  // 初期化済みフラグ
    };
}
