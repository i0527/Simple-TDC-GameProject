#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <entt/entt.hpp>

namespace UI {
    /// @brief ゲーム全体のUI・フォント管理クラス（Singleton）
    /// raygui と Dear ImGui (rlImGui) を統合管理し、
    /// 複数サイズのフォントをキャッシュして効率的に提供
    class UIManager {
    public:
        static UIManager& GetInstance();
        
        // コピー・ムーブ禁止
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;
        UIManager(UIManager&&) = delete;
        UIManager& operator=(UIManager&&) = delete;
        
        /// @brief UIマネージャーを初期化
        /// @param fontPath デフォルトフォントのパス（例: "assets/fonts/NotoSansJP-Medium.ttf"）
        /// @param baseFontSize 基本フォントサイズ（デフォルト: 18.0f）
        void Initialize(const std::string& fontPath, float baseFontSize = 18.0f);
        
        /// @brief UIマネージャーを終了
        void Shutdown();
        
        /// @brief ImGui描画開始
        void BeginImGui();
        
        /// @brief ImGui描画終了
        void EndImGui();
        
        /// @brief デフォルトフォントへの参照を取得（効率化：コピーなし）
        /// @return デフォルトフォントへのconst参照
        [[nodiscard]] const Font& GetFont() const noexcept { return baseFont_; }
        
        /// @brief 指定サイズのフォントを取得（キャッシュ付き）
        /// @param fontSize フォントサイズ
        /// @return 指定サイズのフォントへのconst参照
        [[nodiscard]] const Font& GetFont(int fontSize);
        
        /// @brief デフォルトフォントのベースサイズを取得
        /// @return ベースフォントサイズ
        [[nodiscard]] int GetBaseFontSize() const noexcept { return baseFontSize_; }
        
        /// @brief フォントが初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }
        
        /// @brief サンプルUIを描画（デモ用）
        void DrawSampleUI();
        
        /// @brief デバッグウィンドウを描画（エンティティ情報表示・編集）
        /// @param registry ECS registry
        void DrawDebugWindow(entt::registry& registry);
        
    private:
        UIManager() = default;
        ~UIManager() = default;
        
        /// @brief フォントをロード
        /// @param fontPath フォントファイルパス
        /// @param fontSize フォントサイズ
        /// @return ロードされたフォント
        Font LoadFontWithGlyphs(const std::string& fontPath, int fontSize);
        
        /// @brief rayguiにフォントを設定
        void SetupRayguiFont();
        
        /// @brief ImGuiにフォントを設定
        /// @param fontPath フォントファイルパス
        void SetupImGuiFont(const std::string& fontPath);
        
        std::string fontPath_;                           ///< ロードしたフォントファイルパス
        Font baseFont_{};                                ///< ベースフォント
        int baseFontSize_{18};                           ///< ベースフォントサイズ
        std::unordered_map<int, Font> fontCache_;        ///< サイズ別フォントキャッシュ
        bool initialized_{false};                        ///< 初期化フラグ
    };
    
    /// @brief テキスト描画ヘルパー関数（フォント取得を簡略化）
    /// @param text 描画するテキスト
    /// @param position 描画位置
    /// @param fontSize フォントサイズ
    /// @param color テキスト色
    inline void DrawText(const char* text, Vector2 position, int fontSize, Color color) {
        auto& uiMgr = UIManager::GetInstance();
        DrawTextEx(uiMgr.GetFont(fontSize), text, position, static_cast<float>(fontSize), 1.0f, color);
    }
}

