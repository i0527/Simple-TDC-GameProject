#pragma once

#include "../api/GameModuleAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../ecs/IModule.hpp"
#include <algorithm>
#include <memory>
#include <vector>


namespace game {
namespace core {

/// @brief モジュール管理システム
///
/// 責務:
/// - モジュールの登録・管理（所有権を持つ）
/// - モジュールのライフサイクル管理（Initialize, Update, Render, Shutdown）
/// - 優先順位に基づいた実行順序の管理
///
/// GameSystemとの関係:
/// - GameSystemがModuleSystemを所有
/// - SharedContextは参照として受け取る（所有権なし）
/// - モジュールの実行はGameSystemのメインループから呼び出される
class ModuleSystem {
public:
  /// @brief コンストラクタ
  /// @param gameAPI GameModuleAPIへのポインタ（所有権は持たない）
  explicit ModuleSystem(GameModuleAPI *gameAPI);

  ~ModuleSystem() = default;

  /// @brief モジュールを登録
  /// @tparam ModuleType モジュール型
  template <typename ModuleType> void RegisterModule();

  /// @brief すべてのモジュールを初期化
  /// @param ctx 共有コンテキスト
  /// @return 成功時true、失敗時false
  bool Initialize(SharedContext &ctx);

  /// @brief すべてのモジュールを更新
  /// @param ctx 共有コンテキスト
  /// @param dt デルタタイム（秒）
  void Update(SharedContext &ctx, float dt);

  /// @brief すべてのモジュールを描画
  /// @param ctx 共有コンテキスト
  void Render(SharedContext &ctx);

  /// @brief すべてのモジュールをシャットダウン
  /// @param ctx 共有コンテキスト
  void Shutdown(SharedContext &ctx);

  /// @brief 登録されているモジュール数を取得
  /// @return モジュール数
  size_t GetModuleCount() const { return modules_.size(); }

private:
  GameModuleAPI *gameAPI_;
  std::vector<std::unique_ptr<IModule>> modules_;

  /// @brief モジュールを優先順位でソート
  void SortModulesByPriority();
};

// ========== テンプレート実装 ==========

template <typename ModuleType> void ModuleSystem::RegisterModule() {
  auto module = std::make_unique<ModuleType>();
  modules_.push_back(std::move(module));
}

} // namespace core
} // namespace game
