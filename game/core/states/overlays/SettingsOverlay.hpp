#pragma once

#include "IOverlay.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/GameConfig.hpp"
#include <memory>
#include <string>

namespace game {
namespace core {

class AudioControlAPI;

/// @brief 設定データ構造
struct SettingsData {
    float masterVolume = 1.0f;
    float bgmVolume = 1.0f;
    float seVolume = 1.0f;
    bool isFullscreen = false;  // 後方互換性のため残す（windowModeに移行予定）
    bool showFPS = false;
    bool showCursor = false;  // カーソル表示（デフォルト: オフ）
    Resolution resolution = Resolution::FHD;  // 解像度プリセット
    WindowMode windowMode = WindowMode::Windowed;  // ウィンドウモード
};

/// @brief 設定オーバ�Eレイ
///
/// 設定画面を表示するオーバ�Eレイ、E
/// Panel/Buttonコンポ�Eネントを使用します、E
class SettingsOverlay : public IOverlay {
public:
    SettingsOverlay();
    ~SettingsOverlay() = default;

    // IOverlay実裁E
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Settings; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;
    bool RequestQuit() const override;

private:
    BaseSystemAPI* systemAPI_ = nullptr;
    AudioControlAPI* audioAPI_ = nullptr;
    bool isInitialized_ = false;
    mutable bool requestClose_ = false;
    mutable bool hasTransitionRequest_ = false;
    mutable GameState requestedNextState_;
    
    // 設定データ
    SettingsData currentSettings_;
    SettingsData savedSettings_;
    std::string settingsFilePath_ = "data/settings.json";
    
    // UI状慁E
    bool isDraggingSlider_ = false;
    int draggedSliderId_ = -1;  // 0=マスター, 1=BGM, 2=SE
    
    // UIホバー状慁E
    bool resetButtonHovered_ = false;
    bool closeButtonHovered_ = false;
    bool titleButtonHovered_ = false;
    bool quitButtonHovered_ = false;
    bool fullscreenButtonHovered_ = false;
    bool fpsCheckboxHovered_ = false;
    bool cursorCheckboxHovered_ = false;
    bool resolutionFHDButtonHovered_ = false;
    bool resolutionHDButtonHovered_ = false;
    bool windowModeWindowedButtonHovered_ = false;
    bool windowModeFullscreenButtonHovered_ = false;
    mutable bool requestQuit_ = false;
    
    // プライベ�EトメソチE��
    void LoadSettings();
    void SaveSettings();
    bool HasUnsavedChanges() const;
    void ApplySettings();
    void ResetToDefaults();
    
    // UI描画メソチE��
    void RenderVolumeSection(float x, float y, float width, float height);
    void RenderDisplaySection(SharedContext& ctx, float x, float y, float width, float height);
    void RenderSlider(float x, float y, float width, float height, const char* label, float* value, int sliderId);
    void RenderButton(SharedContext& ctx, float x, float y, float width, float height,
                      const char* label, bool& isHovered, bool isEnabled = true);
    void RenderCheckbox(SharedContext& ctx, float x, float y, float size,
                        const char* label, bool* value, bool& isHovered);
    bool IsPointInRect(float x, float y, float width, float height, Vec2 point) const;
    
    // 入力�E琁E
    void ProcessMouseInput(SharedContext& ctx);
    void ProcessSliderDrag(SharedContext& ctx);
};

} // namespace core
} // namespace game
