#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>

namespace New::Game::Editor {

class HotReloadSystem {
public:
  using Callback = std::function<void(const std::string &)>;
  using BatchCallback = std::function<void(const std::vector<std::string> &)>;

  HotReloadSystem() = default;
  ~HotReloadSystem() = default;

  // path: 監視対象ファイル or ディレクトリ
  void AddWatch(const std::string &path, Callback callback);
  // 全監視対象をクリア
  void Clear();
  // ポーリング呼び出し（例: 毎フレームまたは一定間隔）
  void Update();
  // 変更があったパスをまとめて通知するバッチコールバック
  void SetBatchCallback(BatchCallback callback) { batchCallback_ = callback; }
  // ポーリング間隔(ms)を設定
  void SetIntervalMs(int ms) { intervalMs_ = ms < 0 ? 0 : ms; }
  // 監視対象が存在しない場合の警告を抑止する
  void SetSuppressMissingWarnings(bool suppress) {
    suppressMissingWarnings_ = suppress;
  }

private:
  struct WatchEntry {
    std::string path;
    Callback callback;
    long long lastWriteTime = 0;
    bool isDirectory = false;
  };

  std::unordered_map<std::string, WatchEntry> watches_;
  BatchCallback batchCallback_{};
  int intervalMs_ = 500;
  std::chrono::steady_clock::time_point nextCheckTime_{
      std::chrono::steady_clock::now()};
  bool suppressMissingWarnings_ = false;

  long long GetLastWriteTime(const std::string &path, bool isDirectory) const;
};

} // namespace New::Game::Editor
