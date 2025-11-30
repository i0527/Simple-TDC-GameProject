/**
 * @file FileUtils.cpp
 * @brief UTF-8対応ファイルユーティリティ（Windows実装）
 * 
 * Windows APIを使用するため、raylib.hとの競合を避けるために
 * 実装ファイルに分離
 */

#ifdef _WIN32
// Windows.hの問題を回避するためのマクロ定義
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#endif

#include "Core/FileUtils.h"

#ifdef _WIN32

namespace Core {

std::wstring FileUtils::Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
    
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), 
                                   static_cast<int>(utf8.size()), nullptr, 0);
    if (size <= 0) {
        return std::wstring();
    }
    
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), 
                        static_cast<int>(utf8.size()), &wide[0], size);
    return wide;
}

std::string FileUtils::WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return std::string();
    }
    
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), 
                                   static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return std::string();
    }
    
    std::string utf8(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), 
                        static_cast<int>(wide.size()), &utf8[0], size, nullptr, nullptr);
    return utf8;
}

} // namespace Core

#endif // _WIN32
