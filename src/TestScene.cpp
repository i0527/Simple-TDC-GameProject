#include "SceneManager.h"
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "Components.h"
#include "Game/Systems/InputSystem.h"
#include "Game/Systems/MovementSystem.h"
#include "Game/Systems/AnimationSystem.h"
#include <raylib.h>
#include <entt/entt.hpp>
#include <iostream>
#include <vector>

// TestScene: JSON�t�@�C���ƃe�N�X�`���A�g���X(�X�v���C�g�V�[�g)�̓ǂݍ��݃e�X�g
// �EConfigManager ����l���擾���ă��O�o��
// �EImageManager �� Aseprite �`�� JSON + PNG ��ǂݍ���
// �EFrame ���� EnTT �R���|�[�l���g�iSpriteAnimation, SpriteFrame, SpriteTexture�j�ɓ���
// �EAnimationSystem �ŃA�j���[�V�������[�v���Ǘ�
// �EInputSystem �� MovementSystem �ŃL�[���͂ɉ����Ĉړ�

class TestScene : public Core::IScene {
public:
    void Initialize(entt::registry& registry) override {
        std::cout << "TestScene Initialize" << std::endl;

        // Config �l�̎擾�e�X�g
        Core::ConfigManager& cfg = Core::ConfigManager::GetInstance();
        int w = cfg.GetInt("window.width", 0);
        int h = cfg.GetInt("window.height", 0);
        std::string title = cfg.GetString("window.title", "none");
        std::cout << "Config window.width=" << w << " window.height=" << h << " window.title=" << title << std::endl;

        // �X�v���C�g�V�[�g�ǂݍ��݃e�X�g
        Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
        auto& imageMgr = rm.GetImageManager();

        // cupslime �X�v���C�g�V�[�g�ǂݍ���
        const std::string spriteJson = "assets/json/cupslime.json";
        const std::string spriteImage = "assets/atlas/cupslime.png";
        imageMgr.LoadSpriteSheet("cupslime", spriteJson, spriteImage);

        // �ǂݍ��ݍς݃t���[�������擾
        std::vector<std::string> frameNames = imageMgr.GetAllFrameNames("cupslime");
        if (frameNames.empty()) {
            std::cout << "No frames loaded for cupslime, using fallback circle entity." << std::endl;
            // �t�H�[���o�b�N: �~��`�悷��G���e�B�e�B
            auto entity = registry.create();
            registry.emplace<Components::Position>(entity, 400.f, 300.f);
            registry.emplace<Components::Renderable>(entity, RED, 30.f);
            fallback_ = true;
            return;
        }

        fallback_ = false;
        
        // �t���[���������O�o��
        std::cout << "Loaded " << frameNames.size() << " frames:" << std::endl;
        for (size_t i = 0; i < frameNames.size(); ++i) {
            auto info = imageMgr.GetFrameInfo(frameNames[i]);
            std::cout << "  Frame " << i << ": " << frameNames[i] 
                     << " [" << info.rect.x << ", " << info.rect.y 
                     << ", " << info.rect.width << ", " << info.rect.height << "]"
                     << " duration: " << info.duration << "ms" << std::endl;
        }

        // �A�j���[�V�����t���G���e�B�e�B�𐶐�
        auto entity = registry.create();
        registry.emplace<Components::Position>(entity, 400.f, 300.f);
        
        // Velocity �R���|�[�l���g�F�L�[���͂ōX�V����鑬�x
        registry.emplace<Components::Velocity>(entity, 0.0f, 0.0f);
        
        // Player �^�O�FInputSystem ����������ăL�[���͏���
        registry.emplace<Components::Player>(entity);
        
        // SpriteAnimation �R���|�[�l���g�F�A�j���[�V��������
        auto& anim = registry.emplace<Components::SpriteAnimation>(entity);
        anim.spriteName = "cupslime";
        anim.frames = frameNames;
        anim.currentFrameIndex = 0;
        anim.elapsedTime = 0.0f;
        anim.isPlaying = true;
        anim.isLooping = true;
        
        // SpriteFrame �R���|�[�l���g�F�t���[�����
        auto firstFrameInfo = imageMgr.GetFrameInfo(frameNames[0]);
        registry.emplace<Components::SpriteFrame>(entity, 
            Components::SpriteFrame{frameNames[0], firstFrameInfo.rect});
        
        // SpriteTexture �R���|�[�l���g�F�e�N�X�`���Q��
        registry.emplace<Components::SpriteTexture>(entity, 
            Components::SpriteTexture{"cupslime"});
        
        animatedEntity_ = entity;
        
        std::cout << "TestScene: Entity with animation, input, and movement components created." << std::endl;
    }

    void Update(entt::registry& registry, float deltaTime) override {
        if (fallback_) return;
        
        // �L�[���͏����i�e�X�g�p�AInputManager ����Ȃ��j
        Systems::InputSystem::Update(registry);
        
        // �ړ��X�V
        Systems::MovementSystem::Update(registry, deltaTime);
        
        // �A�j���[�V�����X�V
        Systems::AnimationSystem::Update(registry, deltaTime);
    }

    void Render(entt::registry& registry) override {
        if (fallback_) {
            auto view = registry.view<Components::Position, Components::Renderable>();
            for (auto e : view) {
                auto& pos = view.get<Components::Position>(e);
                auto& rend = view.get<Components::Renderable>(e);
                DrawCircle((int)pos.x, (int)pos.y, rend.radius, rend.color);
            }
            DrawText("Fallback: No sprite sheet found", 10, 120, 16, DARKGRAY);
            return;
        }

        // �X�v���C�g�`��
        Systems::SpriteRenderSystem::Render(registry);

        // �f�o�b�O���\��
        if (auto* anim = registry.try_get<Components::SpriteAnimation>(animatedEntity_)) {
            if (!anim->frames.empty()) {
                DrawText(("Frame: " + anim->frames[anim->currentFrameIndex] + 
                         " [" + std::to_string(anim->currentFrameIndex) + "/" + 
                         std::to_string(anim->frames.size()) + "]").c_str(), 10, 100, 16, DARKGRAY);
            }
        }

        if (auto* pos = registry.try_get<Components::Position>(animatedEntity_)) {
            DrawText(("Position: (" + std::to_string((int)pos->x) + ", " + 
                     std::to_string((int)pos->y) + ")").c_str(), 10, 120, 16, DARKGRAY);
        }

        if (auto* vel = registry.try_get<Components::Velocity>(animatedEntity_)) {
            DrawText(("Velocity: (" + std::to_string((int)vel->dx) + ", " + 
                     std::to_string((int)vel->dy) + ")").c_str(), 10, 140, 16, DARKGRAY);
        }
        
        DrawText("TestScene: cupslime with keyboard control (Arrow Keys)", 10, 80, 20, DARKGRAY);
        DrawText("Use Arrow Keys (UP/DOWN/LEFT/RIGHT) to move", 10, 160, 16, DARKGRAY);
    }

    void Shutdown(entt::registry& registry) override {
        std::cout << "TestScene Shutdown" << std::endl;
    }

private:
    bool fallback_{true};
    entt::entity animatedEntity_{entt::null};
};

// �V�[���o�^�p�w���p�[�֐��iGame::InitializeScenes ����Ăяo���j
std::unique_ptr<Core::IScene> CreateTestScene() {
    return std::make_unique<TestScene>();
}