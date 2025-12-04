/**
 * @file DataLoaderBase.h
 * @brief データローダー基底クラス
 * 
 * 全ローダーの共通機能を提供
 */
#pragma once

#include "Data/Registry.h"
#include "Core/FileUtils.h"
#include <nlohmann/json.hpp>
#include <string>
#include <filesystem>
#include <functional>
#include <iostream>

namespace Data {

using json = nlohmann::json;

/**
 * @brief データローダー基底クラス
 */
class DataLoaderBase {
public:
    using ErrorHandler = std::function<void(const std::string& path, const std::string& error)>;
    
    explicit DataLoaderBase(DefinitionRegistry& registry)
        : registry_(registry) {
        // デフォルトエラーハンドラ
        errorHandler_ = [](const std::string& path, const std::string& error) {
            std::cerr << "DataLoader Error [" << path << "]: " << error << "\n";
        };
    }
    
    /**
     * @brief エラーハンドラを設定
     */
    void SetErrorHandler(ErrorHandler handler) {
        errorHandler_ = std::move(handler);
    }
    
protected:
    DefinitionRegistry& registry_;
    ErrorHandler errorHandler_;
    
    /**
     * @brief JSONファイルを読み込む
     */
    json LoadJsonFile(const std::string& filePath) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                errorHandler_(filePath, "Failed to open file");
                return json();
            }
            
            json j;
            file >> j;
            return j;
        } catch (const json::parse_error& e) {
            errorHandler_(filePath, "JSON parse error: " + std::string(e.what()));
            return json();
        } catch (const std::exception& e) {
            errorHandler_(filePath, "Error: " + std::string(e.what()));
            return json();
        }
    }
    
    /**
     * @brief ファイル名から拡張子を除いた名前を取得
     */
    std::string GetFileNameWithoutExtension(const std::string& filePath) {
        namespace fs = std::filesystem;
        return fs::path(filePath).stem().string();
    }
    
    /**
     * @brief ディレクトリ内のファイルを処理
     */
    int LoadDirectory(const std::string& directoryPath, const std::string& extension,
                     std::function<bool(const std::string&)> loader) {
        namespace fs = std::filesystem;
        int count = 0;
        
        if (!fs::exists(directoryPath)) {
            errorHandler_(directoryPath, "Directory does not exist");
            return 0;
        }
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (!entry.is_regular_file()) continue;
            
            std::string path = entry.path().string();
            if (path.length() >= extension.length() && 
                path.substr(path.length() - extension.length()) == extension) {
                if (loader(path)) {
                    count++;
                }
            }
        }
        
        return count;
    }
    
    /**
     * @brief 文字列が指定の拡張子で終わるか確認
     */
    bool StringEndsWith(const std::string& str, const std::string& suffix) {
        if (suffix.size() > str.size()) return false;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
};

} // namespace Data

