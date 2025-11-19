#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Core {
    // 設定管理システムの例外クラス
    class ConfigException : public std::exception {
    public:
        explicit ConfigException(const std::string& message) : message_(message) {}
        
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
    private:
        std::string message_;
    };

    // 設定管理クラス（Singleton）
    class ConfigManager {
    public:
        static ConfigManager& GetInstance();
        
        // インスタンスのコピーを禁止
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;
        
        // 設定ファイルを読み込み
        void LoadConfig(const std::string& filePath);
        
        // 整数値を取得（デフォルト値指定可能）
        int GetInt(const std::string& key, int defaultValue = 0);
        
        // 浮動小数点値を取得（デフォルト値指定可能）
        float GetFloat(const std::string& key, float defaultValue = 0.0f);
        
        // 文字列値を取得（デフォルト値指定可能）
        std::string GetString(const std::string& key, const std::string& defaultValue = "");
        
        // ネストされたキーをドット記法で取得（例: "window.width"）
        json GetValue(const std::string& key);
        
        // 値が存在するか確認
        bool HasKey(const std::string& key);
        
        // 設定全体を取得
        const json& GetAllConfig() const;
        
    private:
        ConfigManager() = default;
        ~ConfigManager() = default;
        
        // ドット記法のキーをネストされたJSONで探索
        json FindValueByDottedKey(const std::string& key);
        
        json config_;
    };
}
