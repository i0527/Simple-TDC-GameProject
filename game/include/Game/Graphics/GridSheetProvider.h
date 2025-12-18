#ifndef GAME_GRAPHICS_GRIDSHEETPROVIDER_H
#define GAME_GRAPHICS_GRIDSHEETPROVIDER_H

#include <Data/Graphics/IFrameProvider.h>
#include <Data/Graphics/AnimClip.h>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief グリッド形式スプライトシート Provider
 * 
 * Phase 1 実装
 * 
 * グリッドレイアウト（256×256 セルが 16×16 配置など）のスプライトシートから
 * FrameRef を生成し、IFrameProvider インターフェースを提供
 * 
 * 特徴:
 * - JSON ファイル不要（グリッド計算のみ）
 * - clips.json で クリップ定義を記述
 * - 既存ゲーム資産との互換性維持
 * 
 * 使用例:
 * ```cpp
 * GridSheetProvider::Config cfg{256, 256, 16};  // 256×256 のセル、16フレーム/行
 * Texture2D tex = LoadTexture("assets/mainCharacters/Warrior/sprite.png");
 * auto provider = std::make_unique<GridSheetProvider>(tex, cfg);
 * provider->RegisterClip("idle", 0, 8, true, 12.0f);
 * provider->RegisterClip("walk", 8, 8, true, 12.0f);
 * provider->RegisterClip("attack", 16, 12, false, 15.0f);
 * ```
 */
class GridSheetProvider : public IFrameProvider {
public:
    struct Config {
        float cellWidth;      ///< 1セルの幅（ピクセル）
        float cellHeight;     ///< 1セルの高さ（ピクセル）
        int framesPerRow;     ///< 1行のフレーム数
    };
    
    /**
     * @brief コンストラクタ
     * @param texture スプライトシートテクスチャ（所有権なし、呼び出し側で管理）
     * @param config グリッド設定
     */
    GridSheetProvider(Texture2D& texture, const Config& config);
    
    virtual ~GridSheetProvider() = default;
    
    /**
     * @brief クリップを登録
     * 
     * @param name クリップ名（"idle", "walk" など）
     * @param startFrameIndex グリッド内の開始フレーム番号（0ベース）
     * @param frameCount フレーム数
     * @param loop ループ再生するか
     * @param fps 再生速度（FPS）
     */
    void RegisterClip(const std::string& name, 
                     int startFrameIndex, 
                     int frameCount, 
                     bool loop, 
                     float fps);
    
    /**
     * @brief clips.json から一括登録
     * 
     * JSON形式:
     * ```json
     * {
     *   "idle": {"start": 0, "count": 8, "loop": true, "fps": 12.0},
     *   "walk": {"start": 8, "count": 8, "loop": true, "fps": 12.0},
     *   "attack": {"start": 16, "count": 12, "loop": false, "fps": 15.0}
     * }
     * ```
     * 
     * @param jsonPath clips.json のファイルパス
     * @return 成功した場合 true
     */
    bool LoadFromJson(const std::string& jsonPath);
    
    // ===== IFrameProvider インターフェース実装 =====
    
    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    float GetClipFps(const std::string& clipName) const override;
    bool IsLooping(const std::string& clipName) const override;
    
private:
    struct ClipDef {
        int startFrameIndex;
        int frameCount;
        bool loop;
        float fps;
    };
    
    Texture2D* texture_;
    Config config_;
    std::unordered_map<std::string, ClipDef> clips_;
    
    /**
     * @brief フレームインデックスから Grid 座標を計算
     * @param frameIndex グリッド内のフレーム番号（0ベース）
     * @return (col, row) タプル
     */
    std::pair<int, int> FrameIndexToGridCoord(int frameIndex) const;
    
    /**
     * @brief グリッド座標から FrameRef を生成
     * @param col グリッドの列
     * @param row グリッドの行
     * @return DrawTexturePro で即座に使用可能な FrameRef
     */
    FrameRef CreateFrameRef(int col, int row) const;
};

#endif // GAME_GRAPHICS_GRIDSHEETPROVIDER_H
