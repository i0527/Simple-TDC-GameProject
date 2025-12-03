#include "Scenes/TDGameScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include <iostream>

namespace Scenes {

TDGameScene::TDGameScene()
    : isInitialized_(false)
    , targetSceneName_("sample") {  // 繝・ヵ繧ｩ繝ｫ繝医・sample繧ｷ繝ｼ繝ｳ
}

void TDGameScene::Initialize(entt::registry& registry) {
    std::cout << "TD Game Scene Initialized" << std::endl;
    
    // 譌｢蟄倥・Game繧ｯ繝ｩ繧ｹ縺ｮ繧ｷ繝ｼ繝ｳ邂｡逅・す繧ｹ繝・Β繧剃ｽｿ逕ｨ
    // Game繧ｯ繝ｩ繧ｹ縺ｯ譌｢縺ｫ繧ｷ繝ｼ繝ｳ・・ample縲》est・峨ｒ逋ｻ骭ｲ縺励※縺・ｋ縺溘ａ縲・
    // 縺薙％縺ｧ縺ｯ譌｢蟄倥・繧ｷ繝ｼ繝ｳ縺ｫ驕ｷ遘ｻ縺吶ｋ縺縺・
    
    Core::SceneManager& sceneManager = Core::SceneManager::GetInstance();
    
    // sample繧ｷ繝ｼ繝ｳ縺悟ｭ伜惠縺吶ｋ縺狗｢ｺ隱・
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
    
    // 蜈･蜉帛・逅・
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    // ESC繧ｭ繝ｼ縺ｧ繝帙・繝繧ｷ繝ｼ繝ｳ縺ｸ謌ｻ繧・
    if (inputManager.IsKeyPressed(KEY_ESCAPE)) {
        Core::SceneManager::GetInstance().ChangeScene("home");
        return;
    }
    
    // 豕ｨ諢・ TDGameScene縺ｯ譌｢蟄倥・Game繧ｯ繝ｩ繧ｹ縺ｮ繧ｷ繝ｼ繝ｳ邂｡逅・す繧ｹ繝・Β繧剃ｽｿ逕ｨ縺励※縺・∪縺・
    // Game::Run()縺梧里縺ｫ繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ縺ｨ謠冗判繧貞・逅・＠縺ｦ縺・ｋ縺溘ａ縲・
    // 縺薙％縺ｧ縺ｯ蜈･蜉帛・逅・ｼ・SC繧ｭ繝ｼ・峨・縺ｿ繧定｡後＞縺ｾ縺・
    // 螳滄圀縺ｮ繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ縺ｨ謠冗判縺ｯ縲；ame::Run()蜀・・
    // sceneManager_.UpdateCurrentScene()縺ｨsceneManager_.RenderCurrentScene()縺ｧ蜃ｦ逅・＆繧後∪縺・
}

void TDGameScene::Render(entt::registry& registry) {
    if (!isInitialized_) return;
    
    // 豕ｨ諢・ TDGameScene縺ｯ譌｢蟄倥・Game繧ｯ繝ｩ繧ｹ縺ｮ繧ｷ繝ｼ繝ｳ邂｡逅・す繧ｹ繝・Β繧剃ｽｿ逕ｨ縺励※縺・∪縺・
    // Game::Run()縺梧里縺ｫ繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ縺ｨ謠冗判繧貞・逅・＠縺ｦ縺・ｋ縺溘ａ縲・
    // 縺薙％縺ｧ縺ｯ菴輔ｂ縺吶ｋ蠢・ｦ√・縺ゅｊ縺ｾ縺帙ｓ
    // 螳滄圀縺ｮ繧ｷ繝ｼ繝ｳ縺ｮ謠冗判縺ｯ縲；ame::Run()蜀・・
    // sceneManager_.RenderCurrentScene()縺ｧ蜃ｦ逅・＆繧後∪縺・
}

void TDGameScene::Shutdown(entt::registry& registry) {
    std::cout << "TD Game Scene Shutdown" << std::endl;
    isInitialized_ = false;
}

} // namespace Scenes

