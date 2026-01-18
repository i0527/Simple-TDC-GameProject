#include "../../utils/Log.h"
#include "BaseSystemAPI.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <rlImGui.h>
#include <imgui.h>

namespace game {
    namespace core {
        namespace {
            std::string NormalizeSlashes(std::string s) {
                std::replace(s.begin(), s.end(), '\\', '/');
                return s;
            }

            bool StartsWith(const std::string& s, const std::string& prefix) {
                return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
            }

            // テクスチャキーの正規化:
            // - '\' -> '/'
            // - "data/" プレフィックスは除去
            // - "assets/" 系は拡張子が無ければ .png を補う
            std::string NormalizeTextureKey(std::string key) {
                key = NormalizeSlashes(std::move(key));

                // 余計な "./" を除去
                if (StartsWith(key, "./")) {
                    key = key.substr(2);
                }

                // data/ プレフィックスを除去（呼び出し側が data/assets/... を渡しても許容）
                if (StartsWith(key, "data/")) {
                    key = key.substr(5);
                }

                // assets/ 配下の拡張子補完（"assets/..../move" のような入力を許容）
                if (StartsWith(key, "assets/")) {
                    const std::filesystem::path p(key);
                    if (!p.has_extension()) {
                        key += ".png";
                    }
                }

                return key;
            }

            // data/ を基準にした相対パス（例: data/assets/characters/A/move.png -> assets/characters/A/move.png）
            std::string MakeAssetsRelativeKey(const std::filesystem::path& fullPath) {
                try {
                    std::filesystem::path rel = std::filesystem::relative(fullPath, std::filesystem::path("data"));
                    return rel.generic_string();
                } catch (...) {
                    // relative が失敗した場合はフォールバック（スラッシュ正規化のみ）
                    return NormalizeSlashes(fullPath.string());
                }
            }
        } // namespace

        // ========== コンストラクタ・デストラクタ ==========

        BaseSystemAPI::BaseSystemAPI()
            : currentResolution_(Resolution::FHD)
            , screenWidth_(GetResolutionWidth(Resolution::FHD))
            , screenHeight_(GetResolutionHeight(Resolution::FHD))
            , mainRenderTexture_({0})
            , isInitialized_(false)
            , resourcesInitialized_(false)
            , imGuiInitialized_(false)
            , imGuiJapaneseFont_(nullptr)
            , currentResourceIndex_(0)
            , scanningCompleted_(false)
            , masterVolume_(1.0f)
            , seVolume_(1.0f)
            , bgmVolume_(1.0f)
            , currentMusic_(nullptr)
            , currentMusicName_("")
            , fpsDisplayEnabled_(false)
            , logInitialized_(false)
            , logDirectory_("logs")
            , logFileName_("game.log")
        {
            GenerateFontCodepoints();
        }

        BaseSystemAPI::~BaseSystemAPI() {
            if (isInitialized_) {
                Shutdown();
            }
        }

        // ========== 初期化・終了 ==========

        bool BaseSystemAPI::Initialize(Resolution initialResolution) {
            // ログシステムを最初に初期化
            try {
                InitializeLogSystem();
            } catch (const std::exception& e) {
                // ログシステムの初期化に失敗した場合、標準エラー出力に出力して続行
                std::cerr << "BaseSystemAPI::Initialize: Log system initialization failed: " 
                          << e.what() << ". Continuing without file logging." << std::endl;
                // ログシステムの初期化失敗は致命的ではないため、続行
            }

            screenWidth_ = GetResolutionWidth(initialResolution);
            screenHeight_ = GetResolutionHeight(initialResolution);
            currentResolution_ = initialResolution;

            // Raylib ウィンドウ初期化
            InitWindow(screenWidth_, screenHeight_, "Cat Tower Defense");
            if (!IsWindowReady()) {
                if (logger_) {
                    logger_->error("BaseSystemAPI: Failed to initialize Raylib window");
                } else {
                    std::cerr << "BaseSystemAPI: Failed to initialize Raylib window" << std::endl;
                }
                return false;
            }

            // ウィンドウのリサイズを無効化
            ClearWindowState(FLAG_WINDOW_RESIZABLE);

            SetTargetFPS(TARGET_FPS);

            // オーディオデバイス初期化
            InitAudioDevice();

            // RenderTexture 作成（内部解像度で）
            RecreateRenderTexture();
            if (mainRenderTexture_.id == 0) {
                if (logger_) {
                    logger_->error("BaseSystemAPI: Failed to create RenderTexture");
                } else {
                    std::cerr << "BaseSystemAPI: Failed to create RenderTexture" << std::endl;
                }
                CloseAudioDevice();
                CloseWindow();
                return false;
            }

            // ImGUI初期化はSetDefaultFont()で行う（フォント設定後に初期化）

            isInitialized_ = true;

            if (logger_) {
                logger_->info("BaseSystemAPI: Initialized with resolution {}x{} (internal {}x{})",
                            screenWidth_, screenHeight_, INTERNAL_WIDTH, INTERNAL_HEIGHT);
            }
            return true;
        }

        void BaseSystemAPI::Shutdown() {
            if (!isInitialized_) {
                return;
            }

            // 明確な順序でリソースを解放

            // 1. リソースキャッシュのクリア
            LOG_INFO("BaseSystemAPI shutdown: {} textures, {} sounds, {} musics, {} fonts",
                     textures_.size(), sounds_.size(), musics_.size(), fonts_.size());

            // デフォルトフォントの参照を先にクリア（循環参照を防ぐ）
            defaultFont_.reset();

            // リソースを解放順序に注意して解放
            // 1. 音楽（ストリーミングリソース）
            musics_.clear();
            
            // 2. サウンド
            sounds_.clear();
            
            // 3. フォント（GPUテクスチャを含む）
            fonts_.clear();
            
            // 4. テクスチャ（最後に解放）
            textures_.clear();

            // 2. ImGUI終了処理（RenderTexture解放の前）
            if (imGuiInitialized_) {
                rlImGuiShutdown();
                imGuiInitialized_ = false;
            }

            // 3. RenderTextureの解放
            if (mainRenderTexture_.id != 0) {
                UnloadRenderTexture(mainRenderTexture_);
                mainRenderTexture_ = {0};
            }

            // 4. オーディオ管理のクリーンアップ
            // 全Soundを停止
            for (auto& pair : playingSounds_) {
                Sound* sound = pair.second;
                if (sound) {
                    ::StopSound(*sound);
                }
            }
            playingSounds_.clear();

            // Musicを停止
            if (currentMusic_ != nullptr) {
                ::StopMusicStream(*currentMusic_);
            }
            currentMusic_ = nullptr;
            currentMusicName_.clear();

            // 5. オーディオデバイスの終了
            CloseAudioDevice();

            // 6. ウィンドウの終了
            if (IsWindowReady()) {
                CloseWindow();
            }

            isInitialized_ = false;
            resourcesInitialized_ = false;

            // ログシステムを最後にシャットダウン
            ShutdownLogSystem();
        }

        bool BaseSystemAPI::IsInitialized() const {
            return isInitialized_;
        }

        // ========== ログ管理 ==========

        void BaseSystemAPI::SetLogPath(const std::string& directory, const std::string& filename) {
            if (logInitialized_) {
                std::cerr << "BaseSystemAPI::SetLogPath: Cannot change log path after initialization" << std::endl;
                return;
            }
            if (directory.empty() || filename.empty()) {
                std::cerr << "BaseSystemAPI::SetLogPath: Directory and filename cannot be empty" << std::endl;
                return;
            }
            logDirectory_ = directory;
            logFileName_ = filename;
        }

        void BaseSystemAPI::SetLogLevel(spdlog::level::level_enum level) {
            if (logger_) {
                logger_->set_level(level);
                spdlog::set_level(level);
            }
        }

        void BaseSystemAPI::InitializeLogSystem() {
            // 初期化状態チェック
            if (logInitialized_) {
                return;
            }

            try {
                // ログディレクトリを作成
                std::filesystem::path logDir = logDirectory_;
                if (!std::filesystem::exists(logDir)) {
                    try {
                        std::filesystem::create_directories(logDir);
                    } catch (const std::filesystem::filesystem_error& e) {
                        std::cerr << "BaseSystemAPI::InitializeLogSystem: Failed to create log directory '" 
                                  << logDirectory_ << "': " << e.what() << std::endl;
                        throw;
                    }
                }

                // コンソールシンク（色付き）
                auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                consoleSink->set_level(spdlog::level::trace);
                consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

                // ファイルシンク
                std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink = nullptr;
                std::vector<spdlog::sink_ptr> sinks{consoleSink};

                try {
                    std::filesystem::path logFilePath = logDir / logFileName_;
                    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                        logFilePath.string(), true);
                    fileSink->set_level(spdlog::level::trace);
                    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
                    sinks.push_back(fileSink);
                } catch (const std::exception& e) {
                    std::cerr << "BaseSystemAPI::InitializeLogSystem: Failed to create file sink '" 
                              << (logDir / logFileName_).string() << "': " << e.what() 
                              << ". Falling back to console-only logging." << std::endl;
                    // ファイルシンクの作成に失敗した場合はコンソールのみで続行
                }

                // マルチシンクロガーを作成
                logger_ = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
                logger_->set_level(spdlog::level::trace);
                logger_->flush_on(spdlog::level::warn);

                // デフォルトロガーとして設定
                spdlog::set_default_logger(logger_);

                // グローバルログレベルを設定
                spdlog::set_level(spdlog::level::trace);

                // エラーハンドラーを設定
                spdlog::set_error_handler([](const std::string& msg) {
                    std::cerr << "spdlog error: " << msg << std::endl;
                });

                logInitialized_ = true;
                logger_->info("BaseSystemAPI: Log system initialized (directory: {}, file: {})", 
                             logDirectory_, logFileName_);
            } catch (const std::exception& e) {
                std::cerr << "BaseSystemAPI::InitializeLogSystem: Failed to initialize log system: " 
                          << e.what() << std::endl;
                // 初期化失敗時は例外を再スロー（呼び出し元で処理）
                throw;
            }
        }

        void BaseSystemAPI::ShutdownLogSystem() {
            if (!logInitialized_) {
                return;
            }

            if (logger_) {
                try {
                    logger_->info("BaseSystemAPI: Log system shutting down");
                    logger_->flush();
                } catch (const std::exception& e) {
                    std::cerr << "BaseSystemAPI::ShutdownLogSystem: Error during flush: " 
                              << e.what() << std::endl;
                }
            }

            spdlog::shutdown();
            logger_.reset();
            logInitialized_ = false;
        }

        // ========== リソース管理：初期化 ==========

        void BaseSystemAPI::InitializeResources() {
            LOG_INFO("BaseSystemAPI resources initialized");
        }

        void BaseSystemAPI::InitializeResources(ProgressCallback callback) {
            if (resourcesInitialized_) {
                LOG_WARN("BaseSystemAPI::InitializeResources: Already initialized");
                return;
            }

            // ファイルリストをスキャン
            int totalFiles = ScanResourceFiles();
            if (totalFiles == 0) {
                LOG_WARN("BaseSystemAPI::InitializeResources: No resource files found");
                resourcesInitialized_ = true;
                return;
            }

            // 進捗通知
            if (callback) {
                callback({0, totalFiles, "リソース読み込みを開始します..."});
            }

            // 全リソースを順次読み込む
            while (HasMoreResources()) {
                if (!LoadNextResource(callback)) {
                    break;
                }
            }

            resourcesInitialized_ = true;
            LOG_INFO("BaseSystemAPI::InitializeResources: Initialization completed");
        }

        bool BaseSystemAPI::IsResourcesInitialized() const {
            return resourcesInitialized_;
        }

        // ========== リソース管理：テクスチャ ==========

        void* BaseSystemAPI::GetTexture(const std::string& name) {
            const std::string key = NormalizeTextureKey(name);

            // キャッシュから検索
            auto it = textures_.find(key);
            if (it != textures_.end()) {
                return it->second.get();
            }

            // ファイルパスを構築
            // nameがassets/で始まる場合は、data/をプレフィックスとして追加
            // nameが既に.pngで終わっている場合は追加しない
            std::string path;
            if (key.length() >= 7 && key.substr(0, 7) == "assets/") {
                path = "data/" + key;
            } else {
                // 旧形式のサポート（data/assets/textures/ を想定）
                path = "data/assets/textures/" + key;
            }
            if (path.length() < 4 || path.substr(path.length() - 4) != ".png") {
                path += ".png";
            }

            // テクスチャを読み込み
            Texture2D texture = ::LoadTexture(path.c_str());

            // 読み込み失敗時はプレースホルダーを生成
            if (texture.id == 0) {
                LOG_WARN("Failed to load texture: {}, creating placeholder", path);
                texture = CreatePlaceholderTexture(name);
            } else {
                LOG_INFO("Loaded texture: {}", path);
            }

            // shared_ptr + カスタムデリータで管理
            auto texturePtr =
                std::shared_ptr<Texture2D>(new Texture2D(texture), [](Texture2D* t) {
                    if (t && t->id != 0) {
                        UnloadTexture(*t);
                    }
                    delete t;
                });

            // キャッシュに追加
            textures_[key] = texturePtr;
            return texturePtr.get();
        }

        bool BaseSystemAPI::HasTexture(const std::string& name) const {
            const std::string key = NormalizeTextureKey(name);
            return textures_.find(key) != textures_.end();
        }

        // ========== リソース管理：サウンド・ミュージック ==========

        void* BaseSystemAPI::GetSound(const std::string& name) {
            // キャッシュから検索
            auto it = sounds_.find(name);
            if (it != sounds_.end()) {
                return it->second.get();
            }

            // ファイルパスを構築（data/assets/sounds/ を想定）
            std::string path = "data/assets/sounds/" + name + ".wav";

            // サウンドを読み込み
            Sound sound = ::LoadSound(path.c_str());

            // 読み込み失敗時の処理
            if (sound.frameCount == 0) {
                LOG_ERROR("Failed to load sound: {}", path);
                return nullptr;
            }

            LOG_INFO("Loaded sound: {}", path);

            // shared_ptr + カスタムデリータで管理
            auto soundPtr = std::shared_ptr<Sound>(new Sound(sound), [](Sound* s) {
                if (s && s->frameCount != 0) {
                    UnloadSound(*s);
                }
                delete s;
            });

            // キャッシュに追加
            sounds_[name] = soundPtr;
            return soundPtr.get();
        }

        void* BaseSystemAPI::GetMusic(const std::string& name) {
            // キャッシュから検索
            auto it = musics_.find(name);
            if (it != musics_.end()) {
                return it->second.get();
            }

            // ファイルパスを構築（data/assets/music/ を想定）
            std::string path = "data/assets/music/" + name + ".mp3";

            // ミュージックを読み込み
            Music music = ::LoadMusicStream(path.c_str());

            // 読み込み失敗時の処理
            if (music.frameCount == 0) {
                LOG_ERROR("Failed to load music: {}", path);
                return nullptr;
            }

            LOG_INFO("Loaded music: {}", path);

            // shared_ptr + カスタムデリータで管理
            auto musicPtr = std::shared_ptr<Music>(new Music(music), [](Music* m) {
                if (m && m->frameCount != 0) {
                    UnloadMusicStream(*m);
                }
                delete m;
            });

            // キャッシュに追加
            musics_[name] = musicPtr;
            return musicPtr.get();
        }

        // ========== リソース管理：フォント ==========

        void* BaseSystemAPI::GetFont(const std::string& name) {
            // キャッシュから検索
            auto it = fonts_.find(name);
            if (it != fonts_.end()) {
                return it->second.get();
            }

            // ファイルパスを構築（data/assets/fonts/ を想定）
            std::string path = "data/assets/fonts/" + name;

            // フォントを読み込み（48pxで読み込む）
            // 日本語対応のためにコードポイントを指定
            Font font = ::LoadFontEx(path.c_str(), 48, fontCodepoints_.data(),
                                   static_cast<int>(fontCodepoints_.size()));

            // 読み込み失敗時の処理
            if (font.baseSize == 0) {
                LOG_ERROR("Failed to load font: {}", path);
                return nullptr;
            }

            LOG_INFO("Loaded font: {}", path);

            // shared_ptr + カスタムデリータで管理
            auto fontPtr = std::shared_ptr<Font>(new Font(font), [](Font* f) {
                if (f && f->baseSize != 0) {
                    UnloadFont(*f);
                }
                delete f;
            });

            // キャッシュに追加
            fonts_[name] = fontPtr;
            return fontPtr.get();
        }

        void BaseSystemAPI::SetDefaultFont(const std::string& name, int fontSize) {
            // 既に同じフォントがデフォルトに設定されている場合はスキップ
            if (defaultFont_ && fonts_.find(name) != fonts_.end() && 
                defaultFont_ == fonts_[name]) {
                LOG_DEBUG("BaseSystemAPI::SetDefaultFont: Font '{}' is already set as default", name);
                return;
            }
            
            auto fontPtr = static_cast<Font*>(GetFont(name));
            if (fontPtr && fontPtr->baseSize != 0) {
                // デフォルトフォントとして保持
                defaultFont_ = std::static_pointer_cast<Font>(fonts_[name]);

                // Raylib デフォルトフォント設定
                SetTextureFilter(fontPtr->texture, TEXTURE_FILTER_BILINEAR);
                LOG_INFO("BaseSystemAPI::SetDefaultFont: Set default font '{}' with size {}",
                        name, fontSize);

                // ImGUIの初期化と日本語フォントの設定
                if (!imGuiInitialized_) {
                    try {
                        // 1. rlImGuiSetupで初期化（FontAwesome有効）
                        rlImGuiSetup(true);
                        
                        // 2. ImGuiIOを取得してカスタムフォントを追加
                        ImGuiIO& io = ImGui::GetIO();
                        std::string fontPath = "data/assets/fonts/" + name;
                        
                        // フォント設定
                        ImFontConfig config;
                        config.MergeMode = false;  // マージせず独立
                        config.OversampleH = 2;
                        config.OversampleV = 2;
                        config.PixelSnapH = true;
                        
                        // 日本語グリフ範囲を指定
                        // GetGlyphRangesJapanese()を使用して日本語の全グリフを含める
                        const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesJapanese();
                        
                        // 3. 日本語フォントをデフォルトとして追加
                        ImFont* japaneseFont = io.Fonts->AddFontFromFileTTF(
                            fontPath.c_str(),
                            static_cast<float>(fontSize),
                            &config,
                            glyphRanges
                        );
                        
                        if (japaneseFont) {
                            // 4. デフォルトフォントとして設定
                            io.FontDefault = japaneseFont;
                            
                            // 5. フォントアトラスを構築（重要！）
                            io.Fonts->Build();
                            
                            imGuiJapaneseFont_ = japaneseFont;
                            imGuiInitialized_ = true;
                            
                            LOG_INFO("BaseSystemAPI::SetDefaultFont: ImGui initialized with Japanese font '{}'", name);
                            LOG_INFO("BaseSystemAPI::SetDefaultFont: Font size: {}px", fontSize);
                        } else {
                            // フォント追加失敗時はデフォルトで初期化
                            imGuiInitialized_ = true;
                            LOG_ERROR("BaseSystemAPI::SetDefaultFont: Failed to add Japanese font '{}', using default", fontPath);
                        }
                    } catch (const std::exception& e) {
                        LOG_ERROR("BaseSystemAPI::SetDefaultFont: Error initializing ImGui: {}", e.what());
                        // 例外発生時もデフォルトで初期化を試みる
                        if (!imGuiInitialized_) {
                            imGuiInitialized_ = true;
                        }
                    }
                }
            } else {
                LOG_WARN("BaseSystemAPI::SetDefaultFont: Failed to load font '{}', using Raylib default", name);
            }
        }

        void* BaseSystemAPI::GetDefaultFont() const {
            return defaultFont_.get();
        }

        // ========== 描画管理：解像度 ==========

        bool BaseSystemAPI::SetResolution(Resolution newResolution) {
            if (newResolution == currentResolution_) {
                return true;  // 既に同じ解像度
            }

            int newWidth = GetResolutionWidth(newResolution);
            int newHeight = GetResolutionHeight(newResolution);

            // ウィンドウサイズを変更（リサイズは無効化されたまま）
            SetWindowSize(newWidth, newHeight);

            screenWidth_ = newWidth;
            screenHeight_ = newHeight;
            currentResolution_ = newResolution;

            spdlog::info("BaseSystemAPI: Resolution changed to {}x{}", newWidth, newHeight);
            return true;
        }

        Resolution BaseSystemAPI::GetCurrentResolution() const {
            return currentResolution_;
        }

        int BaseSystemAPI::GetScreenWidth() const {
            return screenWidth_;
        }

        int BaseSystemAPI::GetScreenHeight() const {
            return screenHeight_;
        }

        int BaseSystemAPI::GetInternalWidth() const {
            return INTERNAL_WIDTH;
        }

        int BaseSystemAPI::GetInternalHeight() const {
            return INTERNAL_HEIGHT;
        }

        // ========== 描画管理：フレーム制御 ==========

        void BaseSystemAPI::BeginRender() {
            BeginTextureMode(mainRenderTexture_);
            ClearBackground(WHITE);
        }

        void BaseSystemAPI::BeginRenderEx(bool clearBackground) {
            BeginTextureMode(mainRenderTexture_);
            if (clearBackground) {
                ClearBackground(WHITE);
            }
        }

        void BaseSystemAPI::EndRender() {
            EndTextureMode();
        }

        void BaseSystemAPI::EndFrame(ImGuiRenderCallback imGuiCallback) {
            // 画面への描画開始
            BeginDrawing();
            ClearBackground(BLACK);
            
            // RenderTextureを画面にスケーリング描画
            // 内部解像度（1920x1080）から画面解像度へのスケーリング
            DrawTexturePro(
                mainRenderTexture_.texture,
                {0, 0, static_cast<float>(INTERNAL_WIDTH), -static_cast<float>(INTERNAL_HEIGHT)},
                {0, 0, static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)},
                {0, 0},
                0.0f,
                WHITE
            );
            
            // ImGUI描画フレーム開始（外部からImGUI描画コードを呼び出せるようにする）
            if (imGuiInitialized_) {
                rlImGuiBegin();
                
                // コールバックが指定されている場合、ImGuiフレーム内で呼び出す
                if (imGuiCallback) {
                    imGuiCallback();
                }
                
                rlImGuiEnd();
            }
            
            // 画面への描画終了
            EndDrawing();
        }

        // ========== 描画管理：ImGUI ==========

        void BaseSystemAPI::BeginImGui() {
            if (!imGuiInitialized_) {
                return;
            }
            // BeginDrawing()が既に呼ばれていることを前提とする
            // EndFrame()の前で呼び出す場合は、EndFrame()内でBeginDrawing()が呼ばれる前に
            // 自分でBeginDrawing()を呼ぶ必要がある
            // 推奨: EndFrame()の後にBeginImGui()/EndImGui()を呼び出す場合は、
            // EndFrame()を修正してBeginDrawing()/EndDrawing()を外部で管理する
            rlImGuiBegin();
        }

        void BaseSystemAPI::EndImGui() {
            if (!imGuiInitialized_) {
                return;
            }
            rlImGuiEnd();
        }

        bool BaseSystemAPI::IsImGuiInitialized() const {
            return imGuiInitialized_;
        }

        void* BaseSystemAPI::GetImGuiJapaneseFont() const {
            return imGuiJapaneseFont_;
        }

        // ========== 描画管理：スケーリング ==========

        float BaseSystemAPI::GetScaleFactor() const {
            // スケール係数 = ウィンドウ幅 / 内部幅
            return static_cast<float>(screenWidth_) / static_cast<float>(INTERNAL_WIDTH);
        }

        Vector2 BaseSystemAPI::ScalePosition(float internalX, float internalY) const {
            float scale = GetScaleFactor();
            return {internalX * scale, internalY * scale};
        }

        float BaseSystemAPI::ScaleSize(float internalSize) const {
            return internalSize * GetScaleFactor();
        }

        // ========== 描画管理：テキスト描画 ==========

        void BaseSystemAPI::DrawTextRaylib(const std::string& text, float x, float y, float fontSize, Color color) {
            ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
        }

        void BaseSystemAPI::DrawTextRaylibEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color) {
            ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing, color);
        }

        void BaseSystemAPI::DrawTextDefault(const std::string& text, float x, float y, float fontSize, Color color) {
            Font* font = GetDefaultFontInternal();
            if (font) {
                ::DrawTextEx(*font, text.c_str(), {x, y}, fontSize, 1.0f, color);
            } else {
                // フォント未設定時はRaylibデフォルトにフォールバック
                ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
            }
        }

        void BaseSystemAPI::DrawTextDefaultEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color) {
            Font* font = GetDefaultFontInternal();
            if (font) {
                ::DrawTextEx(*font, text.c_str(), position, fontSize, spacing, color);
            } else {
                // フォント未設定時はRaylibデフォルトにフォールバック
                ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing, color);
            }
        }

        void BaseSystemAPI::DrawTextWithFont(Font* font, const std::string& text, float x, float y, float fontSize, Color color) {
            if (font) {
                ::DrawTextEx(*font, text.c_str(), {x, y}, fontSize, 1.0f, color);
            } else {
                ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
            }
        }

        void BaseSystemAPI::DrawTextWithFontEx(Font* font, const std::string& text, Vector2 position, float fontSize, float spacing, Color color) {
            if (font) {
                ::DrawTextEx(*font, text.c_str(), position, fontSize, spacing, color);
            } else {
                ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing, color);
            }
        }

        Vector2 BaseSystemAPI::MeasureTextDefault(const std::string& text, float fontSize, float spacing) const {
            Font* font = GetDefaultFontInternal();
            if (font) {
                return ::MeasureTextEx(*font, text.c_str(), fontSize, spacing);
            } else {
                return ::MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, spacing);
            }
        }

        Vector2 BaseSystemAPI::MeasureTextWithFont(Font* font, const std::string& text, float fontSize, float spacing) const {
            if (font) {
                return ::MeasureTextEx(*font, text.c_str(), fontSize, spacing);
            } else {
                return ::MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, spacing);
            }
        }

        // ========== プライベートメソッド ==========

        void BaseSystemAPI::RecreateRenderTexture() {
            // 既存のRenderTextureをアンロード
            if (mainRenderTexture_.id != 0) {
                UnloadRenderTexture(mainRenderTexture_);
            }
            // 新しいRenderTextureを作成（内部解像度で）
            mainRenderTexture_ = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);
        }

        Font* BaseSystemAPI::GetDefaultFontInternal() const {
            return defaultFont_.get();
        }

        Texture2D BaseSystemAPI::CreatePlaceholderTexture(const std::string& name) {
            const int size = 64;
            Image image = GenImageColor(size, size, MAGENTA);

            // チェッカーパターンを描画
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    Color color = ((x / 8 + y / 8) % 2 == 0) ? MAGENTA : YELLOW;
                    ImageDrawPixel(&image, x, y, color);
                }
            }

            Texture2D texture = LoadTextureFromImage(image);
            UnloadImage(image);

            LOG_INFO("Created placeholder texture for: {}", name);
            return texture;
        }

        // ========== リソース管理：進捗管理 ==========

        int BaseSystemAPI::ScanResourceFiles() {
            resourceFileList_.clear();
            currentResourceIndex_ = 0;

            try {
                // フォントファイルをスキャン
                ScanDirectory("data/assets/fonts", ResourceType::Font, {".ttf"});

                // テクスチャファイルをスキャン
                ScanDirectory("data/assets/textures", ResourceType::Texture, {".png"});
                ScanDirectoryRecursive("data/assets/characters", ResourceType::Texture, {".png"});

                // サウンドファイルをスキャン（将来用、現在は未使用）
                // ScanDirectoryRecursive("data/assets/sounds", ResourceType::Sound, {".mp3", ".wav"});

                // JSONファイルをスキャン（data直下の定義ファイル）
                ScanDirectory("data", ResourceType::Json, {".json"});

                scanningCompleted_ = true;
                LOG_INFO("BaseSystemAPI: Scanned {} resource files", resourceFileList_.size());
                return static_cast<int>(resourceFileList_.size());
            } catch (const std::exception& e) {
                LOG_ERROR("BaseSystemAPI: Error scanning resource files: {}", e.what());
                return 0;
            }
        }

        bool BaseSystemAPI::LoadNextResource(ProgressCallback callback) {
            if (currentResourceIndex_ >= resourceFileList_.size()) {
                return false;
            }

            const auto& fileInfo = resourceFileList_[currentResourceIndex_];
            std::string message;

            try {
                switch (fileInfo.type) {
                case ResourceType::Font:
                    LoadFont(fileInfo.path, fileInfo.name);
                    message = "フォントを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Texture:
                    LoadTexture(fileInfo.path, fileInfo.name);
                    message = "テクスチャを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Sound:
                    LoadSound(fileInfo.path, fileInfo.name);
                    message = "サウンドを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Json:
                    LoadJson(fileInfo.path, fileInfo.name);
                    message = "設定ファイルを読み込み中: " + fileInfo.name;
                    break;
                }

                currentResourceIndex_++;

                if (callback) {
                    LoadProgress progress;
                    progress.current = static_cast<int>(currentResourceIndex_);
                    progress.total = static_cast<int>(resourceFileList_.size());
                    progress.message = message;
                    callback(progress);
                }

                // 最後のリソースを読み込んだタイミングで診断ログを出す
                if (currentResourceIndex_ >= resourceFileList_.size()) {
                    LOG_INFO("BaseSystemAPI: Resource loading completed. textures={}, sounds={}, musics={}, fonts={}",
                             textures_.size(), sounds_.size(), musics_.size(), fonts_.size());
                }

                return true;
            } catch (const std::exception& e) {
                LOG_WARN("BaseSystemAPI: Failed to load resource {}: {}", fileInfo.path, e.what());
                currentResourceIndex_++;
                return true; // エラーでも次のリソースへ
            }
        }

        bool BaseSystemAPI::HasMoreResources() const {
            return currentResourceIndex_ < resourceFileList_.size();
        }

        LoadProgress BaseSystemAPI::GetCurrentProgress() const {
            LoadProgress progress;
            progress.current = static_cast<int>(currentResourceIndex_);
            progress.total = static_cast<int>(resourceFileList_.size());

            if (currentResourceIndex_ < resourceFileList_.size()) {
                const auto& fileInfo = resourceFileList_[currentResourceIndex_];
                switch (fileInfo.type) {
                case ResourceType::Font:
                    progress.message = "フォントを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Texture:
                    progress.message = "テクスチャを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Sound:
                    progress.message = "サウンドを読み込み中: " + fileInfo.name;
                    break;
                case ResourceType::Json:
                    progress.message = "設定ファイルを読み込み中: " + fileInfo.name;
                    break;
                }
            } else {
                progress.message = "初期化完了";
            }

            return progress;
        }

        void BaseSystemAPI::ResetLoadingState() {
            resourceFileList_.clear();
            currentResourceIndex_ = 0;
            scanningCompleted_ = false;
        }

        // ========== プライベートメソッド：リソース管理 ==========

        void BaseSystemAPI::ScanDirectory(const std::string& dirPath, ResourceType type,
                                         const std::vector<std::string>& extensions) {
            if (!std::filesystem::exists(dirPath)) {
                return;
            }

            try {
                for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                        if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                            ResourceFileInfo info;
                            info.type = type;
                            // パスは OS 依存区切りを避けるため generic_string() を使用
                            info.path = entry.path().generic_string();

                            // テクスチャは実利用キー（assets/.../file.png）に合わせて相対キーを使う
                            if (type == ResourceType::Texture) {
                                info.name = MakeAssetsRelativeKey(entry.path());
                                info.name = NormalizeTextureKey(info.name);
                            } else {
                                info.name = entry.path().stem().string();
                            }
                            resourceFileList_.push_back(info);
                        }
                    }
                }
            } catch (const std::exception& e) {
                LOG_WARN("BaseSystemAPI: Error scanning directory {}: {}", dirPath, e.what());
            }
        }

        void BaseSystemAPI::ScanDirectoryRecursive(const std::string& dirPath, ResourceType type,
                                                   const std::vector<std::string>& extensions) {
            if (!std::filesystem::exists(dirPath)) {
                return;
            }

            try {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                        if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                            ResourceFileInfo info;
                            info.type = type;
                            info.path = entry.path().generic_string();

                            // テクスチャは実利用キー（assets/.../file.png）に合わせて相対キーを使う
                            if (type == ResourceType::Texture) {
                                info.name = MakeAssetsRelativeKey(entry.path());
                                info.name = NormalizeTextureKey(info.name);
                            } else {
                                info.name = entry.path().stem().string();
                            }
                            resourceFileList_.push_back(info);
                        }
                    }
                }
            } catch (const std::exception& e) {
                LOG_WARN("BaseSystemAPI: Error scanning directory recursively {}: {}", dirPath, e.what());
            }
        }

        void BaseSystemAPI::LoadFont(const std::string& path, const std::string& name) {
            // フォントは現在の実装では直接キャッシュしないが、将来的な拡張のために用意
            LOG_DEBUG("Font loaded: {}", path);
        }

        void BaseSystemAPI::LoadTexture(const std::string& path, const std::string& name) {
            const std::string key = NormalizeTextureKey(name);

            // 既にキャッシュにある場合はスキップ
            if (textures_.find(key) != textures_.end()) {
                return;
            }

            Texture2D texture = ::LoadTexture(path.c_str());

            if (texture.id == 0) {
                LOG_WARN("Failed to load texture: {}, creating placeholder", path);
                texture = CreatePlaceholderTexture(name);
            }

            auto texturePtr =
                std::shared_ptr<Texture2D>(new Texture2D(texture), [](Texture2D* t) {
                    if (t && t->id != 0) {
                        UnloadTexture(*t);
                    }
                    delete t;
                });

            textures_[key] = texturePtr;

            // 互換: assets/textures 配下のみ、旧形式（stem / filename）でも引けるように別名キーを登録
            if (StartsWith(key, "assets/textures/")) {
                const std::filesystem::path p(key);
                const std::string filename = p.filename().generic_string(); // e.g. foo.png
                const std::string stem = p.stem().string();                 // e.g. foo

                if (!stem.empty() && textures_.find(stem) == textures_.end()) {
                    textures_[stem] = texturePtr;
                } else if (!stem.empty()) {
                    LOG_DEBUG("BaseSystemAPI: texture alias collision (stem): {}", stem);
                }

                if (!filename.empty() && textures_.find(filename) == textures_.end()) {
                    textures_[filename] = texturePtr;
                } else if (!filename.empty()) {
                    LOG_DEBUG("BaseSystemAPI: texture alias collision (filename): {}", filename);
                }
            }
        }

        void BaseSystemAPI::LoadSound(const std::string& path, const std::string& name) {
            std::string ext = std::filesystem::path(path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == ".mp3") {
                // MP3はMusicとして扱う
                // 既にキャッシュにある場合はスキップ
                if (musics_.find(name) != musics_.end()) {
                    return;
                }

                Music music = ::LoadMusicStream(path.c_str());
                if (music.frameCount == 0) {
                    LOG_WARN("Failed to load music: {}", path);
                    return;
                }

                auto musicPtr = std::shared_ptr<Music>(new Music(music), [](Music* m) {
                    if (m && m->frameCount != 0) {
                        UnloadMusicStream(*m);
                    }
                    delete m;
                });

                musics_[name] = musicPtr;
            } else if (ext == ".wav") {
                // WAVはSoundとして扱う
                // 既にキャッシュにある場合はスキップ
                if (sounds_.find(name) != sounds_.end()) {
                    return;
                }

                Sound sound = ::LoadSound(path.c_str());
                if (sound.frameCount == 0) {
                    LOG_WARN("Failed to load sound: {}", path);
                    return;
                }

                auto soundPtr = std::shared_ptr<Sound>(new Sound(sound), [](Sound* s) {
                    if (s && s->frameCount != 0) {
                        UnloadSound(*s);
                    }
                    delete s;
                });

                sounds_[name] = soundPtr;
            }
        }

        void BaseSystemAPI::LoadJson(const std::string& path, const std::string& name) {
            // JSONファイルの読み込みは現在の実装ではキャッシュしない
            LOG_DEBUG("JSON loaded: {}", path);
        }

        void BaseSystemAPI::GenerateFontCodepoints() {
            if (!fontCodepoints_.empty())
                return;

            // ASCII (0x20 - 0x7E)
            for (int i = 0x20; i <= 0x7E; ++i)
                fontCodepoints_.push_back(i);

            // ひらがな・カタカナ・記号 (0x3000 - 0x30FF)
            for (int i = 0x3000; i <= 0x30FF; ++i)
                fontCodepoints_.push_back(i);

            // 半角カタカナなど (0xFF00 - 0xFFEF)
            for (int i = 0xFF00; i <= 0xFFEF; ++i)
                fontCodepoints_.push_back(i);

            // 常用漢字など (0x4E00 - 0x9FAF) CJK Unified Ideographs
            for (int i = 0x4E00; i <= 0x9FAF; ++i)
                fontCodepoints_.push_back(i);

            // 矢印記号 (U+2190-U+21FF) Arrows
            for (int i = 0x2190; i <= 0x21FF; ++i)
                fontCodepoints_.push_back(i);

            // General Punctuation (U+2000-U+206F)
            for (int i = 0x2000; i <= 0x206F; ++i)
                fontCodepoints_.push_back(i);

            // Miscellaneous Symbols (U+2600-U+26FF)
            for (int i = 0x2600; i <= 0x26FF; ++i)
                fontCodepoints_.push_back(i);

            // Dingbats (U+2700-U+27BF)
            for (int i = 0x2700; i <= 0x27BF; ++i)
                fontCodepoints_.push_back(i);

            // Miscellaneous Symbols and Pictographs, Emoticons, etc. (U+1F300-U+1F9FF)
            for (int i = 0x1F300; i <= 0x1F9FF; ++i)
                fontCodepoints_.push_back(i);

            // Supplemental Symbols and Pictographs (U+1FA00-U+1FAFF)
            for (int i = 0x1FA00; i <= 0x1FAFF; ++i)
                fontCodepoints_.push_back(i);

            LOG_INFO("Generated font codepoints: {} characters (including emoji ranges)", fontCodepoints_.size());
        }

        // ========== 描画管理：基本図形 ==========

        void BaseSystemAPI::DrawRectangle(float x, float y, float width, float height, Color color) {
            ::DrawRectangle((int)x, (int)y, (int)width, (int)height, color);
        }

        void BaseSystemAPI::DrawRectangleLines(float x, float y, float width, float height, float thickness, Color color) {
            ::DrawRectangleLinesEx({x, y, width, height}, thickness, color);
        }

        void BaseSystemAPI::DrawCircle(float centerX, float centerY, float radius, Color color) {
            ::DrawCircle((int)centerX, (int)centerY, radius, color);
        }

        void BaseSystemAPI::DrawCircleLines(float centerX, float centerY, float radius, float thickness, Color color) {
            ::DrawCircleLines((int)centerX, (int)centerY, radius, color);
        }

        void BaseSystemAPI::DrawLine(float startX, float startY, float endX, float endY, float thickness, Color color) {
            ::DrawLineEx({startX, startY}, {endX, endY}, thickness, color);
        }

        void BaseSystemAPI::DrawProgressBar(float x, float y, float width, float height, float progress,
                                            Color fillColor, Color emptyColor, Color outlineColor) {
            // クランプ進捗値
            progress = std::max(0.0f, std::min(1.0f, progress));
            
            // 背景（未進捗部分）
            ::DrawRectangle((int)x, (int)y, (int)width, (int)height, emptyColor);
            
            // 進捗部分
            float fillWidth = width * progress;
            ::DrawRectangle((int)x, (int)y, (int)fillWidth, (int)height, fillColor);
            
            // 枠線
            if (outlineColor.a != 0) {
                float thickness = 2.0f;
                ::DrawRectangleLinesEx({x, y, width, height}, thickness, outlineColor);
            }
        }

        // ========== 描画管理：拡張図形 ==========

        void BaseSystemAPI::DrawPixel(int x, int y, Color color) {
            ::DrawPixel(x, y, color);
        }

        void BaseSystemAPI::DrawPixelV(Vector2 position, Color color) {
            ::DrawPixelV(position, color);
        }

        void BaseSystemAPI::DrawLineV(Vector2 startPos, Vector2 endPos, Color color) {
            ::DrawLineV(startPos, endPos, color);
        }

        void BaseSystemAPI::DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color) {
            ::DrawLineBezier(startPos, endPos, thick, color);
        }

        void BaseSystemAPI::DrawLineStrip(Vector2* points, int pointCount, Color color) {
            ::DrawLineStrip(points, pointCount, color);
        }

        void BaseSystemAPI::DrawLineStrip(const std::vector<Vector2>& points, Color color) {
            if (!points.empty()) {
                ::DrawLineStrip(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), color);
            }
        }

        void BaseSystemAPI::DrawCircleV(Vector2 center, float radius, Color color) {
            ::DrawCircleV(center, radius, color);
        }

        void BaseSystemAPI::DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color) {
            ::DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
        }

        void BaseSystemAPI::DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color) {
            ::DrawCircleSectorLines(center, radius, startAngle, endAngle, segments, color);
        }

        void BaseSystemAPI::DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2) {
            ::DrawCircleGradient(centerX, centerY, radius, color1, color2);
        }

        void BaseSystemAPI::DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color) {
            ::DrawEllipse(centerX, centerY, radiusH, radiusV, color);
        }

        void BaseSystemAPI::DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color) {
            ::DrawEllipseLines(centerX, centerY, radiusH, radiusV, color);
        }

        void BaseSystemAPI::DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color) {
            ::DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
        }

        void BaseSystemAPI::DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color) {
            ::DrawRingLines(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
        }

        void BaseSystemAPI::DrawRectangleV(Vector2 position, Vector2 size, Color color) {
            ::DrawRectangleV(position, size, color);
        }

        void BaseSystemAPI::DrawRectangleRec(Rectangle rec, Color color) {
            ::DrawRectangleRec(rec, color);
        }

        void BaseSystemAPI::DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color) {
            ::DrawRectanglePro(rec, origin, rotation, color);
        }

        void BaseSystemAPI::DrawRectangleGradientV(int x, int y, int width, int height, Color color1, Color color2) {
            ::DrawRectangleGradientV(x, y, width, height, color1, color2);
        }

        void BaseSystemAPI::DrawRectangleGradientH(int x, int y, int width, int height, Color color1, Color color2) {
            ::DrawRectangleGradientH(x, y, width, height, color1, color2);
        }

        void BaseSystemAPI::DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4) {
            ::DrawRectangleGradientEx(rec, col1, col2, col3, col4);
        }

        void BaseSystemAPI::DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color) {
            ::DrawRectangleRounded(rec, roundness, segments, color);
        }

        void BaseSystemAPI::DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color) {
            ::DrawRectangleRoundedLines(rec, roundness, segments, color);
        }

        void BaseSystemAPI::DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
            ::DrawTriangle(v1, v2, v3, color);
        }

        void BaseSystemAPI::DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
            ::DrawTriangleLines(v1, v2, v3, color);
        }

        void BaseSystemAPI::DrawTriangleFan(Vector2* points, int pointCount, Color color) {
            ::DrawTriangleFan(points, pointCount, color);
        }

        void BaseSystemAPI::DrawTriangleFan(const std::vector<Vector2>& points, Color color) {
            if (!points.empty()) {
                ::DrawTriangleFan(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), color);
            }
        }

        void BaseSystemAPI::DrawTriangleStrip(Vector2* points, int pointCount, Color color) {
            ::DrawTriangleStrip(points, pointCount, color);
        }

        void BaseSystemAPI::DrawTriangleStrip(const std::vector<Vector2>& points, Color color) {
            if (!points.empty()) {
                ::DrawTriangleStrip(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), color);
            }
        }

        void BaseSystemAPI::DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color) {
            ::DrawPoly(center, sides, radius, rotation, color);
        }

        void BaseSystemAPI::DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color) {
            ::DrawPolyLines(center, sides, radius, rotation, color);
        }

        void BaseSystemAPI::DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color) {
            ::DrawPolyLinesEx(center, sides, radius, rotation, lineThick, color);
        }

        // ========== 描画管理：スプライン ==========

        void BaseSystemAPI::DrawSplineLinear(Vector2* points, int pointCount, float thick, Color color) {
            ::DrawSplineLinear(points, pointCount, thick, color);
        }

        void BaseSystemAPI::DrawSplineLinear(const std::vector<Vector2>& points, float thick, Color color) {
            if (!points.empty()) {
                ::DrawSplineLinear(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), thick, color);
            }
        }

        void BaseSystemAPI::DrawSplineBasis(Vector2* points, int pointCount, float thick, Color color) {
            ::DrawSplineBasis(points, pointCount, thick, color);
        }

        void BaseSystemAPI::DrawSplineBasis(const std::vector<Vector2>& points, float thick, Color color) {
            if (!points.empty()) {
                ::DrawSplineBasis(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), thick, color);
            }
        }

        void BaseSystemAPI::DrawSplineCatmullRom(Vector2* points, int pointCount, float thick, Color color) {
            ::DrawSplineCatmullRom(points, pointCount, thick, color);
        }

        void BaseSystemAPI::DrawSplineCatmullRom(const std::vector<Vector2>& points, float thick, Color color) {
            if (!points.empty()) {
                ::DrawSplineCatmullRom(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), thick, color);
            }
        }

        void BaseSystemAPI::DrawSplineBezierQuadratic(Vector2* points, int pointCount, float thick, Color color) {
            ::DrawSplineBezierQuadratic(points, pointCount, thick, color);
        }

        void BaseSystemAPI::DrawSplineBezierQuadratic(const std::vector<Vector2>& points, float thick, Color color) {
            if (!points.empty()) {
                ::DrawSplineBezierQuadratic(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), thick, color);
            }
        }

        void BaseSystemAPI::DrawSplineBezierCubic(Vector2* points, int pointCount, float thick, Color color) {
            ::DrawSplineBezierCubic(points, pointCount, thick, color);
        }

        void BaseSystemAPI::DrawSplineBezierCubic(const std::vector<Vector2>& points, float thick, Color color) {
            if (!points.empty()) {
                ::DrawSplineBezierCubic(const_cast<Vector2*>(points.data()), static_cast<int>(points.size()), thick, color);
            }
        }

        // ========== 描画管理：テクスチャ ==========

        void BaseSystemAPI::DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
            ::DrawTexture(texture, posX, posY, tint);
        }

        void BaseSystemAPI::DrawTextureV(Texture2D texture, Vector2 position, Color tint) {
            ::DrawTextureV(texture, position, tint);
        }

        void BaseSystemAPI::DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint) {
            ::DrawTextureEx(texture, position, rotation, scale, tint);
        }

        void BaseSystemAPI::DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint) {
            ::DrawTextureRec(texture, source, position, tint);
        }

        void BaseSystemAPI::DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) {
            ::DrawTexturePro(texture, source, dest, origin, rotation, tint);
        }

        void BaseSystemAPI::DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint) {
            ::DrawTextureNPatch(texture, nPatchInfo, dest, origin, rotation, tint);
        }

        // ========== 描画管理：テキスト拡張 ==========

        void BaseSystemAPI::DrawTextPro(Font font, const std::string& text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint) {
            ::DrawTextPro(font, text.c_str(), position, origin, rotation, fontSize, spacing, tint);
        }

        void BaseSystemAPI::DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint) {
            ::DrawTextCodepoint(font, codepoint, position, fontSize, tint);
        }

        void BaseSystemAPI::DrawTextCodepoints(Font font, const int* codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint) {
            ::DrawTextCodepoints(font, codepoints, codepointCount, position, fontSize, spacing, tint);
        }

        // ========== オーディオ管理 ==========

        void BaseSystemAPI::UpdateAudio(float deltaTime) {
            if (!isInitialized_) {
                return;
            }

            // Musicのストリーミング更新
            if (currentMusic_ != nullptr) {
                try {
                    if (::IsMusicStreamPlaying(*currentMusic_)) {
                        ::UpdateMusicStream(*currentMusic_);
                    } else {
                        // 再生が終了した場合はクリア
                        currentMusic_ = nullptr;
                        currentMusicName_.clear();
                    }
                } catch (...) {
                    // 例外が発生した場合は、Musicが無効になったと判断してクリア
                    LOG_WARN("BaseSystemAPI: Music became invalid during update");
                    currentMusic_ = nullptr;
                    currentMusicName_.clear();
                }
            }

            // 再生終了したSoundをクリーンアップ
            auto it = playingSounds_.begin();
            while (it != playingSounds_.end()) {
                Sound* sound = it->second;
                if (sound) {
                    try {
                        if (!::IsSoundPlaying(*sound)) {
                            it = playingSounds_.erase(it);
                            continue;
                        }
                    } catch (...) {
                        // 例外が発生した場合は、Soundが無効になったと判断して削除
                        LOG_WARN("BaseSystemAPI: Sound became invalid during update");
                        it = playingSounds_.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }

        bool BaseSystemAPI::PlaySound(const std::string& name) {
            if (!isInitialized_) {
                LOG_ERROR("BaseSystemAPI: Not initialized");
                return false;
            }

            // BaseSystemAPIからSoundを取得
            void* soundPtr = GetSound(name);
            if (!soundPtr) {
                LOG_ERROR("BaseSystemAPI: Failed to get sound: {}", name);
                return false;
            }

            Sound* sound = static_cast<Sound*>(soundPtr);

            // Soundを再生（重複可能）
            ::PlaySound(*sound);

            // ボリュームを設定
            UpdateSoundVolume(sound);

            // 再生中のSoundとして登録
            playingSounds_[name] = sound;

            LOG_DEBUG("BaseSystemAPI: Playing sound: {}", name);
            return true;
        }

        bool BaseSystemAPI::PlayMusic(const std::string& name) {
            if (!isInitialized_) {
                LOG_ERROR("BaseSystemAPI: Not initialized");
                return false;
            }

            // 既に同じMusicが再生中の場合は何もしない
            if (currentMusic_ != nullptr && currentMusicName_ == name && ::IsMusicStreamPlaying(*currentMusic_)) {
                LOG_DEBUG("BaseSystemAPI: Music already playing: {}", name);
                return true;
            }

            // 既存のMusicを停止（重複不可）
            if (currentMusic_ != nullptr) {
                ::StopMusicStream(*currentMusic_);
            }

            // BaseSystemAPIからMusicを取得
            void* musicPtr = GetMusic(name);
            if (!musicPtr) {
                LOG_ERROR("BaseSystemAPI: Failed to get music: {}", name);
                return false;
            }

            Music* music = static_cast<Music*>(musicPtr);

            // Musicを再生
            ::PlayMusicStream(*music);

            // ボリュームを設定
            UpdateMusicVolume(music);

            // 現在のMusicとして設定
            currentMusic_ = music;
            currentMusicName_ = name;

            LOG_INFO("BaseSystemAPI: Playing music: {}", name);
            return true;
        }

        void BaseSystemAPI::StopSound() {
            if (!isInitialized_) {
                return;
            }

            // 全Soundを停止
            for (auto& pair : playingSounds_) {
                Sound* sound = pair.second;
                if (sound) {
                    ::StopSound(*sound);
                }
            }

            playingSounds_.clear();
            LOG_DEBUG("BaseSystemAPI: Stopped all sounds");
        }

        void BaseSystemAPI::StopSound(const std::string& name) {
            if (!isInitialized_) {
                return;
            }

            auto it = playingSounds_.find(name);
            if (it != playingSounds_.end()) {
                Sound* sound = it->second;
                if (sound) {
                    ::StopSound(*sound);
                }
                playingSounds_.erase(it);
                LOG_DEBUG("BaseSystemAPI: Stopped sound: {}", name);
            }
        }

        void BaseSystemAPI::StopMusic() {
            if (!isInitialized_ || !currentMusic_) {
                return;
            }

            ::StopMusicStream(*currentMusic_);
            currentMusic_ = nullptr;
            currentMusicName_.clear();
            LOG_DEBUG("BaseSystemAPI: Stopped music");
        }

        bool BaseSystemAPI::IsSoundPlaying(const std::string& name) const {
            if (!isInitialized_) {
                return false;
            }

            auto it = playingSounds_.find(name);
            if (it != playingSounds_.end()) {
                Sound* sound = it->second;
                if (sound) {
                    try {
                        return ::IsSoundPlaying(*sound);
                    } catch (...) {
                        return false;
                    }
                }
            }

            return false;
        }

        bool BaseSystemAPI::IsMusicPlaying() const {
            if (!isInitialized_ || !currentMusic_) {
                return false;
            }

            try {
                return ::IsMusicStreamPlaying(*currentMusic_);
            } catch (...) {
                return false;
            }
        }

        std::string BaseSystemAPI::GetCurrentMusicName() const {
            if (!isInitialized_ || !currentMusic_) {
                return "";
            }

            return currentMusicName_;
        }

        void BaseSystemAPI::SetMasterVolume(float volume) {
            masterVolume_ = ClampVolume(volume);
            
            // Raylibのマスターボリュームを設定
            ::SetMasterVolume(masterVolume_);

            // 既存のSound/Musicのボリュームを更新
            for (auto& pair : playingSounds_) {
                if (pair.second) {
                    UpdateSoundVolume(pair.second);
                }
            }
            if (currentMusic_) {
                UpdateMusicVolume(currentMusic_);
            }

            LOG_DEBUG("BaseSystemAPI: Master volume set to {:.2f}", masterVolume_);
        }

        void BaseSystemAPI::SetSEVolume(float volume) {
            seVolume_ = ClampVolume(volume);

            // 既存のSoundのボリュームを更新
            for (auto& pair : playingSounds_) {
                if (pair.second) {
                    UpdateSoundVolume(pair.second);
                }
            }

            LOG_DEBUG("BaseSystemAPI: SE volume set to {:.2f}", seVolume_);
        }

        void BaseSystemAPI::SetBGMVolume(float volume) {
            bgmVolume_ = ClampVolume(volume);

            // 既存のMusicのボリュームを更新
            if (currentMusic_) {
                UpdateMusicVolume(currentMusic_);
            }

            LOG_DEBUG("BaseSystemAPI: BGM volume set to {:.2f}", bgmVolume_);
        }

        float BaseSystemAPI::GetMasterVolume() const {
            return masterVolume_;
        }

        float BaseSystemAPI::GetSEVolume() const {
            return seVolume_;
        }

        float BaseSystemAPI::GetBGMVolume() const {
            return bgmVolume_;
        }

        // ========== 入力処理：状態更新 ==========

        void BaseSystemAPI::UpdateInput() {
            inputState_.mouseButtonConsumed.reset();
            Vector2 currentPos = ::GetMousePosition();
            inputState_.mouseDeltaX = currentPos.x - inputState_.mouseX;
            inputState_.mouseDeltaY = currentPos.y - inputState_.mouseY;
            inputState_.mouseX = currentPos.x;
            inputState_.mouseY = currentPos.y;
        }

        // ========== 入力処理：キーボード ==========

        bool BaseSystemAPI::IsKeyPressed(int key) {
            return ::IsKeyPressed(key);
        }

        bool BaseSystemAPI::IsKeyPressedRepeat(int key) {
            return ::IsKeyPressedRepeat(key);
        }

        bool BaseSystemAPI::IsKeyDown(int key) {
            return ::IsKeyDown(key);
        }

        bool BaseSystemAPI::IsKeyReleased(int key) {
            return ::IsKeyReleased(key);
        }

        bool BaseSystemAPI::IsKeyUp(int key) {
            return ::IsKeyUp(key);
        }

        int BaseSystemAPI::GetKeyPressed() {
            return ::GetKeyPressed();
        }

        int BaseSystemAPI::GetCharPressed() {
            return ::GetCharPressed();
        }

        void BaseSystemAPI::SetExitKey(int key) {
            ::SetExitKey(key);
        }

        // ========== 入力処理：マウス ==========

        bool BaseSystemAPI::IsMouseButtonPressed(int button) {
            if (button < 0 || button >= 8) return false;
            return ::IsMouseButtonPressed(button) && !inputState_.mouseButtonConsumed[button];
        }

        bool BaseSystemAPI::IsMouseButtonDown(int button) {
            return ::IsMouseButtonDown(button);
        }

        bool BaseSystemAPI::IsMouseButtonReleased(int button) {
            return ::IsMouseButtonReleased(button);
        }

        bool BaseSystemAPI::IsMouseButtonUp(int button) {
            return ::IsMouseButtonUp(button);
        }

        int BaseSystemAPI::GetMouseX() {
            return static_cast<int>(inputState_.mouseX);
        }

        int BaseSystemAPI::GetMouseY() {
            return static_cast<int>(inputState_.mouseY);
        }

        Vector2 BaseSystemAPI::GetMousePosition() {
            return {inputState_.mouseX, inputState_.mouseY};
        }

        Vector2 BaseSystemAPI::GetMouseDelta() {
            return {inputState_.mouseDeltaX, inputState_.mouseDeltaY};
        }

        float BaseSystemAPI::GetMouseWheelMove() {
            return ::GetMouseWheelMove();
        }

        Vector2 BaseSystemAPI::GetMouseWheelMoveV() {
            return ::GetMouseWheelMoveV();
        }

        void BaseSystemAPI::SetMousePosition(int x, int y) {
            ::SetMousePosition(x, y);
            inputState_.mouseX = static_cast<float>(x);
            inputState_.mouseY = static_cast<float>(y);
        }

        void BaseSystemAPI::SetMouseOffset(int offsetX, int offsetY) {
            ::SetMouseOffset(offsetX, offsetY);
        }

        void BaseSystemAPI::SetMouseScale(float scaleX, float scaleY) {
            ::SetMouseScale(scaleX, scaleY);
        }

        void BaseSystemAPI::SetMouseCursor(int cursor) {
            ::SetMouseCursor(cursor);
        }

        void BaseSystemAPI::ConsumeMouseButton(int button) {
            if (button >= 0 && button < 8) {
                inputState_.mouseButtonConsumed[button] = true;
            }
        }

        // ========== 入力処理：ゲームパッド ==========

        bool BaseSystemAPI::IsGamepadAvailable(int gamepad) {
            return ::IsGamepadAvailable(gamepad);
        }

        const char* BaseSystemAPI::GetGamepadName(int gamepad) {
            return ::GetGamepadName(gamepad);
        }

        bool BaseSystemAPI::IsGamepadButtonPressed(int gamepad, int button) {
            return ::IsGamepadButtonPressed(gamepad, button);
        }

        bool BaseSystemAPI::IsGamepadButtonDown(int gamepad, int button) {
            return ::IsGamepadButtonDown(gamepad, button);
        }

        bool BaseSystemAPI::IsGamepadButtonReleased(int gamepad, int button) {
            return ::IsGamepadButtonReleased(gamepad, button);
        }

        bool BaseSystemAPI::IsGamepadButtonUp(int gamepad, int button) {
            return ::IsGamepadButtonUp(gamepad, button);
        }

        float BaseSystemAPI::GetGamepadAxisMovement(int gamepad, int axis) {
            return ::GetGamepadAxisMovement(gamepad, axis);
        }

        // ========== 入力処理：タッチ・ジェスチャー ==========

        int BaseSystemAPI::GetTouchPointCount() {
            return ::GetTouchPointCount();
        }

        Vector2 BaseSystemAPI::GetTouchPosition(int index) {
            return ::GetTouchPosition(index);
        }

        bool BaseSystemAPI::IsGestureDetected(unsigned int gesture) {
            return ::IsGestureDetected(gesture);
        }

        int BaseSystemAPI::GetGestureDetected() {
            return ::GetGestureDetected();
        }

        // ========== タイミング ==========

        float BaseSystemAPI::GetFrameTime() {
            return ::GetFrameTime();
        }

        int BaseSystemAPI::GetFPS() {
            return ::GetFPS();
        }

        void BaseSystemAPI::SetTargetFPS(int fps) {
            ::SetTargetFPS(fps);
        }

        double BaseSystemAPI::GetTime() {
            return ::GetTime();
        }

        // ========== ウィンドウ管理 ==========

        bool BaseSystemAPI::WindowShouldClose() {
            return ::WindowShouldClose();
        }

        bool BaseSystemAPI::IsWindowReady() {
            return ::IsWindowReady();
        }

        bool BaseSystemAPI::IsFullscreen() const {
            return ::IsWindowFullscreen();
        }

        void BaseSystemAPI::ToggleFullscreen() {
            ::ToggleFullscreen();
            LOG_DEBUG("BaseSystemAPI: Fullscreen toggled");
        }

        void BaseSystemAPI::SetFullscreen(bool fullscreen) {
            bool current = ::IsWindowFullscreen();
            if (current != fullscreen) {
                ::ToggleFullscreen();
                LOG_DEBUG("BaseSystemAPI: Fullscreen set to {}", fullscreen);
            }
        }

        bool BaseSystemAPI::IsFPSDisplayEnabled() const {
            return fpsDisplayEnabled_;
        }

        void BaseSystemAPI::SetFPSDisplayEnabled(bool enabled) {
            fpsDisplayEnabled_ = enabled;
            LOG_DEBUG("BaseSystemAPI: FPS display set to {}", enabled);
        }

        // ========== 衝突判定 ==========

        bool BaseSystemAPI::CheckCollisionPointRec(Vector2 point, Rectangle rec) {
            return ::CheckCollisionPointRec(point, rec);
        }

        bool BaseSystemAPI::CheckCollisionRecs(Rectangle rec1, Rectangle rec2) {
            return ::CheckCollisionRecs(rec1, rec2);
        }

        bool BaseSystemAPI::CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2) {
            return ::CheckCollisionCircles(center1, radius1, center2, radius2);
        }

        bool BaseSystemAPI::CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
            return ::CheckCollisionCircleRec(center, radius, rec);
        }

        Rectangle BaseSystemAPI::GetCollisionRec(Rectangle rec1, Rectangle rec2) {
            return ::GetCollisionRec(rec1, rec2);
        }

        float BaseSystemAPI::ClampVolume(float volume) const {
            return std::max(0.0f, std::min(1.0f, volume));
        }

        void BaseSystemAPI::UpdateSoundVolume(Sound* sound) const {
            if (!sound) {
                return;
            }

            // 実際の音量 = マスターボリューム * SEボリューム
            float finalVolume = masterVolume_ * seVolume_;
            ::SetSoundVolume(*sound, finalVolume);
        }

        void BaseSystemAPI::UpdateMusicVolume(Music* music) const {
            if (!music) {
                return;
            }

            // 実際の音量 = マスターボリューム * BGMボリューム
            float finalVolume = masterVolume_ * bgmVolume_;
            ::SetMusicVolume(*music, finalVolume);
        }


    } // namespace core
} // namespace game
