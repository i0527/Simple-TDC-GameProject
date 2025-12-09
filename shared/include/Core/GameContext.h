#pragma once

#include "Core/EventSystem.h"
#include "Core/FileWatcher.h"
#include "Core/SettingsManager.h"
#include <memory>
#include <string>

namespace Shared::Core {

/// @brief ゲーム全体のコンテキストを管理
/// @details パス管理、イベントシステム、ファイル監視を一元化
class GameContext {
public:
  GameContext() = default;
  ~GameContext() = default;

  /// @brief GameContextを初期化
  /// @param config_path 設定ファイルのパス（assets/config.json）
  /// @return 初期化成功時 true
  bool Initialize(const std::string &config_path);

  /// @brief GameContextをシャットダウン
  void Shutdown();

  // パス管理
  /// @brief データファイルの絶対パスを取得
  /// @param relative_path 相対パス（例: "definitions/entities_debug.json"）
  /// @return 絶対パス
  std::string GetDataPath(const std::string &relative_path) const;

  /// @brief アセットファイルの絶対パスを取得
  /// @param relative_path 相対パス（例: "fonts/mplus.ttf"）
  /// @return 絶対パス
  std::string GetAssetsPath(const std::string &relative_path) const;

  // マネージャーアクセス
  EventSystem &GetEventSystem();
  const EventSystem &GetEventSystem() const;

  FileWatcher &GetFileWatcher();
  const FileWatcher &GetFileWatcher() const;

  SettingsManager &GetSettingsManager();
  const SettingsManager &GetSettingsManager() const;

private:
  std::string data_path_;
  std::string assets_path_;

  std::unique_ptr<EventSystem> event_system_;
  std::unique_ptr<FileWatcher> file_watcher_;
  std::unique_ptr<SettingsManager> settings_manager_;
};

} // namespace Shared::Core
