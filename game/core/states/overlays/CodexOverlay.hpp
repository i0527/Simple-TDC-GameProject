#pragma once

#include "IOverlay.hpp"
#include "../../ecs/entities/Character.hpp"
#include "../../ecs/entities/ItemPassiveManager.hpp"
#include "../../system/PlayerDataManager.hpp"
#include <memory>
#include <string>
#include <array>
#include <vector>

namespace game {
namespace core {

/// @brief アニメーションタイチE
enum class AnimationType {
    Move,   // 移動アニメーション
    Attack  // 攻撁E��ニメーション
};

/// @brief 図鑑タチE
enum class CodexTab {
    Characters,
    Equipment,
    Passives
};

/// @brief 図鑑オーバ�Eレイ
///
/// 3パネルレイアウト�E図鑑画面を表示するオーバ�Eレイ、E
/// 左�E�リスト、中央�E�ビューア+スチE�Eタス、右�E�説昁E
class CodexOverlay : public IOverlay {
public:
    CodexOverlay();
    ~CodexOverlay() = default;

    // IOverlay実裁E
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Codex; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    enum class DropdownKind {
        None,
        EquipmentSlot,
        PassiveSlot
    };

    struct CodexEntry {
        enum class Type {
            Character,
            Equipment,
            Passive
        };

        Type type = Type::Character;
        std::string id;
        std::string name;
        std::string description;
        bool is_discovered = true;

        // 参�E先（所有権なし！E
        const entities::Character* character = nullptr;
        const entities::Equipment* equipment = nullptr;
        const entities::PassiveSkill* passive = nullptr;
    };

    // ========== UI パネル構造 ==========
    // コンチE��チE��域: 1880x900
    // マ�Eジン: 吁E��ネル間に20px、エチE��に10px
    
    struct ListPanel {
        float x = 10.0f, y = 70.0f;
        float width = 520.0f, height = 820.0f;  // 左パネル: グリチE��一覧
        float margin_right = 20.0f;

        float padding = 16.0f;
        float card_width = 150.0f;
        float card_height = 120.0f;
        float card_gap = 12.0f;
    };
    
    struct CharacterViewport {
        float x = 380.0f, y = 70.0f;
        float width = 740.0f, height = 400.0f;  // 中央上部: ビューア
        float margin_bottom = 20.0f;
        
        float animation_timer = 0.0f;
        int animation_frame = 0;
        float animation_speed = 0.1f; // legacy (unused)
        AnimationType current_animation = AnimationType::Move;
        bool is_paused = false;
        float speed_multiplier = 1.0f; // 0.25/0.5/1/2...
        float zoom = 1.0f;            // 0.5..2.0
        bool has_error = false;
        std::string error_message;
    };
    
    struct StatusPanel {
        // 右パネル: スチE�Eタス/試着/統計（縦長で使ぁE��すく�E�E
        float x = 1140.0f, y = 70.0f;
        float width = 730.0f, height = 820.0f;
        float margin_right = 20.0f;
        float padding = 20.0f;
        float line_height = 34.0f;
        int font_size = 24;
    };
    
    struct InfoPanel {
        // 中央下部: 説明（折り返し/スクロール�E�E
        float x = 380.0f, y = 490.0f;
        float width = 740.0f, height = 400.0f;
        float padding = 20.0f;
        float line_height = 36.0f;
        int font_size = 22;
    };
    
    // ========== UI 要素 ==========
    ListPanel list_panel_;
    CharacterViewport character_viewport_;
    StatusPanel status_panel_;
    InfoPanel info_panel_;

    // タブ�Eエントリ状慁E
    CodexTab activeTab_ = CodexTab::Characters;
    std::array<std::vector<CodexEntry>, 3> tabEntries_{};
    std::array<int, 3> tabSelectedIndex_{{-1, -1, -1}};
    std::array<int, 3> tabScrollOffset_{{0, 0, 0}};

    // 試着�E��Eレビュー専用・保存なし！E
    std::string tryOnCharacterId_;
    PlayerDataManager::CharacterState tryOnState_{};

    // ドロチE�Eダウン�E�裁E��/パッシブ選択！E
    DropdownKind dropdownKind_ = DropdownKind::None;
    int dropdownSlotIndex_ = -1; // 0..2
    float dropdownScrollPx_ = 0.0f;

    // 右説明パネル�E�折り返し/スクロールのキャチE��ュ�E�E
    float infoScrollPx_ = 0.0f;
    float infoCachedMaxWidth_ = -1.0f;
    std::string infoCachedKey_;
    std::vector<std::string> infoWrappedLines_;
    
    // ========== シスチE�� ==========
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // ========== 描画ヘルパ�E ==========
    void LayoutPanels();
    void RenderTabBar(float offsetX, float offsetY);
    void RenderListPanel();
    void RenderCharacterViewport();
    void RenderStatusPanel(SharedContext& ctx);
    void RenderInfoPanel();
    
    // ========== イベント�E琁E==========
    void SwitchTab(CodexTab tab);
    void OnListItemClick(int index);
    void OnListScroll(int delta);
    
    // ========== ユーチE��リチE�� ==========
    int TabIndex(CodexTab tab) const;
    CodexEntry* GetSelectedEntry();
    const CodexEntry* GetSelectedEntry() const;
    const entities::Character* GetSelectedCharacter() const;
    const entities::Equipment* GetSelectedEquipment() const;
    const entities::PassiveSkill* GetSelectedPassive() const;
    
    // ========== IDソート用 ==========
    static int ExtractIdNumber(const std::string& id);
    void SortCharactersById(std::vector<CodexEntry>& entries);
    void EnsureEntriesLoaded(SharedContext& ctx);
};

} // namespace core
} // namespace game
