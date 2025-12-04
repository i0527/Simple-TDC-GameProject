/**
 * @file EntityFactory.h
 * @brief エンティティファクトリ
 * 
 * 定義データからエンティティを生成するファクトリ。
 * JSONで定義されたキャラクターやオブジェクトを
 * ECSエンティティとして実体化する。
 */
#pragma once

#include "World.h"
#include "Data/Registry.h"
#include "../Game/Components/GameComponents.h"
#include "../Domain/TD/Components/TDComponents.h"
#include "Components/CoreComponents.h"
#include <entt/entt.hpp>
#include <string>
#include <functional>
#include <unordered_map>
#include <cmath>

namespace Core {
namespace TD = Domain::TD;
namespace GameComponents = Game::Components;
namespace TDComponents = TD::Components;

/**
 * @brief エンティティファクトリ
 * 
 * 定義データからエンティティを生成。
 * カスタムビルダーを登録することで拡張可能。
 * 
 * 使用例:
 * @code
 * EntityFactory factory(world, registry);
 * 
 * // キャラクター生成
 * auto entity = factory.CreateCharacter("cupslime", 100.0f, 200.0f, false);
 * 
 * // カスタムビルダー登録
 * factory.RegisterBuilder("boss", [](World& w, entt::entity e, const CharacterDef& def) {
 *     w.Emplace<TD::Components::BossUnit>(e);
 * });
 * @endcode
 */
class EntityFactory {
public:
    using CharacterBuilder = std::function<void(World&, entt::entity, const Data::CharacterDef&)>;
    using PostCreateHook = std::function<void(World&, entt::entity)>;
    
    /**
     * @brief コンストラクタ
     * @param world ECSワールド
     * @param definitions 定義レジストリ
     */
    EntityFactory(World& world, Data::DefinitionRegistry& definitions)
        : world_(world)
        , definitions_(definitions) {}
    
    // ===== キャラクター生成 =====
    
    /**
     * @brief キャラクターを生成
     * @param characterId キャラクター定義ID
     * @param x 初期X座標
     * @param y 初期Y座標
     * @param isEnemy 敵側かどうか
     * @param level レベル
     * @return 生成されたエンティティ
     */
    entt::entity CreateCharacter(
        const std::string& characterId,
        float x, float y,
        bool isEnemy = false,
        int level = 1
    ) {
        const auto* def = definitions_.TryGetCharacter(characterId);
        if (!def) {
            std::cerr << "EntityFactory: Character not found: " << characterId 
                      << " - Creating fallback entity\n";
            return CreateFallbackCharacter(characterId, x, y, isEnemy, level);
        }
        
        auto entity = world_.Create();
        
        // === Core コンポーネント ===
        world_.Emplace<Core::Components::Position>(entity, x, y);
        world_.Emplace<Core::Components::Scale>(entity, def->visual.scale, def->visual.scale);
        world_.Emplace<Core::Components::Identity>(entity, characterId, "unit", def->name);
        
        // === Game コンポーネント ===
        const std::string& spriteSource = !def->visual.sprite.atlasPath.empty()
            ? def->visual.sprite.atlasPath
            : def->visual.sprite.jsonPath;
        
        Game::Components::Sprite sprite;
        sprite.textureName = spriteSource;
        sprite.flipX = isEnemy;
        world_.Emplace<Game::Components::Sprite>(entity, sprite);
        
        Game::Components::SpriteSheet sheet;
        sheet.textureName = spriteSource;
        sheet.frameWidth = def->visual.frameWidth;
        sheet.frameHeight = def->visual.frameHeight;
        sheet.framesPerRow = def->visual.framesPerRow;
        world_.Emplace<Game::Components::SpriteSheet>(entity, sheet);
        
        Game::Components::AnimationData animData;
        for (const auto& [name, animDef] : def->visual.animations) {
            Game::Components::AnimationData::AnimInfo info;
            for (const auto& frame : animDef.frames) {
                info.frames.push_back({frame.index, frame.duration, frame.tag});
            }
            info.loop = animDef.loop;
            info.nextAnimation = animDef.nextAnimation;
            animData.animations[name] = std::move(info);
        }
        animData.defaultAnimation = def->visual.defaultAnimation;
        world_.Emplace<Game::Components::AnimationData>(entity, std::move(animData));
        
        Game::Components::Animation anim;
        anim.currentAnimation = def->visual.defaultAnimation;
        world_.Emplace<Game::Components::Animation>(entity, anim);
        
        Game::Components::RenderOrder order;
        order.layer = 10;
        order.orderInLayer = isEnemy ? 0 : 1;
        world_.Emplace<Game::Components::RenderOrder>(entity, order);
        
        Game::Components::FallbackVisual fallback;
        fallback.shape = isEnemy 
            ? Game::Components::FallbackVisual::Shape::Diamond 
            : Game::Components::FallbackVisual::Shape::Circle;
        fallback.primaryColor = isEnemy ? RED : BLUE;
        fallback.secondaryColor = isEnemy ? MAROON : DARKBLUE;
        fallback.size = static_cast<float>(def->visual.frameWidth) * def->visual.scale * 0.6f;
        fallback.showAnimationIndicator = true;
        world_.Emplace<Game::Components::FallbackVisual>(entity, fallback);
        
        // === TD コンポーネント ===
        TDComponents::Unit unit;
        unit.definitionId = characterId;
        unit.isEnemy = isEnemy;
        unit.level = level;
        world_.Emplace<TDComponents::Unit>(entity, unit);
        
        TDComponents::Stats stats;
        float levelMultiplier = std::pow(def->healthGrowth, level - 1);
        float attackMultiplier = std::pow(def->attackGrowth, level - 1);
        stats.maxHealth = def->stats.hp * levelMultiplier;
        stats.currentHealth = stats.maxHealth;
        stats.attack = def->stats.attack * attackMultiplier;
        stats.defense = def->stats.defense;
        stats.moveSpeed = def->stats.moveSpeed;
        stats.attackInterval = def->stats.attackInterval;
        stats.knockbackResist = def->stats.knockbackResist;
        world_.Emplace<TDComponents::Stats>(entity, stats);
        
        TDComponents::Combat combat;
        combat.attackType = def->combat.attackType;
        combat.attackRange = def->combat.attackRangeArea;
        combat.hitbox = def->combat.hitbox;
        combat.attackCount = def->combat.attackCount;
        combat.criticalChance = def->combat.criticalChance;
        combat.criticalMultiplier = def->combat.criticalMultiplier;
        world_.Emplace<TDComponents::Combat>(entity, combat);
        
        TDComponents::Movement movement;
        movement.direction = isEnemy ? -1.0f : 1.0f;
        world_.Emplace<TDComponents::Movement>(entity, movement);
        
        world_.Emplace<TDComponents::StatModifiers>(entity);
        world_.Emplace<TDComponents::StatusEffects>(entity);
        
        // 陣営タグ
        if (isEnemy) {
            world_.Emplace<TD::Components::EnemyUnit>(entity);
        } else {
            world_.Emplace<TD::Components::AllyUnit>(entity);
        }
        
        // カスタムビルダー適用
        for (const auto& trait : def->traits) {
            auto it = builders_.find(trait);
            if (it != builders_.end()) {
                it->second(world_, entity, *def);
            }
        }
        
        // 汎用ビルダー適用
        auto builderIt = builders_.find(characterId);
        if (builderIt != builders_.end()) {
            builderIt->second(world_, entity, *def);
        }
        
        // ポストフック実行
        for (const auto& hook : postCreateHooks_) {
            hook(world_, entity);
        }
        
        return entity;
    }
    
    /**
     * @brief キャラクターをレーン付きで生成
     */
    entt::entity CreateCharacterInLane(
        const std::string& characterId,
        float x,
        int laneIndex,
        float laneY,
        bool isEnemy = false,
        int level = 1
    ) {
        auto entity = CreateCharacter(characterId, x, laneY, isEnemy, level);
        if (entity != entt::null) {
            world_.Emplace<TD::Components::Lane>(entity, laneIndex, laneY);
        }
        return entity;
    }
    
    // ===== 投射物生成 =====
    
    /**
     * @brief 投射物を生成
     */
    entt::entity CreateProjectile(
        entt::entity source,
        entt::entity target,
        float damage,
        float speed,
        float x, float y,
        const std::string& spritePath = ""
    ) {
        auto entity = world_.Create();
        
        world_.Emplace<Core::Components::Position>(entity, x, y);
        world_.Emplace<Core::Components::Identity>(entity, "", "projectile", "Projectile");
        
        TD::Components::Projectile proj;
        proj.source = source;
        proj.target = target;
        proj.damage = damage;
        proj.speed = speed;
        world_.Emplace<TD::Components::Projectile>(entity, proj);
        
        if (!spritePath.empty()) {
            Game::Components::Sprite sprite;
            sprite.textureName = spritePath;
            world_.Emplace<Game::Components::Sprite>(entity, sprite);
        }
        
        Game::Components::RenderOrder order;
        order.layer = 15;  // キャラより上
        world_.Emplace<Game::Components::RenderOrder>(entity, order);
        
        return entity;
    }
    
    // ===== 拠点生成 =====
    
    /**
     * @brief 拠点を生成
     */
    entt::entity CreateBase(
        float x, float y,
        float health,
        bool isPlayerBase,
        const std::string& spritePath = ""
    ) {
        auto entity = world_.Create();
        
        world_.Emplace<Core::Components::Position>(entity, x, y);
        world_.Emplace<Core::Components::Identity>(entity, 
            isPlayerBase ? "player_base" : "enemy_base", 
            "base", 
            isPlayerBase ? "Player Base" : "Enemy Base");
        
        TD::Components::Base base;
        base.isPlayerBase = isPlayerBase;
        base.health = health;
        base.maxHealth = health;
        world_.Emplace<TD::Components::Base>(entity, base);
        
        if (!spritePath.empty()) {
            Game::Components::Sprite sprite;
            sprite.textureName = spritePath;
            world_.Emplace<Game::Components::Sprite>(entity, sprite);
        }
        
        Game::Components::RenderOrder order;
        order.layer = 5;  // 背景に近い
        world_.Emplace<Game::Components::RenderOrder>(entity, order);
        
        return entity;
    }
    
    // ===== カスタマイズ =====
    
    /**
     * @brief カスタムビルダーを登録
     * @param key トリガーキー（characterIdまたはtrait）
     * @param builder ビルダー関数
     */
    void RegisterBuilder(const std::string& key, CharacterBuilder builder) {
        builders_[key] = std::move(builder);
    }
    
    /**
     * @brief ポストフックを追加
     * 全エンティティ生成後に呼ばれる
     */
    void AddPostCreateHook(PostCreateHook hook) {
        postCreateHooks_.push_back(std::move(hook));
    }
    
    /**
     * @brief ビルダーをクリア
     */
    void ClearBuilders() {
        builders_.clear();
    }
    
    // ===== フォールバック生成 =====
    
    /**
     * @brief 定義が存在しない場合のフォールバックキャラクター生成
     * 
     * JSONがない、または読み込めない場合に
     * デフォルト値でテスト用エンティティを生成する
     */
    entt::entity CreateFallbackCharacter(
        const std::string& characterId,
        float x, float y,
        bool isEnemy = false,
        int level = 1
    ) {
        auto entity = world_.Create();
        
        // === Core コンポーネント ===
        world_.Emplace<Core::Components::Position>(entity, x, y);
        world_.Emplace<Core::Components::Scale>(entity, 1.0f, 1.0f);
        world_.Emplace<Core::Components::Identity>(entity, 
            characterId.empty() ? "fallback_unit" : characterId, 
            "unit", 
            "Fallback Unit");
        
        // === Game コンポーネント ===
        // 空のスプライト（フォールバック描画用）
        Game::Components::Sprite sprite;
        sprite.textureName = "";  // 空 = フォールバック描画
        sprite.flipX = isEnemy;
        world_.Emplace<Game::Components::Sprite>(entity, sprite);
        
        // テスト用アニメーションデータ（4フレームのループ）
        Game::Components::AnimationData animData;
        Game::Components::AnimationData::AnimInfo idleAnim;
        idleAnim.loop = true;
        idleAnim.nextAnimation = "";
        for (int i = 0; i < 4; ++i) {
            idleAnim.frames.push_back({i, 0.2f, ""});
        }
        animData.animations["idle"] = idleAnim;
        
        // 攻撃アニメーション（3フレーム、attack_hitタグ付き）
        Game::Components::AnimationData::AnimInfo attackAnim;
        attackAnim.loop = false;
        attackAnim.nextAnimation = "idle";
        attackAnim.frames.push_back({0, 0.1f, ""});
        attackAnim.frames.push_back({1, 0.1f, "attack_hit"});
        attackAnim.frames.push_back({2, 0.15f, ""});
        animData.animations["attack"] = attackAnim;
        
        // 死亡アニメーション
        Game::Components::AnimationData::AnimInfo dieAnim;
        dieAnim.loop = false;
        dieAnim.nextAnimation = "";
        dieAnim.frames.push_back({0, 0.2f, ""});
        dieAnim.frames.push_back({1, 0.2f, ""});
        animData.animations["die"] = dieAnim;
        
        animData.defaultAnimation = "idle";
        world_.Emplace<Game::Components::AnimationData>(entity, std::move(animData));
        
        // アニメーション状態
        Game::Components::Animation anim;
        anim.currentAnimation = "idle";
        anim.isPlaying = true;
        world_.Emplace<Game::Components::Animation>(entity, anim);
        
        // 描画順
        Game::Components::RenderOrder order;
        order.layer = 10;
        order.orderInLayer = isEnemy ? 0 : 1;
        world_.Emplace<Game::Components::RenderOrder>(entity, order);
        
        // フォールバック描画設定
        Game::Components::FallbackVisual fallback;
        fallback.shape = isEnemy 
            ? Game::Components::FallbackVisual::Shape::Triangle 
            : Game::Components::FallbackVisual::Shape::Rectangle;
        fallback.primaryColor = isEnemy ? ORANGE : SKYBLUE;
        fallback.secondaryColor = isEnemy ? RED : BLUE;
        fallback.size = 40.0f;
        fallback.showAnimationIndicator = true;
        world_.Emplace<Game::Components::FallbackVisual>(entity, fallback);
        
        // === TD コンポーネント ===
        TD::Components::Unit unit;
        unit.definitionId = characterId.empty() ? "fallback" : characterId;
        unit.isEnemy = isEnemy;
        unit.level = level;
        world_.Emplace<TD::Components::Unit>(entity, unit);
        
        // デフォルトステータス
        TD::Components::Stats stats;
        stats.maxHealth = 100.0f * level;
        stats.currentHealth = stats.maxHealth;
        stats.attack = 10.0f * level;
        stats.defense = 0.0f;
        stats.moveSpeed = 50.0f;
        stats.attackInterval = 1.0f;
        stats.knockbackResist = 0.0f;
        world_.Emplace<TD::Components::Stats>(entity, stats);
        
        // デフォルト戦闘情報
        TD::Components::Combat combat;
        combat.attackType = Data::AttackType::Single;
        combat.attackRange = { 20.0f, -20.0f, 60.0f, 40.0f };
        combat.hitbox = { -15.0f, -20.0f, 30.0f, 40.0f };
        combat.attackCount = 1;
        world_.Emplace<TD::Components::Combat>(entity, combat);
        
        // 移動状態
        TD::Components::Movement movement;
        movement.direction = isEnemy ? -1.0f : 1.0f;
        world_.Emplace<TD::Components::Movement>(entity, movement);
        
        // その他必須コンポーネント
        world_.Emplace<TD::Components::StatModifiers>(entity);
        world_.Emplace<TD::Components::StatusEffects>(entity);
        
        // 陣営タグ
        if (isEnemy) {
            world_.Emplace<TD::Components::EnemyUnit>(entity);
        } else {
            world_.Emplace<TD::Components::AllyUnit>(entity);
        }
        
        std::cout << "EntityFactory: Created fallback " 
                  << (isEnemy ? "enemy" : "ally") 
                  << " for '" << characterId << "'\n";
        
        return entity;
    }

private:
    World& world_;
    Data::DefinitionRegistry& definitions_;
    std::unordered_map<std::string, CharacterBuilder> builders_;
    std::vector<PostCreateHook> postCreateHooks_;
};

} // namespace Core
