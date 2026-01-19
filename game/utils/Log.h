#pragma once

#include <string>

#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
#include <spdlog/spdlog.h>
#endif

// ログマクロ（spdlogベース）
// BaseSystemAPIによって初期化されたspdlogを使用します

#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)

#define LOG_TRACE(...) \
    spdlog::trace(__VA_ARGS__)

#define LOG_DEBUG(...) \
    spdlog::debug(__VA_ARGS__)

#define LOG_INFO(...) \
    spdlog::info(__VA_ARGS__)

#define LOG_WARN(...) \
    spdlog::warn(__VA_ARGS__)

#define LOG_ERROR(...) \
    spdlog::error(__VA_ARGS__)

#define LOG_CRITICAL(...) \
    spdlog::critical(__VA_ARGS__)

namespace game {
    namespace utils {
        /// @brief ログユーティリティクラス（非推奨）
        /// 
        /// 後方互換性のため残されていますが、使用は推奨されません。
        /// ログシステムはBaseSystemAPIによって自動的に初期化・終了されます。
        /// 
        /// @deprecated BaseSystemAPIがログシステムを管理するため、このクラスは不要です。
        class Log {
        public:
            /// @brief ログシステムの初期化（非推奨）
            /// 
            /// このメソッドは何もしません。
            /// ログシステムはBaseSystemAPI::Initialize()によって自動的に初期化されます。
            /// 
            /// @deprecated BaseSystemAPIが自動的に初期化するため、このメソッドは不要です。
            static void Initialize() {
                // 何もしない - BaseSystemAPIが自動的に初期化
            }

            /// @brief ログシステムのシャットダウン（非推奨）
            /// 
            /// このメソッドは何もしません。
            /// ログシステムはBaseSystemAPI::Shutdown()によって自動的に終了されます。
            /// 
            /// @deprecated BaseSystemAPIが自動的に終了するため、このメソッドは不要です。
            static void Shutdown() {
                // 何もしない - BaseSystemAPIが自動的に終了
            }

            /// @brief ログレベルを設定（非推奨）
            /// 
            /// BaseSystemAPI::SetLogLevel()を使用してください。
            /// 
            /// @param level ログレベル文字列（"trace", "debug", "info", "warn", "error", "critical", "off"）
            /// @deprecated BaseSystemAPI::SetLogLevel()を使用してください。
            static void SetLevel(const std::string& level) {
                spdlog::level::level_enum logLevel = spdlog::level::info;
                
                if (level == "trace") {
                    logLevel = spdlog::level::trace;
                } else if (level == "debug") {
                    logLevel = spdlog::level::debug;
                } else if (level == "info") {
                    logLevel = spdlog::level::info;
                } else if (level == "warn") {
                    logLevel = spdlog::level::warn;
                } else if (level == "error") {
                    logLevel = spdlog::level::err;
                } else if (level == "critical") {
                    logLevel = spdlog::level::critical;
                } else if (level == "off") {
                    logLevel = spdlog::level::off;
                }
                
                // spdlogのグローバルレベルを設定（BaseSystemAPIのロガーも影響を受ける）
                spdlog::set_level(logLevel);
            }
        };
    }
}

#else

#define LOG_TRACE(...) ((void)0)
#define LOG_DEBUG(...) ((void)0)
#define LOG_INFO(...) ((void)0)
#define LOG_WARN(...) ((void)0)
#define LOG_ERROR(...) ((void)0)
#define LOG_CRITICAL(...) ((void)0)

namespace game {
    namespace utils {
        /// @brief ログユーティリティクラス（Webビルド用・無効化）
        class Log {
        public:
            static void Initialize() {}
            static void Shutdown() {}
            static void SetLevel(const std::string&) {}
        };
    }
}

#endif
