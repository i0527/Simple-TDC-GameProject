#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <entt/entt.hpp>

namespace Core {
    // シーンインターフェース
    class IScene {
    public:
        virtual ~IScene() = default;
        
        // シーン初期化
        virtual void Initialize(entt::registry& registry) = 0;
        
        // シーン更新
        virtual void Update(entt::registry& registry, float deltaTime) = 0;
        
        // シーン描画
        virtual void Render(entt::registry& registry) = 0;
        
        // シーン終了
        virtual void Shutdown(entt::registry& registry) = 0;
    };

    // シーン管理クラス（Singleton）
    class SceneManager {
    public:
        static SceneManager& GetInstance();
        
        // インスタンスのコピーを禁止
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        
        // シーンを登録
        void RegisterScene(const std::string& name, std::unique_ptr<IScene> scene);
        
        // シーンに変更（次のフレームで適用）
        void ChangeScene(const std::string& name);
        
        // 現在のシーンを取得
        const std::string& GetCurrentSceneName() const;
        
        // シーンが存在するか確認
        bool HasScene(const std::string& name) const;
        
        // 現在のシーンを更新
        void UpdateCurrentScene(entt::registry& registry, float deltaTime);
        
        // 現在のシーンを描画
        void RenderCurrentScene(entt::registry& registry);
        
        // シーン遷移処理（毎フレーム開始時に呼び出し）
        void ProcessSceneChange(entt::registry& registry);
        
    private:
        SceneManager() = default;
        ~SceneManager() = default;
        
        std::unordered_map<std::string, std::unique_ptr<IScene>> scenes_;
        std::string currentScene_;
        std::string nextScene_;
    };
}
