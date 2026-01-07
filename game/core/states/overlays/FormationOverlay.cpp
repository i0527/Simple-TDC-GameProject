#include "FormationOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <imgui.h>

namespace game {
namespace core {

FormationOverlay::FormationOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

bool FormationOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("FormationOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("FormationOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;

    // UIコンポーネントの初期化
    // ホームスクリーン用のサイズ（ヘッダーとタブバーの間、左右に余白）
    const float marginLeft = 20.0f;
    const float marginRight = 20.0f;
    const float marginTop = 90.0f;
    const float marginBottom = 90.0f;
    const float contentWidth = 1920.0f - marginLeft - marginRight;
    const float contentHeight = 1080.0f - marginTop - marginBottom;
    
    panel_ = std::make_shared<ui::Panel>();
    panel_->SetId("formation_panel");
    panel_->SetPosition(0.0f, 0.0f);  // ImGuiウィンドウ内の相対座標
    panel_->SetSize(contentWidth, contentHeight);
    panel_->Initialize();

    characterList_ = std::make_shared<ui::List>();
    characterList_->SetId("character_list");
    characterList_->SetPosition(20.0f, 20.0f);  // パネル内の相対座標
    characterList_->SetSize(500.0f, contentHeight - 40.0f);
    characterList_->SetItemHeight(50.0f);
    characterList_->Initialize();

    // CharacterManagerから実際のキャラクターデータを取得
    // SharedContext経由でアクセス（Initialize()の後にctxが利用可能になる想定）
    // 実際のデータはUpdate()やRender()で取得することも可能
    // ここでは空のリストで初期化し、Update()でデータを設定

    // 選択変更コールバック
    characterList_->SetOnSelectionChanged([this](const ui::ListItem& item) {
        LOG_INFO("Character selected: {}", item.id);
    });

    closeButton_ = std::make_shared<ui::Button>();
    closeButton_->SetId("close_button");
    closeButton_->SetPosition(contentWidth - 170.0f, 20.0f);  // パネル内の相対座標
    closeButton_->SetSize(150.0f, 40.0f);
    closeButton_->SetLabel("Close");
    closeButton_->Initialize();

    closeButton_->SetOnClickCallback([this]() {
        requestClose_ = true;
    });

    panel_->AddChild(characterList_);
    panel_->AddChild(closeButton_);

    isInitialized_ = true;
    LOG_INFO("FormationOverlay initialized");
    return true;
}

void FormationOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // CharacterManagerからキャラクターデータを取得してリストに反映
    if (ctx.characterManager && characterList_) {
        // リストが空の場合のみ追加（重複を防ぐ）
        if (characterList_->GetItemCount() == 0) {
            auto characterIds = ctx.characterManager->GetAllCharacterIds();
            for (const auto& id : characterIds) {
                auto character = ctx.characterManager->GetCharacterTemplate(id);
                if (character) {
                    ui::ListItem item;
                    item.id = character->id;
                    item.label = character->name;
                    item.value = "Lv." + std::to_string(character->level) + 
                                " (" + character->rarity_name + ")";
                    item.enabled = true;
                    characterList_->AddItem(item);
                }
            }
        }
    }

    // UIコンポーネントの更新
    if (panel_) {
        panel_->Update(deltaTime);
    }

    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) { // ESC key
        requestClose_ = true;
    }
}

void FormationOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // UIコンポーネントの描画
    if (panel_) {
        panel_->Render();
    }
}

void FormationOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    if (panel_) {
        panel_->Shutdown();
        panel_.reset();
    }

    if (characterList_) {
        characterList_->Shutdown();
        characterList_.reset();
    }

    if (closeButton_) {
        closeButton_->Shutdown();
        closeButton_.reset();
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("FormationOverlay shutdown");
}

bool FormationOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool FormationOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
