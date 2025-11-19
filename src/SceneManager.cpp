#include "SceneManager.h"
#include <iostream>

namespace Core {
    SceneManager& SceneManager::GetInstance() {
        static SceneManager instance;
        return instance;
    }
    
    void SceneManager::RegisterScene(const std::string& name, std::unique_ptr<IScene> scene) {
        // 既に登録されていないか確認
        if (HasScene(name)) {
            std::cerr << "Scene already registered: " << name << std::endl;
            return;
        }
        
        if (!scene) {
            std::cerr << "Cannot register null scene: " << name << std::endl;
            return;
        }
        
        scenes_[name] = std::move(scene);
        std::cout << "Scene registered: " << name << std::endl;
    }
    
    void SceneManager::ChangeScene(const std::string& name) {
        // シーンが存在するか確認
        if (!HasScene(name)) {
            std::cerr << "Scene not found: " << name << std::endl;
            return;
        }
        
        nextScene_ = name;
        std::cout << "Scene change scheduled: " << name << std::endl;
    }
    
    const std::string& SceneManager::GetCurrentSceneName() const {
        return currentScene_;
    }
    
    bool SceneManager::HasScene(const std::string& name) const {
        return scenes_.find(name) != scenes_.end();
    }
    
    void SceneManager::UpdateCurrentScene(entt::registry& registry, float deltaTime) {
        if (currentScene_.empty() || !HasScene(currentScene_)) {
            return;
        }
        
        scenes_[currentScene_]->Update(registry, deltaTime);
    }
    
    void SceneManager::RenderCurrentScene(entt::registry& registry) {
        if (currentScene_.empty() || !HasScene(currentScene_)) {
            return;
        }
        
        scenes_[currentScene_]->Render(registry);
    }
    
    void SceneManager::ProcessSceneChange(entt::registry& registry) {
        // シーン変更がスケジュールされているか確認
        if (nextScene_.empty() || nextScene_ == currentScene_) {
            return;
        }
        
        // 現在のシーンをシャットダウン
        if (!currentScene_.empty() && HasScene(currentScene_)) {
            std::cout << "Shutting down scene: " << currentScene_ << std::endl;
            scenes_[currentScene_]->Shutdown(registry);
            // エンティティをすべてクリア
            registry.clear();
        }
        
        // 新しいシーンに遷移
        currentScene_ = nextScene_;
        nextScene_.clear();
        
        std::cout << "Scene changed to: " << currentScene_ << std::endl;
        
        // 新しいシーンを初期化
        if (HasScene(currentScene_)) {
            scenes_[currentScene_]->Initialize(registry);
        }
    }
}
