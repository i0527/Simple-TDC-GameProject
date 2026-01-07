#include "StageSelectOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <imgui.h>

namespace game {
namespace core {

StageSelectOverlay::StageSelectOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

bool StageSelectOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("StageSelectOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("StageSelectOverlay: systemAPI is null");
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
    panel_->SetId("stage_select_panel");
    panel_->SetPosition(0.0f, 0.0f);  // ImGuiウィンドウ内の相対座標
    panel_->SetSize(contentWidth, contentHeight);
    panel_->Initialize();

    tileGrid_ = std::make_shared<ui::Tile>();
    tileGrid_->SetId("stage_tile_grid");
    tileGrid_->SetPosition(20.0f, 20.0f);  // パネル内の相対座標
    tileGrid_->SetSize(contentWidth - 40.0f, contentHeight - 40.0f);
    tileGrid_->SetGridSize(4, 3);
    tileGrid_->SetTileSize(200.0f, 150.0f);
    tileGrid_->Initialize();

    // サンプルステージデータを追加
    for (int i = 1; i <= 12; ++i) {
        ui::TileData stageData;
        stageData.id = "stage_" + std::to_string(i);
        stageData.label = "Stage " + std::to_string(i);
        stageData.imageId = "";
        stageData.enabled = true;
        tileGrid_->AddTile(stageData);
    }

    // タイル選択コールバック
    tileGrid_->SetOnTileSelected([this](const ui::TileData& tile) {
        LOG_INFO("Stage selected: {}", tile.id);
        // ステージ選択時にゲーム画面に遷移
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Game;
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

    panel_->AddChild(tileGrid_);
    panel_->AddChild(closeButton_);

    isInitialized_ = true;
    LOG_INFO("StageSelectOverlay initialized");
    return true;
}

void StageSelectOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
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

void StageSelectOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // UIコンポーネントの描画
    if (panel_) {
        panel_->Render();
    }
}

void StageSelectOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    if (panel_) {
        panel_->Shutdown();
        panel_.reset();
    }

    if (tileGrid_) {
        tileGrid_->Shutdown();
        tileGrid_.reset();
    }

    if (closeButton_) {
        closeButton_->Shutdown();
        closeButton_.reset();
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("StageSelectOverlay shutdown");
}

bool StageSelectOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool StageSelectOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
