#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Shared::Core {

/// @brief ファイル変更監視システム
/// @details ファイルの変更を検知してコールバックを実行
class FileWatcher {
public:
    FileWatcher() = default;
    ~FileWatcher() = default;

    /// @brief ファイルを監視対象に追加
    /// @param path 監視するファイルパス
    /// @param callback ファイル変更時に呼ばれるコールバック
    void Watch(const std::string& path, std::function<void()> callback);

    /// @brief 監視対象からファイルを削除
    /// @param path 監視を解除するファイルパス
    void Unwatch(const std::string& path);

    /// @brief 変更チェック（毎フレーム呼び出し）
    /// @details 監視対象ファイルの更新日時をチェックし、変更があればコールバックを実行
    void CheckChanges();

    /// @brief すべての監視を解除
    void ClearAll();

private:
    struct WatchedFile {
        std::string path;
        std::filesystem::file_time_type last_write_time;
        std::function<void()> on_changed;
    };

    std::vector<WatchedFile> watched_files_;
};

} // namespace Shared::Core
