#include "Game/Managers/StageManager.h"
#include "Data/Loaders/StageLoader.h"
#include <iostream>

namespace Game::Managers {

StageManager::StageManager(Shared::Core::GameContext& context,
                         Shared::Data::DefinitionRegistry& definitions)
    : context_(context), definitions_(definitions) {}

bool StageManager::Initialize() {
    RegisterHotReloadCallback();
    std::cout << "StageManager initialized" << std::endl;
    return true;
}

void StageManager::Shutdown() {
    std::cout << "StageManager shutdown" << std::endl;
}

const Shared::Data::StageDef* StageManager::GetStage(const std::string& id) const {
    return definitions_.GetStage(id);
}

std::vector<const Shared::Data::StageDef*> StageManager::GetAllStages() const {
    return definitions_.GetAllStages();
}

void StageManager::RegisterHotReloadCallback() {
    // ステージ定義ファイルの監視
    std::string stage_path = context_.GetDataPath("stages_debug.json");
    
    context_.GetFileWatcher().Watch(stage_path, [this]() {
        OnStagesReloaded();
    });
}

void StageManager::OnStagesReloaded() {
    std::cout << "Stages reloaded" << std::endl;

    // JSON 再ロード
    std::string stage_path = context_.GetDataPath("stages_debug.json");
    Shared::Data::StageLoader::LoadFromJson(stage_path, definitions_);

    // イベント発行
    nlohmann::json event_data;
    event_data["type"] = "stages_reloaded";
    context_.GetEventSystem().Emit("data_reloaded", event_data);
}

} // namespace Game::Managers
