/**
 * @file HTTPServer.h
 * @brief HTTPサーバー管理クラス
 * 
 * Phase 4: REST APIとホットリロード機能の基盤
 * cpp-httplibを使用したHTTPサーバー実装
 */
#pragma once

#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"
#include "Data/Definitions/CharacterDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/UILayoutDef.h"
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <cstdint>

// 前方宣言
namespace httplib {
    struct Request;
    struct Response;
}

namespace Core {

/**
 * @brief ログレベル
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief リクエスト情報
 */
struct RequestInfo {
    std::string requestId;
    std::string method;
    std::string path;
    std::string clientIp;
    std::chrono::steady_clock::time_point startTime;
    int statusCode = 0;
    size_t responseSize = 0;
    std::chrono::milliseconds duration{0};
};

/**
 * @brief レート制限情報
 */
struct RateLimitInfo {
    std::vector<std::chrono::steady_clock::time_point> requestTimes;
    std::chrono::steady_clock::time_point windowStart;
    int requestCount = 0;
};

/**
 * @brief パフォーマンス統計情報
 */
struct PerformanceStats {
    uint64_t totalRequests = 0;
    uint64_t successfulRequests = 0;
    uint64_t failedRequests = 0;
    std::chrono::milliseconds totalResponseTime{0};
    std::chrono::milliseconds minResponseTime{INT_MAX};
    std::chrono::milliseconds maxResponseTime{0};
    std::chrono::steady_clock::time_point startTime;
    std::unordered_map<std::string, uint64_t> requestsByMethod;
    std::unordered_map<std::string, uint64_t> requestsByPath;
    std::unordered_map<int, uint64_t> requestsByStatusCode;
};

/**
 * @brief HTTPサーバー管理クラス
 * 
 * REST APIを提供し、データ定義のCRUD操作と
 * ホットリロード機能を提供する。
 */
class HTTPServer {
public:
    HTTPServer();
    ~HTTPServer();

    // コピー禁止
    HTTPServer(const HTTPServer&) = delete;
    HTTPServer& operator=(const HTTPServer&) = delete;

    /**
     * @brief サーバーを初期化
     * @param port リスニングポート（デフォルト: 8080）
     * @param definitionsPath 定義ファイルのパス
     * @param registry 定義レジストリへの参照
     * @param loader 定義ローダーへの参照
     * @return 成功時true
     */
    bool Initialize(
        int port = 8080,
        const std::string& definitionsPath = "assets/definitions",
        Data::DefinitionRegistry* registry = nullptr,
        Data::DefinitionLoader* loader = nullptr
    );

    /**
     * @brief サーバーを開始（別スレッドで実行）
     */
    void Start();

    /**
     * @brief サーバーを停止
     */
    void Stop();

    /**
     * @brief サーバーが動作中か確認
     */
    bool IsRunning() const { return running_; }

    /**
     * @brief ポート番号を取得
     */
    int GetPort() const { return port_; }

    /**
     * @brief エラーハンドラーを設定
     */
    void SetErrorHandler(std::function<void(const std::string&)> handler) {
        errorHandler_ = std::move(handler);
    }

    /**
     * @brief ファイル変更通知のコールバックを設定
     * 
     * ホットリロード時に呼び出されるコールバック
     */
    void SetFileChangedCallback(std::function<void(const std::string&)> callback) {
        fileChangedCallback_ = std::move(callback);
    }

    /**
     * @brief WebSocket接続されたクライアントに通知を送信
     * 
     * UIエディター用のリアルタイム更新通知
     * @param eventType イベントタイプ（"file_changed", "reload"等）
     * @param data 追加データ（JSON文字列）
     */
    void BroadcastToClients(const std::string& eventType, const std::string& data = "");

    /**
     * @brief ロギングを有効化/無効化
     */
    void SetLoggingEnabled(bool enabled) { loggingEnabled_ = enabled; }

    /**
     * @brief ログレベルを設定
     */
    void SetLogLevel(LogLevel level) { logLevel_ = level; }

    /**
     * @brief レート制限を有効化/無効化
     */
    void SetRateLimitEnabled(bool enabled) { rateLimitEnabled_ = enabled; }

    /**
     * @brief レート制限の設定（1分間あたりのリクエスト数）
     */
    void SetRateLimit(int requestsPerMinute) { rateLimitPerMinute_ = requestsPerMinute; }

    /**
     * @brief リクエストボディサイズ制限を設定（バイト単位）
     * @param maxSize 最大サイズ（デフォルト: 10MB）
     */
    void SetMaxBodySize(size_t maxSize) { maxBodySize_ = maxSize; }

    /**
     * @brief リクエストタイムアウトを設定（秒単位）
     * @param timeoutSeconds タイムアウト時間（デフォルト: 30秒）
     */
    void SetRequestTimeout(int timeoutSeconds) { requestTimeoutSeconds_ = timeoutSeconds; }

    /**
     * @brief 開発モードを有効化/無効化
     * 開発モードではセキュリティ制限が緩和され、詳細なログが出力されます
     */
    void SetDevelopmentMode(bool enabled) { 
        developmentMode_ = enabled;
        if (enabled) {
            // 開発モードではセキュリティ制限を緩和
            rateLimitEnabled_ = false;
            maxBodySize_ = 100 * 1024 * 1024;  // 100MB
            requestTimeoutSeconds_ = 300;  // 5分
            loggingEnabled_ = true;
            logLevel_ = LogLevel::DEBUG;
        }
    }

    /**
     * @brief 開発モードが有効か確認
     */
    bool IsDevelopmentMode() const { return developmentMode_; }

    /**
     * @brief パフォーマンス統計を取得
     */
    nlohmann::json GetPerformanceStats() const;

    /**
     * @brief パフォーマンス統計をリセット
     */
    void ResetPerformanceStats();

private:
    /**
     * @brief REST APIルートを設定
     */
    void SetupRoutes();

    /**
     * @brief サーバースレッドのエントリーポイント
     */
    void ServerThread();

    /**
     * @brief キャラクター定義API
     */
    void SetupCharacterRoutes();
    
    /**
     * @brief ステージ定義API
     */
    void SetupStageRoutes();
    
    /**
     * @brief UIレイアウト定義API
     */
    void SetupUIRoutes();

    /**
     * @brief WebSocket接続を設定
     */
    void SetupWebSocket();

    /**
     * @brief バッチ操作APIを設定
     */
    void SetupBatchRoutes();

    /**
     * @brief 検索・フィルタリングAPIを設定
     */
    void SetupSearchRoutes();

    /**
     * @brief エクスポート/インポートAPIを設定
     */
    void SetupExportImportRoutes();

    /**
     * @brief 統計情報APIを設定
     */
    void SetupStatsRoutes();

    /**
     * @brief 設定APIを設定
     */
    void SetupConfigRoutes();

    /**
     * @brief ファイル監視スレッドのエントリーポイント
     */
    void FileWatcherThread();

    /**
     * @brief ファイル変更をチェック
     */
    void CheckFileChanges();

    /**
     * @brief リクエストIDを生成
     */
    std::string GenerateRequestId();

    /**
     * @brief ログを記録
     */
    void Log(LogLevel level, const std::string& message, const std::string& requestId = "");

    /**
     * @brief リクエストログを記録
     */
    void LogRequest(const std::string& method, const std::string& path, const std::string& clientIp, const std::string& requestId);

    /**
     * @brief レスポンスログを記録
     */
    void LogResponse(const RequestInfo& info);

    /**
     * @brief エラーログを記録
     */
    void LogError(const std::string& message, const std::string& requestId = "", const std::string& details = "");

    /**
     * @brief ログレベル名を取得
     */
    std::string GetLogLevelName(LogLevel level) const;

    /**
     * @brief タイムスタンプ文字列を生成
     */
    std::string GetTimestamp() const;

    /**
     * @brief レート制限をチェック
     * @param clientIp クライアントIPアドレス
     * @return レート制限超過時false、許可時true
     */
    bool CheckRateLimit(const std::string& clientIp);

    /**
     * @brief レート制限情報をクリーンアップ（古いエントリを削除）
     */
    void CleanupRateLimitInfo();

    /**
     * @brief リクエストボディサイズをチェック
     * @param bodySize リクエストボディサイズ
     * @return サイズ超過時false、許可時true
     */
    bool CheckBodySize(size_t bodySize) const;

    /**
     * @brief 詳細なエラーレスポンスを作成（開発モード用）
     */
    nlohmann::json CreateDetailedErrorResponse(
        int status,
        const std::string& error,
        const std::string& details,
        const std::string& requestId,
        const std::string& file = "",
        int line = 0,
        const std::string& stackTrace = ""
    ) const;

    /**
     * @brief 例外から詳細なエラー情報を抽出（開発モード用）
     */
    nlohmann::json ExtractExceptionDetails(const std::exception& e, const std::string& requestId) const;

private:
    int port_ = 8080;
    std::string definitionsPath_;
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;

    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> serverThread_;
    std::unique_ptr<std::thread> fileWatcherThread_;
    
    // cpp-httplibのサーバーインスタンス（実装詳細は.cppに隠蔽）
    class Impl;
    std::unique_ptr<Impl> impl_;

    std::function<void(const std::string&)> errorHandler_;
    std::function<void(const std::string&)> fileChangedCallback_;

    // ファイル監視用
    std::mutex fileModificationTimeMutex_;
    std::unordered_map<std::string, std::filesystem::file_time_type> fileModificationTimes_;

    // WebSocket接続管理用
    std::mutex clientsMutex_;
    std::vector<void*> websocketClients_;  // httplib::WebSocket*を格納

    // ロギング用
    bool loggingEnabled_ = true;
    LogLevel logLevel_ = LogLevel::INFO;
    std::mutex logMutex_;
    std::atomic<uint64_t> requestIdCounter_{0};
    std::unordered_map<std::string, RequestInfo> activeRequests_;
    std::mutex activeRequestsMutex_;

    // レート制限用
    bool rateLimitEnabled_ = true;
    int rateLimitPerMinute_ = 100;  // デフォルト: 1分間に100リクエスト
    std::mutex rateLimitMutex_;
    std::unordered_map<std::string, RateLimitInfo> rateLimitInfo_;
    std::chrono::steady_clock::time_point lastCleanupTime_;

    // セキュリティ・パフォーマンス設定
    size_t maxBodySize_ = 10 * 1024 * 1024;  // デフォルト: 10MB
    int requestTimeoutSeconds_ = 30;  // デフォルト: 30秒

    // 開発モード設定
    bool developmentMode_ = false;  // デフォルト: 本番モード

    // パフォーマンスモニタリング用
    mutable std::mutex performanceStatsMutex_;
    PerformanceStats performanceStats_;

} // namespace Core

