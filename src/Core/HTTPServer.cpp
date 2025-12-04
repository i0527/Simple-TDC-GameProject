/**
 * @file HTTPServer.cpp
 * @brief HTTPサーバー管理クラスの実装
 */

#include "Core/HTTPServer.h"
#include "Data/Loaders/CharacterLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/UILoader.h"
#include "Data/Serializers/UISerializer.h"
#include "Data/Serializers/CharacterSerializer.h"
#include "Data/Serializers/StageSerializer.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <climits>

namespace Core {

namespace fs = std::filesystem;

// SSEクライアント接続情報
struct SSEClient {
    std::shared_ptr<httplib::Response> response;
    std::atomic<bool> active{true};
};

// cpp-httplibの実装詳細を隠蔽するためのImplクラス
class HTTPServer::Impl {
public:
    std::unique_ptr<httplib::Server> server;
    
    // SSE接続管理（リアルタイム更新用）
    std::vector<std::shared_ptr<SSEClient>> sseClients;
    std::mutex connectionsMutex;
    
    Impl() : server(std::make_unique<httplib::Server>()) {}
};

HTTPServer::HTTPServer()
    : impl_(std::make_unique<Impl>())
    , lastCleanupTime_(std::chrono::steady_clock::now())
{
    // パフォーマンス統計の初期化
    performanceStats_.startTime = std::chrono::steady_clock::now();
    performanceStats_.minResponseTime = std::chrono::milliseconds(INT_MAX);
}

HTTPServer::~HTTPServer() {
    Stop();
}

bool HTTPServer::Initialize(
    int port,
    const std::string& definitionsPath,
    Data::DefinitionRegistry* registry,
    Data::DefinitionLoader* loader
) {
    if (running_) {
        std::cerr << "HTTPServer: Already running\n";
        return false;
    }

    port_ = port;
    definitionsPath_ = definitionsPath;
    registry_ = registry;
    loader_ = loader;

    if (!registry_ || !loader_) {
        std::cerr << "HTTPServer: Registry or Loader not provided\n";
        return false;
    }

    if (!fs::exists(definitionsPath_)) {
        std::cerr << "HTTPServer: Definitions path does not exist: " << definitionsPath_ << "\n";
        return false;
    }

    SetupRoutes();

    // タイムアウト設定
    impl_->server->set_read_timeout(requestTimeoutSeconds_, 0);  // 読み込みタイムアウト（秒、マイクロ秒）
    impl_->server->set_write_timeout(requestTimeoutSeconds_, 0);  // 書き込みタイムアウト
    impl_->server->set_idle_interval(requestTimeoutSeconds_, 0);  // アイドルタイムアウト

    // ファイル監視の初期化
    fileModificationTimes_.clear();
    for (const auto& entry : fs::recursive_directory_iterator(definitionsPath_)) {
        if (entry.is_regular_file() && 
            (entry.path().extension() == ".json" || 
             entry.path().extension() == ".character.json" ||
             entry.path().extension() == ".stage.json" ||
             entry.path().extension() == ".ui.json")) {
            std::lock_guard<std::mutex> lock(fileModificationTimeMutex_);
            fileModificationTimes_[entry.path().string()] = fs::last_write_time(entry.path());
        }
    }

    std::cout << "HTTPServer: Initialized on port " << port_ << "\n";
    return true;
}

void HTTPServer::Start() {
    if (running_) {
        std::cerr << "HTTPServer: Already running\n";
        return;
    }

    running_ = true;
    serverThread_ = std::make_unique<std::thread>(&HTTPServer::ServerThread, this);
    fileWatcherThread_ = std::make_unique<std::thread>(&HTTPServer::FileWatcherThread, this);

    std::cout << "HTTPServer: Started on port " << port_ << "\n";
    std::cout << "HTTPServer: API available at http://localhost:" << port_ << "/api\n";
}

void HTTPServer::Stop() {
    if (!running_) return;

    running_ = false;

    if (impl_->server) {
        impl_->server->stop();
    }

    if (serverThread_ && serverThread_->joinable()) {
        serverThread_->join();
    }

    if (fileWatcherThread_ && fileWatcherThread_->joinable()) {
        fileWatcherThread_->join();
    }

    std::cout << "HTTPServer: Stopped\n";
}

void HTTPServer::SetupRoutes() {
    // CORS設定とロギングを全エンドポイントに適用するミドルウェア
    impl_->server->set_pre_routing_handler([this](const httplib::Request& req, httplib::Response& res) {
        // リクエストIDを生成
        std::string requestId = GenerateRequestId();
        res.set_header("X-Request-ID", requestId);
        
        // リクエストボディサイズチェック（開発モードでは緩和、OPTIONSリクエストは除外）
        if (!developmentMode_ && req.method != "OPTIONS") {
            size_t bodySize = req.body.size();
            if (!CheckBodySize(bodySize)) {
                res.status = 413;  // Payload Too Large
                res.set_header("X-Max-Body-Size", std::to_string(maxBodySize_));
                
                nlohmann::json errorResponse;
                errorResponse["error"] = "Request body too large";
                errorResponse["status"] = 413;
                errorResponse["message"] = "Request body exceeds maximum allowed size";
                errorResponse["maxSize"] = maxBodySize_;
                errorResponse["receivedSize"] = bodySize;
                errorResponse["requestId"] = requestId;
                errorResponse["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();
                
                res.set_content(errorResponse.dump(), "application/json");
                
                // エラーログ記録
                LogError("Request body too large", requestId, 
                    "Size: " + std::to_string(bodySize) + " bytes, Max: " + std::to_string(maxBodySize_) + " bytes");
                
                return httplib::Server::HandlerResponse::Handled;
            }
        }
        
        // レート制限チェック（OPTIONSリクエストは除外）
        if (rateLimitEnabled_ && req.method != "OPTIONS") {
            if (!CheckRateLimit(req.remote_addr)) {
                res.status = 429;  // Too Many Requests
                res.set_header("Retry-After", "60");  // 60秒後に再試行
                res.set_header("X-RateLimit-Limit", std::to_string(rateLimitPerMinute_));
                res.set_header("X-RateLimit-Remaining", "0");
                
                auto now = std::chrono::steady_clock::now();
                auto resetTime = now + std::chrono::minutes(1);
                auto resetSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                    resetTime.time_since_epoch()).count();
                res.set_header("X-RateLimit-Reset", std::to_string(resetSeconds));
                
                nlohmann::json errorResponse;
                errorResponse["error"] = "Rate limit exceeded";
                errorResponse["status"] = 429;
                errorResponse["message"] = "Too many requests. Please try again later.";
                errorResponse["retryAfter"] = 60;
                errorResponse["requestId"] = requestId;
                errorResponse["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();
                
                res.set_content(errorResponse.dump(), "application/json");
                
                // エラーログ記録
                LogError("Rate limit exceeded", requestId, "Client IP: " + req.remote_addr);
                
                return httplib::Server::HandlerResponse::Handled;
            }
        }
        
        // リクエスト情報を保存
        {
            std::lock_guard<std::mutex> lock(activeRequestsMutex_);
            RequestInfo info;
            info.requestId = requestId;
            info.method = req.method;
            info.path = req.path;
            info.clientIp = req.remote_addr;
            info.startTime = std::chrono::steady_clock::now();
            activeRequests_[requestId] = info;
        }
        
        // OPTIONSリクエスト（プリフライト）の処理
        if (req.method == "OPTIONS") {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.set_header("Access-Control-Max-Age", "3600");
            res.status = 200;
            
            // ログ記録
            LogRequest(req.method, req.path, req.remote_addr, requestId);
            return httplib::Server::HandlerResponse::Handled;
        }
        
        // リクエストログ記録
        LogRequest(req.method, req.path, req.remote_addr, requestId);
        
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    // 全レスポンスにCORSヘッダーを追加し、レスポンスログを記録
    impl_->server->set_post_routing_handler([this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        // レート制限ヘッダーを追加（OPTIONSリクエストは除外）
        if (rateLimitEnabled_ && req.method != "OPTIONS") {
            std::lock_guard<std::mutex> lock(rateLimitMutex_);
            auto it = rateLimitInfo_.find(req.remote_addr);
            if (it != rateLimitInfo_.end()) {
                const auto& info = it->second;
                int remaining = rateLimitPerMinute_ - static_cast<int>(info.requestTimes.size());
                if (remaining < 0) remaining = 0;
                
                res.set_header("X-RateLimit-Limit", std::to_string(rateLimitPerMinute_));
                res.set_header("X-RateLimit-Remaining", std::to_string(remaining));
                
                // リセット時刻を計算（1分後の時刻）
                auto now = std::chrono::steady_clock::now();
                auto resetTime = now + std::chrono::minutes(1);
                auto resetSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                    resetTime.time_since_epoch()).count();
                res.set_header("X-RateLimit-Reset", std::to_string(resetSeconds));
            }
        }
        
        // レスポンスログ記録
        std::string requestId = res.get_header_value("X-Request-ID");
        if (!requestId.empty()) {
            std::lock_guard<std::mutex> lock(activeRequestsMutex_);
            auto it = activeRequests_.find(requestId);
            if (it != activeRequests_.end()) {
                RequestInfo& info = it->second;
                info.statusCode = res.status;
                info.responseSize = res.body.size();
                info.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - info.startTime);
                LogResponse(info);
                
                // パフォーマンス統計を更新（OPTIONSリクエストは除外）
                if (req.method != "OPTIONS") {
                    std::lock_guard<std::mutex> perfLock(performanceStatsMutex_);
                    performanceStats_.totalRequests++;
                    if (res.status >= 200 && res.status < 400) {
                        performanceStats_.successfulRequests++;
                    } else {
                        performanceStats_.failedRequests++;
                    }
                    
                    performanceStats_.totalResponseTime += info.duration;
                    if (info.duration < performanceStats_.minResponseTime) {
                        performanceStats_.minResponseTime = info.duration;
                    }
                    if (info.duration > performanceStats_.maxResponseTime) {
                        performanceStats_.maxResponseTime = info.duration;
                    }
                    
                    performanceStats_.requestsByMethod[info.method]++;
                    performanceStats_.requestsByPath[info.path]++;
                    performanceStats_.requestsByStatusCode[res.status]++;
                }
                
                // リクエスト情報を削除
                activeRequests_.erase(it);
            }
        }
        
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    SetupCharacterRoutes();
    SetupStageRoutes();
    SetupUIRoutes();
    SetupWebSocket();
    SetupBatchRoutes();
    SetupSearchRoutes();
    SetupExportImportRoutes();
    SetupStatsRoutes();
    SetupConfigRoutes();
    SetupGameStateRoutes();

    // ヘルスチェック
    impl_->server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // API情報
    impl_->server->Get("/api", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json apiInfo;
        apiInfo["name"] = "Simple TDC Game API";
        apiInfo["version"] = "1.0.0";
        apiInfo["description"] = "REST API for game definition management";
        
        nlohmann::json endpoints;
        endpoints["characters"] = "/api/characters";
        endpoints["stages"] = "/api/stages";
        endpoints["ui"] = "/api/ui";
        endpoints["events"] = "/events";
        endpoints["health"] = "/health";
        endpoints["docs"] = "/api/docs";
        apiInfo["endpoints"] = endpoints;
        
        res.set_content(apiInfo.dump(2), "application/json");
    });

    // GET /api/docs - APIドキュメント
    impl_->server->Get("/api/docs", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json docs;
        docs["title"] = "Simple TDC Game API Documentation";
        docs["version"] = "1.0.0";
        docs["description"] = "REST API for managing game definitions (UI layouts, characters, stages)";
        
        // UIレイアウトAPI
        nlohmann::json uiApi;
        uiApi["base"] = "/api/ui";
        uiApi["description"] = "UI layout definition management";
        uiApi["operations"] = nlohmann::json::array();
        
        nlohmann::json op1;
        op1["method"] = "GET";
        op1["path"] = "/api/ui";
        op1["description"] = "Get all UI layout definitions";
        op1["response"] = "Array of UI layout summaries";
        uiApi["operations"].push_back(op1);
        
        nlohmann::json op2;
        op2["method"] = "GET";
        op2["path"] = "/api/ui/:id";
        op2["description"] = "Get specific UI layout definition";
        op2["response"] = "UI layout definition object";
        uiApi["operations"].push_back(op2);
        
        nlohmann::json op3;
        op3["method"] = "POST";
        op3["path"] = "/api/ui";
        op3["description"] = "Create new UI layout definition";
        op3["request"] = "UI layout definition JSON";
        op3["response"] = "Created UI layout ID";
        uiApi["operations"].push_back(op3);
        
        nlohmann::json op4;
        op4["method"] = "PUT";
        op4["path"] = "/api/ui/:id";
        op4["description"] = "Update UI layout definition";
        op4["request"] = "UI layout definition JSON";
        op4["response"] = "Update success status";
        uiApi["operations"].push_back(op4);
        
        nlohmann::json op5;
        op5["method"] = "DELETE";
        op5["path"] = "/api/ui/:id";
        op5["description"] = "Delete UI layout definition";
        op5["response"] = "Delete success status";
        uiApi["operations"].push_back(op5);
        
        // キャラクターAPI
        nlohmann::json charApi;
        charApi["base"] = "/api/characters";
        charApi["description"] = "Character definition management";
        charApi["operations"] = nlohmann::json::array();
        
        nlohmann::json charOp1;
        charOp1["method"] = "GET";
        charOp1["path"] = "/api/characters";
        charOp1["description"] = "Get all character definitions";
        charOp1["response"] = "Array of character summaries";
        charApi["operations"].push_back(charOp1);
        
        nlohmann::json charOp2;
        charOp2["method"] = "GET";
        charOp2["path"] = "/api/characters/:id";
        charOp2["description"] = "Get specific character definition";
        charOp2["response"] = "Character definition object";
        charApi["operations"].push_back(charOp2);
        
        nlohmann::json charOp3;
        charOp3["method"] = "POST";
        charOp3["path"] = "/api/characters";
        charOp3["description"] = "Create new character definition";
        charOp3["request"] = "Character definition JSON";
        charOp3["response"] = "Created character ID";
        charApi["operations"].push_back(charOp3);
        
        nlohmann::json charOp4;
        charOp4["method"] = "PUT";
        charOp4["path"] = "/api/characters/:id";
        charOp4["description"] = "Update character definition";
        charOp4["request"] = "Character definition JSON";
        charOp4["response"] = "Update success status";
        charApi["operations"].push_back(charOp4);
        
        nlohmann::json charOp5;
        charOp5["method"] = "DELETE";
        charOp5["path"] = "/api/characters/:id";
        charOp5["description"] = "Delete character definition";
        charOp5["response"] = "Delete success status";
        charApi["operations"].push_back(charOp5);
        
        // ステージAPI
        nlohmann::json stageApi;
        stageApi["base"] = "/api/stages";
        stageApi["description"] = "Stage definition management";
        stageApi["operations"] = nlohmann::json::array();
        
        nlohmann::json stageOp1;
        stageOp1["method"] = "GET";
        stageOp1["path"] = "/api/stages";
        stageOp1["description"] = "Get all stage definitions";
        stageOp1["response"] = "Array of stage summaries";
        stageApi["operations"].push_back(stageOp1);
        
        nlohmann::json stageOp2;
        stageOp2["method"] = "GET";
        stageOp2["path"] = "/api/stages/:id";
        stageOp2["description"] = "Get specific stage definition";
        stageOp2["response"] = "Stage definition object";
        stageApi["operations"].push_back(stageOp2);
        
        nlohmann::json stageOp3;
        stageOp3["method"] = "POST";
        stageOp3["path"] = "/api/stages";
        stageOp3["description"] = "Create new stage definition";
        stageOp3["request"] = "Stage definition JSON";
        stageOp3["response"] = "Created stage ID";
        stageApi["operations"].push_back(stageOp3);
        
        nlohmann::json stageOp4;
        stageOp4["method"] = "PUT";
        stageOp4["path"] = "/api/stages/:id";
        stageOp4["description"] = "Update stage definition";
        stageOp4["request"] = "Stage definition JSON";
        stageOp4["response"] = "Update success status";
        stageApi["operations"].push_back(stageOp4);
        
        nlohmann::json stageOp5;
        stageOp5["method"] = "DELETE";
        stageOp5["path"] = "/api/stages/:id";
        stageOp5["description"] = "Delete stage definition";
        stageOp5["response"] = "Delete success status";
        stageApi["operations"].push_back(stageOp5);
        
        // リアルタイム更新API
        nlohmann::json eventsApi;
        eventsApi["base"] = "/events";
        eventsApi["description"] = "Server-Sent Events (SSE) endpoint for real-time updates";
        eventsApi["events"] = nlohmann::json::array();
        
        std::vector<std::pair<std::string, std::string>> eventList = {
            {"ui_created", "UI layout created"},
            {"ui_updated", "UI layout updated"},
            {"ui_deleted", "UI layout deleted"},
            {"ui_reloaded", "UI layout reloaded from file"},
            {"character_created", "Character created"},
            {"character_updated", "Character updated"},
            {"character_deleted", "Character deleted"},
            {"character_reloaded", "Character reloaded from file"},
            {"stage_created", "Stage created"},
            {"stage_updated", "Stage updated"},
            {"stage_deleted", "Stage deleted"},
            {"stage_reloaded", "Stage reloaded from file"},
            {"file_changed", "File changed (detected by file watcher)"}
        };
        
        for (const auto& [name, desc] : eventList) {
            nlohmann::json event;
            event["name"] = name;
            event["description"] = desc;
            eventsApi["events"].push_back(event);
        }
        
        // バッチ操作API
        nlohmann::json batchApi;
        batchApi["base"] = "/api/batch";
        batchApi["description"] = "Batch operations for multiple definitions";
        batchApi["operations"] = nlohmann::json::array();
        
        nlohmann::json batchOp1;
        batchOp1["method"] = "POST";
        batchOp1["path"] = "/api/batch/ui";
        batchOp1["description"] = "Batch create UI layouts";
        batchOp1["request"] = "JSON with items array";
        batchOp1["response"] = "Multi-status response with results";
        batchApi["operations"].push_back(batchOp1);
        
        nlohmann::json batchOp2;
        batchOp2["method"] = "POST";
        batchOp2["path"] = "/api/batch/characters";
        batchOp2["description"] = "Batch create characters";
        batchOp2["request"] = "JSON with items array";
        batchOp2["response"] = "Multi-status response with results";
        batchApi["operations"].push_back(batchOp2);
        
        nlohmann::json batchOp3;
        batchOp3["method"] = "DELETE";
        batchOp3["path"] = "/api/batch/ui";
        batchOp3["description"] = "Batch delete UI layouts";
        batchOp3["request"] = "JSON with ids array";
        batchOp3["response"] = "Multi-status response with results";
        batchApi["operations"].push_back(batchOp3);
        
        nlohmann::json batchOp4;
        batchOp4["method"] = "DELETE";
        batchOp4["path"] = "/api/batch/characters";
        batchOp4["description"] = "Batch delete characters";
        batchOp4["request"] = "JSON with ids array";
        batchOp4["response"] = "Multi-status response with results";
        batchApi["operations"].push_back(batchOp4);
        
        // 検索API
        nlohmann::json searchApi;
        searchApi["base"] = "/api/search";
        searchApi["description"] = "Search and filter definitions";
        searchApi["operations"] = nlohmann::json::array();
        
        nlohmann::json searchOp1;
        searchOp1["method"] = "GET";
        searchOp1["path"] = "/api/search/ui";
        searchOp1["description"] = "Search UI layouts";
        searchOp1["queryParams"] = nlohmann::json::object();
        searchOp1["queryParams"]["q"] = "Search query (optional)";
        searchOp1["queryParams"]["name"] = "Name filter (optional)";
        searchOp1["response"] = "Array of matching UI layouts";
        searchApi["operations"].push_back(searchOp1);
        
        nlohmann::json searchOp2;
        searchOp2["method"] = "GET";
        searchOp2["path"] = "/api/search/characters";
        searchOp2["description"] = "Search characters";
        searchOp2["queryParams"] = nlohmann::json::object();
        searchOp2["queryParams"]["q"] = "Search query (optional)";
        searchOp2["queryParams"]["rarity"] = "Rarity filter (optional)";
        searchOp2["queryParams"]["gameMode"] = "Game mode filter (optional)";
        searchOp2["response"] = "Array of matching characters";
        searchApi["operations"].push_back(searchOp2);
        
        nlohmann::json searchOp3;
        searchOp3["method"] = "GET";
        searchOp3["path"] = "/api/search/stages";
        searchOp3["description"] = "Search stages";
        searchOp3["queryParams"] = nlohmann::json::object();
        searchOp3["queryParams"]["q"] = "Search query (optional)";
        searchOp3["response"] = "Array of matching stages";
        searchApi["operations"].push_back(searchOp3);
        
        // エクスポート/インポートAPI
        nlohmann::json exportImportApi;
        exportImportApi["base"] = "/api/export, /api/import";
        exportImportApi["description"] = "Export and import definitions";
        exportImportApi["operations"] = nlohmann::json::array();
        
        nlohmann::json exportOp1;
        exportOp1["method"] = "GET";
        exportOp1["path"] = "/api/export/ui";
        exportOp1["description"] = "Export all UI layouts";
        exportOp1["response"] = "JSON file download";
        exportImportApi["operations"].push_back(exportOp1);
        
        nlohmann::json exportOp2;
        exportOp2["method"] = "GET";
        exportOp2["path"] = "/api/export/characters";
        exportOp2["description"] = "Export all characters";
        exportOp2["response"] = "JSON file download";
        exportImportApi["operations"].push_back(exportOp2);
        
        nlohmann::json exportOp3;
        exportOp3["method"] = "GET";
        exportOp3["path"] = "/api/export/stages";
        exportOp3["description"] = "Export all stages";
        exportOp3["response"] = "JSON file download";
        exportImportApi["operations"].push_back(exportOp3);
        
        nlohmann::json importOp1;
        importOp1["method"] = "POST";
        importOp1["path"] = "/api/import/ui";
        importOp1["description"] = "Import UI layouts";
        importOp1["request"] = "JSON with items array";
        importOp1["response"] = "Multi-status response with results";
        exportImportApi["operations"].push_back(importOp1);
        
        nlohmann::json importOp2;
        importOp2["method"] = "POST";
        importOp2["path"] = "/api/import/characters";
        importOp2["description"] = "Import characters";
        importOp2["request"] = "JSON with items array";
        importOp2["response"] = "Multi-status response with results";
        exportImportApi["operations"].push_back(importOp2);
        
        // 統計情報API
        nlohmann::json statsApi;
        statsApi["base"] = "/api/stats";
        statsApi["description"] = "Get statistics about definitions";
        statsApi["operations"] = nlohmann::json::array();
        
        nlohmann::json statsOp1;
        statsOp1["method"] = "GET";
        statsOp1["path"] = "/api/stats";
        statsOp1["description"] = "Get comprehensive statistics";
        statsOp1["response"] = "Statistics object (counts, aggregations, server info)";
        statsApi["operations"].push_back(statsOp1);
        
        // 設定API
        nlohmann::json configApi;
        configApi["base"] = "/api/config";
        configApi["description"] = "Server configuration";
        configApi["operations"] = nlohmann::json::array();
        
        nlohmann::json configOp1;
        configOp1["method"] = "GET";
        configOp1["path"] = "/api/config";
        configOp1["description"] = "Get server configuration";
        configOp1["response"] = "Configuration object";
        configApi["operations"].push_back(configOp1);
        
        nlohmann::json configOp2;
        configOp2["method"] = "PUT";
        configOp2["path"] = "/api/config";
        configOp2["description"] = "Update server configuration (limited)";
        configOp2["request"] = "Configuration JSON";
        configOp2["response"] = "Update status";
        configApi["operations"].push_back(configOp2);
        
        // イベントリストに新規イベントを追加
        eventList.push_back({"ui_imported", "UI layout imported"});
        eventList.push_back({"character_imported", "Character imported"});
        
        docs["apis"] = nlohmann::json::object();
        docs["apis"]["ui"] = uiApi;
        docs["apis"]["characters"] = charApi;
        docs["apis"]["stages"] = stageApi;
        docs["apis"]["events"] = eventsApi;
        docs["apis"]["batch"] = batchApi;
        docs["apis"]["search"] = searchApi;
        docs["apis"]["exportImport"] = exportImportApi;
        docs["apis"]["stats"] = statsApi;
        docs["apis"]["config"] = configApi;
        
        res.set_content(docs.dump(2), "application/json");
    });
}

void HTTPServer::SetupCharacterRoutes() {
    // GET /api/characters - 全キャラクター一覧
    impl_->server->Get("/api/characters", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto ids = registry_->GetAllCharacterIds();
            nlohmann::json json = nlohmann::json::array();
            for (const auto& id : ids) {
                const auto& def = registry_->GetCharacter(id);
                nlohmann::json charJson;
                charJson["id"] = def.id;
                charJson["name"] = def.name;
                charJson["description"] = def.description;
                json.push_back(charJson);
            }
            res.set_content(json.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // GET /api/characters/:id - 特定キャラクター取得
    impl_->server->Get("/api/characters/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto& def = registry_->GetCharacter(req.path_params.at("id"));
            auto json = Data::CharacterSerializer::Serialize(def);
            res.set_content(json.dump(2), "application/json");
        } catch (const Data::DefinitionNotFoundException& e) {
            res.status = 404;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // POST /api/characters - 新規キャラクター作成
    impl_->server->Post("/api/characters", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            // JSONからCharacterDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::CharacterSerializer::Deserialize(json);
            
            // バリデーション
            std::string validationError;
            if (!ValidateCharacterDef(def, validationError)) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Validation failed", validationError, requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterCharacter(def);
            
            // キャッシュを無効化
            InvalidateCache("characters");
            
            // ファイルに保存
            std::string filePath = definitionsPath_ + "/characters/" + def.id + ".character.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            
            // 既存ファイルの場合はバックアップを作成
            std::string backupPath = CreateBackup(filePath);
            
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = def.id;
                notification["file"] = filePath;
                notification["created"] = true;
                if (!backupPath.empty()) {
                    notification["backup"] = backupPath;
                }
                BroadcastToClients("character_created", notification.dump());
                
                res.status = 201;
                nlohmann::json response;
                response["success"] = true;
                response["id"] = def.id;
                if (!backupPath.empty()) {
                    response["backup"] = backupPath;
                }
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(
                    CreateErrorResponse(500, "Failed to save file", "Could not open file for writing", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // PUT /api/characters/:id - キャラクター更新
    impl_->server->Put("/api/characters/:id", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string id = req.path_params.at("id");
            
            // JSONからCharacterDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::CharacterSerializer::Deserialize(json);
            
            // IDを確認（パスパラメータと一致させる）
            if (def.id.empty() || def.id != id) {
                def.id = id;
            }
            
            // バリデーション
            std::string validationError;
            if (!ValidateCharacterDef(def, validationError)) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Validation failed", validationError, requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterCharacter(def);
            
            // キャッシュを無効化
            InvalidateCache("characters");
            
            // ファイルに保存
            std::string filePath = definitionsPath_ + "/characters/" + id + ".character.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            
            // 既存ファイルの場合はバックアップを作成
            std::string backupPath = CreateBackup(filePath);
            
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = id;
                notification["file"] = filePath;
                notification["updated"] = true;
                if (!backupPath.empty()) {
                    notification["backup"] = backupPath;
                }
                BroadcastToClients("character_updated", notification.dump());
                
                res.set_content(
                    nlohmann::json{{"success", true}, {"id", id}}.dump(),
                    "application/json"
                );
            } else {
                res.status = 500;
                res.set_content(
                    CreateErrorResponse(500, "Failed to save file", "Could not open file for writing", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // DELETE /api/characters/:id - キャラクター削除
    impl_->server->Delete("/api/characters/:id", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string id = req.path_params.at("id");
            
            // ファイル削除
            std::string filePath = definitionsPath_ + "/characters/" + id + ".character.json";
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
                
                // キャッシュを無効化
                InvalidateCache("characters");
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = id;
                notification["deleted"] = true;
                BroadcastToClients("character_deleted", notification.dump());
                
                res.set_content(
                    nlohmann::json{{"success", true}, {"id", id}}.dump(),
                    "application/json"
                );
            } else {
                res.status = 404;
                res.set_content(
                    CreateErrorResponse(404, "Character not found", "The requested character does not exist", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Internal server error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupStageRoutes() {
    // GET /api/stages - 全ステージ一覧
    impl_->server->Get("/api/stages", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto ids = registry_->GetAllStageIds();
            nlohmann::json json = nlohmann::json::array();
            for (const auto& id : ids) {
                const auto& def = registry_->GetStage(id);
                nlohmann::json stageJson;
                stageJson["id"] = def.id;
                stageJson["name"] = def.name;
                stageJson["description"] = def.description;
                json.push_back(stageJson);
            }
            res.set_content(json.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // GET /api/stages/:id - 特定ステージ取得
    impl_->server->Get("/api/stages/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto& def = registry_->GetStage(req.path_params.at("id"));
            auto json = Data::StageSerializer::Serialize(def);
            res.set_content(json.dump(2), "application/json");
        } catch (const Data::DefinitionNotFoundException& e) {
            res.status = 404;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // POST /api/stages - ステージ新規作成
    impl_->server->Post("/api/stages", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            // JSONからStageDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::StageSerializer::Deserialize(json);
            
            // バリデーション
            std::string validationError;
            if (!ValidateStageDef(def, validationError)) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Validation failed", validationError, requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterStage(def);
            
            // キャッシュを無効化
            InvalidateCache("stages");
            
            // ファイルに保存
            std::string filePath = definitionsPath_ + "/stages/" + def.id + ".stage.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            
            // 既存ファイルの場合はバックアップを作成
            std::string backupPath = CreateBackup(filePath);
            
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = def.id;
                notification["file"] = filePath;
                notification["created"] = true;
                if (!backupPath.empty()) {
                    notification["backup"] = backupPath;
                }
                BroadcastToClients("stage_created", notification.dump());
                
                res.status = 201;
                nlohmann::json response;
                response["success"] = true;
                response["id"] = def.id;
                if (!backupPath.empty()) {
                    response["backup"] = backupPath;
                }
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(
                    CreateErrorResponse(500, "Failed to save file", "Could not open file for writing", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // PUT /api/stages/:id - ステージ更新
    impl_->server->Put("/api/stages/:id", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string id = req.path_params.at("id");
            
            // JSONからStageDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::StageSerializer::Deserialize(json);
            
            // IDを確認（パスパラメータと一致させる）
            if (def.id.empty() || def.id != id) {
                def.id = id;
            }
            
            // バリデーション
            std::string validationError;
            if (!ValidateStageDef(def, validationError)) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Validation failed", validationError, requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterStage(def);
            
            // キャッシュを無効化
            InvalidateCache("stages");
            
            // ファイルに保存
            std::string filePath = definitionsPath_ + "/stages/" + id + ".stage.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            
            // バックアップを作成
            std::string backupPath = CreateBackup(filePath);
            
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = id;
                notification["file"] = filePath;
                notification["updated"] = true;
                if (!backupPath.empty()) {
                    notification["backup"] = backupPath;
                }
                BroadcastToClients("stage_updated", notification.dump());
                
                nlohmann::json response;
                response["success"] = true;
                response["id"] = id;
                if (!backupPath.empty()) {
                    response["backup"] = backupPath;
                }
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(
                    CreateErrorResponse(500, "Failed to save file", "Could not open file for writing", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // DELETE /api/stages/:id - ステージ削除
    impl_->server->Delete("/api/stages/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string id = req.path_params.at("id");
            
            // ファイル削除
            std::string filePath = definitionsPath_ + "/stages/" + id + ".stage.json";
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = id;
                notification["deleted"] = true;
                BroadcastToClients("stage_deleted", notification.dump());
                
                res.set_content(
                    nlohmann::json{{"success", true}, {"id", id}}.dump(),
                    "application/json"
                );
            } else {
                res.status = 404;
                res.set_content(
                    nlohmann::json{{"error", "Stage not found"}}.dump(),
                    "application/json"
                );
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupUIRoutes() {
    // GET /api/ui - 全UIレイアウト一覧
    impl_->server->Get("/api/ui", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto ids = registry_->GetAllUILayoutIds();
            nlohmann::json json = nlohmann::json::array();
            for (const auto& id : ids) {
                const auto& def = registry_->GetUILayout(id);
                nlohmann::json uiJson;
                uiJson["id"] = def.id;
                uiJson["name"] = def.name;
                json.push_back(uiJson);
            }
            res.set_content(json.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // POST /api/ui - UIレイアウト新規作成
    impl_->server->Post("/api/ui", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            // JSONからUILayoutDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::UISerializer::Deserialize(json);
            
            if (def.id.empty()) {
                res.status = 400;
                res.set_content(
                    nlohmann::json{{"error", "id is required"}}.dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterUILayout(def);
            
            // ファイルに保存
            std::string filePath = definitionsPath_ + "/ui/" + def.id + ".ui.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = def.id;
                notification["file"] = filePath;
                notification["created"] = true;
                BroadcastToClients("ui_created", notification.dump());
                
                res.status = 201;
                res.set_content(
                    nlohmann::json{{"success", true}, {"id", def.id}}.dump(),
                    "application/json"
                );
            } else {
                res.status = 500;
                res.set_content(
                    nlohmann::json{{"error", "Failed to save file"}}.dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", "Invalid JSON: " + std::string(e.what())}}.dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // GET /api/ui/:id - 特定UIレイアウト取得（リアルタイム編集用）
    impl_->server->Get("/api/ui/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto& def = registry_->GetUILayout(req.path_params.at("id"));
            auto json = Data::UISerializer::Serialize(def);
            res.set_content(json.dump(2), "application/json");
        } catch (const Data::DefinitionNotFoundException& e) {
            res.status = 404;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // PUT /api/ui/:id - UIレイアウト更新（リアルタイム編集用）
    impl_->server->Put("/api/ui/:id", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string id = req.path_params.at("id");
            
            // JSONからUILayoutDefをパース
            auto json = nlohmann::json::parse(req.body);
            auto def = Data::UISerializer::Deserialize(json);
            
            // IDを確認（パスパラメータと一致させる）
            if (def.id.empty() || def.id != id) {
                def.id = id;
            }
            
            // バリデーション
            std::string validationError;
            if (!ValidateUILayoutDef(def, validationError)) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Validation failed", validationError, requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            // レジストリに登録
            registry_->RegisterUILayout(def);
            
            // キャッシュを無効化
            InvalidateCache("ui");
            
            // ファイルに保存（ホットリロード用）
            std::string filePath = definitionsPath_ + "/ui/" + id + ".ui.json";
            std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
            
            // バックアップを作成
            std::string backupPath = CreateBackup(filePath);
            
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << json.dump(2);
                file.close();
                
                // SSE経由でクライアントに通知（リアルタイム更新）
                nlohmann::json notification;
                notification["id"] = id;
                notification["file"] = filePath;
                notification["updated"] = true;
                if (!backupPath.empty()) {
                    notification["backup"] = backupPath;
                }
                BroadcastToClients("ui_updated", notification.dump());
                
                nlohmann::json response;
                response["success"] = true;
                response["id"] = id;
                if (!backupPath.empty()) {
                    response["backup"] = backupPath;
                }
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(
                    CreateErrorResponse(500, "Failed to save file", "Could not open file for writing", requestId).dump(),
                    "application/json"
                );
            }
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // DELETE /api/ui/:id - UIレイアウト削除
    impl_->server->Delete("/api/ui/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string id = req.path_params.at("id");
            
            // ファイル削除
            std::string filePath = definitionsPath_ + "/ui/" + id + ".ui.json";
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
                
                // キャッシュを無効化
                InvalidateCache("ui");
                InvalidateCache("ui_" + id);
                
                // SSE経由でクライアントに通知
                nlohmann::json notification;
                notification["id"] = id;
                notification["deleted"] = true;
                BroadcastToClients("ui_deleted", notification.dump());
                
                res.set_content(
                    nlohmann::json{{"success", true}, {"id", id}}.dump(),
                    "application/json"
                );
            } else {
                res.status = 404;
                res.set_content(
                    nlohmann::json{{"error", "UI layout not found"}}.dump(),
                    "application/json"
                );
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupWebSocket() {
    // WebSocket接続エンドポイント（/ws）- UIエディター用リアルタイム通信
    // 注意: cpp-httplib v0.14.3のWebSocket APIは異なる可能性があります
    // まずは基本的なエンドポイントを用意し、後で完全な実装を追加
    
    // WebSocketエンドポイント（実装は後で完全に追加）
    impl_->server->Get("/ws", [](const httplib::Request& req, httplib::Response& res) {
        res.status = 501; // Not Implemented
        res.set_content(
            R"({"error":"WebSocket support is under development. Use /events for SSE instead."})",
            "application/json"
        );
    });

    // Server-Sent Events (SSE)を使用したリアルタイム更新実装
    // GET /events - SSEストリーム（UIエディター用リアルタイム更新）
    impl_->server->Get("/events", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");
        res.set_header("Connection", "keep-alive");
        res.set_header("Access-Control-Allow-Origin", "*");
        
        // SSE接続を確立
        auto client = std::make_shared<SSEClient>();
        client->response = std::make_shared<httplib::Response>(res);
        
        {
            std::lock_guard<std::mutex> lock(impl_->connectionsMutex);
            impl_->sseClients.push_back(client);
        }
        
        std::cout << "HTTPServer: SSE client connected (total: " 
                  << impl_->sseClients.size() << ")\n";
        
        // 接続確認メッセージを送信
        res.set_content(": connected\n\n", "text/event-stream");
        
        // 接続を維持（クライアントが切断するまで待機）
        // 注意: cpp-httplibのSSE実装は簡易的なものなので、
        // 実際の実装では別スレッドでメッセージ送信を行う必要がある場合があります
        // 現在は接続確立のみで、メッセージ送信はBroadcastToClientsで行う
    });
}

void HTTPServer::SetupBatchRoutes() {
    // POST /api/batch/ui - UIレイアウトのバッチ作成
    impl_->server->Post("/api/batch/ui", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            if (!json.contains("items") || !json["items"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid request", "items array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& itemJson : json["items"]) {
                try {
                    auto def = Data::UISerializer::Deserialize(itemJson);
                    
                    std::string validationError;
                    if (!ValidateUILayoutDef(def, validationError)) {
                        results.push_back(nlohmann::json{
                            {"id", def.id.empty() ? "unknown" : def.id},
                            {"success", false},
                            {"error", validationError}
                        });
                        failureCount++;
                        continue;
                    }
                    
                    registry_->RegisterUILayout(def);
                    
                    std::string filePath = definitionsPath_ + "/ui/" + def.id + ".ui.json";
                    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
                    std::string backupPath = CreateBackup(filePath);
                    
                    std::ofstream file(filePath);
                    if (file.is_open()) {
                        file << itemJson.dump(2);
                        file.close();
                        
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", true},
                            {"backup", backupPath.empty() ? nullptr : backupPath}
                        });
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = def.id;
                        notification["file"] = filePath;
                        notification["created"] = true;
                        BroadcastToClients("ui_created", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", false},
                            {"error", "Failed to save file"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["items"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 201 : 207; // 207 Multi-Status
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // POST /api/batch/characters - キャラクターのバッチ作成
    impl_->server->Post("/api/batch/characters", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            if (!json.contains("items") || !json["items"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid request", "items array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& itemJson : json["items"]) {
                try {
                    auto def = Data::CharacterSerializer::Deserialize(itemJson);
                    
                    std::string validationError;
                    if (!ValidateCharacterDef(def, validationError)) {
                        results.push_back(nlohmann::json{
                            {"id", def.id.empty() ? "unknown" : def.id},
                            {"success", false},
                            {"error", validationError}
                        });
                        failureCount++;
                        continue;
                    }
                    
                    registry_->RegisterCharacter(def);
                    
                    std::string filePath = definitionsPath_ + "/characters/" + def.id + ".character.json";
                    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
                    std::string backupPath = CreateBackup(filePath);
                    
                    std::ofstream file(filePath);
                    if (file.is_open()) {
                        file << itemJson.dump(2);
                        file.close();
                        
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", true},
                            {"backup", backupPath.empty() ? nullptr : backupPath}
                        });
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = def.id;
                        notification["file"] = filePath;
                        notification["created"] = true;
                        BroadcastToClients("character_created", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", false},
                            {"error", "Failed to save file"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["items"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 201 : 207;
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // DELETE /api/batch/ui - UIレイアウトのバッチ削除
    impl_->server->Delete("/api/batch/ui", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            if (!json.contains("ids") || !json["ids"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid request", "ids array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& idJson : json["ids"]) {
                std::string id = idJson.get<std::string>();
                try {
                    std::string filePath = definitionsPath_ + "/ui/" + id + ".ui.json";
                    if (std::filesystem::exists(filePath)) {
                        std::filesystem::remove(filePath);
                        results.push_back(nlohmann::json{{"id", id}, {"success", true}});
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = id;
                        notification["deleted"] = true;
                        BroadcastToClients("ui_deleted", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", id},
                            {"success", false},
                            {"error", "File not found"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"id", id},
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["ids"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 200 : 207;
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });

    // DELETE /api/batch/characters - キャラクターのバッチ削除
    impl_->server->Delete("/api/batch/characters", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            if (!json.contains("ids") || !json["ids"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid request", "ids array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& idJson : json["ids"]) {
                std::string id = idJson.get<std::string>();
                try {
                    std::string filePath = definitionsPath_ + "/characters/" + id + ".character.json";
                    if (std::filesystem::exists(filePath)) {
                        std::filesystem::remove(filePath);
                        results.push_back(nlohmann::json{{"id", id}, {"success", true}});
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = id;
                        notification["deleted"] = true;
                        BroadcastToClients("character_deleted", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", id},
                            {"success", false},
                            {"error", "File not found"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"id", id},
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["ids"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 200 : 207;
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            nlohmann::json errorResponse = CreateErrorResponse(400, "Invalid JSON", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
                errorResponse["jsonParseError"] = true;
                errorResponse["byte"] = e.byte;
            }
            res.set_content(errorResponse.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json errorResponse = CreateErrorResponse(500, "Internal server error", e.what(), requestId);
            if (developmentMode_) {
                errorResponse["exceptionDetails"] = ExtractExceptionDetails(e, requestId);
            }
            res.set_content(errorResponse.dump(), "application/json");
        }
    });
}

void HTTPServer::SetupSearchRoutes() {
    // GET /api/search/ui?q=query - UIレイアウト検索
    impl_->server->Get("/api/search/ui", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string query = req.get_param_value("q", "");
            std::string nameFilter = req.get_param_value("name", "");
            
            auto ids = registry_->GetAllUILayoutIds();
            nlohmann::json results = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetUILayout(id);
                
                // 検索条件チェック
                bool matches = true;
                
                if (!query.empty()) {
                    // ID、名前、説明で検索
                    std::string lowerQuery = query;
                    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                    
                    std::string lowerId = def.id;
                    std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), ::tolower);
                    
                    std::string lowerName = def.name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    
                    if (lowerId.find(lowerQuery) == std::string::npos &&
                        lowerName.find(lowerQuery) == std::string::npos) {
                        matches = false;
                    }
                }
                
                if (!nameFilter.empty()) {
                    if (def.name.find(nameFilter) == std::string::npos) {
                        matches = false;
                    }
                }
                
                if (matches) {
                    nlohmann::json item;
                    item["id"] = def.id;
                    item["name"] = def.name;
                    results.push_back(item);
                }
            }
            
            nlohmann::json response;
            response["query"] = query;
            response["count"] = results.size();
            response["results"] = results;
            
            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Search error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/search/characters?q=query&rarity=rare - キャラクター検索
    impl_->server->Get("/api/search/characters", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string query = req.get_param_value("q", "");
            std::string rarityFilter = req.get_param_value("rarity", "");
            std::string gameModeFilter = req.get_param_value("gameMode", "");
            
            auto ids = registry_->GetAllCharacterIds();
            nlohmann::json results = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetCharacter(id);
                
                // 検索条件チェック
                bool matches = true;
                
                if (!query.empty()) {
                    std::string lowerQuery = query;
                    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                    
                    std::string lowerId = def.id;
                    std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), ::tolower);
                    
                    std::string lowerName = def.name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    
                    std::string lowerDesc = def.description;
                    std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
                    
                    if (lowerId.find(lowerQuery) == std::string::npos &&
                        lowerName.find(lowerQuery) == std::string::npos &&
                        lowerDesc.find(lowerQuery) == std::string::npos) {
                        matches = false;
                    }
                }
                
                if (!rarityFilter.empty()) {
                    std::string defRarity = Data::CharacterSerializer::Serialize(def)["rarity"].get<std::string>();
                    if (defRarity != rarityFilter) {
                        matches = false;
                    }
                }
                
                if (!gameModeFilter.empty()) {
                    std::string defGameMode = Data::CharacterSerializer::Serialize(def)["gameMode"].get<std::string>();
                    if (defGameMode != gameModeFilter && defGameMode != "both") {
                        matches = false;
                    }
                }
                
                if (matches) {
                    nlohmann::json item;
                    item["id"] = def.id;
                    item["name"] = def.name;
                    item["rarity"] = Data::CharacterSerializer::Serialize(def)["rarity"];
                    item["gameMode"] = Data::CharacterSerializer::Serialize(def)["gameMode"];
                    results.push_back(item);
                }
            }
            
            nlohmann::json response;
            response["query"] = query;
            response["filters"] = nlohmann::json::object();
            if (!rarityFilter.empty()) response["filters"]["rarity"] = rarityFilter;
            if (!gameModeFilter.empty()) response["filters"]["gameMode"] = gameModeFilter;
            response["count"] = results.size();
            response["results"] = results;
            
            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Search error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/search/stages?q=query - ステージ検索
    impl_->server->Get("/api/search/stages", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            std::string query = req.get_param_value("q", "");
            
            auto ids = registry_->GetAllStageIds();
            nlohmann::json results = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetStage(id);
                
                // 検索条件チェック
                bool matches = true;
                
                if (!query.empty()) {
                    std::string lowerQuery = query;
                    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                    
                    std::string lowerId = def.id;
                    std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), ::tolower);
                    
                    std::string lowerName = def.name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    
                    std::string lowerDesc = def.description;
                    std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
                    
                    if (lowerId.find(lowerQuery) == std::string::npos &&
                        lowerName.find(lowerQuery) == std::string::npos &&
                        lowerDesc.find(lowerQuery) == std::string::npos) {
                        matches = false;
                    }
                }
                
                if (matches) {
                    nlohmann::json item;
                    item["id"] = def.id;
                    item["name"] = def.name;
                    item["description"] = def.description;
                    item["waveCount"] = def.waves.size();
                    results.push_back(item);
                }
            }
            
            nlohmann::json response;
            response["query"] = query;
            response["count"] = results.size();
            response["results"] = results;
            
            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Search error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupExportImportRoutes() {
    // GET /api/export/ui - UIレイアウト全件エクスポート
    impl_->server->Get("/api/export/ui", [this](const httplib::Request&, httplib::Response& res) {
        try {
            auto ids = registry_->GetAllUILayoutIds();
            nlohmann::json exportData;
            exportData["version"] = "1.0";
            exportData["type"] = "ui_layouts";
            exportData["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            exportData["items"] = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetUILayout(id);
                exportData["items"].push_back(Data::UISerializer::Serialize(def));
            }
            
            res.set_header("Content-Type", "application/json");
            res.set_header("Content-Disposition", "attachment; filename=ui_layouts_export.json");
            res.set_content(exportData.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Export error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/export/characters - キャラクター全件エクスポート
    impl_->server->Get("/api/export/characters", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto ids = registry_->GetAllCharacterIds();
            nlohmann::json exportData;
            exportData["version"] = "1.0";
            exportData["type"] = "characters";
            exportData["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            exportData["items"] = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetCharacter(id);
                exportData["items"].push_back(Data::CharacterSerializer::Serialize(def));
            }
            
            res.set_header("Content-Type", "application/json");
            res.set_header("Content-Disposition", "attachment; filename=characters_export.json");
            res.set_content(exportData.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Export error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/export/stages - ステージ全件エクスポート
    impl_->server->Get("/api/export/stages", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto ids = registry_->GetAllStageIds();
            nlohmann::json exportData;
            exportData["version"] = "1.0";
            exportData["type"] = "stages";
            exportData["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            exportData["items"] = nlohmann::json::array();
            
            for (const auto& id : ids) {
                const auto& def = registry_->GetStage(id);
                exportData["items"].push_back(Data::StageSerializer::Serialize(def));
            }
            
            res.set_header("Content-Type", "application/json");
            res.set_header("Content-Disposition", "attachment; filename=stages_export.json");
            res.set_content(exportData.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Export error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // POST /api/import/ui - UIレイアウト一括インポート
    impl_->server->Post("/api/import/ui", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            
            if (!json.contains("items") || !json["items"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid import data", "items array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& itemJson : json["items"]) {
                try {
                    auto def = Data::UISerializer::Deserialize(itemJson);
                    
                    std::string validationError;
                    if (!ValidateUILayoutDef(def, validationError)) {
                        results.push_back(nlohmann::json{
                            {"id", def.id.empty() ? "unknown" : def.id},
                            {"success", false},
                            {"error", validationError}
                        });
                        failureCount++;
                        continue;
                    }
                    
                    registry_->RegisterUILayout(def);
                    
                    std::string filePath = definitionsPath_ + "/ui/" + def.id + ".ui.json";
                    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
                    std::string backupPath = CreateBackup(filePath);
                    
                    std::ofstream file(filePath);
                    if (file.is_open()) {
                        file << itemJson.dump(2);
                        file.close();
                        
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", true},
                            {"action", backupPath.empty() ? "created" : "updated"},
                            {"backup", backupPath.empty() ? nullptr : backupPath}
                        });
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = def.id;
                        notification["file"] = filePath;
                        notification["imported"] = true;
                        BroadcastToClients("ui_imported", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", false},
                            {"error", "Failed to save file"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["items"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 200 : 207;
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            res.set_content(
                CreateErrorResponse(400, "Invalid JSON", e.what(), requestId).dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Import error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // POST /api/import/characters - キャラクター一括インポート
    impl_->server->Post("/api/import/characters", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            
            if (!json.contains("items") || !json["items"].is_array()) {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid import data", "items array is required", requestId).dump(),
                    "application/json"
                );
                return;
            }
            
            nlohmann::json results = nlohmann::json::array();
            int successCount = 0;
            int failureCount = 0;
            
            for (const auto& itemJson : json["items"]) {
                try {
                    auto def = Data::CharacterSerializer::Deserialize(itemJson);
                    
                    std::string validationError;
                    if (!ValidateCharacterDef(def, validationError)) {
                        results.push_back(nlohmann::json{
                            {"id", def.id.empty() ? "unknown" : def.id},
                            {"success", false},
                            {"error", validationError}
                        });
                        failureCount++;
                        continue;
                    }
                    
                    registry_->RegisterCharacter(def);
                    
                    std::string filePath = definitionsPath_ + "/characters/" + def.id + ".character.json";
                    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
                    std::string backupPath = CreateBackup(filePath);
                    
                    std::ofstream file(filePath);
                    if (file.is_open()) {
                        file << itemJson.dump(2);
                        file.close();
                        
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", true},
                            {"action", backupPath.empty() ? "created" : "updated"},
                            {"backup", backupPath.empty() ? nullptr : backupPath}
                        });
                        successCount++;
                        
                        nlohmann::json notification;
                        notification["id"] = def.id;
                        notification["file"] = filePath;
                        notification["imported"] = true;
                        BroadcastToClients("character_imported", notification.dump());
                    } else {
                        results.push_back(nlohmann::json{
                            {"id", def.id},
                            {"success", false},
                            {"error", "Failed to save file"}
                        });
                        failureCount++;
                    }
                } catch (const std::exception& e) {
                    results.push_back(nlohmann::json{
                        {"success", false},
                        {"error", e.what()}
                    });
                    failureCount++;
                }
            }
            
            nlohmann::json response;
            response["success"] = failureCount == 0;
            response["total"] = json["items"].size();
            response["successCount"] = successCount;
            response["failureCount"] = failureCount;
            response["results"] = results;
            
            res.status = failureCount == 0 ? 200 : 207;
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            res.set_content(
                CreateErrorResponse(400, "Invalid JSON", e.what(), requestId).dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Import error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupStatsRoutes() {
    // GET /api/stats - 統計情報取得
    impl_->server->Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            nlohmann::json stats;
            
            // UIレイアウト統計
            auto uiIds = registry_->GetAllUILayoutIds();
            nlohmann::json uiStats;
            uiStats["total"] = uiIds.size();
            uiStats["ids"] = uiIds;
            stats["ui"] = uiStats;
            
            // キャラクター統計
            auto charIds = registry_->GetAllCharacterIds();
            nlohmann::json charStats;
            charStats["total"] = charIds.size();
            
            // レアリティ別集計
            std::map<std::string, int> rarityCount;
            for (const auto& id : charIds) {
                const auto& def = registry_->GetCharacter(id);
                std::string rarity = Data::CharacterSerializer::Serialize(def)["rarity"].get<std::string>();
                rarityCount[rarity]++;
            }
            charStats["byRarity"] = nlohmann::json::object();
            for (const auto& [rarity, count] : rarityCount) {
                charStats["byRarity"][rarity] = count;
            }
            
            // ゲームモード別集計
            std::map<std::string, int> gameModeCount;
            for (const auto& id : charIds) {
                const auto& def = registry_->GetCharacter(id);
                std::string gameMode = Data::CharacterSerializer::Serialize(def)["gameMode"].get<std::string>();
                gameModeCount[gameMode]++;
            }
            charStats["byGameMode"] = nlohmann::json::object();
            for (const auto& [mode, count] : gameModeCount) {
                charStats["byGameMode"][mode] = count;
            }
            
            charStats["ids"] = charIds;
            stats["characters"] = charStats;
            
            // ステージ統計
            auto stageIds = registry_->GetAllStageIds();
            nlohmann::json stageStats;
            stageStats["total"] = stageIds.size();
            
            // Wave数統計
            int totalWaves = 0;
            int maxWaves = 0;
            int minWaves = INT_MAX;
            for (const auto& id : stageIds) {
                const auto& def = registry_->GetStage(id);
                int waveCount = def.waves.size();
                totalWaves += waveCount;
                maxWaves = std::max(maxWaves, waveCount);
                minWaves = std::min(minWaves, waveCount);
            }
            stageStats["totalWaves"] = totalWaves;
            stageStats["averageWaves"] = stageIds.empty() ? 0.0 : (double)totalWaves / stageIds.size();
            stageStats["maxWaves"] = maxWaves == INT_MAX ? 0 : maxWaves;
            stageStats["minWaves"] = minWaves == INT_MAX ? 0 : minWaves;
            stageStats["ids"] = stageIds;
            stats["stages"] = stageStats;
            
            // ファイル統計
            nlohmann::json fileStats;
            int uiFileCount = 0;
            int charFileCount = 0;
            int stageFileCount = 0;
            
            try {
                for (const auto& entry : fs::recursive_directory_iterator(definitionsPath_)) {
                    if (!entry.is_regular_file()) continue;
                    std::string path = entry.path().string();
                    if (path.find("/ui/") != std::string::npos && path.ends_with(".ui.json")) {
                        uiFileCount++;
                    } else if (path.find("/characters/") != std::string::npos && path.ends_with(".character.json")) {
                        charFileCount++;
                    } else if (path.find("/stages/") != std::string::npos && path.ends_with(".stage.json")) {
                        stageFileCount++;
                    }
                }
            } catch (const std::exception& e) {
                // ファイルシステムエラーは無視
            }
            
            fileStats["ui"] = uiFileCount;
            fileStats["characters"] = charFileCount;
            fileStats["stages"] = stageFileCount;
            stats["files"] = fileStats;
            
            // サーバー情報
            nlohmann::json serverInfo;
            serverInfo["port"] = port_;
            serverInfo["running"] = running_.load();
            serverInfo["definitionsPath"] = definitionsPath_;
            stats["server"] = serverInfo;
            
            // パフォーマンス統計
            stats["performance"] = GetPerformanceStats();
            
            res.set_content(stats.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Stats error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupConfigRoutes() {
    // GET /api/config - 設定取得
    impl_->server->Get("/api/config", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            nlohmann::json config;
            config["port"] = port_;
            config["definitionsPath"] = definitionsPath_;
            config["fileWatcherEnabled"] = fileWatcherThread_ != nullptr;
            config["fileWatcherInterval"] = 500; // ms
            config["developmentMode"] = developmentMode_;
            
            // セキュリティ設定
            nlohmann::json security;
            security["rateLimitEnabled"] = rateLimitEnabled_;
            security["rateLimitPerMinute"] = rateLimitPerMinute_;
            security["maxBodySize"] = maxBodySize_;
            security["requestTimeoutSeconds"] = requestTimeoutSeconds_;
            config["security"] = security;
            
            res.set_content(config.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Config error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // PUT /api/config - 設定更新（一部のみ）
    impl_->server->Put("/api/config", [this](const httplib::Request& req, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            auto json = nlohmann::json::parse(req.body);
            nlohmann::json response;
            response["updated"] = nlohmann::json::array();
            response["errors"] = nlohmann::json::array();
            
            // ポート番号の変更は実行時には不可（再起動が必要）
            if (json.contains("port")) {
                response["errors"].push_back("Port cannot be changed at runtime. Restart required.");
            }
            
            // 定義パスの変更は実行時には不可（再起動が必要）
            if (json.contains("definitionsPath")) {
                response["errors"].push_back("Definitions path cannot be changed at runtime. Restart required.");
            }
            
            // 開発モードの更新
            if (json.contains("developmentMode") && json["developmentMode"].is_boolean()) {
                SetDevelopmentMode(json["developmentMode"].get<bool>());
                response["updated"].push_back("developmentMode");
            }
            
            // セキュリティ設定の更新
            if (json.contains("security")) {
                const auto& security = json["security"];
                
                if (security.contains("rateLimitEnabled") && security["rateLimitEnabled"].is_boolean()) {
                    rateLimitEnabled_ = security["rateLimitEnabled"].get<bool>();
                    response["updated"].push_back("rateLimitEnabled");
                }
                
                if (security.contains("rateLimitPerMinute") && security["rateLimitPerMinute"].is_number()) {
                    int newLimit = security["rateLimitPerMinute"].get<int>();
                    if (newLimit > 0 && newLimit <= 10000) {
                        rateLimitPerMinute_ = newLimit;
                        response["updated"].push_back("rateLimitPerMinute");
                    } else {
                        response["errors"].push_back("rateLimitPerMinute must be between 1 and 10000");
                    }
                }
                
                if (security.contains("maxBodySize") && security["maxBodySize"].is_number()) {
                    size_t newSize = security["maxBodySize"].get<size_t>();
                    if (newSize > 0 && newSize <= 100 * 1024 * 1024) {  // 最大100MB
                        maxBodySize_ = newSize;
                        response["updated"].push_back("maxBodySize");
                    } else {
                        response["errors"].push_back("maxBodySize must be between 1 and 104857600 (100MB)");
                    }
                }
                
                if (security.contains("requestTimeoutSeconds") && security["requestTimeoutSeconds"].is_number()) {
                    int newTimeout = security["requestTimeoutSeconds"].get<int>();
                    if (newTimeout > 0 && newTimeout <= 300) {  // 最大5分
                        requestTimeoutSeconds_ = newTimeout;
                        // タイムアウト設定を更新
                        impl_->server->set_read_timeout(requestTimeoutSeconds_, 0);
                        impl_->server->set_write_timeout(requestTimeoutSeconds_, 0);
                        impl_->server->set_idle_interval(requestTimeoutSeconds_, 0);
                        response["updated"].push_back("requestTimeoutSeconds");
                    } else {
                        response["errors"].push_back("requestTimeoutSeconds must be between 1 and 300");
                    }
                }
            }
            
            // パフォーマンス統計のリセット
            if (json.contains("resetPerformanceStats") && json["resetPerformanceStats"].is_boolean() && json["resetPerformanceStats"].get<bool>()) {
                ResetPerformanceStats();
                response["updated"].push_back("performanceStats");
            }
            
            if (response["updated"].empty() && response["errors"].empty()) {
                response["message"] = "No configurable settings provided";
            }
            
            res.set_content(response.dump(2), "application/json");
        } catch (const nlohmann::json::parse_error& e) {
            res.status = 400;
            res.set_content(
                CreateErrorResponse(400, "Invalid JSON", e.what(), requestId).dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Config error", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::SetupGameStateRoutes() {
    // GET /api/game/state - ゲーム状態取得（デバッグ/プレビュー用）
    impl_->server->Get("/api/game/state", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            if (!gameStateCallback_) {
                res.status = 503;  // Service Unavailable
                res.set_content(
                    CreateErrorResponse(503, "Game state not available", 
                        "Game state callback not set", requestId).dump(),
                    "application/json"
                );
                return;
            }

            nlohmann::json gameState = gameStateCallback_();
            res.set_content(gameState.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Failed to get game state", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/game/td/state - TDモード専用ゲーム状態取得
    impl_->server->Get("/api/game/td/state", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            if (!gameStateCallback_) {
                res.status = 503;
                res.set_content(
                    CreateErrorResponse(503, "Game state not available", 
                        "Game state callback not set", requestId).dump(),
                    "application/json"
                );
                return;
            }

            nlohmann::json gameState = gameStateCallback_();
            
            // TDモードでない場合はエラー
            if (!gameState.contains("mode") || gameState["mode"] != "TD") {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid game mode", 
                        "Current game mode is not TD", requestId).dump(),
                    "application/json"
                );
                return;
            }

            // TD専用の状態情報を返す
            nlohmann::json tdState;
            tdState["mode"] = "TD";
            if (gameState.contains("scene")) {
                tdState["scene"] = gameState["scene"];
            }
            if (gameState.contains("td")) {
                tdState["td"] = gameState["td"];
            }
            if (gameState.contains("entities")) {
                tdState["entities"] = gameState["entities"];
            }
            
            res.set_content(tdState.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Failed to get TD game state", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });

    // GET /api/game/roguelike/state - Roguelikeモード専用ゲーム状態取得
    impl_->server->Get("/api/game/roguelike/state", [this](const httplib::Request&, httplib::Response& res) {
        std::string requestId = res.get_header_value("X-Request-ID");
        try {
            if (!gameStateCallback_) {
                res.status = 503;
                res.set_content(
                    CreateErrorResponse(503, "Game state not available", 
                        "Game state callback not set", requestId).dump(),
                    "application/json"
                );
                return;
            }

            nlohmann::json gameState = gameStateCallback_();
            
            // Roguelikeモードでない場合はエラー
            if (!gameState.contains("mode") || gameState["mode"] != "Roguelike") {
                res.status = 400;
                res.set_content(
                    CreateErrorResponse(400, "Invalid game mode", 
                        "Current game mode is not Roguelike", requestId).dump(),
                    "application/json"
                );
                return;
            }

            // Roguelike専用の状態情報を返す
            nlohmann::json roguelikeState;
            roguelikeState["mode"] = "Roguelike";
            if (gameState.contains("scene")) {
                roguelikeState["scene"] = gameState["scene"];
            }
            if (gameState.contains("roguelike")) {
                roguelikeState["roguelike"] = gameState["roguelike"];
            }
            if (gameState.contains("entities")) {
                roguelikeState["entities"] = gameState["entities"];
            }
            
            res.set_content(roguelikeState.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(
                CreateErrorResponse(500, "Failed to get Roguelike game state", e.what(), requestId).dump(),
                "application/json"
            );
        }
    });
}

void HTTPServer::BroadcastToClients(const std::string& eventType, const std::string& data) {
    nlohmann::json message;
    message["type"] = eventType;
    message["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    if (!data.empty()) {
        try {
            message["data"] = nlohmann::json::parse(data);
        } catch (...) {
            message["data"] = data; // JSONとしてパースできない場合は文字列として扱う
        }
    }

    std::string messageStr = message.dump();
    
    // SSE形式でメッセージをフォーマット
    std::stringstream sseMessage;
    sseMessage << "event: " << eventType << "\n";
    sseMessage << "data: " << messageStr << "\n\n";

    // SSEクライアントにブロードキャスト（UIエディター用リアルタイム更新）
    std::lock_guard<std::mutex> lock(impl_->connectionsMutex);
    for (auto it = impl_->sseClients.begin(); it != impl_->sseClients.end();) {
        auto client = *it;
        if (client && client->active.load()) {
            // SSEメッセージを送信
            // 注意: cpp-httplibのSSE実装は制限があるため、
            // 実際の実装では別スレッドでストリーミング送信を行う必要がある場合があります
            // 現在は簡易実装として、クライアントリストを管理
            ++it;
        } else {
            // 非アクティブなクライアントを削除
            it = impl_->sseClients.erase(it);
        }
    }
    
    std::cout << "HTTPServer: Broadcasting event '" << eventType << "' to " 
              << impl_->sseClients.size() << " SSE clients\n";
}

void HTTPServer::ServerThread() {
    try {
        impl_->server->listen("0.0.0.0", port_);
    } catch (const std::exception& e) {
        if (errorHandler_) {
            errorHandler_(std::string("HTTPServer error: ") + e.what());
        } else {
            std::cerr << "HTTPServer: Error in server thread: " << e.what() << "\n";
        }
        running_ = false;
    }
}

void HTTPServer::FileWatcherThread() {
    while (running_) {
        CheckFileChanges();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

std::string HTTPServer::CreateBackup(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        return ""; // ファイルが存在しない場合はバックアップ不要
    }
    
    try {
        // バックアップディレクトリを作成
        std::string backupDir = definitionsPath_ + "/.backups";
        std::filesystem::create_directories(backupDir);
        
        // タイムスタンプ付きバックアップファイル名を生成
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        
        std::filesystem::path originalPath(filePath);
        std::string backupFileName = originalPath.stem().string() + "_" + ss.str() + originalPath.extension().string();
        std::string backupPath = backupDir + "/" + backupFileName;
        
        // ファイルをコピー
        std::filesystem::copy_file(filePath, backupPath, std::filesystem::copy_options::overwrite_existing);
        
        return backupPath;
    } catch (const std::exception& e) {
        std::cerr << "HTTPServer: Failed to create backup: " << e.what() << "\n";
        return "";
    }
}

nlohmann::json HTTPServer::CreateErrorResponse(int status, const std::string& error, const std::string& details, const std::string& requestId) {
    // エラーログを記録
    if (status >= 400) {
        LogError(error, requestId, details);
    }
    
    nlohmann::json response;
    response["error"] = error;
    response["status"] = status;
    if (!details.empty()) {
        response["details"] = details;
    }
    response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    if (!requestId.empty()) {
        response["requestId"] = requestId;
    }
    return response;
}

bool HTTPServer::ValidateCharacterDef(const Data::CharacterDef& def, std::string& errorMessage) {
    if (def.id.empty()) {
        errorMessage = "Character ID is required";
        return false;
    }
    
    if (def.name.empty()) {
        errorMessage = "Character name is required";
        return false;
    }
    
    if (def.stats.hp <= 0) {
        errorMessage = "Character HP must be greater than 0";
        return false;
    }
    
    if (def.stats.attack < 0) {
        errorMessage = "Character attack cannot be negative";
        return false;
    }
    
    return true;
}

bool HTTPServer::ValidateStageDef(const Data::StageDef& def, std::string& errorMessage) {
    if (def.id.empty()) {
        errorMessage = "Stage ID is required";
        return false;
    }
    
    if (def.name.empty()) {
        errorMessage = "Stage name is required";
        return false;
    }
    
    if (def.baseHealth <= 0) {
        errorMessage = "Base health must be greater than 0";
        return false;
    }
    
    if (def.laneCount <= 0) {
        errorMessage = "Lane count must be greater than 0";
        return false;
    }
    
    return true;
}

bool HTTPServer::ValidateUILayoutDef(const Data::UILayoutDef& def, std::string& errorMessage) {
    if (def.id.empty()) {
        errorMessage = "UI layout ID is required";
        return false;
    }
    
    if (def.name.empty()) {
        errorMessage = "UI layout name is required";
        return false;
    }
    
    if (def.baseWidth <= 0 || def.baseHeight <= 0) {
        errorMessage = "Base width and height must be greater than 0";
        return false;
    }
    
    return true;
}

void HTTPServer::CheckFileChanges() {
    std::vector<std::string> changedFiles;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(definitionsPath_)) {
            if (!entry.is_regular_file()) continue;
            
            auto ext = entry.path().extension();
            if (ext != ".json" && ext != ".character.json" && 
                ext != ".stage.json" && ext != ".ui.json") {
                continue;
            }

            std::string pathStr = entry.path().string();
            
            try {
                auto currentTime = fs::last_write_time(entry.path());

                {
                    std::lock_guard<std::mutex> lock(fileModificationTimeMutex_);
                    auto it = fileModificationTimes_.find(pathStr);
                    
                    if (it == fileModificationTimes_.end()) {
                        // 新規ファイル
                        fileModificationTimes_[pathStr] = currentTime;
                        changedFiles.push_back(pathStr);
                    } else if (it->second != currentTime) {
                        // ファイル変更
                        it->second = currentTime;
                        changedFiles.push_back(pathStr);
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                // ファイルアクセスエラーはログに記録して続行
                std::cerr << "HTTPServer: File access error for " << pathStr << ": " << e.what() << "\n";
                continue;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "HTTPServer: Directory iteration error: " << e.what() << "\n";
        if (errorHandler_) {
            errorHandler_("File watcher error: " + std::string(e.what()));
        }
        return;
    }

    // 変更されたファイルがある場合、コールバックを呼び出し＆SSE通知
    for (const auto& file : changedFiles) {
        // ファイルタイプを判定
        std::string fileType = "unknown";
        if (file.find("ui") != std::string::npos || file.find(".ui.json") != std::string::npos) {
            fileType = "ui";
        } else if (file.find("character") != std::string::npos || file.find(".character.json") != std::string::npos) {
            fileType = "character";
        } else if (file.find("stage") != std::string::npos || file.find(".stage.json") != std::string::npos) {
            fileType = "stage";
        }

        // SSE経由でクライアントに通知（特にUIエディター用）
        nlohmann::json notification;
        notification["file"] = file;
        notification["type"] = fileType;
        BroadcastToClients("file_changed", notification.dump());

        // コールバックを呼び出し（ホットリロード処理）
        if (fileChangedCallback_) {
            try {
                fileChangedCallback_(file);
            } catch (const std::exception& e) {
                std::cerr << "HTTPServer: Error in file changed callback for " << file << ": " << e.what() << "\n";
                if (errorHandler_) {
                    errorHandler_("File reload error: " + std::string(e.what()));
                }
            }
        }
    }
}

// ============================================================================
// ロギング機能実装
// ============================================================================

std::string HTTPServer::GenerateRequestId() {
    uint64_t id = requestIdCounter_.fetch_add(1) + 1;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << id;
    return oss.str();
}

std::string HTTPServer::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string HTTPServer::GetLogLevelName(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void HTTPServer::Log(LogLevel level, const std::string& message, const std::string& requestId) {
    if (!loggingEnabled_ || level < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    std::ostringstream oss;
    oss << "[" << GetTimestamp() << "] "
        << "[" << GetLogLevelName(level) << "]";
    
    if (!requestId.empty()) {
        oss << " [ReqID:" << requestId << "]";
    }
    
    oss << " " << message;
    
    if (level >= LogLevel::ERROR) {
        std::cerr << oss.str() << std::endl;
    } else {
        std::cout << oss.str() << std::endl;
    }
}

void HTTPServer::LogRequest(const std::string& method, const std::string& path, const std::string& clientIp, const std::string& requestId) {
    if (!loggingEnabled_) {
        return;
    }
    
    std::ostringstream oss;
    oss << method << " " << path << " from " << clientIp;
    Log(LogLevel::INFO, oss.str(), requestId);
    
    // リクエスト情報を保存
    RequestInfo info;
    info.requestId = requestId;
    info.method = method;
    info.path = path;
    info.clientIp = clientIp;
    info.startTime = std::chrono::steady_clock::now();
    
    {
        std::lock_guard<std::mutex> lock(activeRequestsMutex_);
        activeRequests_[requestId] = info;
    }
}

void HTTPServer::LogResponse(const RequestInfo& info) {
    if (!loggingEnabled_) {
        return;
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - info.startTime);
    
    std::ostringstream oss;
    oss << info.method << " " << info.path
        << " -> " << info.statusCode
        << " (" << duration.count() << "ms, " << info.responseSize << " bytes)";
    
    LogLevel level = (info.statusCode >= 400) ? LogLevel::WARNING : LogLevel::INFO;
    Log(level, oss.str(), info.requestId);
    
    // アクティブリクエストから削除
    {
        std::lock_guard<std::mutex> lock(activeRequestsMutex_);
        activeRequests_.erase(info.requestId);
    }
}

void HTTPServer::LogError(const std::string& message, const std::string& requestId, const std::string& details) {
    std::ostringstream oss;
    oss << message;
    if (!details.empty()) {
        oss << " - " << details;
    }
    Log(LogLevel::ERROR, oss.str(), requestId);
}

bool HTTPServer::CheckRateLimit(const std::string& clientIp) {
    if (!rateLimitEnabled_) {
        return true;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto oneMinuteAgo = now - std::chrono::minutes(1);
    
    std::lock_guard<std::mutex> lock(rateLimitMutex_);
    
    // 定期的にクリーンアップ（5分ごと）
    if (now - lastCleanupTime_ > std::chrono::minutes(5)) {
        CleanupRateLimitInfo();
        lastCleanupTime_ = now;
    }
    
    // クライアントIPのレート制限情報を取得または作成
    auto& info = rateLimitInfo_[clientIp];
    
    // 1分以上前のリクエストを削除
    info.requestTimes.erase(
        std::remove_if(info.requestTimes.begin(), info.requestTimes.end(),
            [oneMinuteAgo](const auto& time) {
                return time < oneMinuteAgo;
            }),
        info.requestTimes.end()
    );
    
    // レート制限チェック
    if (static_cast<int>(info.requestTimes.size()) >= rateLimitPerMinute_) {
        return false;  // レート制限超過
    }
    
    // 現在のリクエストを記録
    info.requestTimes.push_back(now);
    
    return true;  // 許可
}

void HTTPServer::CleanupRateLimitInfo() {
    auto now = std::chrono::steady_clock::now();
    auto fiveMinutesAgo = now - std::chrono::minutes(5);
    
    // 5分以上アクティブでないエントリを削除
    auto it = rateLimitInfo_.begin();
    while (it != rateLimitInfo_.end()) {
        auto& info = it->second;
        
        // 1分以上前のリクエストを削除
        info.requestTimes.erase(
            std::remove_if(info.requestTimes.begin(), info.requestTimes.end(),
                [fiveMinutesAgo](const auto& time) {
                    return time < fiveMinutesAgo;
                }),
            info.requestTimes.end()
        );
        
        // リクエストが空の場合はエントリを削除
        if (info.requestTimes.empty()) {
            it = rateLimitInfo_.erase(it);
        } else {
            ++it;
        }
    }
}

bool HTTPServer::CheckBodySize(size_t bodySize) const {
    return bodySize <= maxBodySize_;
}

nlohmann::json HTTPServer::CreateDetailedErrorResponse(
    int status,
    const std::string& error,
    const std::string& details,
    const std::string& requestId,
    const std::string& file,
    int line,
    const std::string& stackTrace
) const {
    nlohmann::json response = CreateErrorResponse(status, error, details, requestId);
    
    // 開発モードでは追加情報を付与
    if (developmentMode_) {
        if (!file.empty()) {
            response["file"] = file;
        }
        if (line > 0) {
            response["line"] = line;
        }
        if (!stackTrace.empty()) {
            response["stackTrace"] = stackTrace;
        }
        response["developmentMode"] = true;
    }
    
    return response;
}

nlohmann::json HTTPServer::ExtractExceptionDetails(const std::exception& e, const std::string& requestId) const {
    nlohmann::json details;
    details["type"] = typeid(e).name();
    details["message"] = e.what();
    
    if (developmentMode_) {
        // 開発モードでは追加情報を提供
        details["exceptionType"] = typeid(e).name();
        
        // 特定の例外型の詳細情報
        try {
            const auto* jsonError = dynamic_cast<const nlohmann::json::exception*>(&e);
            if (jsonError) {
                details["jsonError"] = true;
                details["jsonErrorId"] = jsonError->id;
            }
        } catch (...) {
            // 型変換失敗は無視
        }
        
        try {
            const auto* filesystemError = dynamic_cast<const std::filesystem::filesystem_error*>(&e);
            if (filesystemError) {
                details["filesystemError"] = true;
                details["path1"] = filesystemError->path1().string();
                details["path2"] = filesystemError->path2().string();
            }
        } catch (...) {
            // 型変換失敗は無視
        }
    }
    
    return details;
}

nlohmann::json HTTPServer::GetPerformanceStats() const {
    std::lock_guard<std::mutex> lock(performanceStatsMutex_);
    
    nlohmann::json stats;
    stats["totalRequests"] = performanceStats_.totalRequests;
    stats["successfulRequests"] = performanceStats_.successfulRequests;
    stats["failedRequests"] = performanceStats_.failedRequests;
    
    if (performanceStats_.totalRequests > 0) {
        stats["successRate"] = static_cast<double>(performanceStats_.successfulRequests) / performanceStats_.totalRequests;
        stats["averageResponseTime"] = performanceStats_.totalResponseTime.count() / static_cast<double>(performanceStats_.totalRequests);
    } else {
        stats["successRate"] = 0.0;
        stats["averageResponseTime"] = 0.0;
    }
    
    if (performanceStats_.minResponseTime.count() != INT_MAX) {
        stats["minResponseTime"] = performanceStats_.minResponseTime.count();
    } else {
        stats["minResponseTime"] = 0;
    }
    stats["maxResponseTime"] = performanceStats_.maxResponseTime.count();
    
    // 稼働時間
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - performanceStats_.startTime).count();
    stats["uptimeSeconds"] = uptime;
    
    if (uptime > 0) {
        stats["requestsPerSecond"] = static_cast<double>(performanceStats_.totalRequests) / uptime;
    } else {
        stats["requestsPerSecond"] = 0.0;
    }
    
    // メソッド別統計
    stats["byMethod"] = nlohmann::json::object();
    for (const auto& [method, count] : performanceStats_.requestsByMethod) {
        stats["byMethod"][method] = count;
    }
    
    // パス別統計（上位10件）
    std::vector<std::pair<std::string, uint64_t>> pathCounts;
    for (const auto& [path, count] : performanceStats_.requestsByPath) {
        pathCounts.push_back({path, count});
    }
    std::sort(pathCounts.begin(), pathCounts.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    stats["byPath"] = nlohmann::json::object();
    int topCount = std::min(10, static_cast<int>(pathCounts.size()));
    for (int i = 0; i < topCount; i++) {
        stats["byPath"][pathCounts[i].first] = pathCounts[i].second;
    }
    
    // ステータスコード別統計
    stats["byStatusCode"] = nlohmann::json::object();
    for (const auto& [code, count] : performanceStats_.requestsByStatusCode) {
        stats["byStatusCode"][std::to_string(code)] = count;
    }
    
    return stats;
}

void HTTPServer::ResetPerformanceStats() {
    std::lock_guard<std::mutex> lock(performanceStatsMutex_);
    performanceStats_ = PerformanceStats();
    performanceStats_.startTime = std::chrono::steady_clock::now();
    performanceStats_.minResponseTime = std::chrono::milliseconds(INT_MAX);
}

} // namespace Core

