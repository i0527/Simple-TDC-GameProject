/**
 * @file FileUtils.h
 * @brief UTF-8対応ファイルユーティリティ
 * 
 * 日本語を含むUTF-8ファイルの読み書きをサポート
 * Windows API使用部分はFileUtils.cppに実装
 */

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <filesystem>
#include <iostream>
#include <cstdint>

namespace Core {

/**
 * @class FileUtils
 * @brief UTF-8ファイル操作ユーティリティ
 */
class FileUtils {
public:
    /**
     * @brief UTF-8ファイルを読み込む
     * @param path ファイルパス
     * @return ファイル内容（失敗時はnullopt）
     */
    static std::optional<std::string> ReadUtf8File(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return std::nullopt;
        }
        
        // ファイルサイズ取得
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (size <= 0) {
            return std::string();
        }
        
        // 内容読み込み
        std::string content(static_cast<size_t>(size), '\0');
        if (!file.read(&content[0], size)) {
            std::cerr << "Failed to read file: " << path << std::endl;
            return std::nullopt;
        }
        
        // BOMスキップ（UTF-8 BOM: EF BB BF）
        if (content.size() >= 3 &&
            static_cast<uint8_t>(content[0]) == 0xEF &&
            static_cast<uint8_t>(content[1]) == 0xBB &&
            static_cast<uint8_t>(content[2]) == 0xBF) {
            content = content.substr(3);
        }
        
        return content;
    }
    
    /**
     * @brief UTF-8ファイルに書き込む（BOMなし）
     * @param path ファイルパス
     * @param content 内容
     * @param withBom BOMを付けるか（デフォルト: false）
     * @return 成功時true
     */
    static bool WriteUtf8File(const std::string& path, const std::string& content, bool withBom = false) {
        // ディレクトリが存在しない場合は作成
        std::filesystem::path filePath(path);
        if (filePath.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(filePath.parent_path(), ec);
            if (ec) {
                std::cerr << "Failed to create directory: " << ec.message() << std::endl;
                return false;
            }
        }
        
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }
        
        // BOM書き込み（オプション）
        if (withBom) {
            file.put(static_cast<char>(0xEF));
            file.put(static_cast<char>(0xBB));
            file.put(static_cast<char>(0xBF));
        }
        
        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        return file.good();
    }
    
    /**
     * @brief 行単位でUTF-8ファイルを読み込む
     * @param path ファイルパス
     * @return 行の配列（失敗時はnullopt）
     */
    static std::optional<std::vector<std::string>> ReadUtf8Lines(const std::string& path) {
        auto content = ReadUtf8File(path);
        if (!content) {
            return std::nullopt;
        }
        
        std::vector<std::string> lines;
        std::string line;
        
        for (char c : *content) {
            if (c == '\n') {
                // CRを削除
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                lines.push_back(std::move(line));
                line.clear();
            } else {
                line += c;
            }
        }
        
        // 最後の行（改行なし）
        if (!line.empty()) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(std::move(line));
        }
        
        return lines;
    }
    
    /**
     * @brief ファイルが存在するか確認
     * @param path ファイルパス
     * @return 存在する場合true
     */
    static bool FileExists(const std::string& path) {
        std::error_code ec;
        return std::filesystem::exists(path, ec);
    }
    
    /**
     * @brief ディレクトリが存在するか確認
     * @param path ディレクトリパス
     * @return 存在する場合true
     */
    static bool DirectoryExists(const std::string& path) {
        std::error_code ec;
        return std::filesystem::is_directory(path, ec);
    }
    
    /**
     * @brief ファイル拡張子を取得
     * @param path ファイルパス
     * @return 拡張子（ドットなし、小文字）
     */
    static std::string GetExtension(const std::string& path) {
        std::filesystem::path p(path);
        std::string ext = p.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        // 小文字に変換
        for (char& c : ext) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c + ('a' - 'A'));
            }
        }
        return ext;
    }
    
    /**
     * @brief ファイル名（拡張子なし）を取得
     * @param path ファイルパス
     * @return ファイル名
     */
    static std::string GetFileNameWithoutExtension(const std::string& path) {
        std::filesystem::path p(path);
        return p.stem().string();
    }
    
    /**
     * @brief ディレクトリパスを取得
     * @param path ファイルパス
     * @return ディレクトリパス
     */
    static std::string GetDirectory(const std::string& path) {
        std::filesystem::path p(path);
        return p.parent_path().string();
    }
    
    /**
     * @brief パスを結合
     * @param base ベースパス
     * @param relative 相対パス
     * @return 結合されたパス
     */
    static std::string JoinPath(const std::string& base, const std::string& relative) {
        std::filesystem::path result = std::filesystem::path(base) / relative;
        return result.string();
    }
    
    /**
     * @brief パスを正規化（スラッシュを統一）
     * @param path パス
     * @return 正規化されたパス
     */
    static std::string NormalizePath(const std::string& path) {
        std::string result = path;
        // バックスラッシュをスラッシュに統一
        for (char& c : result) {
            if (c == '\\') {
                c = '/';
            }
        }
        return result;
    }

#ifdef _WIN32
    /**
     * @brief UTF-8文字列をワイド文字列に変換（Windows用）
     * @param utf8 UTF-8文字列
     * @return ワイド文字列
     * @note 実装はFileUtils.cppにあります
     */
    static std::wstring Utf8ToWide(const std::string& utf8);
    
    /**
     * @brief ワイド文字列をUTF-8文字列に変換（Windows用）
     * @param wide ワイド文字列
     * @return UTF-8文字列
     * @note 実装はFileUtils.cppにあります
     */
    static std::string WideToUtf8(const std::wstring& wide);
#endif
};

} // namespace Core
