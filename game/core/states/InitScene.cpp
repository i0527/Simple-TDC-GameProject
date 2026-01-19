#include "InitScene.hpp"

#include "../../utils/Log.h"
#include <algorithm>
#include "../config/RenderPrimitives.hpp"

namespace game {
namespace core {
namespace states {

namespace {
std::string TrimPrefix(const std::string& text, const std::string& prefix) {
    if (text.rfind(prefix, 0) == 0) {
        return text.substr(prefix.size());
    }
    return text;
}

std::string TruncateText(const std::string& text, size_t maxLength) {
    if (text.size() <= maxLength) {
        return text;
    }
    return text.substr(0, maxLength - 3) + "...";
}

std::string ExtractFileName(const std::string& path) {
    size_t slashPos = path.find_last_of("/\\");
    if (slashPos == std::string::npos) {
        return path;
    }
    return path.substr(slashPos + 1);
}
} // namespace

InitScene::InitScene()
    : systemAPI_(nullptr),
      sharedContext_(nullptr),
      isInitialized_(false),
      requestTransition_(false),
      nextState_(GameState::Initializing),
      requestQuit_(false) {}

InitScene::~InitScene() {
    Shutdown();
}

bool InitScene::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("InitScene::Initialize: systemAPI is null");
        return false;
    }
    if (isInitialized_) {
        LOG_WARN("InitScene already initialized");
        return false;
    }

    systemAPI_ = systemAPI;
    initState_ = InitState();
    requestTransition_ = false;
    requestQuit_ = false;
    nextState_ = GameState::Initializing;
    isInitialized_ = true;

    try {
        // リソースマネージャーの初期化
        systemAPI_->Resource().InitializeResources();

        // デフォルトフォントを設定
        systemAPI_->Resource().SetDefaultFont("NotoSansJP-Medium.ttf", 32);
        LOG_INFO("Default font set successfully");

        // ディレクトリスキャンを実行
        initState_.totalProgress = systemAPI_->Resource().ScanResourceFiles();
        initState_.scanningCompleted = true;
        initState_.currentMessage = "ファイルリストを構築しました";
        LOG_INFO("Scanned {} resource files", initState_.totalProgress);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to scan resource files: {}", e.what());
        initState_.initializationFailed = true;
        initState_.errorMessage =
            std::string("ファイルスキャンエラー: ") + e.what();
        return false;
    }

    return true;
}

void InitScene::Update(float deltaTime) {
    if (!isInitialized_) {
        LOG_ERROR("InitScene not initialized");
        return;
    }

    if (initState_.initializationFailed) {
        initState_.errorDisplayTime += deltaTime;
        if (initState_.errorDisplayTime >= 5.0f) {
            requestQuit_ = true;
        }
        return;
    }

    if (initState_.initializationCompleted) {
        // 初期化完了後、少し待ってから遷移可能にする
        initState_.completionDelay -= deltaTime;
        if (initState_.completionDelay <= 0.0f) {
            requestTransition_ = true;
            nextState_ = GameState::Title;
        }
        return;
    }

    // スキャン完了後、リソース読み込みを開始
    if (initState_.scanningCompleted && !initState_.initializationStarted) {
        initState_.initializationStarted = true;
        LOG_INFO("Starting resource loading");
    }

    // 読み込み処理（毎フレーム1つずつ）
    if (initState_.scanningCompleted && initState_.initializationStarted) {
        if (systemAPI_->Resource().HasMoreResources()) {
            updateCurrentDisplay(systemAPI_->Resource().GetCurrentProgress());
        } else {
            initState_.initializationCompleted = true;
            initState_.currentMessage = "初期化完了";
            initState_.currentPath = "";
            LOG_INFO("Resource initialization completed successfully");
            return;
        }

        const float frameBudget = std::min(deltaTime, 1.0f / 60.0f);
        const double startTime = GetTime();
        bool loadedAny = false;
        while (systemAPI_->Resource().HasMoreResources()) {
            if (loadedAny) {
                const double elapsed = GetTime() - startTime;
                if (elapsed >= static_cast<double>(frameBudget)) {
                    break;
                }
            }

            try {
                bool hasMore =
                    systemAPI_->Resource().LoadNextResource(
                        [this](const LoadProgress& progress) {
                        initState_.currentProgress = progress.current;
                        initState_.totalProgress = progress.total;
                        updateCategoryStats(progress);
                    });
                loadedAny = true;

                if (!hasMore) {
                    initState_.initializationCompleted = true;
                    initState_.currentMessage = "初期化完了";
                    initState_.currentPath = "";
                    LOG_INFO("Resource initialization completed successfully");
                    break;
                }
            } catch (const std::exception& e) {
                loadedAny = true;
                LOG_WARN("Error loading resource: {}", e.what());
                if (!systemAPI_->Resource().HasMoreResources()) {
                    initState_.initializationCompleted = true;
                    initState_.currentMessage = "初期化完了";
                    initState_.currentPath = "";
                    break;
                }
            }
        }
    }
}

void InitScene::Render() {
    if (!isInitialized_) {
        return;
    }

    if (initState_.initializationFailed) {
        renderErrorScreen();
    } else if (!initState_.initializationCompleted) {
        renderInitScreen();
    }
}

void InitScene::Shutdown() {
    if (!isInitialized_) {
        return;
    }
    initState_ = InitState();
    isInitialized_ = false;
    systemAPI_ = nullptr;
    sharedContext_ = nullptr;
    requestTransition_ = false;
    requestQuit_ = false;
    nextState_ = GameState::Initializing;
}

bool InitScene::RequestTransition(GameState& nextState) {
    if (requestTransition_) {
        nextState = nextState_;
        requestTransition_ = false;
        return true;
    }
    return false;
}

bool InitScene::RequestQuit() {
    bool result = requestQuit_;
    requestQuit_ = false;
    return result;
}

void InitScene::renderInitScreen() {
    // ゲームタイトル表示
    const char* title = "ゲームタイトル";
    float titleFontSize = 60.0f;
    Vec2 titleSize =
        systemAPI_->Render().MeasureTextDefaultCore(title, titleFontSize, 1.0f);
    float titleX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - titleSize.x / 2.0f;
    float titleY =
        systemAPI_->Render().GetInternalHeight() / 2.0f - 100.0f;
    systemAPI_->Render().DrawTextDefault(title, titleX, titleY, titleFontSize,
                                         ToCoreColor(WHITE));

    // 進捗バー描画
    float progress = (initState_.totalProgress > 0)
                         ? (static_cast<float>(initState_.currentProgress) /
                            initState_.totalProgress)
                         : 0.0f;
    float barX = systemAPI_->Render().GetInternalWidth() / 2.0f - 300.0f;
    float barY = systemAPI_->Render().GetInternalHeight() / 2.0f;
    systemAPI_->Render().DrawProgressBar(barX, barY, 600.0f, 30.0f, progress,
                                         ToCoreColor(BLUE),
                                         ToCoreColor(DARKBLUE),
                                         ToCoreColor(WHITE));

    // 進捗テキスト
    std::string progressText = initState_.currentMessage;
    if (initState_.totalProgress > 0) {
        progressText += " (" + std::to_string(initState_.currentProgress) + "/" +
                        std::to_string(initState_.totalProgress) + ")";
    }
    float textFontSize = 24.0f;
    Vec2 textSize =
        systemAPI_->Render().MeasureTextDefaultCore(progressText, textFontSize, 1.0f);
    float textX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - textSize.x / 2.0f;
    float textY = barY + 50.0f;
    systemAPI_->Render().DrawTextDefault(progressText, textX, textY, textFontSize,
                                         ToCoreColor(BLACK));

    // パーセント表示（バーの横）
    int percentage = 0;
    if (initState_.totalProgress > 0) {
        percentage = (initState_.currentProgress * 100) / initState_.totalProgress;
    }
    std::string percentText = std::to_string(percentage) + "%";
    float percentFontSize = 22.0f;
    Vec2 percentSize =
        systemAPI_->Render().MeasureTextDefaultCore(percentText, percentFontSize, 1.0f);
    float percentX = barX + 600.0f + 12.0f;
    float percentY = barY + 30.0f / 2.0f - percentSize.y / 2.0f;
    systemAPI_->Render().DrawTextDefault(percentText, percentX, percentY,
                                         percentFontSize, ToCoreColor(BLACK));

    // 読み込み中のパス表示
    std::string pathText =
        "パス: " +
        (initState_.currentPath.empty() ? "-" : initState_.currentPath);
    float pathFontSize = 18.0f;
    Vec2 pathSize =
        systemAPI_->Render().MeasureTextDefaultCore(pathText, pathFontSize, 1.0f);
    float pathX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - pathSize.x / 2.0f;
    float pathY = textY + 30.0f;
    systemAPI_->Render().DrawTextDefault(pathText, pathX, pathY, pathFontSize,
                                         ToCoreColor(DARKGRAY));

    // カテゴリ別パネル
    float panelWidth = 860.0f;
    float panelHeight = 260.0f;
    float panelX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - panelWidth / 2.0f;
    float panelY = textY + 80.0f;
    systemAPI_->Render().DrawRectangle(panelX, panelY, panelWidth, panelHeight,
                                       ToCoreColor(Fade(LIGHTGRAY, 0.5f)));
    systemAPI_->Render().DrawRectangleLines(panelX, panelY, panelWidth,
                                            panelHeight, 2.0f,
                                            ToCoreColor(DARKGRAY));

    float rowFontSize = 20.0f;
    float rowX = panelX + 30.0f;
    float rowY = panelY + 20.0f;
    float rowGap = 44.0f;

    auto drawRow = [&](const char* label, const InitState::CategoryStats& stats) {
        std::string nameText =
            stats.lastName.empty() ? "-" : TruncateText(stats.lastName, 32);
        std::string rowText = std::string(label) + ": " +
                              std::to_string(stats.loaded) + " / last: " + nameText;
        systemAPI_->Render().DrawTextDefault(rowText, rowX, rowY, rowFontSize,
                                             ToCoreColor(BLACK));
        rowY += rowGap;
    };

    drawRow("フォント", initState_.font);
    drawRow("テクスチャ", initState_.texture);
    drawRow("サウンド", initState_.sound);
    drawRow("設定ファイル", initState_.json);
    drawRow("その他", initState_.other);
}

void InitScene::renderErrorScreen() {
    // エラータイトル
    const char* errorTitle = "初期化エラー";
    float titleFontSize = 48.0f;
    Vec2 titleSize =
        systemAPI_->Render().MeasureTextDefaultCore(errorTitle, titleFontSize, 1.0f);
    float titleX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - titleSize.x / 2.0f;
    float titleY = 200.0f;
    systemAPI_->Render().DrawTextDefault(errorTitle, titleX, titleY,
                                         titleFontSize, ToCoreColor(RED));

    // エラーメッセージ
    float messageFontSize = 24.0f;
    Vec2 messageSize = systemAPI_->Render().MeasureTextDefaultCore(
        initState_.errorMessage, messageFontSize, 1.0f);
    float messageX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - messageSize.x / 2.0f;
    float messageY = 300.0f;
    systemAPI_->Render().DrawTextDefault(initState_.errorMessage, messageX,
                                         messageY, messageFontSize,
                                         ToCoreColor(DARKGRAY));

    // 終了メッセージ
    const char* closeMessage = "5秒後にアプリケーションを終了します...";
    float closeFontSize = 20.0f;
    Vec2 closeSize =
        systemAPI_->Render().MeasureTextDefaultCore(closeMessage, closeFontSize, 1.0f);
    float closeX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - closeSize.x / 2.0f;
    float closeY = 500.0f;
    systemAPI_->Render().DrawTextDefault(closeMessage, closeX, closeY,
                                         closeFontSize, ToCoreColor(LIGHTGRAY));

    // 残り時間表示
    float remainingTime = 5.0f - initState_.errorDisplayTime;
    if (remainingTime < 0.0f) {
        remainingTime = 0.0f;
    }
    std::string timeText =
        "残り時間: " + std::to_string(static_cast<int>(remainingTime + 0.5f)) +
        "秒";
    Vec2 timeSize =
        systemAPI_->Render().MeasureTextDefaultCore(timeText, closeFontSize, 1.0f);
    float timeX =
        systemAPI_->Render().GetInternalWidth() / 2.0f - timeSize.x / 2.0f;
    float timeY = 550.0f;
    systemAPI_->Render().DrawTextDefault(timeText, timeX, timeY, closeFontSize,
                                         ToCoreColor(YELLOW));
}

void InitScene::updateCategoryStats(const LoadProgress& progress) {
    const std::string& message = progress.message;
    if (message.find("フォントを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "フォントを読み込み中: ");
        initState_.font.loaded++;
        initState_.font.lastName = ExtractFileName(path);
        initState_.currentMessage = "フォントを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("テクスチャを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "テクスチャを読み込み中: ");
        initState_.texture.loaded++;
        initState_.texture.lastName = ExtractFileName(path);
        initState_.currentMessage = "テクスチャを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("サウンドを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "サウンドを読み込み中: ");
        initState_.sound.loaded++;
        initState_.sound.lastName = ExtractFileName(path);
        initState_.currentMessage = "サウンドを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("設定ファイルを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "設定ファイルを読み込み中: ");
        initState_.json.loaded++;
        initState_.json.lastName = ExtractFileName(path);
        initState_.currentMessage = "設定ファイルを読み込み中";
        initState_.currentPath = path;
        return;
    }

    initState_.other.loaded++;
    initState_.other.lastName = message;
    initState_.currentMessage = "読み込み中";
    initState_.currentPath = message;
}

void InitScene::updateCurrentDisplay(const LoadProgress& progress) {
    const std::string& message = progress.message;
    if (message.find("フォントを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "フォントを読み込み中: ");
        initState_.currentMessage = "フォントを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("テクスチャを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "テクスチャを読み込み中: ");
        initState_.currentMessage = "テクスチャを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("サウンドを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "サウンドを読み込み中: ");
        initState_.currentMessage = "サウンドを読み込み中";
        initState_.currentPath = path;
        return;
    }
    if (message.find("設定ファイルを読み込み中:") == 0) {
        std::string path = TrimPrefix(message, "設定ファイルを読み込み中: ");
        initState_.currentMessage = "設定ファイルを読み込み中";
        initState_.currentPath = path;
        return;
    }

    initState_.currentMessage = "読み込み中";
    initState_.currentPath = message;
}

} // namespace states
} // namespace core
} // namespace game
