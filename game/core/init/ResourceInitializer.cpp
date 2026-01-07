#include "ResourceInitializer.hpp"
#include "../../utils/Log.h"
#include <raylib.h>

namespace game {
namespace core {

ResourceInitializer::ResourceInitializer()
    : systemAPI_(nullptr)
    , isInitialized_(false)
{
}

ResourceInitializer::~ResourceInitializer() {
    if (isInitialized_) {
        LOG_WARN("ResourceInitializer not properly shutdown");
    }
}

bool ResourceInitializer::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("ResourceInitializer already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("ResourceInitializer: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    isInitialized_ = true;

    try {
        // リソースマネージャーの初期化
        systemAPI_->InitializeResources();

        // デフォルトフォントを設定
        systemAPI_->SetDefaultFont("NotoSansJP-Medium.ttf", 32);
        LOG_INFO("Default font set successfully");

        // ディレクトリスキャンを実行
        initState_.totalProgress = systemAPI_->ScanResourceFiles();
        initState_.scanningCompleted = true;
        initState_.currentMessage = "ファイルリストを構築しました";
        LOG_INFO("Scanned {} resource files", initState_.totalProgress);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to scan resource files: {}", e.what());
        initState_.initializationFailed = true;
        initState_.errorMessage = std::string("ファイルスキャンエラー: ") + e.what();
        return false;
    }

    return true;
}

bool ResourceInitializer::Update(float deltaTime) {
    if (!isInitialized_) {
        LOG_ERROR("ResourceInitializer not initialized");
        return false;
    }

    if (initState_.initializationFailed) {
        initState_.errorDisplayTime += deltaTime;
        return true;
    }

    if (initState_.initializationCompleted) {
        // 初期化完了後、少し待ってから遷移可能にする
        initState_.completionDelay -= deltaTime;
        return true;
    }

    // スキャン完了後、リソース読み込みを開始
    if (initState_.scanningCompleted && !initState_.initializationStarted) {
        initState_.initializationStarted = true;
        LOG_INFO("Starting resource loading");
    }

    // 読み込み処理（毎フレーム1つずつ）
    if (initState_.scanningCompleted && initState_.initializationStarted) {
        try {
            bool hasMore = systemAPI_->LoadNextResource(
                [this](const LoadProgress& progress) {
                    initState_.currentProgress = progress.current;
                    initState_.totalProgress = progress.total;
                    initState_.currentMessage = progress.message;
                });

            if (!hasMore) {
                initState_.initializationCompleted = true;
                initState_.currentMessage = "初期化完了";
                LOG_INFO("Resource initialization completed successfully");
            }
        } catch (const std::exception& e) {
            LOG_WARN("Error loading resource: {}", e.what());
            if (!systemAPI_->HasMoreResources()) {
                initState_.initializationCompleted = true;
                initState_.currentMessage = "初期化完了";
            }
        }
    }

    return true;
}

void ResourceInitializer::Render() {
    if (!isInitialized_) {
        return;
    }

    if (initState_.initializationFailed) {
        renderErrorScreen();
    } else if (!initState_.initializationCompleted) {
        renderInitScreen();
    }
}

bool ResourceInitializer::IsCompleted() const {
    return isInitialized_ && initState_.initializationCompleted && 
           initState_.completionDelay <= 0.0f && !initState_.initializationFailed;
}

bool ResourceInitializer::HasFailed() const {
    return isInitialized_ && initState_.initializationFailed;
}

bool ResourceInitializer::ShouldShutdown() const {
    return HasFailed() && initState_.errorDisplayTime >= 5.0f;
}

void ResourceInitializer::Reset() {
    if (isInitialized_) {
        LOG_WARN("ResourceInitializer: Reset called while initialized");
    }
    initState_ = InitState();
    isInitialized_ = false;
    systemAPI_ = nullptr;
}

void ResourceInitializer::renderInitScreen() {
    // ゲームタイトル表示
    const char* title = "ゲームタイトル";
    float titleFontSize = 60.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(title, titleFontSize, 1.0f);
    float titleX = systemAPI_->GetInternalWidth() / 2.0f - titleSize.x / 2.0f;
    float titleY = systemAPI_->GetInternalHeight() / 2.0f - 100.0f;
    systemAPI_->DrawTextDefault(title, titleX, titleY, titleFontSize, WHITE);

    // 進捗バー描画
    float progress = (initState_.totalProgress > 0)
                         ? (static_cast<float>(initState_.currentProgress) /
                            initState_.totalProgress)
                         : 0.0f;
    float barX = systemAPI_->GetInternalWidth() / 2.0f - 300.0f;
    float barY = systemAPI_->GetInternalHeight() / 2.0f;
    systemAPI_->DrawProgressBar(barX, barY, 600.0f, 30.0f, progress, BLUE, DARKBLUE);

    // 進捗テキスト
    std::string progressText = initState_.currentMessage;
    if (initState_.totalProgress > 0) {
        int percentage = (initState_.currentProgress * 100) / initState_.totalProgress;
        progressText += " (" + std::to_string(initState_.currentProgress) + "/" +
                       std::to_string(initState_.totalProgress) + " - " +
                       std::to_string(percentage) + "%)";
    }

    float textFontSize = 24.0f;
    Vector2 textSize = systemAPI_->MeasureTextDefault(progressText, textFontSize, 1.0f);
    float textX = systemAPI_->GetInternalWidth() / 2.0f - textSize.x / 2.0f;
    float textY = barY + 50.0f;
    systemAPI_->DrawTextDefault(progressText, textX, textY, textFontSize, BLACK);
}

void ResourceInitializer::renderErrorScreen() {
    // エラータイトル
    const char* errorTitle = "初期化エラー";
    float titleFontSize = 48.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(errorTitle, titleFontSize, 1.0f);
    float titleX = systemAPI_->GetInternalWidth() / 2.0f - titleSize.x / 2.0f;
    float titleY = 200.0f;
    systemAPI_->DrawTextDefault(errorTitle, titleX, titleY, titleFontSize, RED);

    // エラーメッセージ
    float messageFontSize = 24.0f;
    Vector2 messageSize = systemAPI_->MeasureTextDefault(initState_.errorMessage,
                                                          messageFontSize, 1.0f);
    float messageX = systemAPI_->GetInternalWidth() / 2.0f - messageSize.x / 2.0f;
    float messageY = 300.0f;
    systemAPI_->DrawTextDefault(initState_.errorMessage, messageX, messageY,
                                messageFontSize, DARKGRAY);

    // 終了メッセージ
    const char* closeMessage = "5秒後にアプリケーションを終了します...";
    float closeFontSize = 20.0f;
    Vector2 closeSize = systemAPI_->MeasureTextDefault(closeMessage, closeFontSize, 1.0f);
    float closeX = systemAPI_->GetInternalWidth() / 2.0f - closeSize.x / 2.0f;
    float closeY = 500.0f;
    systemAPI_->DrawTextDefault(closeMessage, closeX, closeY, closeFontSize, LIGHTGRAY);

    // 残り時間表示
    float remainingTime = 5.0f - initState_.errorDisplayTime;
    if (remainingTime < 0.0f)
        remainingTime = 0.0f;
    std::string timeText =
        "残り時間: " + std::to_string(static_cast<int>(remainingTime + 0.5f)) + "秒";
    Vector2 timeSize = systemAPI_->MeasureTextDefault(timeText, closeFontSize, 1.0f);
    float timeX = systemAPI_->GetInternalWidth() / 2.0f - timeSize.x / 2.0f;
    float timeY = 550.0f;
    systemAPI_->DrawTextDefault(timeText, timeX, timeY, closeFontSize, YELLOW);
}

} // namespace core
} // namespace game
