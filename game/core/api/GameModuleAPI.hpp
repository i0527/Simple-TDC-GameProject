#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <cassert>

namespace game {
    namespace core {

        /// @brief EnTTベースのゲームモジュールAPI
        /// 
        /// EnTTレジストリをラップし、シンプルで使いやすいAPIを提供します。
        /// メインコードを短く、わかりやすくすることを目的としています。
        class GameModuleAPI {
        public:
            GameModuleAPI();
            ~GameModuleAPI() = default;

            // レジストリアクセス（直接アクセスが必要な場合）
            entt::registry& Registry() { return registry_; }
            const entt::registry& Registry() const { return registry_; }

            // ========== エンティティ操作 ==========

            /// @brief エンティティを作成
            entt::entity Create();

            /// @brief エンティティを削除
            void Destroy(entt::entity e);

            /// @brief エンティティが有効かどうかを確認
            bool Valid(entt::entity e) const;

            /// @brief 生存エンティティ数を取得
            size_t Count() const;

            /// @brief 全エンティティを削除
            void Clear();

            // ========== コンポーネント操作 ==========

            /// @brief コンポーネントを追加
            template<typename T, typename... Args>
            T& Add(entt::entity e, Args&&... args);

            /// @brief コンポーネントを取得
            template<typename T>
            T& Get(entt::entity e);

            /// @brief コンポーネントを取得（const版）
            template<typename T>
            const T& Get(entt::entity e) const;

            /// @brief コンポーネントを取得（存在しない場合はnullptr）
            template<typename T>
            T* Try(entt::entity e);

            /// @brief コンポーネントを取得（const版、存在しない場合はnullptr）
            template<typename T>
            const T* Try(entt::entity e) const;

            /// @brief コンポーネントを削除
            template<typename T>
            void Remove(entt::entity e);

            /// @brief コンポーネントの存在確認
            template<typename T>
            bool Has(entt::entity e) const;

            /// @brief 全コンポーネントの存在確認
            template<typename... T>
            bool HasAll(entt::entity e) const;

            // ========== ビュー作成 ==========

            /// @brief ビューを作成
            template<typename... T>
            auto View();

            /// @brief 除外ビューを作成
            template<typename... T, typename... Exclude>
            auto View(entt::exclude_t<Exclude...>);

            // ========== コンテキスト変数 ==========

            /// @brief コンテキスト変数を設定または取得
            template<typename T, typename... Args>
            T& Ctx(Args&&... args);

            /// @brief コンテキスト変数を取得
            template<typename T>
            T& Ctx();

            /// @brief コンテキスト変数を取得（const版）
            template<typename T>
            const T& Ctx() const;

            /// @brief コンテキスト変数の存在確認
            template<typename T>
            bool HasCtx() const;

        private:
            entt::registry registry_;
        };

        // ========== テンプレート実装 ==========

        template<typename T, typename... Args>
        inline T& GameModuleAPI::Add(entt::entity e, Args&&... args) {
            return registry_.emplace<T>(e, std::forward<Args>(args)...);
        }

        template<typename T>
        inline T& GameModuleAPI::Get(entt::entity e) {
            return registry_.get<T>(e);
        }

        template<typename T>
        inline const T& GameModuleAPI::Get(entt::entity e) const {
            return registry_.get<T>(e);
        }

        template<typename T>
        inline T* GameModuleAPI::Try(entt::entity e) {
            return registry_.try_get<T>(e);
        }

        template<typename T>
        inline const T* GameModuleAPI::Try(entt::entity e) const {
            return registry_.try_get<T>(e);
        }

        template<typename T>
        inline void GameModuleAPI::Remove(entt::entity e) {
            registry_.remove<T>(e);
        }

        template<typename T>
        inline bool GameModuleAPI::Has(entt::entity e) const {
            return registry_.all_of<T>(e);
        }

        template<typename... T>
        inline bool GameModuleAPI::HasAll(entt::entity e) const {
            return registry_.all_of<T...>(e);
        }

        template<typename... T>
        inline auto GameModuleAPI::View() {
            return registry_.view<T...>();
        }

        template<typename... T, typename... Exclude>
        inline auto GameModuleAPI::View(entt::exclude_t<Exclude...>) {
            return registry_.view<T...>(entt::exclude<Exclude...>);
        }

        template<typename T, typename... Args>
        inline T& GameModuleAPI::Ctx(Args&&... args) {
            if (registry_.ctx().contains<T>()) {
                return registry_.ctx().get<T>();
            }
            return registry_.ctx().emplace<T>(std::forward<Args>(args)...);
        }

        template<typename T>
        inline T& GameModuleAPI::Ctx() {
            return registry_.ctx().get<T>();
        }

        template<typename T>
        inline const T& GameModuleAPI::Ctx() const {
            return registry_.ctx().get<T>();
        }

        template<typename T>
        inline bool GameModuleAPI::HasCtx() const {
            return registry_.ctx().contains<T>();
        }

    } // namespace core
} // namespace game
