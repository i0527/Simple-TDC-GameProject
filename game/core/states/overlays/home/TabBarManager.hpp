#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace game {
namespace core {
    class BaseSystemAPI;
}
}

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

enum class HomeTab {
    StageSelect = 0,  // ステージ
    Formation = 1,    // 編成
    Unit = 2,          // ユニット(new)
    Enhancement = 3,  // 強化
    Gacha = 4,        // ガチャ
    Codex = 5,        // 図鑑
    Settings = 6,     // 設定
    COUNT = 7
};

struct TabButton {
    HomeTab tab_id;
    std::string label;
    float x, y;
    float width, height;
    bool is_selected;
};

class TabBarManager {
public:
    TabBarManager();
    ~TabBarManager();

    // 初期化
    bool Initialize();

    // UI更新・描画
    void Update(float deltaTime);
    void Render(BaseSystemAPI* systemAPI);

    // タブ選択
    void SelectTab(HomeTab tab);
    HomeTab GetSelectedTab() const { return current_tab_; }

    // マウスイベント
    bool OnMouseClick(float x, float y);
    bool OnMouseHover(float x, float y);

    // コールバック設定
    void SetOnTabChanged(std::function<void(HomeTab)> callback) {
        on_tab_changed_ = callback;
    }

    // タブ情報取得
    const TabButton& GetTabButton(HomeTab tab) const;
    const std::vector<TabButton>& GetAllTabs() const { return tabs_; }

private:
    std::vector<TabButton> tabs_;
    HomeTab current_tab_;
    int hovered_tab_index_;
    std::function<void(HomeTab)> on_tab_changed_;

    // 初期タブ配置計算
    void LayoutTabs();
};

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
