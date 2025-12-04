/**
 * @file World.h
 * @brief ECSワールドラッパー
 * 
 * entt::registry と entt::dispatcher を統合し、
 * エンティティ操作とイベント管理の統一インターフェースを提供。
 */
#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace Core {

/**
 * @brief ECSワールドクラス
 * 
 * registry（エンティティ・コンポーネント管理）と
 * dispatcher（イベント管理）を統合。
 * 
 * 使用例:
 * @code
 * World world;
 * 
 * // エンティティ作成
 * auto entity = world.Create();
 * world.Emplace<Position>(entity, 100.0f, 200.0f);
 * 
 * // イベント購読
 * world.Subscribe<DamageEvent>([](const DamageEvent& e) {
 *     std::cout << "Damage: " << e.amount << std::endl;
 * });
 * 
 * // イベント発行
 * world.Emit(DamageEvent{entity, 50});
 * @endcode
 */
class World {
public:
    World() = default;
    ~World() = default;
    
    // コピー禁止
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    // ムーブ許可
    World(World&&) = default;
    World& operator=(World&&) = default;
    
    // ===== Registry アクセス =====
    
    /**
     * @brief レジストリへの参照を取得
     */
    entt::registry& Registry() { return registry_; }
    const entt::registry& Registry() const { return registry_; }
    
    // ===== Dispatcher アクセス =====
    
    /**
     * @brief ディスパッチャーへの参照を取得
     */
    entt::dispatcher& Dispatcher() { return dispatcher_; }
    const entt::dispatcher& Dispatcher() const { return dispatcher_; }
    
    // ===== エンティティ操作 =====
    
    /**
     * @brief 新しいエンティティを作成
     * @return 作成されたエンティティ
     */
    entt::entity Create() {
        return registry_.create();
    }
    
    /**
     * @brief エンティティを破棄
     * @param entity 破棄するエンティティ
     */
    void Destroy(entt::entity entity) {
        if (registry_.valid(entity)) {
            registry_.destroy(entity);
        }
    }
    
    /**
     * @brief エンティティが有効か確認
     * @param entity 確認するエンティティ
     * @return 有効ならtrue
     */
    bool Valid(entt::entity entity) const {
        return registry_.valid(entity);
    }
    
    /**
     * @brief 全エンティティをクリア
     */
    void Clear() {
        registry_.clear();
    }
    
    // ===== コンポーネント操作 =====
    
    /**
     * @brief コンポーネントを追加
     * @tparam T コンポーネントの型
     * @tparam Args コンストラクタ引数の型
     * @param entity 対象エンティティ
     * @param args コンストラクタ引数
     * @return 追加されたコンポーネントへの参照（タグコンポーネントの場合は参照なし）
     */
    template<typename T, typename... Args>
    decltype(auto) Emplace(entt::entity entity, Args&&... args) {
        return registry_.emplace<T>(entity, std::forward<Args>(args)...);
    }
    
    /**
     * @brief コンポーネントを追加または置換
     * @tparam T コンポーネントの型
     * @tparam Args コンストラクタ引数の型
     * @param entity 対象エンティティ
     * @param args コンストラクタ引数
     * @return コンポーネントへの参照
     */
    template<typename T, typename... Args>
    decltype(auto) EmplaceOrReplace(entt::entity entity, Args&&... args) {
        return registry_.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }
    
    /**
     * @brief コンポーネントを取得
     * @tparam T コンポーネントの型
     * @param entity 対象エンティティ
     * @return コンポーネントへの参照
     */
    template<typename T>
    T& Get(entt::entity entity) {
        return registry_.get<T>(entity);
    }
    
    /**
     * @brief コンポーネントを取得（const版）
     */
    template<typename T>
    const T& Get(entt::entity entity) const {
        return registry_.get<T>(entity);
    }
    
    /**
     * @brief コンポーネントを安全に取得
     * @tparam T コンポーネントの型
     * @param entity 対象エンティティ
     * @return コンポーネントへのポインタ（存在しなければnullptr）
     */
    template<typename T>
    T* TryGet(entt::entity entity) {
        return registry_.try_get<T>(entity);
    }
    
    /**
     * @brief コンポーネントを安全に取得（const版）
     */
    template<typename T>
    const T* TryGet(entt::entity entity) const {
        return registry_.try_get<T>(entity);
    }
    
    /**
     * @brief コンポーネントを持っているか確認
     * @tparam Ts 確認するコンポーネントの型（複数可）
     * @param entity 対象エンティティ
     * @return すべて持っていればtrue
     */
    template<typename... Ts>
    bool HasAll(entt::entity entity) const {
        return registry_.all_of<Ts...>(entity);
    }
    
    /**
     * @brief いずれかのコンポーネントを持っているか確認
     * @tparam Ts 確認するコンポーネントの型（複数可）
     * @param entity 対象エンティティ
     * @return いずれか持っていればtrue
     */
    template<typename... Ts>
    bool HasAny(entt::entity entity) const {
        return registry_.any_of<Ts...>(entity);
    }
    
    /**
     * @brief コンポーネントを削除
     * @tparam T コンポーネントの型
     * @param entity 対象エンティティ
     */
    template<typename T>
    void Remove(entt::entity entity) {
        registry_.remove<T>(entity);
    }
    
    /**
     * @brief ビューを取得
     * @tparam Ts 取得するコンポーネントの型
     * @return ビュー
     */
    template<typename... Ts>
    auto View() {
        return registry_.view<Ts...>();
    }
    
    /**
     * @brief ビューを取得（const版）
     */
    template<typename... Ts>
    auto View() const {
        return registry_.view<Ts...>();
    }
    
    // ===== イベント操作 =====
    
    /**
     * @brief イベントを即時発行（同期）
     * @tparam Event イベントの型
     * @param event 発行するイベント
     */
    template<typename Event>
    void Emit(const Event& event) {
        dispatcher_.trigger(event);
    }
    
    /**
     * @brief イベントをキューに追加（非同期）
     * @tparam Event イベントの型
     * @param event 追加するイベント
     */
    template<typename Event>
    void Enqueue(const Event& event) {
        dispatcher_.enqueue(event);
    }
    
    /**
     * @brief キューに溜まったイベントを処理
     */
    void ProcessEvents() {
        dispatcher_.update();
    }
    
    /**
     * @brief イベントを購読（自由関数/ラムダ）
     * @tparam Event イベントの型
     * @param callback コールバック関数
     */
    template<typename Event>
    void Subscribe(std::function<void(const Event&)> callback) {
        // ラムダをキャプチャするためにshared_ptrでラップ
        auto callbackPtr = std::make_shared<std::function<void(const Event&)>>(std::move(callback));
        dispatcher_.sink<Event>().connect([callbackPtr](const Event& e) {
            (*callbackPtr)(e);
        });
    }
    
    /**
     * @brief イベント購読を解除（特定の型の全コールバック）
     * @tparam Event イベントの型
     */
    template<typename Event>
    void Unsubscribe() {
        dispatcher_.sink<Event>().disconnect();
    }
    
    // ===== ユーティリティ =====
    
    /**
     * @brief 生存エンティティ数を取得
     */
    size_t EntityCount() const {
        return registry_.storage<entt::entity>()->in_use();
    }
    
    /**
     * @brief 遅延破棄リストにエンティティを追加
     * @param entity 破棄予定のエンティティ
     */
    void MarkForDestruction(entt::entity entity) {
        pendingDestruction_.push_back(entity);
    }
    
    /**
     * @brief 遅延破棄を実行
     */
    void FlushDestruction() {
        for (auto entity : pendingDestruction_) {
            if (registry_.valid(entity)) {
                registry_.destroy(entity);
            }
        }
        pendingDestruction_.clear();
    }

private:
    entt::registry registry_;
    entt::dispatcher dispatcher_;
    std::vector<entt::entity> pendingDestruction_;  // 遅延破棄リスト
};

} // namespace Core
