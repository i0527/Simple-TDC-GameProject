#pragma once

#include "../config/SharedContext.hpp"

namespace game {
    namespace core {

        /// @brief モジュールインターフェース
        /// 
        /// すべてのゲームモジュールが実装する必要があるインターフェース。
        /// ライフサイクル（初期化・更新・描画・終了）と優先順位管理を提供します。
        class IModule {
        public:
            virtual ~IModule() = default;
            
            /// @brief モジュールの初期化
            /// @param ctx 共有コンテキスト
            /// @return 成功時true、失敗時false
            virtual bool Initialize(SharedContext& ctx) = 0;
            
            /// @brief モジュールの更新処理
            /// @param ctx 共有コンテキスト
            /// @param dt デルタタイム（秒）
            virtual void Update(SharedContext& ctx, float dt) = 0;
            
            /// @brief モジュールの描画処理
            /// @param ctx 共有コンテキスト
            virtual void Render(SharedContext& ctx) = 0;
            
            /// @brief モジュールのシャットダウン処理
            /// @param ctx 共有コンテキスト
            virtual void Shutdown(SharedContext& ctx) = 0;
            
            /// @brief モジュール名を取得
            /// @return モジュール名
            virtual const char* GetName() const = 0;
            
            /// @brief 更新処理の優先順位を取得
            /// @return 優先順位（小さい値から順に実行される）
            virtual int GetUpdatePriority() const { return 0; }
            
            /// @brief 描画処理の優先順位を取得
            /// @return 優先順位（小さい値から順に実行される）
            virtual int GetRenderPriority() const { return 0; }
        };

    } // namespace core
} // namespace game
