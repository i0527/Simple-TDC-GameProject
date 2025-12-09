#include "Core/FileWatcher.h"
#include <algorithm>
#include <iostream>

namespace Shared::Core {

void FileWatcher::Watch(const std::string& path, std::function<void()> callback) {
    // 既存の監視を削除
    Unwatch(path);

    try {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Warning: File does not exist: " << path << std::endl;
            // ファイルが存在しなくても監視対象に追加（後で作成される可能性）
        }

        WatchedFile wf;
        wf.path = path;
        wf.on_changed = std::move(callback);

        if (std::filesystem::exists(path)) {
            wf.last_write_time = std::filesystem::last_write_time(path);
        }

        watched_files_.push_back(std::move(wf));

    } catch (const std::exception& e) {
        std::cerr << "Error watching file '" << path << "': " << e.what() << std::endl;
    }
}

void FileWatcher::Unwatch(const std::string& path) {
    watched_files_.erase(
        std::remove_if(watched_files_.begin(), watched_files_.end(),
                      [&path](const WatchedFile& wf) { return wf.path == path; }),
        watched_files_.end()
    );
}

void FileWatcher::CheckChanges() {
    for (auto& wf : watched_files_) {
        try {
            if (!std::filesystem::exists(wf.path)) {
                continue;
            }

            auto current_time = std::filesystem::last_write_time(wf.path);

            // 初回チェック時は last_write_time を設定
            if (wf.last_write_time == std::filesystem::file_time_type{}) {
                wf.last_write_time = current_time;
                continue;
            }

            // 変更を検知
            if (current_time != wf.last_write_time) {
                wf.last_write_time = current_time;
                
                if (wf.on_changed) {
                    wf.on_changed();
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Error checking file '" << wf.path << "': " << e.what() << std::endl;
        }
    }
}

void FileWatcher::ClearAll() {
    watched_files_.clear();
}

} // namespace Shared::Core
