#include "InitScene.hpp"

#include "../../utils/Log.h"
#include "../config/RenderPrimitives.hpp"
#include "../ui/OverlayColors.hpp"
#include <algorithm>
#include <cmath>


namespace game {
namespace core {
namespace states {

namespace {
std::string TrimPrefix(const std::string &text, const std::string &prefix) {
  if (text.rfind(prefix, 0) == 0) {
    return text.substr(prefix.size());
  }
  return text;
}

std::string TruncateText(const std::string &text, size_t maxLength) {
  if (text.size() <= maxLength) {
    return text;
  }
  return text.substr(0, maxLength - 3) + "...";
}

std::string ExtractFileName(const std::string &path) {
  size_t slashPos = path.find_last_of("/\\");
  if (slashPos == std::string::npos) {
    return path;
  }
  return path.substr(slashPos + 1);
}
} // namespace

InitScene::InitScene()
    : systemAPI_(nullptr), sharedContext_(nullptr), isInitialized_(false),
      requestTransition_(false), nextState_(GameState::Initializing),
      requestQuit_(false), animationTime_(0.0f), smoothProgress_(0.0f) {}

InitScene::~InitScene() { Shutdown(); }

bool InitScene::Initialize(BaseSystemAPI *systemAPI) {
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
  animationTime_ = 0.0f;
  smoothProgress_ = 0.0f;
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
  } catch (const std::exception &e) {
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

  // アニメーション時間を更新
  animationTime_ += deltaTime;

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

    // スムーズな進捗バーアニメーション
    float targetProgress =
        (initState_.totalProgress > 0)
            ? (static_cast<float>(initState_.currentProgress) /
               initState_.totalProgress)
            : 0.0f;
    const float smoothSpeed = 5.0f; // アニメーション速度
    smoothProgress_ +=
        (targetProgress - smoothProgress_) * smoothSpeed * deltaTime;

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
        bool hasMore = systemAPI_->Resource().LoadNextResource(
            [this](const LoadProgress &progress) {
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
      } catch (const std::exception &e) {
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
  animationTime_ = 0.0f;
  smoothProgress_ = 0.0f;
}

bool InitScene::RequestTransition(GameState &nextState) {
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
  using namespace ui::OverlayColors;

  const float screenWidth = systemAPI_->Render().GetInternalWidth();
  const float screenHeight = systemAPI_->Render().GetInternalHeight();
  const float centerX = screenWidth / 2.0f;
  const float centerY = screenHeight / 2.0f;

  // ========== グラデーション背景 ==========
  systemAPI_->Render().DrawRectangleGradientV(
      0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight),
      ToCoreColor(PANEL_BG_GITHUB), ToCoreColor(MAIN_BG));

  // ========== ゲームタイトル表示 ==========
  const char *title = "tower of defense";
  float titleFontSize = 64.0f;
  Vec2 titleSize =
      systemAPI_->Render().MeasureTextDefaultCore(title, titleFontSize, 1.0f);
  float titleX = centerX - titleSize.x / 2.0f;
  float titleY = centerY - 280.0f;

  // タイトルにシャドウ効果
  systemAPI_->Render().DrawTextDefault(
      title, titleX + 2.0f, titleY + 2.0f, titleFontSize,
      ToCoreColor({0, 0, 0, 100})); // 半透明黒のシャドウ
  systemAPI_->Render().DrawTextDefault(title, titleX, titleY, titleFontSize,
                                       ToCoreColor(TEXT_MAIN_GITHUB));

  // ========== パーセント表示（進捗バーの上） ==========
  int percentage = 0;
  if (initState_.totalProgress > 0) {
    percentage = (initState_.currentProgress * 100) / initState_.totalProgress;
  }
  std::string percentText = std::to_string(percentage) + "%";
  float percentFontSize = 48.0f;
  Vec2 percentSize = systemAPI_->Render().MeasureTextDefaultCore(
      percentText, percentFontSize, 1.0f);
  float percentX = centerX - percentSize.x / 2.0f;
  float percentY = centerY - 150.0f; // 進捗バーの上に配置

  // パーセント表示のシャドウ
  systemAPI_->Render().DrawTextDefault(
      percentText, percentX + 2.0f, percentY + 2.0f, percentFontSize,
      ToCoreColor({0, 0, 0, 100})); // 半透明黒のシャドウ
  systemAPI_->Render().DrawTextDefault(percentText, percentX, percentY,
                                       percentFontSize,
                                       ToCoreColor(TEXT_PRIMARY));

  // ========== 進捗バー描画 ==========
  float barWidth = 700.0f;
  float barHeight = 40.0f;
  float barX = centerX - barWidth / 2.0f;
  float barY = centerY - 80.0f;

  // スムーズな進捗値を使用
  float progress = std::max(0.0f, std::min(1.0f, smoothProgress_));

  // 進捗バーの背景（シャドウ付き）
  float shadowOffset = 4.0f;
  systemAPI_->Render().DrawRectangle(barX + shadowOffset, barY + shadowOffset,
                                     barWidth, barHeight,
                                     ToCoreColor({0, 0, 0, 80})); // シャドウ

  // 進捗バー本体
  systemAPI_->Render().DrawProgressBar(
      barX, barY, barWidth, barHeight, progress,
      ToCoreColor(ACCENT_BLUE),         // フィル色
      ToCoreColor(PANEL_BG_SECONDARY),  // 背景色
      ToCoreColor(CARD_BORDER_NORMAL)); // ボーダー色

  // 進捗バーにグラデーション効果（上部にハイライト）
  if (progress > 0.0f) {
    float fillWidth = barWidth * progress;
    ColorRGBA highlightColor = ToCoreColor({255, 255, 255, 30});
    systemAPI_->Render().DrawRectangle(barX, barY, fillWidth, barHeight * 0.3f,
                                       highlightColor);
  }

  // ========== 進捗テキスト ==========
  std::string progressText = initState_.currentMessage;
  if (initState_.totalProgress > 0) {
    progressText += " (" + std::to_string(initState_.currentProgress) + "/" +
                    std::to_string(initState_.totalProgress) + ")";
  }
  float textFontSize = 22.0f;
  Vec2 textSize = systemAPI_->Render().MeasureTextDefaultCore(
      progressText, textFontSize, 1.0f);
  float textX = centerX - textSize.x / 2.0f;
  float textY = barY + barHeight + 20.0f;
  systemAPI_->Render().DrawTextDefault(progressText, textX, textY, textFontSize,
                                       ToCoreColor(TEXT_SECONDARY));

  // ========== 読み込み中のパス表示 ==========
  std::string pathText = initState_.currentPath.empty()
                             ? ""
                             : TruncateText(initState_.currentPath, 60);
  if (!pathText.empty()) {
    float pathFontSize = 16.0f;
    Vec2 pathSize = systemAPI_->Render().MeasureTextDefaultCore(
        pathText, pathFontSize, 1.0f);
    float pathX = centerX - pathSize.x / 2.0f;
    float pathY = textY + 35.0f;
    systemAPI_->Render().DrawTextDefault(pathText, pathX, pathY, pathFontSize,
                                         ToCoreColor(TEXT_MUTED));
  }

  // ========== 読み込みインジケーター（アニメーション） ==========
  if (!initState_.initializationCompleted) {
    float indicatorX = centerX;
    float indicatorY = textY + 60.0f;
    float dotRadius = 6.0f;
    float dotSpacing = 20.0f;
    float animSpeed = 2.0f;
    float phase = animationTime_ * animSpeed;

    for (int i = 0; i < 3; ++i) {
      float offset = (i - 1) * dotSpacing;
      float alpha = 0.3f + 0.7f * (0.5f + 0.5f * std::sin(phase + i * 2.0f));
      ColorRGBA dotColor = ToCoreColor({static_cast<uint8_t>(ACCENT_BLUE.r),
                                        static_cast<uint8_t>(ACCENT_BLUE.g),
                                        static_cast<uint8_t>(ACCENT_BLUE.b),
                                        static_cast<uint8_t>(255 * alpha)});
      systemAPI_->Render().DrawCircle(indicatorX + offset, indicatorY,
                                      dotRadius, dotColor);
    }
  }

  // ========== カテゴリ別パネル ==========
  float panelWidth = 1000.0f;
  float panelHeight = 320.0f; // 下方向に40px追加
  float panelX = centerX - panelWidth / 2.0f;
  float panelY = textY + 120.0f;

  // パネル背景（シャドウ付き）
  float panelShadowOffset = 6.0f;
  systemAPI_->Render().DrawRectangle(
      panelX + panelShadowOffset, panelY + panelShadowOffset, panelWidth,
      panelHeight, ToCoreColor({0, 0, 0, 100})); // シャドウ

  // パネル本体
  systemAPI_->Render().DrawRectangle(panelX, panelY, panelWidth, panelHeight,
                                     ToCoreColor(CARD_BG_GITHUB));

  // パネルボーダー
  systemAPI_->Render().DrawRectangleLines(panelX, panelY, panelWidth,
                                          panelHeight, 2.0f,
                                          ToCoreColor(CARD_BORDER_NORMAL));

  // パネルヘッダー
  const char *panelTitle = "読み込み状況";
  float headerFontSize = 24.0f;
  Vec2 headerSize = systemAPI_->Render().MeasureTextDefaultCore(
      panelTitle, headerFontSize, 1.0f);
  float headerX = panelX + 30.0f;
  float headerY = panelY + 20.0f;
  systemAPI_->Render().DrawTextDefault(
      panelTitle, headerX, headerY, headerFontSize, ToCoreColor(TEXT_PRIMARY));

  // ヘッダー下の区切り線
  systemAPI_->Render().DrawLine(panelX + 20.0f, headerY + headerSize.y + 10.0f,
                                panelX + panelWidth - 20.0f,
                                headerY + headerSize.y + 10.0f, 1.0f,
                                ToCoreColor(DIVIDER));

  // カテゴリ行の描画
  float rowFontSize = 18.0f;
  float rowX = panelX + 40.0f;
  float rowY = headerY + headerSize.y + 30.0f;
  float rowGap = 48.0f;

  // レイアウト: ラベル(120px) + カウント(80px) + ファイル名(残り)
  float labelWidth = 120.0f;
  float countWidth = 80.0f;
  float nameX = rowX + labelWidth + countWidth;
  float maxNameWidth = panelWidth - (rowX - panelX) - labelWidth - countWidth -
                       40.0f; // 右マージン40px

  auto drawRow = [&](const char *label, const InitState::CategoryStats &stats,
                     ColorRGBA labelColor) {
    // ラベル
    systemAPI_->Render().DrawTextDefault(label, rowX, rowY, rowFontSize,
                                         labelColor);

    // カウント
    std::string countText = std::to_string(stats.loaded);
    float countX = rowX + labelWidth;
    systemAPI_->Render().DrawTextDefault(countText, countX, rowY, rowFontSize,
                                         ToCoreColor(TEXT_SECONDARY));

    // 最後に読み込んだファイル名（はみだし防止）
    std::string nameText = stats.lastName.empty() ? "-" : stats.lastName;

    // テキスト幅を測定して、はみだす場合は切り詰め
    Vec2 nameSize = systemAPI_->Render().MeasureTextDefaultCore(
        nameText, rowFontSize, 1.0f);
    if (nameSize.x > maxNameWidth) {
      // 切り詰めが必要な場合、適切な長さを計算
      // 大まかな文字数推定（日本語は幅が広いため、35文字程度）
      nameText = TruncateText(stats.lastName, 35);
    }

    systemAPI_->Render().DrawTextDefault(nameText, nameX, rowY, rowFontSize,
                                         ToCoreColor(TEXT_MUTED));

    rowY += rowGap;
  };

  drawRow("フォント", initState_.font, ToCoreColor(ACCENT_BLUE));
  drawRow("テクスチャ", initState_.texture, ToCoreColor(SUCCESS_GREEN));
  drawRow("サウンド", initState_.sound, ToCoreColor(ACCENT_GOLD));
  drawRow("設定ファイル", initState_.json, ToCoreColor(WARNING_ORANGE));
  drawRow("その他", initState_.other, ToCoreColor(TEXT_SECONDARY));
}

void InitScene::renderErrorScreen() {
  using namespace ui::OverlayColors;

  const float screenWidth = systemAPI_->Render().GetInternalWidth();
  const float screenHeight = systemAPI_->Render().GetInternalHeight();
  const float centerX = screenWidth / 2.0f;
  const float centerY = screenHeight / 2.0f;

  // ========== グラデーション背景 ==========
  systemAPI_->Render().DrawRectangleGradientV(
      0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight),
      ToCoreColor({40, 20, 20, 255}), // 暗めの赤系
      ToCoreColor(MAIN_BG));

  // ========== エラーパネル ==========
  float panelWidth = 800.0f;
  float panelHeight = 440.0f;
  float panelX = centerX - panelWidth / 2.0f;
  float panelY = centerY - panelHeight / 2.0f;

  // パネルシャドウ
  float shadowOffset = 8.0f;
  systemAPI_->Render().DrawRectangle(panelX + shadowOffset,
                                     panelY + shadowOffset, panelWidth,
                                     panelHeight, ToCoreColor({0, 0, 0, 150}));

  // パネル背景
  systemAPI_->Render().DrawRectangle(panelX, panelY, panelWidth, panelHeight,
                                     ToCoreColor(CARD_BG_GITHUB));

  // パネルボーダー（エラー色）
  systemAPI_->Render().DrawRectangleLines(
      panelX, panelY, panelWidth, panelHeight, 3.0f, ToCoreColor(DANGER_RED));

  // ========== エラーアイコン（シンプルな×マーク） ==========
  float iconSize = 60.0f;
  float iconX = centerX - iconSize / 2.0f;
  float iconY = panelY + 40.0f;
  float lineThickness = 6.0f;
  systemAPI_->Render().DrawLine(iconX, iconY, iconX + iconSize,
                                iconY + iconSize, lineThickness,
                                ToCoreColor(DANGER_RED));
  systemAPI_->Render().DrawLine(iconX + iconSize, iconY, iconX,
                                iconY + iconSize, lineThickness,
                                ToCoreColor(DANGER_RED));

  // ========== エラータイトル ==========
  const char *errorTitle = "初期化エラー";
  float titleFontSize = 42.0f;
  Vec2 titleSize = systemAPI_->Render().MeasureTextDefaultCore(
      errorTitle, titleFontSize, 1.0f);
  float titleX = centerX - titleSize.x / 2.0f;
  float titleY = iconY + iconSize + 30.0f;

  // タイトルシャドウ
  systemAPI_->Render().DrawTextDefault(errorTitle, titleX + 2.0f, titleY + 2.0f,
                                       titleFontSize,
                                       ToCoreColor({0, 0, 0, 100}));
  systemAPI_->Render().DrawTextDefault(errorTitle, titleX, titleY,
                                       titleFontSize, ToCoreColor(DANGER_RED));

  // ========== エラーメッセージ ==========
  float messageFontSize = 20.0f;
  Vec2 messageSize = systemAPI_->Render().MeasureTextDefaultCore(
      initState_.errorMessage, messageFontSize, 1.0f);
  float messageX = centerX - messageSize.x / 2.0f;
  float messageY = titleY + titleSize.y + 30.0f;

  // メッセージを複数行に分割（長すぎる場合）
  std::string message = initState_.errorMessage;
  if (messageSize.x > panelWidth - 80.0f) {
    // 長いメッセージは切り詰める
    message = TruncateText(message, 50);
    messageSize = systemAPI_->Render().MeasureTextDefaultCore(
        message, messageFontSize, 1.0f);
    messageX = centerX - messageSize.x / 2.0f;
  }

  systemAPI_->Render().DrawTextDefault(message, messageX, messageY,
                                       messageFontSize,
                                       ToCoreColor(TEXT_SECONDARY));

  // ========== 区切り線 ==========
  float dividerY = messageY + messageSize.y + 30.0f;
  systemAPI_->Render().DrawLine(panelX + 40.0f, dividerY,
                                panelX + panelWidth - 40.0f, dividerY, 1.0f,
                                ToCoreColor(DIVIDER));

  // ========== 終了メッセージ ==========
  const char *closeMessage = "アプリケーションを終了します...";
  float closeFontSize = 18.0f;
  Vec2 closeSize = systemAPI_->Render().MeasureTextDefaultCore(
      closeMessage, closeFontSize, 1.0f);
  float closeX = centerX - closeSize.x / 2.0f;
  float closeY = dividerY + 30.0f;
  systemAPI_->Render().DrawTextDefault(closeMessage, closeX, closeY,
                                       closeFontSize, ToCoreColor(TEXT_MUTED));

  // ========== 残り時間表示（アニメーション付き） ==========
  float remainingTime = 5.0f - initState_.errorDisplayTime;
  if (remainingTime < 0.0f) {
    remainingTime = 0.0f;
  }
  int remainingSeconds = static_cast<int>(remainingTime + 0.5f);

  // パルス効果（残り時間が少ない時）
  float pulseAlpha = 1.0f;
  if (remainingTime < 2.0f) {
    pulseAlpha = 0.7f + 0.3f * std::sin(animationTime_ * 8.0f);
  }

  std::string timeText = "残り時間: " + std::to_string(remainingSeconds) + "秒";
  float timeFontSize = 28.0f;
  Vec2 timeSize =
      systemAPI_->Render().MeasureTextDefaultCore(timeText, timeFontSize, 1.0f);
  float timeX = centerX - timeSize.x / 2.0f;
  float timeY = closeY + closeSize.y + 20.0f;

  ColorRGBA timeColor = ToCoreColor({static_cast<uint8_t>(WARNING_ORANGE.r),
                                     static_cast<uint8_t>(WARNING_ORANGE.g),
                                     static_cast<uint8_t>(WARNING_ORANGE.b),
                                     static_cast<uint8_t>(255 * pulseAlpha)});

  // 時間表示のシャドウ
  systemAPI_->Render().DrawTextDefault(timeText, timeX + 2.0f, timeY + 2.0f,
                                       timeFontSize,
                                       ToCoreColor({0, 0, 0, 100}));
  systemAPI_->Render().DrawTextDefault(timeText, timeX, timeY, timeFontSize,
                                       timeColor);
}

void InitScene::updateCategoryStats(const LoadProgress &progress) {
  const std::string &message = progress.message;

  // 英語メッセージパターン（ResourceSystemAPIが生成）
  if (message.find("Loading font: ") == 0) {
    std::string path = TrimPrefix(message, "Loading font: ");
    initState_.font.loaded++;
    initState_.font.lastName = ExtractFileName(path);
    initState_.currentMessage = "フォントを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading texture: ") == 0) {
    std::string path = TrimPrefix(message, "Loading texture: ");
    initState_.texture.loaded++;
    initState_.texture.lastName = ExtractFileName(path);
    initState_.currentMessage = "テクスチャを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading sound: ") == 0) {
    std::string path = TrimPrefix(message, "Loading sound: ");
    initState_.sound.loaded++;
    initState_.sound.lastName = ExtractFileName(path);
    initState_.currentMessage = "サウンドを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading json: ") == 0) {
    std::string path = TrimPrefix(message, "Loading json: ");
    initState_.json.loaded++;
    initState_.json.lastName = ExtractFileName(path);
    initState_.currentMessage = "設定ファイルを読み込み中";
    initState_.currentPath = path;
    return;
  }

  // 日本語メッセージパターン（後方互換性）
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

  // その他
  initState_.other.loaded++;
  initState_.other.lastName = ExtractFileName(message);
  initState_.currentMessage = "読み込み中";
  initState_.currentPath = message;
}

void InitScene::updateCurrentDisplay(const LoadProgress &progress) {
  const std::string &message = progress.message;

  // 英語メッセージパターン（ResourceSystemAPIが生成）
  if (message.find("Loading font: ") == 0) {
    std::string path = TrimPrefix(message, "Loading font: ");
    initState_.currentMessage = "フォントを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading texture: ") == 0) {
    std::string path = TrimPrefix(message, "Loading texture: ");
    initState_.currentMessage = "テクスチャを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading sound: ") == 0) {
    std::string path = TrimPrefix(message, "Loading sound: ");
    initState_.currentMessage = "サウンドを読み込み中";
    initState_.currentPath = path;
    return;
  }
  if (message.find("Loading json: ") == 0) {
    std::string path = TrimPrefix(message, "Loading json: ");
    initState_.currentMessage = "設定ファイルを読み込み中";
    initState_.currentPath = path;
    return;
  }

  // 日本語メッセージパターン（後方互換性）
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
