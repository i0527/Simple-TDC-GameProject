#include "Core/HotReloadSystem.h"
#include <iostream>

namespace fs = std::filesystem;
using namespace std::chrono;

namespace Core {

void HotReloadSystem::Initialize(const std::string& watchPath, float pollIntervalMs) {
    watchPath_ = watchPath;
    pollIntervalMs_ = pollIntervalMs;
    lastCheckTime_ = high_resolution_clock::now();
    
    if (!fs::exists(watchPath_)) {
        std::cerr << "HotReloadSystem: Watch path does not exist: " << watchPath_ << "\n";
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(watchPath_)) {
        if (entry.is_regular_file()) {
            auto path = entry.path().string();
            auto ext = entry.path().extension().string();
            if (ext == ".json" || ext == ".cfg" || ext == ".txt") {
                fileTimes_[path] = fs::last_write_time(entry.path());
            }
        }
    }

    std::cout << "HotReloadSystem initialized for: " << watchPath_ << " (poll interval: " << pollIntervalMs_ << "ms)\n";
}

void HotReloadSystem::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    fileTimes_.clear();
    callbacks_.clear();
}

void HotReloadSystem::Update() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - lastCheckTime_).count();
    
    if (elapsed < static_cast<long long>(pollIntervalMs_)) {
        return;
    }
    
    lastCheckTime_ = now;
    CheckFileChanges();
}

void HotReloadSystem::RegisterCallback(const std::string& pattern, ReloadCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[pattern].push_back(callback);
}

void HotReloadSystem::SetPollInterval(float milliseconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    pollIntervalMs_ = milliseconds;
}

void HotReloadSystem::CheckFileChanges() {
    if (!fs::exists(watchPath_)) return;

    for (const auto& entry : fs::recursive_directory_iterator(watchPath_)) {
        if (!entry.is_regular_file()) continue;

        auto path = entry.path().string();
        auto newTime = fs::last_write_time(entry.path());

        auto it = fileTimes_.find(path);
        if (it == fileTimes_.end()) {
            fileTimes_[path] = newTime;
            NotifyCallbacks(path);
        } else if (it->second != newTime) {
            it->second = newTime;
            NotifyCallbacks(path);
        }
    }
}

void HotReloadSystem::NotifyCallbacks(const std::string& filepath) {
    for (const auto& [pattern, callbackList] : callbacks_) {
        if (MatchPattern(filepath, pattern)) {
            for (const auto& callback : callbackList) {
                try {
                    callback(filepath);
                } catch (const std::exception& e) {
                    std::cerr << "HotReloadSystem: Callback error: " << e.what() << "\n";
                }
            }
        }
    }
}

bool HotReloadSystem::MatchPattern(const std::string& filepath, const std::string& pattern) const {
    if (pattern.empty()) return false;
    
    if (pattern[0] == '*') {
        return filepath.find(pattern.substr(1)) != std::string::npos;
    }
    
    return filepath.find(pattern) != std::string::npos;
}

} // namespace Core

