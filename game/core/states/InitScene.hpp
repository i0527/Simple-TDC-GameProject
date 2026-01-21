#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include <string>

namespace game {
namespace core {
namespace states {

/// @brief 初期化シーンクラス
///
/// 責務:
/// - リソーススキャン・読み込み処理
/// - 進捗管理
/// - 初期化画面・エラー画面の描画
/// - 初期化完了/失敗の状態通知
class InitScene : public IScene {
public:
    InitScene();
    ~InitScene() override;

    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;
    bool RequestTransition(GameState& nextState) override;
    bool RequestQuit() override;

    void SetSharedContext(SharedContext* ctx) override { sharedContext_ = ctx; }

private:
    struct InitState {
        struct CategoryStats {
            int loaded = 0;
            std::string lastName = "";
        };

        bool scanningCompleted = false;
        bool initializationStarted = false;
        bool initializationCompleted = false;
        bool initializationFailed = false;
        int currentProgress = 0;
        int totalProgress = 0;
        std::string currentMessage = "初期化中...";
        std::string currentPath = "";
        std::string errorMessage = "";
        float completionDelay = 0.0f;
        float errorDisplayTime = 0.0f;
        CategoryStats font;
        CategoryStats texture;
        CategoryStats sound;
        CategoryStats json;
        CategoryStats other;
    };

    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_;
    InitState initState_;
    bool isInitialized_;
    bool requestTransition_;
    GameState nextState_;
    bool requestQuit_;

    // アニメーション用
    float animationTime_;
    float smoothProgress_;

    void renderInitScreen();
    void renderErrorScreen();
    void updateCategoryStats(const LoadProgress& progress);
    void updateCurrentDisplay(const LoadProgress& progress);
};

} // namespace states
} // namespace core
} // namespace game
