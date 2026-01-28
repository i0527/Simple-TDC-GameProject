#include "../BaseSystemAPI.hpp"

// 標準ライブラリ
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

// 外部ライブラリ
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#endif

namespace game {
namespace core {

void BaseSystemAPI::SetLogPath(const std::string &directory,
                               const std::string &filename) {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
  if (logInitialized_) {
    std::cerr << "BaseSystemAPI::SetLogPath: Cannot change log path after "
                 "initialization"
              << std::endl;
    return;
  }
  if (directory.empty() || filename.empty()) {
    std::cerr
        << "BaseSystemAPI::SetLogPath: Directory and filename cannot be empty"
        << std::endl;
    return;
  }
  logDirectory_ = directory;
  logFileName_ = filename;
#else
  (void)directory;
  (void)filename;
#endif
}

void BaseSystemAPI::SetLogLevel(LogLevel level) {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
  if (logger_) {
    logger_->set_level(level);
    spdlog::set_level(level);
  }
#else
  (void)level;
#endif
}

void BaseSystemAPI::InitializeLogSystem() {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
  if (logInitialized_) {
    return;
  }

  try {
    std::filesystem::path logDir = logDirectory_;
    if (!std::filesystem::exists(logDir)) {
      try {
        std::filesystem::create_directories(logDir);
      } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "BaseSystemAPI::InitializeLogSystem: Failed to create log "
                     "directory '"
                  << logDirectory_ << "': " << e.what() << std::endl;
        throw;
      }
    }

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink = nullptr;
    std::vector<spdlog::sink_ptr> sinks{consoleSink};

    try {
      std::filesystem::path logFilePath = logDir / logFileName_;
      fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          logFilePath.string(), true);
      fileSink->set_level(spdlog::level::trace);
      fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
      sinks.push_back(fileSink);
    } catch (const std::exception &e) {
      std::cerr
          << "BaseSystemAPI::InitializeLogSystem: Failed to create file sink '"
          << (logDir / logFileName_).string() << "': " << e.what()
          << ". Falling back to console-only logging." << std::endl;
    }

    logger_ = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(),
                                               sinks.end());
    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::warn);

    spdlog::set_default_logger(logger_);

    spdlog::set_level(spdlog::level::trace);

    spdlog::set_error_handler([](const std::string &msg) {
      std::cerr << "spdlog error: " << msg << std::endl;
    });

    logInitialized_ = true;
    logger_->info(
        "BaseSystemAPI: Log system initialized (directory: {}, file: {})",
        logDirectory_, logFileName_);
  } catch (const std::exception &e) {
    std::cerr << "BaseSystemAPI::InitializeLogSystem: Failed to initialize log "
                 "system: "
              << e.what() << std::endl;
    throw;
  }
#else
  // Emscriptenビルド: ログシステムは無効化（パフォーマンス最適化）
  logInitialized_ = true;
#endif
}

void BaseSystemAPI::ShutdownLogSystem() {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
  if (!logInitialized_) {
    return;
  }

  if (logger_) {
    try {
      logger_->info("BaseSystemAPI: Log system shutting down");
      logger_->flush();
    } catch (const std::exception &e) {
      std::cerr << "BaseSystemAPI::ShutdownLogSystem: Error during flush: "
                << e.what() << std::endl;
    }
  }

  spdlog::shutdown();
  logger_.reset();
  logInitialized_ = false;
#else
  logInitialized_ = false;
#endif
}

} // namespace core
} // namespace game
