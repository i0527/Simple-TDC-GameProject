#pragma once

#include "SceneManager.h"
#include <raylib.h>

namespace Scenes {
    /**
     * @brief TD繧ｲ繝ｼ繝譛ｬ菴薙す繝ｼ繝ｳ
     * Game繧ｯ繝ｩ繧ｹ縺ｮ譌｢蟄倥す繝ｼ繝ｳ・・ample縲》est・峨ｒ菴ｿ逕ｨ
     * 縺薙・繧ｷ繝ｼ繝ｳ縺ｯ譌｢蟄倥・Game繧ｯ繝ｩ繧ｹ縺ｮ繧ｷ繝ｼ繝ｳ邂｡逅・す繧ｹ繝・Β繧剃ｽｿ逕ｨ縺吶ｋ縺溘ａ縲・
     * 迚ｹ蛻･縺ｪ螳溯｣・・荳崎ｦ・
     */
    class TDGameScene : public Core::IScene {
    public:
        TDGameScene();
        ~TDGameScene() override = default;
        
        void Initialize(entt::registry& registry) override;
        void Update(entt::registry& registry, float deltaTime) override;
        void Render(entt::registry& registry) override;
        void Shutdown(entt::registry& registry) override;
        
    private:
        bool isInitialized_;
        std::string targetSceneName_;  // "sample" 縺ｾ縺溘・ "test"
    };
}

