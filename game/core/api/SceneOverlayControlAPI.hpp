#pragma once

// 標準ライブラリ
#include <array>
#include <memory>

// プロジェクト内
#include "../config/GameState.hpp"

namespace game {
namespace core {

// 前方宣言
class BaseSystemAPI;
class UISystemAPI;
class OverlayManager;
class IScene;
struct SharedContext;

/// @brief Scene/Overlay制御の更新結果
struct SceneOverlayUpdateResult {
    bool requestShutdown = false;
    bool hasTransition = false;
    GameState nextState = GameState::Initializing;
};

/// @brief Scene/Overlayのライフサイクルと遷移制御を統合するAPI
class SceneOverlayControlAPI {
public:
    SceneOverlayControlAPI();
    ~SceneOverlayControlAPI();

    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI, SharedContext* sharedContext);
    void RegisterScene(GameState state, IScene* scene);
    IScene* GetScene(GameState state) const;

    SceneOverlayUpdateResult Update(GameState state, float deltaTime);
    void Render(GameState state);
    void RenderImGui(GameState state);

    bool InitializeState(GameState state);
    void CleanupState(GameState state);
    void ShutdownAllScenes();
    void Shutdown();

    bool PushOverlay(OverlayState state);
    void PopOverlay();
    void PopAllOverlays();
    bool HasActiveOverlay() const;
    bool IsOverlayActive(OverlayState state) const;

private:
    static constexpr size_t kSceneCount = static_cast<size_t>(GameState::Count);

    static size_t ToIndex(GameState state) {
        return static_cast<size_t>(state);
    }

    bool IsValidState(GameState state) const {
        return ToIndex(state) < kSceneCount;
    }

    BaseSystemAPI* systemAPI_;
    UISystemAPI* uiAPI_;
    SharedContext* sharedContext_;
    std::unique_ptr<OverlayManager> overlayManager_;
    std::array<IScene*, kSceneCount> scenes_;
    GameState lastNonEditorState_ = GameState::Home;
};

} // namespace core
} // namespace game
