/**
 * @file SystemRunner.h
 * @brief システム実行管理
 * 
 * ゲームループ内でのシステム実行順序を管理。
 * フェーズベースの実行で処理順序を保証。
 */
#pragma once

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Core {

// 前方宣言
class World;
class GameContext;

/**
 * @brief システム実行フェーズ
 */
enum class SystemPhase {
    PreUpdate,   // 入力処理、イベント処理など
    Update,      // メインロジック
    PostUpdate,  // 衝突判定、状態更新など
    Render       // 描画（順序重要）
};

/**
 * @brief システム関数の型定義
 */
using SystemFunction = std::function<void(World&, GameContext&, float)>;

/**
 * @brief システムエントリ
 */
struct SystemEntry {
    std::string name;
    SystemFunction function;
    int priority;  // 低い値が先に実行
    bool enabled;
    
    SystemEntry(std::string n, SystemFunction f, int p = 0)
        : name(std::move(n))
        , function(std::move(f))
        , priority(p)
        , enabled(true) {}
};

/**
 * @brief システム実行管理クラス
 * 
 * フェーズごとにシステムを登録し、優先度順に実行。
 * 
 * 使用例:
 * @code
 * SystemRunner runner;
 * 
 * // システム登録
 * runner.Register(SystemPhase::PreUpdate, "Input", InputSystem::Update);
 * runner.Register(SystemPhase::Update, "Movement", MovementSystem::Update, 0);
 * runner.Register(SystemPhase::Update, "Combat", CombatSystem::Update, 10);
 * runner.Register(SystemPhase::Render, "Sprite", SpriteRenderSystem::Update);
 * 
 * // ゲームループ内
 * runner.RunPhase(SystemPhase::PreUpdate, world, context, deltaTime);
 * runner.RunPhase(SystemPhase::Update, world, context, deltaTime);
 * runner.RunPhase(SystemPhase::PostUpdate, world, context, deltaTime);
 * runner.RunPhase(SystemPhase::Render, world, context, deltaTime);
 * @endcode
 */
class SystemRunner {
public:
    SystemRunner() = default;
    ~SystemRunner() = default;
    
    // コピー禁止
    SystemRunner(const SystemRunner&) = delete;
    SystemRunner& operator=(const SystemRunner&) = delete;
    
    // ムーブ許可
    SystemRunner(SystemRunner&&) = default;
    SystemRunner& operator=(SystemRunner&&) = default;
    
    /**
     * @brief システムを登録
     * @param phase 実行フェーズ
     * @param name システム名（デバッグ/操作用）
     * @param func システム関数
     * @param priority 優先度（低いほど先に実行）
     */
    void Register(SystemPhase phase, const std::string& name, 
                  SystemFunction func, int priority = 0) {
        systems_[phase].emplace_back(name, std::move(func), priority);
        needsSort_[phase] = true;
    }
    
    /**
     * @brief システムを登録解除
     * @param phase 対象フェーズ
     * @param name システム名
     * @return 削除されたらtrue
     */
    bool Unregister(SystemPhase phase, const std::string& name) {
        auto& vec = systems_[phase];
        auto it = std::find_if(vec.begin(), vec.end(),
            [&name](const SystemEntry& e) { return e.name == name; });
        
        if (it != vec.end()) {
            vec.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * @brief 特定フェーズのシステムを実行
     * @param phase 実行するフェーズ
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     * @param deltaTime 前フレームからの経過時間
     */
    void RunPhase(SystemPhase phase, World& world, 
                  GameContext& context, float deltaTime) {
        // 必要に応じてソート
        if (needsSort_[phase]) {
            SortPhase(phase);
            needsSort_[phase] = false;
        }
        
        // 有効なシステムのみ実行
        for (auto& entry : systems_[phase]) {
            if (entry.enabled) {
                entry.function(world, context, deltaTime);
            }
        }
    }
    
    /**
     * @brief 全フェーズを順番に実行
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     * @param deltaTime 前フレームからの経過時間
     */
    void RunAll(World& world, GameContext& context, float deltaTime) {
        RunPhase(SystemPhase::PreUpdate, world, context, deltaTime);
        RunPhase(SystemPhase::Update, world, context, deltaTime);
        RunPhase(SystemPhase::PostUpdate, world, context, deltaTime);
        RunPhase(SystemPhase::Render, world, context, deltaTime);
    }
    
    /**
     * @brief システムの有効/無効を切り替え
     * @param phase 対象フェーズ
     * @param name システム名
     * @param enabled 有効にするか
     * @return 対象システムが見つかればtrue
     */
    bool SetEnabled(SystemPhase phase, const std::string& name, bool enabled) {
        auto& vec = systems_[phase];
        auto it = std::find_if(vec.begin(), vec.end(),
            [&name](const SystemEntry& e) { return e.name == name; });
        
        if (it != vec.end()) {
            it->enabled = enabled;
            return true;
        }
        return false;
    }
    
    /**
     * @brief システムが有効か確認
     * @param phase 対象フェーズ
     * @param name システム名
     * @return 有効ならtrue、見つからなければfalse
     */
    bool IsEnabled(SystemPhase phase, const std::string& name) const {
        auto phaseIt = systems_.find(phase);
        if (phaseIt == systems_.end()) return false;
        
        auto it = std::find_if(phaseIt->second.begin(), phaseIt->second.end(),
            [&name](const SystemEntry& e) { return e.name == name; });
        
        return it != phaseIt->second.end() && it->enabled;
    }
    
    /**
     * @brief システムの優先度を変更
     * @param phase 対象フェーズ
     * @param name システム名
     * @param priority 新しい優先度
     * @return 対象システムが見つかればtrue
     */
    bool SetPriority(SystemPhase phase, const std::string& name, int priority) {
        auto& vec = systems_[phase];
        auto it = std::find_if(vec.begin(), vec.end(),
            [&name](const SystemEntry& e) { return e.name == name; });
        
        if (it != vec.end()) {
            it->priority = priority;
            needsSort_[phase] = true;
            return true;
        }
        return false;
    }
    
    /**
     * @brief 登録されているシステム数を取得
     * @param phase 対象フェーズ
     */
    size_t Count(SystemPhase phase) const {
        auto it = systems_.find(phase);
        return it != systems_.end() ? it->second.size() : 0;
    }
    
    /**
     * @brief 全システム数を取得
     */
    size_t TotalCount() const {
        size_t total = 0;
        for (const auto& [phase, vec] : systems_) {
            total += vec.size();
        }
        return total;
    }
    
    /**
     * @brief フェーズ内のシステム一覧を取得
     * @param phase 対象フェーズ
     * @return システム名のリスト（優先度順）
     */
    std::vector<std::string> GetSystemNames(SystemPhase phase) const {
        std::vector<std::string> names;
        auto it = systems_.find(phase);
        if (it != systems_.end()) {
            names.reserve(it->second.size());
            for (const auto& entry : it->second) {
                names.push_back(entry.name);
            }
        }
        return names;
    }
    
    /**
     * @brief 全システムをクリア
     */
    void Clear() {
        systems_.clear();
        needsSort_.clear();
    }
    
    /**
     * @brief 特定フェーズのシステムをクリア
     * @param phase 対象フェーズ
     */
    void ClearPhase(SystemPhase phase) {
        systems_[phase].clear();
        needsSort_[phase] = false;
    }
    
private:
    /**
     * @brief フェーズ内のシステムを優先度でソート
     */
    void SortPhase(SystemPhase phase) {
        auto& vec = systems_[phase];
        std::stable_sort(vec.begin(), vec.end(),
            [](const SystemEntry& a, const SystemEntry& b) {
                return a.priority < b.priority;
            });
    }
    
    std::unordered_map<SystemPhase, std::vector<SystemEntry>> systems_;
    std::unordered_map<SystemPhase, bool> needsSort_;
};

/**
 * @brief フェーズ名を文字列に変換（デバッグ用）
 */
inline const char* ToString(SystemPhase phase) {
    switch (phase) {
        case SystemPhase::PreUpdate:  return "PreUpdate";
        case SystemPhase::Update:     return "Update";
        case SystemPhase::PostUpdate: return "PostUpdate";
        case SystemPhase::Render:     return "Render";
        default:                      return "Unknown";
    }
}

} // namespace Core
