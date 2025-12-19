#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

namespace Shared::Core {

/// @brief 型安全なイベントシグナル
/// @tparam Args コールバック引数
template <typename... Args> class Signal {
public:
  using Slot = std::function<void(Args...)>;

  /// @brief 購読を追加
  /// @param slot コールバック
  /// @return ハンドルID（解除用）
  int Connect(Slot slot) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = next_id_++;
    slots_.push_back({id, std::move(slot)});
    return id;
  }

  /// @brief 購読を解除
  /// @param handle Connectで返されたID
  void Disconnect(int handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                [handle](const SlotInfo &info) {
                                  return info.id == handle;
                                }),
                 slots_.end());
  }

  /// @brief すべての購読を解除
  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    slots_.clear();
  }

  /// @brief シグナルを発行
  void Emit(Args... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &info : slots_) {
      if (info.slot) {
        info.slot(args...);
      }
    }
  }

private:
  struct SlotInfo {
    int id;
    Slot slot;
  };

  std::vector<SlotInfo> slots_;
  int next_id_ = 1;
  std::mutex mutex_;
};

} // namespace Shared::Core
