#pragma once

#include <vector>
#include <map>
#include <string>
#include <raylib.h>
#include <entt/entt.hpp>

namespace game {
namespace core {
namespace gamescene {

/// @brief グリッドセルのタイプ
enum class CellType {
    Normal,      // 通常（配置可能）
    Path,        // 敵の進路
    Blocked,     // 配置不可
    SpawnPoint,  // 敵スポーン位置
    Goal         // ゴール位置
};

/// @brief マップタイル情報
struct MapTile {
    CellType type;
    int gridX;
    int gridY;
};

/// @brief フィールド管理クラス
///
/// 責務:
/// - グリッドベースのマップ管理
/// - ピクセル座標とグリッド座標の変換
/// - マップタイルの描画
/// - ユニット配置の管理
class FieldManager {
public:
    /// @brief コンストラクタ
    /// @param width グリッド幅（セル数）
    /// @param height グリッド高さ（セル数）
    /// @param cellSize セルサイズ（ピクセル）
    /// @param originX フィールド原点X座標（ピクセル）
    /// @param originY フィールド原点Y座標（ピクセル）
    explicit FieldManager(int width = 30, int height = 16, int cellSize = 32,
                         float originX = 640.0f, float originY = 50.0f);
    
    ~FieldManager() = default;

    /// @brief 初期化
    /// @param registry ECSレジストリ（将来のユニット配置用）
    bool Initialize(entt::registry* registry);

    /// @brief シャットダウン
    void Shutdown();

    /// @brief フィールド描画
    /// @param showGrid グリッド線を表示するか
    void Render(bool showGrid = true);

    // ========== 座標変換 ==========

    /// @brief グリッド座標をピクセル座標に変換
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return ピクセル座標
    Vector2 GridToPixel(int gx, int gy) const;

    /// @brief ピクセル座標をグリッド座標に変換
    /// @param px ピクセルX座標
    /// @param py ピクセルY座標
    /// @return グリッド座標（first: gx, second: gy）
    std::pair<int, int> PixelToGrid(float px, float py) const;

    /// @brief グリッド座標が有効範囲内か確認
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return 有効範囲内の場合true
    bool IsValidGridPosition(int gx, int gy) const;

    // ========== ユニット配置管理 ==========

    /// @brief ユニット配置
    /// @param unitEntity ユニットエンティティ
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return 配置成功時true
    bool PlaceUnit(entt::entity unitEntity, int gx, int gy);

    /// @brief ユニット削除
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return 削除成功時true
    bool RemoveUnit(int gx, int gy);

    /// @brief 指定位置のユニットを取得
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return ユニットエンティティ（存在しない場合はentt::null）
    entt::entity GetUnitAt(int gx, int gy) const;

    /// @brief セルが配置可能か確認
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @return 配置可能な場合true
    bool IsPlaceable(int gx, int gy) const;

    // ========== アクセサ ==========

    /// @brief グリッド幅を取得
    int GetWidth() const { return width_; }

    /// @brief グリッド高さを取得
    int GetHeight() const { return height_; }

    /// @brief セルサイズを取得
    int GetCellSize() const { return cellSize_; }

    /// @brief 原点座標を取得
    Vector2 GetOrigin() const { return Vector2{originX_, originY_}; }

    /// @brief タイル情報を取得
    const std::vector<MapTile>& GetTiles() const { return tiles_; }

    /// @brief 敵パスを取得
    const std::vector<Vector2>& GetEnemyPath() const { return enemyPath_; }

private:
    // グリッド設定
    int width_;
    int height_;
    int cellSize_;
    float originX_;
    float originY_;

    // ECSレジストリ（参照）
    entt::registry* registry_;

    // マップデータ
    std::vector<MapTile> tiles_;
    std::vector<Vector2> enemyPath_;

    // ユニット配置マップ（グリッド座標 -> エンティティ）
    std::map<std::pair<int, int>, entt::entity> gridMap_;

    /// @brief デフォルトマップを生成
    void GenerateDefaultMap();

    /// @brief グリッド線を描画
    void DrawGrid();

    /// @brief 背景タイルを描画
    void DrawTiles();

    /// @brief 敵パスを描画
    void DrawEnemyPath();
};

} // namespace gamescene
} // namespace core
} // namespace game
