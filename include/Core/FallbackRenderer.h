/**
 * @file FallbackRenderer.h
 * @brief フォールバック描画ユーティリティ
 * 
 * テクスチャや画像が存在しない場合に、プレースホルダーとして
 * 代替の図形を描画する共通ユーティリティ。
 */
#pragma once

#include "Core/Platform.h"
#include <string>
#include <cmath>
#include <unordered_map>

namespace Core {

/**
 * @brief フォールバック描画形状
 */
enum class FallbackShape {
    Circle,         // 円
    Rectangle,      // 矩形
    Diamond,        // ひし形
    Triangle,       // 三角形
    RoundedRect,    // 角丸矩形
    Hexagon,        // 六角形
};

/**
 * @brief フォールバック描画オプション
 */
struct FallbackDrawOptions {
    FallbackShape shape = FallbackShape::Rectangle;
    Color primaryColor = LIGHTGRAY;
    Color secondaryColor = DARKGRAY;    // アウトライン
    Color textColor = WHITE;
    float borderWidth = 2.0f;
    bool showLabel = true;              // ラベル表示
    bool showMissingIcon = true;        // 「?」マーク表示
    bool animated = false;              // 点滅アニメーション
    int frameIndex = -1;                // フレーム番号表示（-1で非表示）
};

/**
 * @brief フォールバック描画ユーティリティ
 */
class FallbackRenderer {
public:
    /**
     * @brief 矩形エリアにフォールバック描画
     */
    static void DrawFallback(
        Rectangle bounds,
        const std::string& label = "",
        const FallbackDrawOptions& options = {}
    ) {
        // アニメーション（点滅）
        float alpha = 1.0f;
        if (options.animated) {
            alpha = 0.7f + 0.3f * sinf(static_cast<float>(GetTime()) * 4.0f);
        }
        
        Color primary = options.primaryColor;
        primary.a = static_cast<unsigned char>(primary.a * alpha);
        
        Color secondary = options.secondaryColor;
        
        float centerX = bounds.x + bounds.width / 2;
        float centerY = bounds.y + bounds.height / 2;
        float halfW = bounds.width / 2;
        float halfH = bounds.height / 2;
        
        // 形状描画
        switch (options.shape) {
            case FallbackShape::Circle: {
                float radius = std::min(halfW, halfH);
                DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, primary);
                DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius, secondary);
                break;
            }
            
            case FallbackShape::Rectangle:
                DrawRectangleRec(bounds, primary);
                DrawRectangleLinesEx(bounds, options.borderWidth, secondary);
                break;
                
            case FallbackShape::RoundedRect: {
                float roundness = 0.2f;
                DrawRectangleRounded(bounds, roundness, 8, primary);
                DrawRectangleRoundedLinesEx(bounds, roundness, 8, options.borderWidth, secondary);
                break;
            }
            
            case FallbackShape::Diamond: {
                Vector2 points[4] = {
                    { centerX, bounds.y },
                    { bounds.x + bounds.width, centerY },
                    { centerX, bounds.y + bounds.height },
                    { bounds.x, centerY }
                };
                DrawTriangle(points[0], points[1], points[2], primary);
                DrawTriangle(points[0], points[2], points[3], primary);
                for (int i = 0; i < 4; i++) {
                    int next = (i + 1) % 4;
                    DrawLineEx(points[i], points[next], options.borderWidth, secondary);
                }
                break;
            }
            
            case FallbackShape::Triangle: {
                Vector2 v1 = { centerX, bounds.y };
                Vector2 v2 = { bounds.x, bounds.y + bounds.height };
                Vector2 v3 = { bounds.x + bounds.width, bounds.y + bounds.height };
                DrawTriangle(v1, v3, v2, primary);
                DrawLineEx(v1, v2, options.borderWidth, secondary);
                DrawLineEx(v2, v3, options.borderWidth, secondary);
                DrawLineEx(v3, v1, options.borderWidth, secondary);
                break;
            }
            
            case FallbackShape::Hexagon: {
                Vector2 points[6];
                for (int i = 0; i < 6; i++) {
                    float angle = (i * 60.0f - 30.0f) * DEG2RAD;
                    float radius = std::min(halfW, halfH);
                    points[i] = {
                        centerX + cosf(angle) * radius,
                        centerY + sinf(angle) * radius
                    };
                }
                // 中心からの三角形で塗りつぶし
                for (int i = 0; i < 6; i++) {
                    int next = (i + 1) % 6;
                    DrawTriangle({centerX, centerY}, points[next], points[i], primary);
                }
                // 輪郭
                for (int i = 0; i < 6; i++) {
                    int next = (i + 1) % 6;
                    DrawLineEx(points[i], points[next], options.borderWidth, secondary);
                }
                break;
            }
        }
        
        // 「?」マーク（画像不明の場合）
        if (options.showMissingIcon) {
            int fontSize = static_cast<int>(std::min(bounds.width, bounds.height) * 0.4f);
            fontSize = std::max(12, std::min(fontSize, 48));
            
            int textWidth = MeasureText("?", fontSize);
            DrawText("?", 
                     static_cast<int>(centerX - textWidth / 2),
                     static_cast<int>(centerY - fontSize / 2),
                     fontSize, options.textColor);
        }
        
        // ラベル表示
        if (options.showLabel && !label.empty()) {
            int fontSize = 10;
            int textWidth = MeasureText(label.c_str(), fontSize);
            
            // 下部に表示
            float labelY = bounds.y + bounds.height + 2;
            if (labelY + fontSize > bounds.y + bounds.height + 20) {
                // 内部に表示
                labelY = bounds.y + bounds.height - fontSize - 2;
            }
            
            DrawText(label.c_str(),
                     static_cast<int>(centerX - textWidth / 2),
                     static_cast<int>(labelY),
                     fontSize, options.secondaryColor);
        }
        
        // フレーム番号表示
        if (options.frameIndex >= 0) {
            char frameText[8];
            snprintf(frameText, sizeof(frameText), "[%d]", options.frameIndex);
            int fontSize = 10;
            int textWidth = MeasureText(frameText, fontSize);
            
            DrawText(frameText,
                     static_cast<int>(bounds.x + bounds.width - textWidth - 2),
                     static_cast<int>(bounds.y + 2),
                     fontSize, options.secondaryColor);
        }
    }
    
    /**
     * @brief スプライトシート用フォールバック描画
     * 
     * グリッド状に番号付きの矩形を描画
     */
    static void DrawFallbackSpriteSheet(
        float x, float y,
        float width, float height,
        int columns, int rows,
        int highlightFrame = -1,
        Color bgColor = LIGHTGRAY,
        Color gridColor = DARKGRAY,
        Color highlightColor = YELLOW
    ) {
        float cellW = width / columns;
        float cellH = height / rows;
        
        // 背景
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                      static_cast<int>(width), static_cast<int>(height), bgColor);
        
        // グリッドとフレーム番号
        int frame = 0;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < columns; col++) {
                float cellX = x + col * cellW;
                float cellY = y + row * cellH;
                
                // セル境界
                Color lineColor = (frame == highlightFrame) ? highlightColor : gridColor;
                float lineWidth = (frame == highlightFrame) ? 2.0f : 1.0f;
                DrawRectangleLinesEx({cellX, cellY, cellW, cellH}, lineWidth, lineColor);
                
                // フレーム番号
                char numStr[8];
                snprintf(numStr, sizeof(numStr), "%d", frame);
                int fontSize = static_cast<int>(std::min(cellW, cellH) * 0.3f);
                fontSize = std::max(8, std::min(fontSize, 16));
                
                int textW = MeasureText(numStr, fontSize);
                DrawText(numStr,
                         static_cast<int>(cellX + cellW / 2 - textW / 2),
                         static_cast<int>(cellY + cellH / 2 - fontSize / 2),
                         fontSize, gridColor);
                
                frame++;
            }
        }
    }
    
    /**
     * @brief アニメーション用フォールバック描画
     * 
     * 現在のフレームを視覚化
     */
    static void DrawFallbackAnimation(
        float x, float y,
        float size,
        int currentFrame,
        int totalFrames,
        const std::string& animName = "",
        Color baseColor = BLUE,
        bool facingRight = true
    ) {
        // フレームに応じて色を変化
        float hue = (currentFrame % 8) * 45.0f;
        Color frameColor = ColorFromHSV(hue, 0.6f, 0.9f);
        
        // 基本形状（方向を示す矢印付き）
        float halfSize = size / 2;
        
        // 本体
        DrawCircle(static_cast<int>(x), static_cast<int>(y), halfSize, baseColor);
        DrawCircleLines(static_cast<int>(x), static_cast<int>(y), halfSize, DARKGRAY);
        
        // 方向矢印
        float arrowX = facingRight ? halfSize * 0.5f : -halfSize * 0.5f;
        DrawTriangle(
            { x + arrowX, y },
            { x + arrowX * 0.3f, y - halfSize * 0.3f },
            { x + arrowX * 0.3f, y + halfSize * 0.3f },
            frameColor
        );
        
        // フレーム番号
        char frameStr[16];
        snprintf(frameStr, sizeof(frameStr), "%d/%d", currentFrame, totalFrames);
        int fontSize = 12;
        int textW = MeasureText(frameStr, fontSize);
        DrawText(frameStr,
                 static_cast<int>(x - textW / 2),
                 static_cast<int>(y - 6),
                 fontSize, WHITE);
        
        // アニメーション名
        if (!animName.empty()) {
            int nameWidth = MeasureText(animName.c_str(), 10);
            DrawText(animName.c_str(),
                     static_cast<int>(x - nameWidth / 2),
                     static_cast<int>(y + halfSize + 4),
                     10, DARKGRAY);
        }
        
        // 再生インジケータ（回転する点）
        float angle = currentFrame * (360.0f / totalFrames) * DEG2RAD;
        float indicatorX = x + cosf(angle) * halfSize * 0.7f;
        float indicatorY = y + sinf(angle) * halfSize * 0.7f;
        DrawCircle(static_cast<int>(indicatorX), static_cast<int>(indicatorY), 4, frameColor);
    }
    
    /**
     * @brief UI画像用フォールバック描画
     */
    static void DrawFallbackUIImage(
        Rectangle bounds,
        const std::string& imagePath = "",
        Color bgColor = Color{80, 80, 80, 255}
    ) {
        // 背景（チェッカーパターン）
        int checkSize = 8;
        for (int cy = 0; cy < static_cast<int>(bounds.height); cy += checkSize) {
            for (int cx = 0; cx < static_cast<int>(bounds.width); cx += checkSize) {
                bool isDark = ((cx / checkSize) + (cy / checkSize)) % 2 == 0;
                Color checkColor = isDark ? bgColor : Color{
                    static_cast<unsigned char>(bgColor.r + 20),
                    static_cast<unsigned char>(bgColor.g + 20),
                    static_cast<unsigned char>(bgColor.b + 20),
                    bgColor.a
                };
                
                int drawW = std::min(checkSize, static_cast<int>(bounds.width - cx));
                int drawH = std::min(checkSize, static_cast<int>(bounds.height - cy));
                DrawRectangle(
                    static_cast<int>(bounds.x + cx),
                    static_cast<int>(bounds.y + cy),
                    drawW, drawH, checkColor
                );
            }
        }
        
        // 枠線
        DrawRectangleLinesEx(bounds, 1.0f, GRAY);
        
        // 画像アイコン
        float iconSize = std::min(bounds.width, bounds.height) * 0.4f;
        iconSize = std::min(iconSize, 32.0f);
        float iconX = bounds.x + bounds.width / 2;
        float iconY = bounds.y + bounds.height / 2;
        
        // 山のアイコン（画像を示す）
        DrawTriangle(
            { iconX - iconSize * 0.4f, iconY + iconSize * 0.3f },
            { iconX, iconY - iconSize * 0.2f },
            { iconX + iconSize * 0.4f, iconY + iconSize * 0.3f },
            GRAY
        );
        
        // 太陽（右上の円）
        DrawCircle(
            static_cast<int>(iconX + iconSize * 0.25f),
            static_cast<int>(iconY - iconSize * 0.15f),
            iconSize * 0.12f, GRAY
        );
        
        // ファイルパス表示（短縮）
        if (!imagePath.empty()) {
            std::string shortPath = imagePath;
            if (shortPath.length() > 20) {
                shortPath = "..." + shortPath.substr(shortPath.length() - 17);
            }
            
            int fontSize = 8;
            int textW = MeasureText(shortPath.c_str(), fontSize);
            if (textW < bounds.width - 4) {
                DrawText(shortPath.c_str(),
                         static_cast<int>(bounds.x + (bounds.width - textW) / 2),
                         static_cast<int>(bounds.y + bounds.height - fontSize - 2),
                         fontSize, DARKGRAY);
            }
        }
    }
    
    /**
     * @brief パーティクル用フォールバック
     */
    static void DrawFallbackParticle(
        float x, float y,
        float size,
        float alpha = 1.0f,
        Color color = WHITE
    ) {
        Color drawColor = color;
        drawColor.a = static_cast<unsigned char>(255 * alpha);
        
        // 簡易パーティクル（グラデーション円）
        int steps = 4;
        for (int i = steps; i >= 1; i--) {
            float radius = size * (static_cast<float>(i) / steps);
            float a = alpha * (1.0f - static_cast<float>(i - 1) / steps);
            Color stepColor = drawColor;
            stepColor.a = static_cast<unsigned char>(255 * a * 0.5f);
            DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, stepColor);
        }
    }
};

} // namespace Core
