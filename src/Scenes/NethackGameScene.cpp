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
    
    // 譌｢蟄倥・繧ｦ繧｣繝ｳ繝峨え諠・ｱ繧剃ｿ晏ｭ・
    // 豕ｨ諢・ Raylib縺ｫ縺ｯGetWindowTitle()縺後↑縺・◆繧√∝崋螳壼､繧剃ｿ晏ｭ・
    originalWindowTitle_ = "Simple TDC Game";
    originalScreenWidth_ = GetScreenWidth();
    originalScreenHeight_ = GetScreenHeight();
    
    // RoguelikeGame繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧剃ｽ懈・
    game_ = std::make_unique<Roguelike::RoguelikeGame>();
    
    // 譌｢縺ｫ繧ｦ繧｣繝ｳ繝峨え縺碁幕縺・※縺・ｋ縺溘ａ縲√え繧｣繝ｳ繝峨え蛻晄悄蛹悶ｒ繧ｹ繧ｭ繝・・
    if (!game_->Initialize(true)) {
        std::cerr << "Failed to initialize Nethack game" << std::endl;
        return;
    }
    
    isInitialized_ = true;
}

void NethackGameScene::Update(entt::registry& registry, float deltaTime) {
    if (!isInitialized_ || !game_) return;
    
    // ESC繧ｭ繝ｼ縺ｧ繝帙・繝繧ｷ繝ｼ繝ｳ縺ｸ謌ｻ繧・
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    if (inputManager.IsKeyPressed(KEY_ESCAPE)) {
        Core::SceneManager::GetInstance().ChangeScene("home");
        return;
    }
    
    // RoguelikeGame縺ｮUpdate()繧貞他縺ｳ蜃ｺ縺・
    // 豕ｨ諢・ RoguelikeGame::Update()縺ｯ蠑墓焚縺ｪ縺励↑縺ｮ縺ｧ縲‥eltaTime縺ｯ菴ｿ逕ｨ縺輔ｌ縺ｾ縺帙ｓ
    // 縺溘□縺励ヽoguelikeGame縺ｯ蜀・Κ縺ｧGetFrameTime()繧剃ｽｿ逕ｨ縺励※縺・ｋ蜿ｯ閭ｽ諤ｧ縺後≠繧翫∪縺・
    game_->Update();
}

void NethackGameScene::Render(entt::registry& registry) {
    if (!isInitialized_ || !game_) return;
    
    // RenderContent()縺ｯBeginDrawing()縺ｨEndDrawing()繧貞性縺ｾ縺ｪ縺・
    // Game::Render()縺ｧ譌｢縺ｫBeginDrawing()縺悟他縺ｰ繧後※縺・ｋ縺溘ａ縲√％縺薙〒縺ｯRenderContent()繧貞他縺ｶ
    game_->RenderContent();
}

void NethackGameScene::Shutdown(entt::registry& registry) {
    std::cout << "Nethack Game Scene Shutdown" << std::endl;
    
    if (isInitialized_ && game_) {
        // 豕ｨ諢・ RoguelikeGame::Shutdown()縺ｯCloseWindow()繧貞他縺ｳ縺ｾ縺吶′縲・
        // 縺薙ｌ縺ｯ譌｢蟄倥・繧ｦ繧｣繝ｳ繝峨え繧帝哩縺倥※縺励∪縺・◆繧√∝他縺ｰ縺ｪ縺・ｈ縺・↓縺励∪縺・
        // 莉｣繧上ｊ縺ｫ縲∝ｿ・ｦ√↑繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・縺ｮ縺ｿ繧貞ｮ溯｡後＠縺ｾ縺・
        
        // 繧ｦ繧｣繝ｳ繝峨え繧ｿ繧､繝医Ν繧貞・縺ｫ謌ｻ縺・
        if (!originalWindowTitle_.empty()) {
            SetWindowTitle(originalWindowTitle_.c_str());
        }
        
        // 繧ｲ繝ｼ繝縺ｮ繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・・・loseWindow()繧帝勁縺擾ｼ・
        // 谿句ｿｵ縺ｪ縺後ｉ縲ヽoguelikeGame::Shutdown()縺ｯprivate縺ｧ縺ｯ縺ｪ縺・・縺ｧ縲・
        // 逶ｴ謗･蜻ｼ縺ｳ蜃ｺ縺吶％縺ｨ縺後〒縺阪∪縺吶′縲，loseWindow()縺悟性縺ｾ繧後※縺・∪縺・
        // 縺薙・蝠城｡後ｒ蝗樣∩縺吶ｋ縺溘ａ縲“ame_繧偵Μ繧ｻ繝・ヨ縺吶ｋ縺縺代↓縺励∪縺・
        // 螳滄圀縺ｮ繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・縺ｯ繝・せ繝医Λ繧ｯ繧ｿ縺ｧ陦後ｏ繧後∪縺・
        
        game_.reset();
        isInitialized_ = false;
    }
}

} // namespace Scenes

