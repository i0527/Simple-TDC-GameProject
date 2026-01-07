#pragma once

#include "IOverlay.hpp"
#include "../../entities/Character.hpp"
#include <memory>
#include <vector>

namespace game {
namespace core {

/// @brief 図鑑オーバーレイ
///
/// 3パネルレイアウトの図鑑画面を表示するオーバーレイ。
/// 左：リスト、中央：ビューア+ステータス、右：説明
class CodexOverlay : public IOverlay {
public:
    CodexOverlay();
    ~CodexOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Codex; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== UI パネル構造 ==========
    // コンテンツ領域: 1880x900
    // マージン: 各パネル間に20px、エッジに10px
    
    struct ListPanel {
        float x = 10.0f, y = 10.0f;
        float width = 350.0f, height = 880.0f;  // 左パネル: 約19%
        float margin_right = 20.0f;
        
        std::vector<const entities::Character*> entries;  // CharacterManagerから取得
        int selected_index = -1;
        int scroll_offset = 0;
        float item_height = 40.0f;
    };
    
    struct CharacterViewport {
        float x = 380.0f, y = 10.0f;
        float width = 740.0f, height = 430.0f;  // 中央上部: ビューア
        float margin_bottom = 20.0f;
        
        float animation_timer = 0.0f;
        int animation_frame = 0;
        float animation_speed = 0.1f;
        bool has_error = false;
        std::string error_message;
    };
    
    struct StatusPanel {
        float x = 380.0f, y = 460.0f;
        float width = 740.0f, height = 430.0f;  // 中央下部: ステータス
        float margin_right = 20.0f;
        float padding = 20.0f;
        float line_height = 28.0f;
        int font_size = 16;
    };
    
    struct InfoPanel {
        float x = 1140.0f, y = 10.0f;
        float width = 730.0f, height = 880.0f;  // 右パネル: 情報
        float padding = 20.0f;
        float line_height = 24.0f;
        int font_size = 15;
    };
    
    // ========== UI 要素 ==========
    ListPanel list_panel_;
    CharacterViewport character_viewport_;
    StatusPanel status_panel_;
    InfoPanel info_panel_;
    
    // ========== システム ==========
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // ========== 描画ヘルパー ==========
    void RenderListPanel();
    void RenderCharacterViewport();
    void RenderStatusPanel();
    void RenderInfoPanel();
    
    // ========== イベント処理 ==========
    void OnListItemClick(int index);
    void OnListScroll(int delta);
    
    // ========== ユーティリティ ==========
    const entities::Character* GetSelectedCharacter() const;
    
    // ========== IDソート用 ==========
    static int ExtractIdNumber(const std::string& id);
    void SortCharactersById();
};

} // namespace core
} // namespace game