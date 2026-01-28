#ifndef __GAME_CORE_GAMECONFIG_HPP__
#define __GAME_CORE_GAMECONFIG_HPP__

namespace game {
    namespace core {
        /// @brief 解像度プリセット定義
        enum class Resolution {
            FHD,  ///< フルHD: 1920x1080
            HD,   ///< HD: 1280x720
            SD    ///< SD: 854x480
        };

        /// @brief ウィンドウモード定義
        enum class WindowMode {
            Windowed,   ///< 通常ウィンドウ
            Fullscreen, ///< 排他フルスクリーン
            Borderless   ///< フルスクリーンウィンドウ（ボーダーレス）
        };

        /// @brief 指定された解像度の幅を取得
        /// @param res 解像度プリセット
        /// @return 幅（ピクセル）
        inline int GetResolutionWidth(Resolution res) {
            switch (res) {
                case Resolution::FHD: return 1920;
                case Resolution::HD:  return 1280;
                case Resolution::SD:  return 854;
                default:              return 1920;
            }
        }

        /// @brief 指定された解像度の高さを取得
        /// @param res 解像度プリセット
        /// @return 高さ（ピクセル）
        inline int GetResolutionHeight(Resolution res) {
            switch (res) {
                case Resolution::FHD: return 1080;
                case Resolution::HD:  return 720;
                case Resolution::SD:  return 480;
                default:              return 1080;
            }
        }

        // グローバル定数
        constexpr int TARGET_FPS = 60;

        // デバッグUI初期表示状態（ビルド設定で切り替え）
#ifdef _DEBUG
        constexpr bool DEFAULT_DEBUG_UI_VISIBLE = true;
#else
        constexpr bool DEFAULT_DEBUG_UI_VISIBLE = false;
#endif
    }
}

#endif // __GAME_CORE_GAMECONFIG_HPP__
