/**
 * @file Platform.h
 * @brief プラットフォーム互換性ヘッダー
 * 
 * Windows APIとRaylibの間の名前衝突を解決するためのヘッダー。
 * このファイルはRaylibを使用するすべてのソースファイルの先頭で
 * インクルードする必要があります。
 * 
 * 使用方法:
 *   #include "Core/Platform.h"  // raylib.hの代わりに使用
 *   // 以降はRaylibの関数を通常通り使用可能
 */
#pragma once

// ===== Windows対策（Raylibインクルード前に設定）=====

// Windowsのmin/maxマクロを無効化
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Windows.hの不要な機能を除外
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Windows.hのGDI関連を除外
#ifndef NOGDI
#define NOGDI
#endif

// Windows.hをインクルードしてしまった場合の対策
#ifdef _WINDOWS_
    // すでにWindows.hがインクルードされている場合
    #ifdef DrawText
    #undef DrawText
    #endif
    #ifdef DrawTextA
    #undef DrawTextA
    #endif
    #ifdef DrawTextW
    #undef DrawTextW
    #endif
    #ifdef DrawTextEx
    #undef DrawTextEx
    #endif
    #ifdef DrawTextExA
    #undef DrawTextExA
    #endif
    #ifdef DrawTextExW
    #undef DrawTextExW
    #endif
    #ifdef Rectangle
    #undef Rectangle
    #endif
    #ifdef CloseWindow
    #undef CloseWindow
    #endif
    #ifdef ShowCursor
    #undef ShowCursor
    #endif
#endif

// ===== Raylib =====
#include <raylib.h>

// Raylib インクルード後に再度マクロを解除（安全のため）
#ifdef DrawText
#undef DrawText
#endif

#ifdef DrawTextEx
#undef DrawTextEx
#endif

#ifdef Rectangle
#undef Rectangle
#endif

#ifdef CloseWindow
#undef CloseWindow
#endif

#ifdef ShowCursor
#undef ShowCursor
#endif

// ===== 安全なRaylibラッパー関数 =====
namespace Core {

/**
 * @brief テキストを描画（Raylibラッパー）
 * Windows APIとの競合を避けるために明示的なラッパーを提供
 */
inline void RDrawText(const char* text, int posX, int posY, int fontSize, ::Color color) {
    ::DrawText(text, posX, posY, fontSize, color);
}

/**
 * @brief テキスト幅を計測
 */
inline int RMeasureText(const char* text, int fontSize) {
    return ::MeasureText(text, fontSize);
}

/**
 * @brief テキストを描画（拡張版）
 */
inline void RDrawTextEx(::Font font, const char* text, ::Vector2 position, float fontSize, float spacing, ::Color tint) {
    ::DrawTextEx(font, text, position, fontSize, spacing, tint);
}

/**
 * @brief 矩形を描画（塗りつぶし）
 */
inline void RDrawRectangle(int posX, int posY, int width, int height, ::Color color) {
    ::DrawRectangle(posX, posY, width, height, color);
}

/**
 * @brief 矩形を描画（構造体版）
 */
inline void RDrawRectangleRec(::Rectangle rec, ::Color color) {
    ::DrawRectangleRec(rec, color);
}

/**
 * @brief ウィンドウを閉じる
 */
inline void RCloseWindow() {
    ::CloseWindow();
}

/**
 * @brief カーソルを表示
 */
inline void RShowCursor() {
    ::ShowCursor();
}

} // namespace Core
