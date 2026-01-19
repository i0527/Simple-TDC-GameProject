#pragma once

// 標準ライブラリ
#include <array>
#include <functional>
#include <string>
#include <vector>

namespace game {
namespace core {

// 前方宣言
struct SharedContext;
class GameplayDataAPI;

/// @brief シーン共通のデバッグ/チートUI API
class DebugUIAPI {
public:
    DebugUIAPI();
    ~DebugUIAPI() = default;

    bool Initialize(SharedContext* sharedContext);
    void Shutdown();
    bool IsInitialized() const;

    void UpdateToggle();
    void Render();

    int RegisterPanel(const std::string& name,
                      const std::function<void(SharedContext&)>& render);
    void UnregisterPanel(int panelId);

    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool visible) { isVisible_ = visible; }

private:
    struct Panel {
        int id = 0;
        std::string name;
        std::function<void(SharedContext&)> render;
    };

    void RenderCommonPanel(SharedContext& ctx);
    void SyncEditFieldsFromSave(const GameplayDataAPI& gameplayDataAPI);

    SharedContext* sharedContext_;
    bool isInitialized_;
    bool isVisible_;
    int nextPanelId_;
    std::vector<Panel> panels_;

    // テクスチャキャッシュのフィルタ（キー文字列）
    std::array<char, 128> textureFilter_{};

    // 通貨編集用
    bool currencyEditInitialized_ = false;
    int editGold_ = 0;
    int editGems_ = 0;
    int editTickets_ = 0;
    int editMaxTickets_ = 0;

    bool hasLastSaveResult_ = false;
    bool lastSaveResult_ = false;
};

} // namespace core
} // namespace game
