/**
 * @file GameContext.h
 * @brief ゲーム全体の依存性注入（DI）コンテナ
 * 
 * シングルトンを排除し、サービスロケータパターンで
 * 各種マネージャーやシステムへのアクセスを提供。
 * データ駆動設計の中核となるサービス群を管理。
 */
#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <string>
#include <functional>
#include <vector>

namespace Core {

// 前方宣言
class GameContext;

/**
 * @brief サービスが見つからない場合の例外
 */
class ServiceNotFoundException : public std::runtime_error {
public:
    explicit ServiceNotFoundException(const std::string& typeName)
        : std::runtime_error("Service not found: " + typeName) {}
};

/**
 * @brief サービスの循環依存を検出した場合の例外
 */
class CircularDependencyException : public std::runtime_error {
public:
    explicit CircularDependencyException(const std::string& typeName)
        : std::runtime_error("Circular dependency detected: " + typeName) {}
};

/**
 * @brief 依存性注入コンテナ
 * 
 * ゲーム全体で共有するサービス（マネージャー等）を
 * 型安全に登録・取得できるコンテナ。
 * 遅延初期化（ファクトリ登録）をサポート。
 * 
 * 使用例:
 * @code
 * GameContext context;
 * 
 * // 直接登録
 * context.Register<ResourceManager>();
 * 
 * // ファクトリ登録（遅延初期化・依存解決）
 * context.RegisterFactory<EntityFactory>([](GameContext& ctx) {
 *     return std::make_shared<EntityFactory>(ctx.Get<DefinitionRegistry>());
 * });
 * 
 * // サービス取得（ファクトリの場合は初回でインスタンス化）
 * auto& factory = context.Get<EntityFactory>();
 * @endcode
 */
class GameContext {
public:
    using FactoryFunc = std::function<std::shared_ptr<void>(GameContext&)>;
    
    GameContext() = default;
    ~GameContext() = default;
    
    // コピー禁止
    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
    
    // ムーブ許可
    GameContext(GameContext&&) = default;
    GameContext& operator=(GameContext&&) = default;
    
    /**
     * @brief サービスを新規作成して登録
     * @tparam T サービスの型
     * @tparam Args コンストラクタ引数の型
     * @param args コンストラクタ引数
     * @return 登録されたサービスへの参照
     */
    template<typename T, typename... Args>
    T& Register(Args&&... args) {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        auto typeIdx = std::type_index(typeid(T));
        services_[typeIdx] = ptr;
        initOrder_.push_back(typeIdx);
        return *ptr;
    }
    
    /**
     * @brief 既存のインスタンスを登録
     * @tparam T サービスの型
     * @param instance 登録するインスタンス（shared_ptr）
     */
    template<typename T>
    void RegisterInstance(std::shared_ptr<T> instance) {
        auto typeIdx = std::type_index(typeid(T));
        services_[typeIdx] = instance;
        initOrder_.push_back(typeIdx);
    }
    
    /**
     * @brief ファクトリ関数を登録（遅延初期化）
     * @tparam T サービスの型
     * @param factory インスタンス生成関数
     * 
     * ファクトリ内で他のサービスに依存可能。
     * 最初のGet<T>()呼び出し時にインスタンス化される。
     */
    template<typename T>
    void RegisterFactory(std::function<std::shared_ptr<T>(GameContext&)> factory) {
        auto typeIdx = std::type_index(typeid(T));
        factories_[typeIdx] = [factory](GameContext& ctx) -> std::shared_ptr<void> {
            return factory(ctx);
        };
    }
    
    /**
     * @brief サービスを取得
     * @tparam T サービスの型
     * @return サービスへの参照
     * @throws ServiceNotFoundException サービスが登録されていない場合
     * @throws CircularDependencyException 循環依存が検出された場合
     */
    template<typename T>
    T& Get() {
        auto typeIdx = std::type_index(typeid(T));
        
        // 既にインスタンス化済み
        auto it = services_.find(typeIdx);
        if (it != services_.end()) {
            return *std::static_pointer_cast<T>(it->second);
        }
        
        // ファクトリが登録されている場合
        auto factoryIt = factories_.find(typeIdx);
        if (factoryIt != factories_.end()) {
            // 循環依存チェック
            if (resolving_.count(typeIdx) > 0) {
                throw CircularDependencyException(typeid(T).name());
            }
            
            resolving_.insert(typeIdx);
            try {
                auto instance = factoryIt->second(*this);
                services_[typeIdx] = instance;
                initOrder_.push_back(typeIdx);
                resolving_.erase(typeIdx);
                factories_.erase(factoryIt);
                return *std::static_pointer_cast<T>(instance);
            } catch (...) {
                resolving_.erase(typeIdx);
                throw;
            }
        }
        
        throw ServiceNotFoundException(typeid(T).name());
    }
    
    /**
     * @brief サービスを取得（const版）
     */
    template<typename T>
    const T& Get() const {
        auto it = services_.find(std::type_index(typeid(T)));
        if (it == services_.end()) {
            throw ServiceNotFoundException(typeid(T).name());
        }
        return *std::static_pointer_cast<T>(it->second);
    }
    
    /**
     * @brief サービスを取得（存在しなければnullptr）
     * @tparam T サービスの型
     * @return サービスへのポインタ（存在しなければnullptr）
     */
    template<typename T>
    T* TryGet() {
        auto typeIdx = std::type_index(typeid(T));
        
        // 既にインスタンス化済み
        auto it = services_.find(typeIdx);
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second).get();
        }
        
        // ファクトリがあれば解決を試みる
        if (factories_.count(typeIdx) > 0) {
            try {
                return &Get<T>();
            } catch (...) {
                return nullptr;
            }
        }
        
        return nullptr;
    }
    
    /**
     * @brief サービスが登録されているか確認
     * @tparam T サービスの型
     * @return 登録されていればtrue（ファクトリ含む）
     */
    template<typename T>
    bool Has() const {
        auto typeIdx = std::type_index(typeid(T));
        return services_.count(typeIdx) > 0 || factories_.count(typeIdx) > 0;
    }
    
    /**
     * @brief サービスがインスタンス化済みか確認
     * @tparam T サービスの型
     * @return インスタンス化済みならtrue
     */
    template<typename T>
    bool IsInstantiated() const {
        return services_.count(std::type_index(typeid(T))) > 0;
    }
    
    /**
     * @brief サービスを登録解除
     * @tparam T サービスの型
     * @return 登録解除に成功した場合true
     */
    template<typename T>
    bool Unregister() {
        auto typeIdx = std::type_index(typeid(T));
        bool removed = services_.erase(typeIdx) > 0;
        removed |= factories_.erase(typeIdx) > 0;
        return removed;
    }
    
    /**
     * @brief 全サービスをクリア（初期化順の逆順で破棄）
     */
    void Clear() {
        // 逆順でサービスを削除（依存関係を考慮）
        for (auto it = initOrder_.rbegin(); it != initOrder_.rend(); ++it) {
            services_.erase(*it);
        }
        initOrder_.clear();
        factories_.clear();
        resolving_.clear();
    }
    
    /**
     * @brief 登録されているサービス数を取得（インスタンス化済みのみ）
     */
    size_t Size() const {
        return services_.size();
    }
    
    /**
     * @brief 登録されているファクトリ数を取得
     */
    size_t PendingFactoryCount() const {
        return factories_.size();
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    std::unordered_map<std::type_index, FactoryFunc> factories_;
    std::unordered_set<std::type_index> resolving_;  // 循環依存検出用
    std::vector<std::type_index> initOrder_;  // 初期化順序記録
};

} // namespace Core
