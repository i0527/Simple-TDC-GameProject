#include "Core/GameContext.h"
#include "Core/EventSystem.h"
#include "Core/FileWatcher.h"
#include "Core/SettingsManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


namespace Shared::Core {

bool GameContext::Initialize(const std::string &config_path) {
  try {
    std::ifstream file(config_path);
    if (!file.is_open()) {
      std::cerr << "Failed to open config file: " << config_path << std::endl;
      return false;
    }

    nlohmann::json config = nlohmann::json::parse(file);

    // パス設定を読み込み
    data_path_ = config.value("data_path", "assets/definitions");
    assets_path_ = config.value("assets_path", "assets");
    main_characters_path_ =
        config.value("main_characters_path", "assets/mainCharacters");
    sub_characters_path_ =
        config.value("sub_characters_path", "assets/subCharacters");

    // サブシステムを初期化
    event_system_ = std::make_unique<EventSystem>();
    file_watcher_ = std::make_unique<FileWatcher>();
    settings_manager_ = std::make_unique<SettingsManager>();

    // 設定ファイル読み込み（存在しなくてもデフォルトで続行）
    settings_manager_->ResetToDefaults();
    std::string settings_path = "saves/settings.json";
    try {
      if (!std::filesystem::exists("saves")) {
        std::filesystem::create_directories("saves");
      }
    } catch (const std::exception &e) {
      std::cerr << "[GameContext] Failed to prepare saves directory: "
                << e.what() << std::endl;
    }
    settings_manager_->Load(settings_path);

    std::cout << "GameContext initialized successfully" << std::endl;
    std::cout << "  Data path: " << data_path_ << std::endl;
    std::cout << "  Assets path: " << assets_path_ << std::endl;
    std::cout << "  Settings path: " << settings_path << std::endl;

    return true;

  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "JSON parse error in config file: " << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error initializing GameContext: " << e.what() << std::endl;
    return false;
  }
}

void GameContext::Shutdown() {
  if (file_watcher_) {
    file_watcher_->ClearAll();
  }
  if (event_system_) {
    event_system_->ClearAll();
  }

  file_watcher_.reset();
  settings_manager_.reset();
  event_system_.reset();

  std::cout << "GameContext shutdown complete" << std::endl;
}

std::string GameContext::GetDataPath(const std::string &relative_path) const {
  if (relative_path.empty()) {
    return data_path_;
  }
  return data_path_ + "/" + relative_path;
}

std::string GameContext::GetAssetsPath(const std::string &relative_path) const {
  if (relative_path.empty()) {
    return assets_path_;
  }
  return assets_path_ + "/" + relative_path;
}

EventSystem &GameContext::GetEventSystem() { return *event_system_; }

const EventSystem &GameContext::GetEventSystem() const {
  return *event_system_;
}

FileWatcher &GameContext::GetFileWatcher() { return *file_watcher_; }

const FileWatcher &GameContext::GetFileWatcher() const {
  return *file_watcher_;
}

SettingsManager &GameContext::GetSettingsManager() {
  return *settings_manager_;
}

const SettingsManager &GameContext::GetSettingsManager() const {
  return *settings_manager_;
}

} // namespace Shared::Core
