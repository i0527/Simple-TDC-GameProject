/**
 * @file TileRenderer.h
 * @brief タイルレンダリングシステム
 * 
 * NetHack風の文字ベースタイルを正方形テクスチャとして描画。
 * RenderTextureに背景色+文字を描画してキャッシュし、
 * 後から画像テクスチャへの差し替えを可能にする設計。
 */
#pragma once

#include <raylib.h>
#include <unordered_map>
#include <string>
#include "../Components/GridComponents.h"

namespace Roguelike::Rendering {

/**
 * @brief タイルの見た目を定義
 */
struct TileAppearance {
    char symbol = ' ';       // 表示文字
    Color foreground = WHITE; // 文字色
    Color background = BLACK; // 背景色
    
    TileAppearance() = default;
    TileAppearance(char sym, Color fg, Color bg)
        : symbol(sym), foreground(fg), background(bg) {}
};

/**
 * @brief タイルタイプから見た目への変換テーブル
 */
inline TileAppearance GetTileAppearance(Components::TileType type, bool visible, bool explored) {
    // 見えていない＆未探索 → 真っ黒
    if (!visible && !explored) {
        return TileAppearance(' ', BLACK, BLACK);
    }
    
    // 基本の見た目を決定
    TileAppearance appearance;
    switch (type) {
        case Components::TileType::Void:
            appearance = TileAppearance(' ', BLACK, BLACK);
            break;
        case Components::TileType::Floor:
            appearance = TileAppearance('.', LIGHTGRAY, DARKGRAY);
            break;
        case Components::TileType::Wall:
            appearance = TileAppearance('#', WHITE, DARKGRAY);
            break;
        case Components::TileType::Corridor:
            appearance = TileAppearance('#', BROWN, DARKGRAY);
            break;
        case Components::TileType::DoorClosed:
            appearance = TileAppearance('+', YELLOW, BROWN);
            break;
        case Components::TileType::DoorOpen:
            appearance = TileAppearance('\'', YELLOW, BROWN);
            break;
        case Components::TileType::StairsUp:
            appearance = TileAppearance('<', WHITE, GRAY);
            break;
        case Components::TileType::StairsDown:
            appearance = TileAppearance('>', WHITE, GRAY);
            break;
        case Components::TileType::Water:
            appearance = TileAppearance('~', SKYBLUE, DARKBLUE);
            break;
        case Components::TileType::Lava:
            appearance = TileAppearance('~', ORANGE, RED);
            break;
        default:
            appearance = TileAppearance('?', MAGENTA, BLACK);
            break;
    }
    
    // 視界外（探索済み）→ 暗くする
    if (!visible && explored) {
        appearance.foreground = ColorAlpha(appearance.foreground, 0.4f);
        appearance.background = ColorBrightness(appearance.background, -0.5f);
    }
    
    return appearance;
}

/**
 * @brief タイルレンダラー
 * 
 * 文字ベースのタイルをRenderTextureで生成してキャッシュ。
 * 同じ見た目のタイルは同じテクスチャを共有する。
 */
class TileRenderer {
public:
    static constexpr int TILE_SIZE = 32;  // 正方形タイルサイズ（ピクセル）
    
    TileRenderer() = default;
    ~TileRenderer() { Shutdown(); }
    
    // コピー禁止
    TileRenderer(const TileRenderer&) = delete;
    TileRenderer& operator=(const TileRenderer&) = delete;
    
    /**
     * @brief 初期化
     * @param font 使用するフォント（日本語対応推奨）
     */
    void Initialize(Font font) {
        font_ = font;
        isInitialized_ = true;
    }
    
    /**
     * @brief シャットダウン（キャッシュクリア）
     */
    void Shutdown() {
        for (auto& [key, texture] : textureCache_) {
            UnloadTexture(texture);
        }
        textureCache_.clear();
        isInitialized_ = false;
    }
    
    /**
     * @brief タイルテクスチャを取得（キャッシュから or 生成）
     */
    Texture2D GetTileTexture(const TileAppearance& appearance) {
        uint64_t key = MakeCacheKey(appearance);
        
        auto it = textureCache_.find(key);
        if (it != textureCache_.end()) {
            return it->second;
        }
        
        // 新規生成してキャッシュ
        Texture2D texture = GenerateTileTexture(appearance);
        textureCache_[key] = texture;
        return texture;
    }
    
    /**
     * @brief マップ全体を描画
     * @param map マップデータ
     * @param cameraX カメラの中心X座標（タイル単位）
     * @param cameraY カメラの中心Y座標（タイル単位）
     * @param screenWidth 画面幅（ピクセル）
     * @param screenHeight 画面高さ（ピクセル）
     */
    void RenderMap(const Components::MapData& map,
                   int cameraX, int cameraY,
                   int screenWidth, int screenHeight) {
        // 描画するタイル範囲を計算
        int tilesX = (screenWidth / TILE_SIZE) + 2;
        int tilesY = (screenHeight / TILE_SIZE) + 2;
        
        int startX = cameraX - tilesX / 2;
        int startY = cameraY - tilesY / 2;
        int endX = startX + tilesX;
        int endY = startY + tilesY;
        
        // 画面中央オフセット計算
        int offsetX = screenWidth / 2 - (cameraX - startX) * TILE_SIZE - TILE_SIZE / 2;
        int offsetY = screenHeight / 2 - (cameraY - startY) * TILE_SIZE - TILE_SIZE / 2;
        
        for (int y = startY; y <= endY; y++) {
            for (int x = startX; x <= endX; x++) {
                int screenX = (x - startX) * TILE_SIZE + offsetX;
                int screenY = (y - startY) * TILE_SIZE + offsetY;
                
                if (map.InBounds(x, y)) {
                    const auto& tile = map.At(x, y);
                    TileAppearance appearance = GetTileAppearance(
                        tile.type, tile.visible, tile.explored);
                    RenderTile(screenX, screenY, appearance);
                } else {
                    // 範囲外は黒
                    DrawRectangle(screenX, screenY, TILE_SIZE, TILE_SIZE, BLACK);
                }
            }
        }
    }
    
    /**
     * @brief 単一タイルを描画
     */
    void RenderTile(int screenX, int screenY, const TileAppearance& appearance) {
        Texture2D texture = GetTileTexture(appearance);
        DrawTexture(texture, screenX, screenY, WHITE);
    }
    
    /**
     * @brief エンティティ（プレイヤー/モンスター）を描画
     */
    void RenderEntity(int screenX, int screenY, char symbol, Color color) {
        // 背景は透明（タイルの上に重ねる）
        TileAppearance appearance(symbol, color, BLANK);
        
        // キャッシュキーにBLANK背景用の特別処理
        uint64_t key = MakeCacheKey(appearance);
        
        auto it = entityCache_.find(key);
        Texture2D texture;
        if (it != entityCache_.end()) {
            texture = it->second;
        } else {
            texture = GenerateEntityTexture(symbol, color);
            entityCache_[key] = texture;
        }
        
        DrawTexture(texture, screenX, screenY, WHITE);
    }
    
    /**
     * @brief グリッド座標を画面座標に変換
     */
    Vector2 GridToScreen(int gridX, int gridY, int cameraX, int cameraY,
                         int screenWidth, int screenHeight) const {
        float screenX = (gridX - cameraX) * TILE_SIZE + screenWidth / 2.0f - TILE_SIZE / 2.0f;
        float screenY = (gridY - cameraY) * TILE_SIZE + screenHeight / 2.0f - TILE_SIZE / 2.0f;
        return {screenX, screenY};
    }
    
    int GetTileSize() const { return TILE_SIZE; }
    bool IsInitialized() const { return isInitialized_; }

private:
    /**
     * @brief キャッシュキーを生成
     */
    uint64_t MakeCacheKey(const TileAppearance& appearance) const {
        // symbol (8bit) + fg.r (8bit) + fg.g (8bit) + fg.b (8bit) + 
        // bg.r (8bit) + bg.g (8bit) + bg.b (8bit) + bg.a (8bit) = 64bit
        uint64_t key = 0;
        key |= static_cast<uint64_t>(appearance.symbol) << 56;
        key |= static_cast<uint64_t>(appearance.foreground.r) << 48;
        key |= static_cast<uint64_t>(appearance.foreground.g) << 40;
        key |= static_cast<uint64_t>(appearance.foreground.b) << 32;
        key |= static_cast<uint64_t>(appearance.background.r) << 24;
        key |= static_cast<uint64_t>(appearance.background.g) << 16;
        key |= static_cast<uint64_t>(appearance.background.b) << 8;
        key |= static_cast<uint64_t>(appearance.background.a);
        return key;
    }
    
    /**
     * @brief タイルテクスチャを生成（RenderTexture使用）
     */
    Texture2D GenerateTileTexture(const TileAppearance& appearance) {
        RenderTexture2D target = LoadRenderTexture(TILE_SIZE, TILE_SIZE);
        
        BeginTextureMode(target);
        
        // 背景描画
        ClearBackground(appearance.background);
        
        // 文字描画（中央揃え）
        if (appearance.symbol != ' ') {
            const char text[2] = {appearance.symbol, '\0'};
            float fontSize = TILE_SIZE * 0.8f;
            Vector2 textSize = MeasureTextEx(font_, text, fontSize, 1);
            float textX = (TILE_SIZE - textSize.x) / 2.0f;
            float textY = (TILE_SIZE - textSize.y) / 2.0f;
            DrawTextEx(font_, text, {textX, textY}, fontSize, 1, appearance.foreground);
        }
        
        EndTextureMode();
        
        // RenderTextureのテクスチャをコピーして返す
        // 注意: RenderTextureは上下反転しているので、Image経由で修正
        Image image = LoadImageFromTexture(target.texture);
        ImageFlipVertical(&image);
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        UnloadRenderTexture(target);
        
        return texture;
    }
    
    /**
     * @brief エンティティ用テクスチャを生成（透明背景）
     */
    Texture2D GenerateEntityTexture(char symbol, Color color) {
        RenderTexture2D target = LoadRenderTexture(TILE_SIZE, TILE_SIZE);
        
        BeginTextureMode(target);
        
        // 透明背景
        ClearBackground(BLANK);
        
        // 文字描画（中央揃え）
        const char text[2] = {symbol, '\0'};
        float fontSize = TILE_SIZE * 0.9f;  // エンティティは少し大きめ
        Vector2 textSize = MeasureTextEx(font_, text, fontSize, 1);
        float textX = (TILE_SIZE - textSize.x) / 2.0f;
        float textY = (TILE_SIZE - textSize.y) / 2.0f;
        DrawTextEx(font_, text, {textX, textY}, fontSize, 1, color);
        
        EndTextureMode();
        
        Image image = LoadImageFromTexture(target.texture);
        ImageFlipVertical(&image);
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        UnloadRenderTexture(target);
        
        return texture;
    }
    
    Font font_{};
    bool isInitialized_ = false;
    std::unordered_map<uint64_t, Texture2D> textureCache_;  // タイル用
    std::unordered_map<uint64_t, Texture2D> entityCache_;   // エンティティ用
};

} // namespace Roguelike::Rendering

