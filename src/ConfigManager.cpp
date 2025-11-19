#include "ConfigManager.h"
#include <iostream>
#include <fstream>

namespace Core {
    ConfigManager& ConfigManager::GetInstance() {
        static ConfigManager instance;
        return instance;
    }
    
    void ConfigManager::LoadConfig(const std::string& filePath) {
        try {
            std::ifstream file(filePath);
            
            // ファイルが開けたか確認
            if (!file.is_open()) {
                throw ConfigException("Failed to open config file: " + filePath);
            }
            
            // JSONを解析
            config_ = json::parse(file);
            std::cout << "Config loaded successfully: " << filePath << std::endl;
            
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            throw ConfigException("Failed to parse JSON: " + std::string(e.what()));
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw ConfigException("Config loading error: " + std::string(e.what()));
        }
    }
    
    int ConfigManager::GetInt(const std::string& key, int defaultValue) {
        try {
            json value = FindValueByDottedKey(key);
            if (value.is_null()) {
                return defaultValue;
            }
            return value.get<int>();
        } catch (const std::exception& e) {
            std::cerr << "Error getting int value: " << e.what() << std::endl;
            return defaultValue;
        }
    }
    
    float ConfigManager::GetFloat(const std::string& key, float defaultValue) {
        try {
            json value = FindValueByDottedKey(key);
            if (value.is_null()) {
                return defaultValue;
            }
            return value.get<float>();
        } catch (const std::exception& e) {
            std::cerr << "Error getting float value: " << e.what() << std::endl;
            return defaultValue;
        }
    }
    
    std::string ConfigManager::GetString(const std::string& key, const std::string& defaultValue) {
        try {
            json value = FindValueByDottedKey(key);
            if (value.is_null()) {
                return defaultValue;
            }
            return value.get<std::string>();
        } catch (const std::exception& e) {
            std::cerr << "Error getting string value: " << e.what() << std::endl;
            return defaultValue;
        }
    }
    
    json ConfigManager::GetValue(const std::string& key) {
        try {
            return FindValueByDottedKey(key);
        } catch (const std::exception& e) {
            std::cerr << "Error getting value: " << e.what() << std::endl;
            return json();
        }
    }
    
    bool ConfigManager::HasKey(const std::string& key) {
        try {
            json value = FindValueByDottedKey(key);
            return !value.is_null();
        } catch (...) {
            return false;
        }
    }
    
    const json& ConfigManager::GetAllConfig() const {
        return config_;
    }
    
    json ConfigManager::FindValueByDottedKey(const std::string& key) {
        // ドット記法でキーを分割（例: "window.width" → ["window", "width"]）
        std::vector<std::string> keys;
        size_t start = 0;
        size_t end = key.find('.');
        
        while (end != std::string::npos) {
            keys.push_back(key.substr(start, end - start));
            start = end + 1;
            end = key.find('.', start);
        }
        keys.push_back(key.substr(start));
        
        // JSONをネストされたキーで探索
        json current = config_;
        for (const auto& k : keys) {
            if (current.contains(k)) {
                current = current[k];
            } else {
                return json();  // nullを返す
            }
        }
        
        return current;
    }
}
