#include "Game/Editor/HotReload/HotReloadSystem.h"

#include <filesystem>
#include <vector>

#include "Core/TraceCompat.h"

namespace fs = std::filesystem;

namespace New::Game::Editor {

void HotReloadSystem::AddWatch(const std::string &path, Callback callback) {
  const auto p = fs::path(path);
  const bool isDir = fs::exists(p) && fs::is_directory(p);
  WatchEntry entry{path, std::move(callback), GetLastWriteTime(path, isDir),
                   isDir};
  watches_[path] = std::move(entry);
  TRACELOG(LOG_INFO, "HotReload: watch added: %s", path.c_str());
}

void HotReloadSystem::Clear() { watches_.clear(); }

void HotReloadSystem::Update() {
  const auto now = std::chrono::steady_clock::now();
  if (now < nextCheckTime_) {
    return;
  }
  nextCheckTime_ = now + std::chrono::milliseconds(intervalMs_);

  std::vector<std::string> changed;
  for (auto &pair : watches_) {
    auto &entry = pair.second;
    const long long current = GetLastWriteTime(entry.path, entry.isDirectory);
    if (current <= 0) {
      if (!suppressMissingWarnings_) {
        TRACELOG(LOG_WARNING, "HotReload: missing path: %s",
                 entry.path.c_str());
      }
      continue;
    }
    if (entry.lastWriteTime == 0) {
      entry.lastWriteTime = current;
      TRACELOG(LOG_INFO, "HotReload: initial timestamp: %s",
               entry.path.c_str());
      continue;
    }
    if (current > entry.lastWriteTime) {
      entry.lastWriteTime = current;
      TRACELOG(LOG_INFO, "HotReload: change detected: %s", entry.path.c_str());
      if (entry.callback) {
        entry.callback(entry.path);
      }
      changed.push_back(entry.path);
    }
  }
  if (!changed.empty() && batchCallback_) {
    batchCallback_(changed);
  }
}

long long HotReloadSystem::GetLastWriteTime(const std::string &path,
                                            bool isDirectory) const {
  try {
    const auto p = fs::path(path);
    if (!fs::exists(p)) {
      return 0;
    }
    if (!isDirectory) {
      const auto time = fs::last_write_time(p).time_since_epoch();
      return std::chrono::duration_cast<std::chrono::milliseconds>(time)
          .count();
    }

    long long latest = 0;
    for (const auto &entry : fs::directory_iterator(p)) {
      const auto time = entry.last_write_time().time_since_epoch();
      const long long ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
      if (ms > latest) {
        latest = ms;
      }
    }
    // fallback to directory time if no children
    if (latest == 0) {
      const auto time = fs::last_write_time(p).time_since_epoch();
      latest =
          std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    }
    return latest;
  } catch (const std::exception &e) {
    TRACELOG(LOG_WARNING, "HotReload: failed to stat %s : %s", path.c_str(),
             e.what());
    return 0;
  }
}

} // namespace New::Game::Editor
