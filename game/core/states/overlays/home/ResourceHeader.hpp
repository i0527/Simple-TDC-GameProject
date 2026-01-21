#pragma once

#include <string>

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

// ゲーム内リソース
struct PlayerResources {
    int gold;           // ゲーム内金貨
    int gems;           // プレミアム通貨
    int tickets;        // 現在のチケット数
    int max_tickets;    // チケット最大値
};

class ResourceHeader {
public:
    ResourceHeader();
    ~ResourceHeader();

    // 初期化
    bool Initialize();

    // リソース更新
    void SetResources(const PlayerResources& resources);
    const PlayerResources& GetResources() const { return resources_; }

    // UI描画
    void Update(float deltaTime);
    void Render(BaseSystemAPI* systemAPI);

    // リソース表示位置
    static constexpr float HEADER_HEIGHT = 90.0f;

private:
    PlayerResources resources_;
    
    // アニメーション用
    float gold_display_current_;   // 現在の表示値（増減アニメ用）
};

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
