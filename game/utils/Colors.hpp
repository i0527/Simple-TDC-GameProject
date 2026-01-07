#pragma once

#include <raylib.h>

// ============================================================================
// Raylib標準カラー定義
// ============================================================================

// 基本色
#ifndef WHITE
#define WHITE      (Color){ 255, 255, 255, 255 }   // White
#endif

#ifndef BLACK
#define BLACK      (Color){ 0, 0, 0, 255 }         // Black
#endif

#ifndef BLANK
#define BLANK      (Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#endif

#ifndef MAGENTA
#define MAGENTA    (Color){ 255, 0, 255, 255 }     // Magenta
#endif

#ifndef RAYWHITE
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
#endif

// グレースケール
#ifndef LIGHTGRAY
#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }   // Light Gray
#endif

#ifndef GRAY
#define GRAY       (Color){ 130, 130, 130, 255 }   // Gray
#endif

#ifndef DARKGRAY
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray
#endif

// 暖色系
#ifndef YELLOW
#define YELLOW     (Color){ 253, 249, 0, 255 }     // Yellow
#endif

#ifndef GOLD
#define GOLD       (Color){ 255, 203, 0, 255 }     // Gold
#endif

#ifndef ORANGE
#define ORANGE     (Color){ 255, 161, 0, 255 }     // Orange
#endif

#ifndef PINK
#define PINK       (Color){ 255, 109, 194, 255 }   // Pink
#endif

#ifndef RED
#define RED        (Color){ 230, 41, 55, 255 }     // Red
#endif

#ifndef MAROON
#define MAROON     (Color){ 190, 33, 55, 255 }     // Maroon
#endif

// 寒色系
#ifndef GREEN
#define GREEN      (Color){ 0, 228, 48, 255 }      // Green
#endif

#ifndef LIME
#define LIME       (Color){ 0, 158, 47, 255 }      // Lime
#endif

#ifndef DARKGREEN
#define DARKGREEN  (Color){ 0, 117, 44, 255 }      // Dark Green
#endif

#ifndef SKYBLUE
#define SKYBLUE    (Color){ 102, 191, 255, 255 }   // Sky Blue
#endif

#ifndef BLUE
#define BLUE       (Color){ 0, 121, 241, 255 }     // Blue
#endif

#ifndef DARKBLUE
#define DARKBLUE   (Color){ 0, 82, 172, 255 }      // Dark Blue
#endif

// 紫系
#ifndef PURPLE
#define PURPLE     (Color){ 200, 122, 255, 255 }   // Purple
#endif

#ifndef VIOLET
#define VIOLET     (Color){ 135, 60, 190, 255 }    // Violet
#endif

#ifndef DARKPURPLE
#define DARKPURPLE (Color){ 112, 31, 126, 255 }    // Dark Purple
#endif

// 茶系
#ifndef BEIGE
#define BEIGE      (Color){ 211, 176, 131, 255 }   // Beige
#endif

#ifndef BROWN
#define BROWN      (Color){ 127, 106, 79, 255 }    // Brown
#endif

#ifndef DARKBROWN
#define DARKBROWN  (Color){ 76, 63, 47, 255 }      // Dark Brown
#endif

// ============================================================================
// 追加カラー定義（Raylib標準と競合しないシンプルな名前）
// ============================================================================

// 基本色
#ifndef CYAN
#define CYAN       (Color){ 0, 255, 255, 255 }     // Cyan
#endif

#ifndef AQUA
#define AQUA       (Color){ 0, 255, 255, 255 }     // Aqua (same as Cyan)
#endif

#ifndef TEAL
#define TEAL       (Color){ 0, 128, 128, 255 }     // Teal
#endif

#ifndef NAVY
#define NAVY       (Color){ 0, 0, 128, 255 }       // Navy
#endif

#ifndef SILVER
#define SILVER     (Color){ 192, 192, 192, 255 }   // Silver
#endif

#ifndef OLIVE
#define OLIVE      (Color){ 128, 128, 0, 255 }     // Olive
#endif

// 暖色系
#ifndef CRIMSON
#define CRIMSON    (Color){ 220, 20, 60, 255 }      // Crimson
#endif

#ifndef CORAL
#define CORAL      (Color){ 255, 127, 80, 255 }    // Coral
#endif

#ifndef SALMON
#define SALMON     (Color){ 250, 128, 114, 255 }   // Salmon
#endif

#ifndef PEACH
#define PEACH      (Color){ 255, 218, 185, 255 }   // Peach
#endif

#ifndef APRICOT
#define APRICOT    (Color){ 255, 205, 178, 255 }   // Apricot
#endif

#ifndef TAN
#define TAN        (Color){ 210, 180, 140, 255 }   // Tan
#endif

#ifndef KHAKI
#define KHAKI      (Color){ 240, 230, 140, 255 }   // Khaki
#endif

// 寒色系
#ifndef TURQUOISE
#define TURQUOISE  (Color){ 64, 224, 208, 255 }    // Turquoise
#endif

#ifndef INDIGO
#define INDIGO     (Color){ 75, 0, 130, 255 }       // Indigo
#endif

#ifndef LAVENDER
#define LAVENDER   (Color){ 230, 230, 250, 255 }   // Lavender
#endif

#ifndef MINT
#define MINT       (Color){ 189, 252, 201, 255 }   // Mint
#endif

// 紫系
#ifndef ORCHID
#define ORCHID     (Color){ 218, 112, 214, 255 }   // Orchid
#endif

#ifndef PLUM
#define PLUM       (Color){ 221, 160, 221, 255 }   // Plum
#endif

#ifndef FUCHSIA
#define FUCHSIA    (Color){ 255, 0, 255, 255 }     // Fuchsia
#endif

// その他
#ifndef LEMON
#define LEMON      (Color){ 255, 250, 205, 255 }   // Lemon
#endif

#ifndef IVORY
#define IVORY      (Color){ 255, 255, 240, 255 }   // Ivory
#endif

#ifndef CREAM
#define CREAM      (Color){ 255, 253, 208, 255 }   // Cream
#endif
