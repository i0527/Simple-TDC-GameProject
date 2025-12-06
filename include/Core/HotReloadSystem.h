#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <filesystem>
#include <mutex>
#include <chrono>

namespace Core {

class HotReloadSystem {
public:
    using ReloadCallback = std::function<void(const std::string&)>;

    HotReloadSystem() = default;
    ~HotReloadSystem() = default;

    void Initialize(const std::string& watchPath, float pollIntervalMs = 500.0f);
    void Shutdown();
    void Update();

    void RegisterCallback(const std::string& pattern, ReloadCallback callback);
    void SetPollInterval(float milliseconds);

private:
    std::string watchPath_;
    std::map<std::string, std::filesystem::file_time_type> fileTimes_;
    std::map<std::string, std::vector<ReloadCallback>> callbacks_;
    std::mutex mutex_;
    
    float pollIntervalMs_ = 500.0f;
    std::chrono::high_resolution_clock::time_point lastCheckTime_;

    void CheckFileChanges();
    void NotifyCallbacks(const std::string& filepath);
    bool MatchPattern(const std::string& filepath, const std::string& pattern) const;
};

} // namespace Core

