#include "Scenes/TDGameScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include <iostream>

namespace Scenes {

TDGameScene::TDGameScene()
    : isInitialized_(false)
    , targetSceneName_("sample") {  // デフォルトはsampleシーン
}

void TDGameScene::Initialize(entt::registry& registry) {
    std::cout << "TD Game Scene Initialized" << std::endl;
    
    // 既存のGameクラスのシーン管理システムを使用
    // Gameクラスは既にシーン（sample、test）を登録しているため、
    // ここでは既存のシーンに遷移するだけ
    
    Core::SceneManager& sceneManager = Core::SceneManager::GetInstance();
    
    // sampleシーンが存在するか確認
    if (sceneManager.HasScene("sample")) {
        targetSceneName_ = "sample";
        sceneManager.ChangeScene("sample");
    } else if (sceneManager.HasScene("test")) {
        targetSceneName_ = "test";
        sceneManager.ChangeScene("test");
    } else {
        std::cerr << "Warning: Neither 'sample' nor 'test' scene found" << std::endl;
    }
    
    isInitialized_ = true;
}

void TDGameScene::Update(entt::registry& registry, float deltaTime) {
    if (!isInitialized_) return;
    
    // 入力処理
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    // ESCキーでホームシーンへ戻る
    if (inputManager.IsKeyPressed(KEY_ESCAPE)) {
        Core::SceneManager::GetInstance().ChangeScene("home");
        return;
    }
    
    // 注意: TDGameSceneは既存のGameクラスのシーン管理システムを使用しています
    // Game::Run()が既にシーンの更新と描画を処理しているため、
    // ここでは入力処理（ESCキー）のみを行います
    // 実際のシーンの更新と描画は、Game::Run()内の
    // sceneManager_.UpdateCurrentScene()とsceneManager_.RenderCurrentScene()で処理されます
}

void TDGameScene::Render(entt::registry& registry) {
    if (!isInitialized_) return;
    
    // 注意: TDGameSceneは既存のGameクラスのシーン管理システムを使用しています
    // Game::Run()が既にシーンの更新と描画を処理しているため、
    // ここでは何もする必要はありません
    // 実際のシーンの描画は、Game::Run()内の
    // sceneManager_.RenderCurrentScene()で処理されます
}

void TDGameScene::Shutdown(entt::registry& registry) {
    std::cout << "TD Game Scene Shutdown" << std::endl;
    isInitialized_ = false;
}

} // namespace Scenes

