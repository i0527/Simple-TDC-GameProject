#pragma once

#include "../IUIComponent.hpp"
#include <vector>
#include <map>
#include <functional>

namespace game {
namespace core {
class BaseSystemAPI;
namespace ui {

/// @brief タイルデータ構造体
struct TileData {
    std::string id;
    std::string label;
    std::string imageId;
    std::map<std::string, std::string> metadata;
    bool enabled = true;
};

/// @brief タイル型UIコンポーネント
///
/// ステージマップ、スキルツリーなどに使用されるタイル型UI。
/// グリッド配置と選択機能を提供します。
class Tile : public IUIComponent {
public:
    Tile();
    ~Tile() = default;

    // IUIComponent実装
    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    void SetPosition(float x, float y) override;
    void SetSize(float width, float height) override;
    Rect GetBounds() const override;
    void SetMargin(const Margin& margin) override;

    void SetVisible(bool visible) override;
    bool IsVisible() const override;
    void SetEnabled(bool enabled) override;
    bool IsEnabled() const override;

    // P1: 構造化イベント
    UIEventResult HandleEvent(const UIEvent& ev) override;

    bool OnMouseClick(float x, float y) override;
    bool OnMouseHover(float x, float y) override;
    bool OnKey(int key) override;

    void AddChild(std::shared_ptr<IUIComponent> child) override;
    void RemoveChild(std::shared_ptr<IUIComponent> child) override;

    UIComponentType GetType() const override { return UIComponentType::Tile; }
    const std::string& GetId() const override;
    void SetId(const std::string& id) override;

    // Tile固有メソッド
    /// @brief タイルを追加
    /// @param data 追加するタイルデータ
    void AddTile(const TileData& data);

    /// @brief タイルを削除
    /// @param id 削除するタイルのID
    void RemoveTile(const std::string& id);

    /// @brief グリッドサイズを設定
    /// @param cols 列数
    /// @param rows 行数
    void SetGridSize(int cols, int rows);

    /// @brief タイルサイズを設定
    /// @param width タイルの幅
    /// @param height タイルの高さ
    void SetTileSize(float width, float height);

    /// @brief 選択インデックスを設定
    /// @param index 選択するインデックス
    void SetSelectedIndex(int index);

    /// @brief 選択インデックスを取得
    /// @return 選択されているインデックス（-1の場合は未選択）
    int GetSelectedIndex() const;

    /// @brief 選択されているタイルを取得
    /// @return 選択されているタイルへのポインタ、未選択の場合はnullptr
    const TileData* GetSelectedTile() const;

    /// @brief タイル選択コールバックを設定
    /// @param callback タイルが選択されたときに呼び出されるコールバック関数
    void SetOnTileSelected(std::function<void(const TileData&)> callback);

    /// @brief オーディオ用システムAPIを設定
    void SetBaseSystemAPI(::game::core::BaseSystemAPI* systemAPI) { baseSystemAPI_ = systemAPI; }

private:
    Rect bounds_;
    Margin margin_;
    std::vector<std::shared_ptr<IUIComponent>> children_;
    bool visible_;
    bool enabled_;
    std::string id_;

    std::vector<TileData> tiles_;
    int cols_;
    int rows_;
    float tileWidth_;
    float tileHeight_;
    int selectedIndex_;
    std::function<void(const TileData&)> onTileSelected_;
    ::game::core::BaseSystemAPI* baseSystemAPI_ = nullptr;
};

} // namespace ui
} // namespace core
} // namespace game
