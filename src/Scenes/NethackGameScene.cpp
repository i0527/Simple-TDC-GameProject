#include "Scenes/NethackGameScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include <iostream>

namespace Scenes {

NethackGameScene::NethackGameScene()
    : isInitialized_(false)
    , originalWindowTitle_("")
    , originalScreenWidth_(0)
    , originalScreenHeight_(0) {
}

void NethackGameScene::Initialize(entt::registry& registry) {
    std::cout << "Nethack Game Scene Initialized" << std::endl;
    
    // 既存のウィンドウ情報を保存
    // 注意: RaylibにはGetWindowTitle()がないため、固定値を保存
    originalWindowTitle_ = "Simple TDC Game";
    originalScreenWidth_ = GetScreenWidth();
    originalScreenHeight_ = GetScreenHeight();
    
    // RoguelikeGameインスタンスを作成
    game_ = std::make_unique<Roguelike::RoguelikeGame>();
    
    // 既にウィンドウが開いているため、ウィンドウ初期化をスキップ
    if (!game_->Initialize(true)) {
        std::cerr << "Failed to initialize Nethack game" << std::endl;
        return;
    }
    
    isInitialized_ = true;
}

void NethackGameScene::Update(entt::registry& registry, float deltaTime) {
    if (!isInitialized_ || !game_) return;
    
    // ESCキーでホームシーンへ戻る
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    if (inputManager.IsKeyPressed(KEY_ESCAPE)) {
        Core::SceneManager::GetInstance().ChangeScene("home");
        return;
    }
    
    // RoguelikeGameのUpdate()を呼び出し
    // 注意: RoguelikeGame::Update()は引数なしなので、deltaTimeは使用されません
    // ただし、RoguelikeGameは内部でGetFrameTime()を使用している可能性があります
    game_->Update();
}

void NethackGameScene::Render(entt::registry& registry) {
    if (!isInitialized_ || !game_) return;
    
    // RenderContent()はBeginDrawing()とEndDrawing()を含まない
    // Game::Render()で既にBeginDrawing()が呼ばれているため、ここではRenderContent()を呼ぶ
    game_->RenderContent();
}

void NethackGameScene::Shutdown(entt::registry& registry) {
    std::cout << "Nethack Game Scene Shutdown" << std::endl;
    
    if (isInitialized_ && game_) {
        // 注意: RoguelikeGame::Shutdown()はCloseWindow()を呼びますが、
        // これは既存のウィンドウを閉じてしまうため、呼ばないようにします
        // 代わりに、必要なクリーンアップのみを実行します
        
        // ウィンドウタイトルを元に戻す
        if (!originalWindowTitle_.empty()) {
            SetWindowTitle(originalWindowTitle_.c_str());
        }
        
        // ゲームのクリーンアップ（CloseWindow()を除く）
        // 残念ながら、RoguelikeGame::Shutdown()はprivateではないので、
        // 直接呼び出すことができますが、CloseWindow()が含まれています
        // この問題を回避するため、game_をリセットするだけにします
        // 実際のクリーンアップはデストラクタで行われます
        
        game_.reset();
        isInitialized_ = false;
    }
}

} // namespace Scenes

