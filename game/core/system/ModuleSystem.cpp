#include "ModuleSystem.hpp"
#include "../../utils/Log.h"

namespace game {
    namespace core {

        ModuleSystem::ModuleSystem(GameModuleAPI* gameAPI)
            : gameAPI_(gameAPI)
        {
        }

        bool ModuleSystem::Initialize(SharedContext& ctx) {
            SortModulesByPriority();
            
            for (auto& module : modules_) {
                if (!module->Initialize(ctx)) {
                    LOG_ERROR("Failed to initialize module: {}", module->GetName());
                    return false;
                }
                LOG_INFO("Module initialized: {}", module->GetName());
            }
            
            return true;
        }

        void ModuleSystem::Update(SharedContext& ctx, float dt) {
            for (auto& module : modules_) {
                module->Update(ctx, dt);
            }
        }

        void ModuleSystem::Render(SharedContext& ctx) {
            for (auto& module : modules_) {
                module->Render(ctx);
            }
        }

        void ModuleSystem::Shutdown(SharedContext& ctx) {
            // 逆順でシャットダウン
            for (auto it = modules_.rbegin(); it != modules_.rend(); ++it) {
                (*it)->Shutdown(ctx);
                LOG_INFO("Module shutdown: {}", (*it)->GetName());
            }
            modules_.clear();
        }

        void ModuleSystem::SortModulesByPriority() {
            // 更新優先順位でソート
            std::sort(modules_.begin(), modules_.end(),
                [](const std::unique_ptr<IModule>& a, const std::unique_ptr<IModule>& b) {
                    return a->GetUpdatePriority() < b->GetUpdatePriority();
                });
        }

    } // namespace core
} // namespace game
