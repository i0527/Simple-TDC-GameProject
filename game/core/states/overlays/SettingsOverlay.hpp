#pragma once

#include "IOverlay.hpp"
#include <memory>
#include <string>

namespace game {
namespace core {

/// @brief 設定データ構造
struct SettingsData {
    float masterVolume = 1.0f;
    float bgmVolume = 1.0f;
    float seVolume = 1.0f;
    bool isFullscreen = false;
    bool showFPS = false;
};

/// @brief 設定オーバーレイ
///
/// 設定画面を表示するオーバーレイ。
/// Panel/Buttonコンポーネントを使用します。
class SettingsOverlay : public IOverlay {
public:
    SettingsOverlay();
    ~SettingsOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Settings; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_ = nullptr;
    bool isInitialized_ = false;
    mutable bool requestClose_ = false;
    mutable bool hasTransitionRequest_ = false;
    mutable GameState requestedNextState_;
    
    // 設定データ
    SettingsData currentSettings_;
    SettingsData savedSettings_;
    std::string settingsFilePath_ = "data/settings.json";
    
    // UI状態
    bool isDraggingSlider_ = false;
    int draggedSliderId_ = -1;  // 0=マスター, 1=BGM, 2=SE
    
    // UIホバー状態
    bool applyButtonHovered_ = false;
    bool resetButtonHovered_ = false;
    bool closeButtonHovered_ = false;
    bool fullscreenButtonHovered_ = false;
    bool fpsCheckboxHovered_ = false;
    
    // プライベートメソッド
    void LoadSettings();
    void SaveSettings();
    bool HasUnsavedChanges() const;
    void ApplySettings();
    void ResetToDefaults();
    
    // UI描画メソッド
    void RenderVolumeSection(float x, float y, float width, float height);
    void RenderDisplaySection(float x, float y, float width, float height);
    void RenderSlider(float x, float y, float width, float height, const char* label, float* value, int sliderId);
    void RenderButton(float x, float y, float width, float height, const char* label, bool& isHovered);
    void RenderCheckbox(float x, float y, float size, const char* label, bool* value, bool& isHovered);
    bool IsPointInRect(float x, float y, float width, float height, Vector2 point) const;
    
    // 入力処理
    void ProcessMouseInput(SharedContext& ctx);
    void ProcessSliderDrag(SharedContext& ctx);
};

} // namespace core
} // namespace game
