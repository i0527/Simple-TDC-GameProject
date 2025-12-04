/**
 * @file GameNew.cpp
 * @brief 新アーキテクチャによるゲームクラス実装
 */

// プラットフォーム互換性ヘッダーを最初にインクルード
#include "Core/Platform.h"

#include "GameNew.h"
#include "Core/Components/CoreComponents.h"
#include "Core/SoundManager.h"
#include "Core/EffectManager.h"
#include "Game/Components/GameComponents.h"
#include "TD/Components/TDComponents.h"
#include "TD/Systems/TDSystems.h"
#include "TD/Managers/WaveManager.h"
#include "TD/Managers/SpawnManager.h"
#include "TD/Managers/GameStateManager.h"

#include <iostream>

// ===== 基本システム（静的関数として定義）=====

namespace Systems {

/**
 * @brief ライフタイムシステム
 * Lifetimeコンポーネントを持つエンティティの寿命を管理
 */
void LifetimeSystem(Core::World& world, Core::GameContext& context, float dt) {
    auto view = world.View<Core::Components::Lifetime>();
    
    for (auto entity : view) {
        auto& lifetime = view.get<Core::Components::Lifetime>(entity);
        lifetime.remaining -= dt;
        
        if (lifetime.remaining <= 0.0f) {
            world.MarkForDestruction(entity);
        }
    }
}

/**
 * @brief 破棄システム
 * 遅延破棄をフラッシュ
 */
void DestructionSystem(Core::World& world, Core::GameContext& context, float dt) {
    // MarkedForDestructionタグを持つエンティティを破棄リストに追加
    auto view = world.View<Core::Components::MarkedForDestruction>();
    for (auto entity : view) {
        world.MarkForDestruction(entity);
    }
    
    // 遅延破棄を実行
    world.FlushDestruction();
}

/**
 * @brief 移動システム
 * Position + Velocity を持つエンティティを移動
 */
void MovementSystem(Core::World& world, Core::GameContext& context, float dt) {
    auto view = world.View<Core::Components::Position, Core::Components::Velocity>();
    
    for (auto entity : view) {
        auto& pos = view.get<Core::Components::Position>(entity);
        const auto& vel = view.get<Core::Components::Velocity>(entity);
        
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    }
}

/**
 * @brief アニメーションシステム
 * Animation + AnimationData を持つエンティティのアニメーションを更新
 */
void AnimationSystem(Core::World& world, Core::GameContext& context, float dt) {
    auto view = world.View<Game::Components::Animation, Game::Components::AnimationData>();
    
    for (auto entity : view) {
        auto& anim = view.get<Game::Components::Animation>(entity);
        const auto& data = view.get<Game::Components::AnimationData>(entity);
        
        if (!anim.isPlaying) continue;
        
        // 現在のアニメーション定義を取得
        auto it = data.animations.find(anim.currentAnimation);
        if (it == data.animations.end()) continue;
        
        const auto& animInfo = it->second;
        if (animInfo.frames.empty()) continue;
        
        // 時間を進める
        anim.elapsedTime += dt * anim.speedMultiplier;
        
        // 現在のフレームの継続時間を取得
        const auto& currentFrame = animInfo.frames[anim.currentFrameIndex];
        
        // フレーム遷移
        if (anim.elapsedTime >= currentFrame.duration) {
            anim.elapsedTime -= currentFrame.duration;
            anim.currentFrameIndex++;
            
            // アニメーション終了チェック
            if (anim.currentFrameIndex >= animInfo.frames.size()) {
                if (animInfo.loop) {
                    anim.currentFrameIndex = 0;
                    // AnimationLoopedイベント発火可能
                } else {
                    // 次のアニメーションへ遷移
                    if (!animInfo.nextAnimation.empty()) {
                        anim.currentAnimation = animInfo.nextAnimation;
                        anim.currentFrameIndex = 0;
                        anim.elapsedTime = 0.0f;
                    } else {
                        anim.currentFrameIndex = animInfo.frames.size() - 1;
                        anim.isPlaying = false;
                    }
                    // AnimationFinishedイベント発火可能
                }
            }
            
            // フレームタグチェック（attack_hit等のイベントポイント）
            if (anim.currentFrameIndex < animInfo.frames.size()) {
                const auto& newFrame = animInfo.frames[anim.currentFrameIndex];
                if (!newFrame.tag.empty()) {
                    // AnimationFrameChangedイベント発火可能
                    world.Emit(Game::AnimationFrameChanged{
                        entity, 
                        anim.currentAnimation, 
                        anim.currentFrameIndex,
                        newFrame.tag
                    });
                }
            }
        }
    }
}

// ===== フォールバック描画ヘルパー =====
namespace GameFallback {

/**
 * @brief フォールバック形状タイプ
 */
enum class FallbackShape {
    Circle,
    Rectangle,
    Diamond,
    Triangle
};

/**
 * @brief Game::ComponentsのShapeからFallbackShapeへの変換
 */
FallbackShape ToFallbackShape(Game::Components::FallbackVisual::Shape shape) {
    switch (shape) {
        case Game::Components::FallbackVisual::Shape::Circle:
            return FallbackShape::Circle;
        case Game::Components::FallbackVisual::Shape::Rectangle:
            return FallbackShape::Rectangle;
        case Game::Components::FallbackVisual::Shape::Diamond:
            return FallbackShape::Diamond;
        case Game::Components::FallbackVisual::Shape::Triangle:
            return FallbackShape::Triangle;
        default:
            return FallbackShape::Circle;
    }
}

/**
 * @brief フォールバック形状を描画
 */
void DrawFallbackShape(
    float x, float y, 
    float size, 
    FallbackShape shape,
    Color primary, 
    Color secondary,
    int frameIndex,
    bool showIndicator
) {
    float halfSize = size / 2.0f;
    
    switch (shape) {
        case FallbackShape::Circle: {
            DrawCircle(static_cast<int>(x), static_cast<int>(y), halfSize, primary);
            DrawCircleLines(static_cast<int>(x), static_cast<int>(y), halfSize, secondary);
            break;
        }
        case FallbackShape::Rectangle: {
            DrawRectangle(
                static_cast<int>(x - halfSize), 
                static_cast<int>(y - halfSize), 
                static_cast<int>(size), 
                static_cast<int>(size), 
                primary
            );
            DrawRectangleLines(
                static_cast<int>(x - halfSize), 
                static_cast<int>(y - halfSize), 
                static_cast<int>(size), 
                static_cast<int>(size), 
                secondary
            );
            break;
        }
        case FallbackShape::Diamond: {
            Vector2 points[4] = {
                { x, y - halfSize },        // 上
                { x + halfSize, y },        // 右
                { x, y + halfSize },        // 下
                { x - halfSize, y }         // 左
            };
            DrawTriangle(points[0], points[1], points[2], primary);
            DrawTriangle(points[0], points[2], points[3], primary);
            DrawLine((int)points[0].x, (int)points[0].y, (int)points[1].x, (int)points[1].y, secondary);
            DrawLine((int)points[1].x, (int)points[1].y, (int)points[2].x, (int)points[2].y, secondary);
            DrawLine((int)points[2].x, (int)points[2].y, (int)points[3].x, (int)points[3].y, secondary);
            DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, secondary);
            break;
        }
        case FallbackShape::Triangle: {
            Vector2 v1 = { x, y - halfSize };                    // 上
            Vector2 v2 = { x - halfSize, y + halfSize * 0.7f };  // 左下
            Vector2 v3 = { x + halfSize, y + halfSize * 0.7f };  // 右下
            DrawTriangle(v1, v3, v2, primary);
            DrawLine((int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, secondary);
            DrawLine((int)v2.x, (int)v2.y, (int)v3.x, (int)v3.y, secondary);
            DrawLine((int)v3.x, (int)v3.y, (int)v1.x, (int)v1.y, secondary);
            break;
        }
    }
    
    // アニメーションインジケータ（フレーム番号表示）
    if (showIndicator && frameIndex >= 0) {
        char frameText[8];
        snprintf(frameText, sizeof(frameText), "%d", frameIndex);
        int textWidth = MeasureText(frameText, 12);
        Core::RDrawText(frameText, 
                 static_cast<int>(x - textWidth / 2), 
                 static_cast<int>(y - 6), 
                 12, BLACK);
    }
}

/**
 * @brief エンティティの状態に応じたフォールバック色を決定
 */
Color DetermineEntityColor(Core::World& world, entt::entity entity, bool isEnemy) {
    using namespace TD::Components;
    
    // 基本色
    Color baseColor = isEnemy ? RED : BLUE;
    
    // 攻撃中は明るく
    if (world.HasAll<Attacking>(entity)) {
        baseColor = isEnemy ? ORANGE : SKYBLUE;
    }
    
    // スタン中は灰色
    if (world.HasAll<Stunned>(entity)) {
        baseColor = GRAY;
    }
    
    // 死亡中は暗く
    if (world.HasAll<Dying>(entity)) {
        auto* dying = world.TryGet<Dying>(entity);
        if (dying) {
            float alpha = 1.0f - dying->animationProgress;
            baseColor.a = static_cast<unsigned char>(255 * alpha);
        }
    }
    
    // 無敵中は点滅
    if (world.HasAll<Invincible>(entity)) {
        auto* inv = world.TryGet<Invincible>(entity);
        if (inv) {
            float blink = sinf(inv->duration * 10.0f);
            if (blink > 0.5f) {
                baseColor = WHITE;
            }
        }
    }
    
    return baseColor;
}

/**
 * @brief エンティティの形状を決定
 */
FallbackShape DetermineEntityShape(
    Core::World& world, 
    entt::entity entity,
    bool isEnemy
) {
    using namespace Game::Components;
    
    // FallbackVisualコンポーネントがあればそれを使用
    auto* fallback = world.TryGet<FallbackVisual>(entity);
    if (fallback) {
        return ToFallbackShape(fallback->shape);
    }
    
    // なければ敵/味方で分ける
    return isEnemy 
        ? FallbackShape::Diamond 
        : FallbackShape::Circle;
}

} // namespace GameFallback

/**
 * @brief スプライト描画システム（フォールバック対応版）
 * 
 * テクスチャがない場合は自動的にフォールバック描画を行う。
 * アニメーション状態を視覚化して動作確認を容易にする。
 */
void SpriteRenderSystem(Core::World& world, Core::GameContext& context, float dt) {
    using namespace Game::Components;
    using namespace Core::Components;
    
    auto view = world.View<Position, Sprite>();
    
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        const auto& sprite = view.get<Sprite>(entity);
        
        // スケール取得
        float scale = 1.0f;
        auto* scaleComp = world.TryGet<Scale>(entity);
        if (scaleComp) {
            scale = scaleComp->x;
        }
        
        // アニメーション情報取得
        int frameIndex = 0;
        std::string currentAnim = "";
        auto* anim = world.TryGet<Animation>(entity);
        auto* animData = world.TryGet<AnimationData>(entity);
        if (anim) {
            frameIndex = anim->currentFrameIndex;
            currentAnim = anim->currentAnimation;
        }
        
        // 敵/味方判定
        bool isEnemy = world.HasAll<TD::Components::EnemyUnit>(entity);
        
        // テクスチャ名が空または読み込み失敗 → フォールバック描画
        bool useTexture = !sprite.textureName.empty();
        // TODO: ResourceManagerからテクスチャ取得を試みる
        // Texture2D* tex = ResourceManager::Get(sprite.textureName);
        // if (!tex) useTexture = false;
        
        // フォールバック描画
        auto* fallback = world.TryGet<FallbackVisual>(entity);
        
        float size = 40.0f * scale;
        Color primary = GameFallback::DetermineEntityColor(world, entity, isEnemy);
        Color secondary = DARKGRAY;
        auto shape = GameFallback::DetermineEntityShape(world, entity, isEnemy);
        bool showIndicator = true;
        
        if (fallback) {
            size = fallback->size * scale;
            secondary = fallback->secondaryColor;
            showIndicator = fallback->showAnimationIndicator;
        }
        
        GameFallback::DrawFallbackShape(
            pos.x, pos.y,
            size,
            shape,
            primary,
            secondary,
            showIndicator ? frameIndex : -1,
            showIndicator
        );
        
        // アニメーション名表示（デバッグ用）
        if (!currentAnim.empty()) {
            int textWidth = MeasureText(currentAnim.c_str(), 10);
            Core::RDrawText(currentAnim.c_str(),
                     static_cast<int>(pos.x - textWidth / 2),
                     static_cast<int>(pos.y + size / 2 + 2),
                     10, DARKGRAY);
        }
        
        // HP表示（Stats持ちのみ）
        auto* stats = world.TryGet<TD::Components::Stats>(entity);
        if (stats && stats->maxHealth > 0) {
            float hpPercent = stats->currentHealth / stats->maxHealth;
            int barWidth = static_cast<int>(size);
            int barHeight = 4;
            int barX = static_cast<int>(pos.x - size / 2);
            int barY = static_cast<int>(pos.y - size / 2 - 8);
            
            // 背景
            DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
            // HP
            Color hpColor = hpPercent > 0.5f ? GREEN : (hpPercent > 0.25f ? YELLOW : RED);
            DrawRectangle(barX, barY, static_cast<int>(barWidth * hpPercent), barHeight, hpColor);
        }
    }
}

/**
 * @brief デバッグ情報表示システム
 */
void DebugRenderSystem(Core::World& world, Core::GameContext& context, float dt) {
    DrawFPS(10, 10);
    
    std::string entityCount = "Entities: " + std::to_string(world.EntityCount());
    Core::RDrawText(entityCount.c_str(), 10, 30, 16, DARKGRAY);
}

} // namespace Systems


// ===== TDシステムラッパー（Managers統合）=====
namespace TDSystemWrappers {

/**
 * @brief WaveManager更新ラッパー
 */
void WaveManagerUpdate(Core::World& world, Core::GameContext& context, float dt) {
    auto* waveManager = context.TryGet<TD::WaveManager>();
    if (waveManager) {
        waveManager->Update(world, context, dt);
    }
}

/**
 * @brief SpawnManager更新ラッパー
 */
void SpawnManagerUpdate(Core::World& world, Core::GameContext& context, float dt) {
    auto* spawnManager = context.TryGet<TD::SpawnManager>();
    if (spawnManager) {
        spawnManager->Update(world, dt);
    }
}

/**
 * @brief GameStateManager更新ラッパー
 */
void GameStateManagerUpdate(Core::World& world, Core::GameContext& context, float dt) {
    auto* gameState = context.TryGet<TD::GameStateManager>();
    auto* waveManager = context.TryGet<TD::WaveManager>();
    if (gameState && waveManager) {
        gameState->Update(world, *waveManager, dt);
    }
}

} // namespace TDSystemWrappers


// ===== GameNew 実装 =====

GameNew::GameNew() = default;

GameNew::~GameNew() {
    if (isInitialized_) {
        Shutdown();
    }
}

void GameNew::Run() {
    Initialize();
    
    while (isRunning_ && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        Update(deltaTime);
        Render();
    }
    
    Shutdown();
}

void GameNew::Initialize(bool skipWindowInit) {
    // Raylib初期化
    if (!skipWindowInit) {
        InitWindow(screenWidth_, screenHeight_, windowTitle_.c_str());
        SetTargetFPS(targetFPS_);
    }
    
    // オーディオデバイス初期化
    InitAudioDevice();
    
    // FHDレンダラー初期化
    renderer_.Initialize(screenWidth_, screenHeight_);
    
    // サービス初期化
    InitializeServices();
    
    // 定義データ読み込み
    LoadDefinitions();
    
    // サウンドマネージャにレジストリを設定してプリロード
    auto* soundManager = context_.TryGet<Core::SoundManager>();
    if (soundManager) {
        soundManager->SetRegistry(&context_.Get<Data::DefinitionRegistry>());
        soundManager->PreloadAll();
    }
    
    // エフェクトマネージャにレジストリを設定
    auto* effectManager = context_.TryGet<Core::EffectManager>();
    if (effectManager) {
        effectManager->SetRegistry(&context_.Get<Data::DefinitionRegistry>());
    }
    
    // 基本システム登録
    RegisterCoreSystems();
    
    // ユーザー初期化コールバック
    if (initCallback_) {
        initCallback_(*this);
    }
    
    isRunning_ = true;
    isInitialized_ = true;
    
    std::cout << "GameNew initialized with FHD rendering (" 
              << Core::GameRenderer::RENDER_WIDTH << "x" 
              << Core::GameRenderer::RENDER_HEIGHT << ")\n";
}

void GameNew::InitializeServices() {
    // DefinitionRegistry を登録
    context_.Register<Data::DefinitionRegistry>();
    
    // EntityFactory をファクトリ登録（遅延初期化）
    context_.RegisterFactory<Core::EntityFactory>([this](Core::GameContext& ctx) {
        return std::make_shared<Core::EntityFactory>(world_, ctx.Get<Data::DefinitionRegistry>());
    });
    
    // SoundManager を登録
    context_.Register<Core::SoundManager>();
    
    // EffectManager を登録（パーティクル・画面エフェクトシステム）
    context_.Register<Core::EffectManager>();
    
    // TD Managers を登録
    context_.Register<TD::WaveManager>();
    context_.Register<TD::SpawnManager>();
    context_.Register<TD::GameStateManager>();
    
    // GameStateManager初期化
    context_.Get<TD::GameStateManager>().Initialize();
    
    std::cout << "Services initialized\n";
}

void GameNew::LoadDefinitions() {
    auto& registry = context_.Get<Data::DefinitionRegistry>();
    Data::DefinitionLoader loader(registry);
    
    loader.SetErrorHandler([](const std::string& path, const std::string& error) {
        std::cerr << "[DefinitionLoader] " << path << ": " << error << "\n";
    });
    
    loader.LoadAll(definitionsPath_);
}

void GameNew::RegisterCoreSystems() {
    using namespace Core;
    
    // PreUpdate: 入力処理など
    
    // Update: ゲームロジック
    // -- 基本システム
    systemRunner_.Register(SystemPhase::Update, "Lifetime", Systems::LifetimeSystem, 0);
    systemRunner_.Register(SystemPhase::Update, "Movement", Systems::MovementSystem, 10);
    systemRunner_.Register(SystemPhase::Update, "Animation", Systems::AnimationSystem, 20);
    
    // -- TD マネージャ
    systemRunner_.Register(SystemPhase::Update, "SpawnManager", TDSystemWrappers::SpawnManagerUpdate, 50);
    systemRunner_.Register(SystemPhase::Update, "WaveManager", TDSystemWrappers::WaveManagerUpdate, 51);
    systemRunner_.Register(SystemPhase::Update, "GameStateManager", TDSystemWrappers::GameStateManagerUpdate, 52);
    
    // -- TD 戦闘システム
    systemRunner_.Register(SystemPhase::Update, "LaneMovement", TD::Systems::LaneMovementSystem, 100);
    systemRunner_.Register(SystemPhase::Update, "Knockback", TD::Systems::KnockbackSystem, 101);
    systemRunner_.Register(SystemPhase::Update, "StatusEffect", TD::Systems::StatusEffectSystem, 110);
    systemRunner_.Register(SystemPhase::Update, "Stun", TD::Systems::StunSystem, 111);
    systemRunner_.Register(SystemPhase::Update, "Invincible", TD::Systems::InvincibleSystem, 112);
    systemRunner_.Register(SystemPhase::Update, "AttackCooldown", TD::Systems::AttackCooldownSystem, 120);
    systemRunner_.Register(SystemPhase::Update, "AttackTrigger", TD::Systems::AttackTriggerSystem, 121);
    systemRunner_.Register(SystemPhase::Update, "AttackExecution", TD::Systems::AttackExecutionSystem, 122);
    systemRunner_.Register(SystemPhase::Update, "DeathCheck", TD::Systems::DeathCheckSystem, 200);
    systemRunner_.Register(SystemPhase::Update, "DeathAnimation", TD::Systems::DeathAnimationSystem, 201);
    
    // PostUpdate: クリーンアップ
    systemRunner_.Register(SystemPhase::PostUpdate, "Destruction", Systems::DestructionSystem, 100);
    
    // Render: 描画
    systemRunner_.Register(SystemPhase::Render, "SpriteRender", Systems::SpriteRenderSystem, 0);
    systemRunner_.Register(SystemPhase::Render, "DebugRender", Systems::DebugRenderSystem, 100);
    
    std::cout << "Core systems registered: " << systemRunner_.TotalCount() << " systems\n";
}

void GameNew::Update(float deltaTime) {
    // イベント処理
    world_.ProcessEvents();
    
    // ポーズ状態チェック
    bool isPaused = false;
    auto* gameState = context_.TryGet<TD::GameStateManager>();
    if (gameState) {
        isPaused = (gameState->GetPhase() == TD::GamePhase::Paused);
    }
    
    // ユーザー更新コールバック（ポーズ中でも呼び出す：入力処理のため）
    if (updateCallback_) {
        updateCallback_(*this, deltaTime);
    }
    
    // ポーズ中はシステム実行をスキップ
    if (isPaused) {
        return;
    }
    
    // システム実行（Render以外）
    systemRunner_.RunPhase(Core::SystemPhase::PreUpdate, world_, context_, deltaTime);
    systemRunner_.RunPhase(Core::SystemPhase::Update, world_, context_, deltaTime);
    systemRunner_.RunPhase(Core::SystemPhase::PostUpdate, world_, context_, deltaTime);
}

void GameNew::Render() {
    // FHDレンダリング開始（1920x1080空間で描画）
    renderer_.BeginRender();
    
    // 描画システム実行
    systemRunner_.RunPhase(Core::SystemPhase::Render, world_, context_, 0.0f);
    
    // ユーザー描画コールバック
    if (renderCallback_) {
        renderCallback_(*this);
    }
    
    // FHDレンダリング終了（スケーリングして画面に表示）
    renderer_.EndRender();
}

void GameNew::Shutdown() {
    if (!isInitialized_) return;
    
    // ユーザーシャットダウンコールバック
    if (shutdownCallback_) {
        shutdownCallback_(*this);
    }
    
    // サウンドリソース解放
    auto* soundManager = context_.TryGet<Core::SoundManager>();
    if (soundManager) {
        soundManager->UnloadAll();
    }
    
    // レンダラーシャットダウン
    renderer_.Shutdown();
    
    // クリーンアップ
    world_.Clear();
    context_.Clear();
    
    // オーディオデバイスを閉じる
    CloseAudioDevice();
    
    CloseWindow();
    
    isInitialized_ = false;
    std::cout << "GameNew shutdown complete\n";
}
