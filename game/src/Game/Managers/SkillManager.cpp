#include "Game/Managers/SkillManager.h"
#include "Data/Loaders/SkillLoader.h"
#include <iostream>

namespace Game::Managers {

SkillManager::SkillManager(Shared::Core::GameContext& context,
                         Shared::Data::DefinitionRegistry& definitions)
    : context_(context), definitions_(definitions) {}

bool SkillManager::Initialize() {
    RegisterHotReloadCallback();
    std::cout << "SkillManager initialized" << std::endl;
    return true;
}

void SkillManager::Shutdown() {
    std::cout << "SkillManager shutdown" << std::endl;
}

const Shared::Data::SkillDef* SkillManager::GetSkill(const std::string& id) const {
    return definitions_.GetSkill(id);
}

std::vector<const Shared::Data::SkillDef*> SkillManager::GetAllSkills() const {
    return definitions_.GetAllSkills();
}

void SkillManager::RegisterHotReloadCallback() {
    // スキル定義ファイルの監視
    std::string skill_path = context_.GetDataPath("abilities_debug.json");
    
    context_.GetFileWatcher().Watch(skill_path, [this]() {
        OnSkillsReloaded();
    });
}

void SkillManager::OnSkillsReloaded() {
    std::cout << "Skills reloaded" << std::endl;

    // JSON 再ロード
    std::string skill_path = context_.GetDataPath("abilities_debug.json");
    Shared::Data::SkillLoader::LoadFromJson(skill_path, definitions_);

    // イベント発行
    nlohmann::json event_data;
    event_data["type"] = "skills_reloaded";
    context_.GetEventSystem().Emit("data_reloaded", event_data);
}

} // namespace Game::Managers
