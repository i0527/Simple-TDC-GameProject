#pragma once

// 標準ライブラリ
#include <cstdint>

// プロジェクト内
#include "RenderTypes.hpp"

namespace game {
namespace core {

// Raylib型の代替となる薄いプリミティブ型
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

inline constexpr Vec2 operator+(const Vec2& a, const Vec2& b) {
    return {a.x + b.x, a.y + b.y};
}

inline constexpr Vec2 operator-(const Vec2& a, const Vec2& b) {
    return {a.x - b.x, a.y - b.y};
}

inline constexpr Vec2 operator*(const Vec2& v, float s) {
    return {v.x * s, v.y * s};
}

inline constexpr Vec2 operator*(float s, const Vec2& v) {
    return {v.x * s, v.y * s};
}

inline constexpr bool operator==(const Vec2& a, const Vec2& b) {
    return a.x == b.x && a.y == b.y;
}

inline constexpr bool operator!=(const Vec2& a, const Vec2& b) {
    return !(a == b);
}

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

inline constexpr bool operator==(const Rect& a, const Rect& b) {
    return a.x == b.x && a.y == b.y && a.width == b.width &&
           a.height == b.height;
}

inline constexpr bool operator!=(const Rect& a, const Rect& b) {
    return !(a == b);
}

struct ColorRGBA {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

inline constexpr bool operator==(const ColorRGBA& a, const ColorRGBA& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

inline constexpr bool operator!=(const ColorRGBA& a, const ColorRGBA& b) {
    return !(a == b);
}

inline constexpr Vector2 ToRaylibVec2(const Vec2& v) {
    return {v.x, v.y};
}

inline constexpr Rectangle ToRaylibRect(const Rect& r) {
    return {r.x, r.y, r.width, r.height};
}

inline constexpr Color ToRaylibColor(const ColorRGBA& c) {
    return {c.r, c.g, c.b, c.a};
}

inline constexpr Vec2 ToCoreVec2(const Vector2& v) {
    return {v.x, v.y};
}

inline constexpr Rect ToCoreRect(const Rectangle& r) {
    return {r.x, r.y, r.width, r.height};
}

inline constexpr ColorRGBA ToCoreColor(const Color& c) {
    return {c.r, c.g, c.b, c.a};
}

} // namespace core
} // namespace game
