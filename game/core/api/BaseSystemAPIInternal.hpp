#pragma once

// 標準ライブラリ
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace game {
    namespace core {
        namespace detail {
            inline std::string NormalizeSlashes(std::string value) {
                std::replace(value.begin(), value.end(), '\\', '/');
                return value;
            }

            inline bool StartsWith(const std::string& value, const std::string& prefix) {
                return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
            }

            inline std::string ToLower(std::string value) {
                std::transform(value.begin(), value.end(), value.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return value;
            }

            // テクスチャキーの正規化:
            // - '\' -> '/'
            // - "data/" プレフィックスは除去
            // - "assets/" 系は拡張子が無ければ .png を補う
            inline std::string NormalizeTextureKey(std::string key) {
                key = NormalizeSlashes(std::move(key));

                // 余計な "./" を除去
                if (StartsWith(key, "./")) {
                    key = key.substr(2);
                }

                // data/ プレフィックスを除去（呼び出し側が data/assets/... を渡しても許容）
                if (StartsWith(key, "data/")) {
                    key = key.substr(5);
                }

                // assets/ 配下の拡張子補完（"assets/..../move" のような入力を許容）
                if (StartsWith(key, "assets/")) {
                    const std::filesystem::path path(key);
                    if (!path.has_extension()) {
                        key += ".png";
                    }
                }

                return key;
            }

            // data/ を基準にした相対パス（例: data/assets/characters/A/move.png -> assets/characters/A/move.png）
            inline std::string MakeAssetsRelativeKey(const std::filesystem::path& fullPath) {
                try {
                    std::filesystem::path rel = std::filesystem::relative(fullPath, std::filesystem::path("data"));
                    return rel.generic_string();
                } catch (...) {
                    // relative が失敗した場合はフォールバック（スラッシュ正規化のみ）
                    return NormalizeSlashes(fullPath.string());
                }
            }
        } // namespace detail
    } // namespace core
} // namespace game
