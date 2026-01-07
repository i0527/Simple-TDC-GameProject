#ifndef __GAME_CORE_BASESYSTEMAPI_HPP__
#define __GAME_CORE_BASESYSTEMAPI_HPP__

#include "../config/GameConfig.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <bitset>
#include <spdlog/spdlog.h>

namespace game {
    namespace core {
        /// @brief リソース読み込み進捗情報
        struct LoadProgress {
            int current;        // 現在の進捗
            int total;          // 全体数
            std::string message; // 現在の作業内容
        };

        /// @brief 進捗通知コールバック型
        using ProgressCallback = std::function<void(const LoadProgress&)>;

        /// @brief ImGui描画コールバック型
        using ImGuiRenderCallback = std::function<void()>;

        /// @brief リソースタイプ
        enum class ResourceType {
            Font,
            Texture,
            Sound,
            Json
        };

        /// @brief リソースファイル情報
        struct ResourceFileInfo {
            ResourceType type;
            std::string path;
            std::string name;
        };

        /// @brief 統合システムAPIクラス
        /// 
        /// RenderingManagerとResouceManagerの機能を統合し、
        /// Raylibの処理を一元管理します。
        /// 初期化・解放処理の順序を厳密に保証します。
        /// 
        /// 責務：
        /// - Raylibウィンドウ・オーディオデバイスの初期化/終了
        /// - リソース管理（テクスチャ、フォント、サウンド、ミュージック）
        /// - 描画管理（RenderTexture、描画フレーム制御、描画プリミティブ）
        /// - 解像度管理
        class BaseSystemAPI {
        public:
            /// @brief デフォルトコンストラクタ
            BaseSystemAPI();
            
            /// @brief デストラクタ
            ~BaseSystemAPI();

            // ========== 初期化・終了 ==========

            /// @brief 初期化
            /// @param initialResolution 初期解像度
            /// @return 成功時は true、失敗時は false
            bool Initialize(Resolution initialResolution);

            /// @brief シャットダウン
            /// 明確な順序でリソースを解放します
            void Shutdown();

            /// @brief 初期化済みかどうか
            /// @return 初期化済みの場合true
            bool IsInitialized() const;

            // ========== ログ管理 ==========

            /// @brief ログファイルパスを設定（初期化前にのみ有効）
            /// @param directory ログディレクトリパス
            /// @param filename ログファイル名
            void SetLogPath(const std::string& directory, const std::string& filename);

            /// @brief ログレベルを設定
            /// @param level ログレベル（trace, debug, info, warn, error, critical, off）
            void SetLogLevel(spdlog::level::level_enum level);

            // ========== リソース管理：初期化 ==========

            /// @brief リソースマネージャーの初期化
            void InitializeResources();

            /// @brief 重い初期化処理（全リソース読み込み）
            /// @param callback 進捗通知コールバック
            void InitializeResources(ProgressCallback callback);

            /// @brief リソース初期化済みかどうか
            bool IsResourcesInitialized() const;

            // ========== リソース管理：テクスチャ ==========

            /// @brief テクスチャを名前で取得
            /// @param name テクスチャの名前（例："enemy_basic"）
            /// @return テクスチャへのvoid*ポインタ
            void* GetTexture(const std::string& name);
            
            /// @brief テクスチャが存在するか確認
            /// @param name テクスチャの名前
            /// @return テクスチャが存在する場合true
            bool HasTexture(const std::string& name) const;

            // ========== リソース管理：サウンド・ミュージック ==========

            /// @brief サウンドを名前で取得
            /// @param name サウンドの名前（例："hit"）
            /// @return サウンドへのvoid*ポインタ
            void* GetSound(const std::string& name);
            
            /// @brief ミュージックを名前で取得
            /// @param name ミュージックの名前（例："bgm_main"）
            /// @return ミュージックへのvoid*ポインタ
            void* GetMusic(const std::string& name);

            // ========== リソース管理：フォント ==========

            /// @brief フォントを名前で取得
            /// @param name フォントの名前（例："default"）
            /// @return フォントへのvoid*ポインタ（Font*）
            void* GetFont(const std::string& name);

            /// @brief デフォルトフォントを設定
            /// @param name フォント名
            /// @param fontSize フォントサイズ
            void SetDefaultFont(const std::string& name, int fontSize);

            /// @brief デフォルトフォントを取得
            /// @return デフォルトフォントへのvoid*ポインタ（Font*）、設定されていない場合はnullptr
            void* GetDefaultFont() const;

            // ========== リソース管理：進捗管理 ==========

            /// @brief ディレクトリスキャンして読み込むファイルリストを構築
            /// @return 総ファイル数
            int ScanResourceFiles();

            /// @brief 次の1つのリソースを読み込む
            /// @param callback 進捗通知コールバック
            /// @return 読み込み成功時true、完了時false
            bool LoadNextResource(ProgressCallback callback = nullptr);

            /// @brief まだ読み込むリソースがあるか
            /// @return 読み込むリソースがある場合true
            bool HasMoreResources() const;

            /// @brief 現在の進捗情報を取得
            /// @return 進捗情報
            LoadProgress GetCurrentProgress() const;

            /// @brief 読み込み状態をリセット
            void ResetLoadingState();

            // ========== 描画管理：解像度 ==========

            /// @brief 解像度を変更
            /// @param newResolution 新しい解像度
            /// @return 成功時は true、失敗時は false
            bool SetResolution(Resolution newResolution);

            /// @brief 現在の解像度を取得
            /// @return 現在の解像度プリセット
            Resolution GetCurrentResolution() const;

            /// @brief 画面幅を取得（ウィンドウサイズ）
            /// @return 画面幅（ピクセル）
            int GetScreenWidth() const;

            /// @brief 画面高さを取得（ウィンドウサイズ）
            /// @return 画面高さ（ピクセル）
            int GetScreenHeight() const;

            /// @brief 内部解像度幅を取得（標準）
            /// @return 内部解像度幅（ピクセル）
            int GetInternalWidth() const;

            /// @brief 内部解像度高さを取得（標準）
            /// @return 内部解像度高さ（ピクセル）
            int GetInternalHeight() const;

            // ========== 描画管理：フレーム制御 ==========

            /// @brief 描画開始
            /// RenderTextureへの描画を開始し、背景をWHITEでクリア
            void BeginRender();

            /// @brief 描画開始（拡張版）
            /// RenderTextureへの描画を開始し、背景をクリアするかどうかを指定可能
            /// @param clearBackground 背景をクリアする場合はtrue、しない場合はfalse
            void BeginRenderEx(bool clearBackground = true);

            /// @brief 描画終了
            /// RenderTextureへの描画を終了
            void EndRender();

            /// @brief フレーム描画終了
            /// RenderTextureを画面へスケーリング描画して終了
            /// @param imGuiCallback ImGuiフレーム内で呼び出されるコールバック（オプション）
            void EndFrame(ImGuiRenderCallback imGuiCallback = nullptr);

            // ========== 描画管理：ImGUI ==========

            /// @brief ImGUIフレーム開始
            /// 画面に直接描画するためのImGUIフレームを開始
            /// EndRender()の後、EndFrame()の前または中で呼び出す必要がある
            void BeginImGui();

            /// @brief ImGUIフレーム終了
            /// ImGUIフレームを終了
            void EndImGui();

            /// @brief ImGUI初期化状態を取得
            /// @return ImGUIが初期化されている場合true
            bool IsImGuiInitialized() const;

            /// @brief ImGUI日本語フォントを取得
            /// @return 日本語フォントへのポインタ（void* = ImFont*）、設定されていない場合nullptr
            void* GetImGuiJapaneseFont() const;

            // ========== 描画管理：スケーリング ==========

            /// @brief 座標をウィンドウスケールに変換
            /// @param internalX 内部座標系X
            /// @param internalY 内部座標系Y
            /// @return ウィンドウスケール後の座標
            Vector2 ScalePosition(float internalX, float internalY) const;

            /// @brief サイズをウィンドウスケールに変換
            /// @param internalSize 内部座標系でのサイズ
            /// @return ウィンドウスケール後のサイズ
            float ScaleSize(float internalSize) const;

            /// @brief スケール係数を取得（ウィンドウ幅 / 内部幅）
            /// @return スケール係数
            float GetScaleFactor() const;

            // ========== 描画管理：テキスト描画 ==========

            // ========== Raylibデフォルトフォント版 ==========
            
            /// @brief Raylibデフォルトフォントでテキストを描画
            void DrawTextRaylib(const std::string& text, float x, float y, float fontSize, Color color);
            
            /// @brief Raylibデフォルトフォントでテキストを詳細指定で描画
            void DrawTextRaylibEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color);

            // ========== デフォルトフォント版（日本語対応） ==========
            
            /// @brief デフォルトフォント（日本語対応）でテキストを描画
            void DrawTextDefault(const std::string& text, float x, float y, float fontSize, Color color);
            
            /// @brief デフォルトフォント（日本語対応）でテキストを詳細指定で描画
            void DrawTextDefaultEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color);

            // ========== フォント明示指定版 ==========
            
            /// @brief 指定フォントでテキストを描画
            void DrawTextWithFont(Font* font, const std::string& text, float x, float y, float fontSize, Color color);
            
            /// @brief 指定フォントでテキストを詳細指定で描画
            void DrawTextWithFontEx(Font* font, const std::string& text, Vector2 position, float fontSize, float spacing, Color color);

            /// @brief デフォルトフォントでテキストサイズを測定
            Vector2 MeasureTextDefault(const std::string& text, float fontSize, float spacing = 1.0f) const;
            
            /// @brief 指定フォントでテキストサイズを測定
            Vector2 MeasureTextWithFont(Font* font, const std::string& text, float fontSize, float spacing = 1.0f) const;

            // ========== 描画管理：基本図形 ==========

            /// @brief 矩形を描画
            void DrawRectangle(float x, float y, float width, float height, Color color);
            
            /// @brief 矩形の枠線を描画
            void DrawRectangleLines(float x, float y, float width, float height, float thickness, Color color);
            
            /// @brief 円を描画
            void DrawCircle(float centerX, float centerY, float radius, Color color);
            
            /// @brief 円の枠線を描画
            void DrawCircleLines(float centerX, float centerY, float radius, float thickness, Color color);
            
            /// @brief 直線を描画
            void DrawLine(float startX, float startY, float endX, float endY, float thickness, Color color);
            
            /// @brief プログレスバーを描画
            void DrawProgressBar(float x, float y, float width, float height, float progress,
                                Color fillColor = SKYBLUE, Color emptyColor = DARKGRAY, Color outlineColor = WHITE);

            // ========== 描画管理：拡張図形 ==========

            /// @brief ピクセルを描画
            void DrawPixel(int x, int y, Color color);
            
            /// @brief ピクセルを描画（Vector2版）
            void DrawPixelV(Vector2 position, Color color);

            /// @brief 直線を描画（Vector2版）
            void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
            
            /// @brief ベジェ曲線を描画
            void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);
            
            /// @brief 連続線を描画
            void DrawLineStrip(Vector2* points, int pointCount, Color color);
            
            /// @brief 連続線を描画（std::vector版）
            void DrawLineStrip(const std::vector<Vector2>& points, Color color);

            /// @brief 円を描画（Vector2版）
            void DrawCircleV(Vector2 center, float radius, Color color);
            
            /// @brief 円の扇形を描画
            void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
            
            /// @brief 円の扇形の枠線を描画
            void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
            
            /// @brief グラデーション円を描画
            void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);

            /// @brief 楕円を描画
            void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);
            
            /// @brief 楕円の枠線を描画
            void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);

            /// @brief リングを描画
            void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
            
            /// @brief リングの枠線を描画
            void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);

            /// @brief 矩形を描画（Vector2版）
            void DrawRectangleV(Vector2 position, Vector2 size, Color color);
            
            /// @brief 矩形を描画（Rectangle版）
            void DrawRectangleRec(Rectangle rec, Color color);
            
            /// @brief 矩形を描画（回転・原点指定版）
            void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);
            
            /// @brief 垂直グラデーション矩形を描画
            void DrawRectangleGradientV(int x, int y, int width, int height, Color color1, Color color2);
            
            /// @brief 水平グラデーション矩形を描画
            void DrawRectangleGradientH(int x, int y, int width, int height, Color color1, Color color2);
            
            /// @brief 四色グラデーション矩形を描画
            void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4);
            
            /// @brief 角丸矩形を描画
            void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
            
            /// @brief 角丸矩形の枠線を描画
            void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);

            /// @brief 三角形を描画
            void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
            
            /// @brief 三角形の枠線を描画
            void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
            
            /// @brief 扇形三角形を描画
            void DrawTriangleFan(Vector2* points, int pointCount, Color color);
            
            /// @brief 扇形三角形を描画（std::vector版）
            void DrawTriangleFan(const std::vector<Vector2>& points, Color color);
            
            /// @brief ストリップ三角形を描画
            void DrawTriangleStrip(Vector2* points, int pointCount, Color color);
            
            /// @brief ストリップ三角形を描画（std::vector版）
            void DrawTriangleStrip(const std::vector<Vector2>& points, Color color);

            /// @brief 正多角形を描画
            void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);
            
            /// @brief 正多角形の枠線を描画
            void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);
            
            /// @brief 正多角形の枠線を描画（拡張版）
            void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color);

            // ========== 描画管理：スプライン ==========

            /// @brief 線形スプラインを描画
            void DrawSplineLinear(Vector2* points, int pointCount, float thick, Color color);
            
            /// @brief 線形スプラインを描画（std::vector版）
            void DrawSplineLinear(const std::vector<Vector2>& points, float thick, Color color);
            
            /// @brief Bスプラインを描画
            void DrawSplineBasis(Vector2* points, int pointCount, float thick, Color color);
            
            /// @brief Bスプラインを描画（std::vector版）
            void DrawSplineBasis(const std::vector<Vector2>& points, float thick, Color color);
            
            /// @brief Catmull-Romスプラインを描画
            void DrawSplineCatmullRom(Vector2* points, int pointCount, float thick, Color color);
            
            /// @brief Catmull-Romスプラインを描画（std::vector版）
            void DrawSplineCatmullRom(const std::vector<Vector2>& points, float thick, Color color);
            
            /// @brief 二次ベジェスプラインを描画
            void DrawSplineBezierQuadratic(Vector2* points, int pointCount, float thick, Color color);
            
            /// @brief 二次ベジェスプラインを描画（std::vector版）
            void DrawSplineBezierQuadratic(const std::vector<Vector2>& points, float thick, Color color);
            
            /// @brief 三次ベジェスプラインを描画
            void DrawSplineBezierCubic(Vector2* points, int pointCount, float thick, Color color);
            
            /// @brief 三次ベジェスプラインを描画（std::vector版）
            void DrawSplineBezierCubic(const std::vector<Vector2>& points, float thick, Color color);

            // ========== 描画管理：テクスチャ ==========

            /// @brief テクスチャを描画
            void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
            
            /// @brief テクスチャを描画（Vector2版）
            void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
            
            /// @brief テクスチャを描画（拡張版）
            void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
            
            /// @brief テクスチャの一部を描画
            void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
            
            /// @brief テクスチャを描画（プロ版）
            void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
            
            /// @brief 9パッチテクスチャを描画
            void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint);

            // ========== 描画管理：テキスト拡張 ==========

            /// @brief テキストを描画（プロ版）
            void DrawTextPro(Font font, const std::string& text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);
            
            /// @brief コードポイントを描画
            void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint);
            
            /// @brief コードポイント列を描画
            void DrawTextCodepoints(Font font, const int* codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint);

            // ========== オーディオ管理 ==========

            /// @brief オーディオ更新処理（毎フレーム呼び出し必須）
            /// @param deltaTime 前フレームからの経過時間（秒）
            void UpdateAudio(float deltaTime);

            // ========== オーディオ管理：再生制御 ==========

            /// @brief Soundを再生（重複可能）
            /// @param name サウンド名
            /// @return 再生成功時true
            bool PlaySound(const std::string& name);

            /// @brief Musicを再生（重複不可、既存を停止）
            /// @param name ミュージック名
            /// @return 再生成功時true
            bool PlayMusic(const std::string& name);

            /// @brief 全Soundを停止
            void StopSound();

            /// @brief 指定したSoundを停止
            /// @param name サウンド名
            void StopSound(const std::string& name);

            /// @brief 現在のMusicを停止
            void StopMusic();

            // ========== オーディオ管理：状態管理 ==========

            /// @brief Soundが再生中か確認
            /// @param name サウンド名
            /// @return 再生中の場合true
            bool IsSoundPlaying(const std::string& name) const;

            /// @brief Musicが再生中か確認
            /// @return 再生中の場合true
            bool IsMusicPlaying() const;

            /// @brief 現在再生中のMusic名を取得
            /// @return Music名（再生中でない場合は空文字列）
            std::string GetCurrentMusicName() const;

            // ========== オーディオ管理：ボリューム管理 ==========

            /// @brief マスターボリュームを設定
            /// @param volume ボリューム（0.0-1.0）
            void SetMasterVolume(float volume);

            /// @brief SEボリュームを設定
            /// @param volume ボリューム（0.0-1.0）
            void SetSEVolume(float volume);

            /// @brief BGMボリュームを設定
            /// @param volume ボリューム（0.0-1.0）
            void SetBGMVolume(float volume);

            /// @brief マスターボリュームを取得
            /// @return マスターボリューム（0.0-1.0）
            float GetMasterVolume() const;

            /// @brief SEボリュームを取得
            /// @return SEボリューム（0.0-1.0）
            float GetSEVolume() const;

            /// @brief BGMボリュームを取得
            /// @return BGMボリューム（0.0-1.0）
            float GetBGMVolume() const;

            // ========== 入力処理：状態更新 ==========

            /// @brief 入力状態を更新（毎フレーム呼び出し必須）
            void UpdateInput();

            // ========== 入力処理：キーボード ==========

            /// @brief キーが押されたか（1フレームのみ）
            bool IsKeyPressed(int key);
            
            /// @brief キーが押されているか（リピート対応）
            bool IsKeyPressedRepeat(int key);
            
            /// @brief キーが押されているか（連続）
            bool IsKeyDown(int key);
            
            /// @brief キーが離されたか（1フレームのみ）
            bool IsKeyReleased(int key);
            
            /// @brief キーが離されているか
            bool IsKeyUp(int key);
            
            /// @brief 押されたキーを取得（1フレームのみ）
            int GetKeyPressed();
            
            /// @brief 押された文字を取得（1フレームのみ）
            int GetCharPressed();
            
            /// @brief 終了キーを設定
            void SetExitKey(int key);

            // ========== 入力処理：マウス ==========

            /// @brief マウスボタンが押されたか（1フレームのみ、consumed機能付き）
            bool IsMouseButtonPressed(int button);
            
            /// @brief マウスボタンが押されているか（連続）
            bool IsMouseButtonDown(int button);
            
            /// @brief マウスボタンが離されたか（1フレームのみ）
            bool IsMouseButtonReleased(int button);
            
            /// @brief マウスボタンが離されているか
            bool IsMouseButtonUp(int button);
            
            /// @brief マウスX座標を取得
            int GetMouseX();
            
            /// @brief マウスY座標を取得
            int GetMouseY();
            
            /// @brief マウス位置を取得
            Vector2 GetMousePosition();
            
            /// @brief マウス移動量を取得
            Vector2 GetMouseDelta();
            
            /// @brief マウスホイールの移動量を取得
            float GetMouseWheelMove();
            
            /// @brief マウスホイールの移動量を取得（Vector2版）
            Vector2 GetMouseWheelMoveV();
            
            /// @brief マウス位置を設定
            void SetMousePosition(int x, int y);
            
            /// @brief マウスオフセットを設定
            void SetMouseOffset(int offsetX, int offsetY);
            
            /// @brief マウススケールを設定
            void SetMouseScale(float scaleX, float scaleY);
            
            /// @brief マウスカーソルを設定
            void SetMouseCursor(int cursor);
            
            /// @brief マウスボタンを消費（重複検出を防ぐ）
            void ConsumeMouseButton(int button);

            // ========== 入力処理：ゲームパッド ==========

            /// @brief ゲームパッドが利用可能か
            bool IsGamepadAvailable(int gamepad);
            
            /// @brief ゲームパッド名を取得
            const char* GetGamepadName(int gamepad);
            
            /// @brief ゲームパッドボタンが押されたか（1フレームのみ）
            bool IsGamepadButtonPressed(int gamepad, int button);
            
            /// @brief ゲームパッドボタンが押されているか（連続）
            bool IsGamepadButtonDown(int gamepad, int button);
            
            /// @brief ゲームパッドボタンが離されたか（1フレームのみ）
            bool IsGamepadButtonReleased(int gamepad, int button);
            
            /// @brief ゲームパッドボタンが離されているか
            bool IsGamepadButtonUp(int gamepad, int button);
            
            /// @brief ゲームパッド軸の移動量を取得
            float GetGamepadAxisMovement(int gamepad, int axis);

            // ========== 入力処理：タッチ・ジェスチャー ==========

            /// @brief タッチポイント数を取得
            int GetTouchPointCount();
            
            /// @brief タッチ位置を取得
            Vector2 GetTouchPosition(int index);
            
            /// @brief ジェスチャーが検出されたか
            bool IsGestureDetected(unsigned int gesture);
            
            /// @brief 検出されたジェスチャーを取得
            int GetGestureDetected();

            // ========== タイミング ==========

            /// @brief フレーム時間を取得（秒）
            float GetFrameTime();
            
            /// @brief FPSを取得
            int GetFPS();
            
            /// @brief ターゲットFPSを設定
            void SetTargetFPS(int fps);
            
            /// @brief 経過時間を取得（秒）
            double GetTime();

            // ========== ウィンドウ管理 ==========

            /// @brief ウィンドウが閉じるべきか
            bool WindowShouldClose();
            
            /// @brief ウィンドウが準備完了か
            bool IsWindowReady();

            // ========== 衝突判定 ==========

            /// @brief 点と矩形の衝突判定
            bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
            
            /// @brief 矩形と矩形の衝突判定
            bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
            
            /// @brief 円と円の衝突判定
            bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);
            
            /// @brief 円と矩形の衝突判定
            bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);
            
            /// @brief 衝突矩形を取得
            Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);

        private:
            // ========== プライベートメソッド ==========

            /// @brief RenderTexture を再作成
            void RecreateRenderTexture();

            /// @brief デフォルトフォントを取得（内部用）
            Font* GetDefaultFontInternal() const;

            /// @brief プレースホルダーテクスチャを生成
            Texture2D CreatePlaceholderTexture(const std::string& name);

            /// @brief ディレクトリをスキャン（非再帰）
            void ScanDirectory(const std::string& dirPath, ResourceType type, const std::vector<std::string>& extensions);

            /// @brief ディレクトリを再帰的にスキャン
            void ScanDirectoryRecursive(const std::string& dirPath, ResourceType type, const std::vector<std::string>& extensions);

            /// @brief フォントを読み込む
            void LoadFont(const std::string& path, const std::string& name);

            /// @brief テクスチャを読み込む
            void LoadTexture(const std::string& path, const std::string& name);

            /// @brief サウンドを読み込む
            void LoadSound(const std::string& path, const std::string& name);

            /// @brief JSONファイルを読み込む
            void LoadJson(const std::string& path, const std::string& name);

            /// @brief フォント用コードポイント配列を生成
            void GenerateFontCodepoints();

            /// @brief ログシステムの初期化（内部用）
            void InitializeLogSystem();

            /// @brief ログシステムのシャットダウン（内部用）
            void ShutdownLogSystem();

            // ========== メンバ変数 ==========

            // 画面解像度
            Resolution currentResolution_;
            int screenWidth_;
            int screenHeight_;

            // 内部解像度（常にFHD固定）
            static constexpr int INTERNAL_WIDTH = 1920;
            static constexpr int INTERNAL_HEIGHT = 1080;

            // RenderTexture（1つに統一）
            RenderTexture2D mainRenderTexture_;

            // 初期化状態
            bool isInitialized_;
            bool resourcesInitialized_;

            // ログ管理
            std::shared_ptr<spdlog::logger> logger_;
            bool logInitialized_;
            std::string logDirectory_;
            std::string logFileName_;

            // リソースキャッシュ（shared_ptrで管理）
            std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures_;
            std::unordered_map<std::string, std::shared_ptr<Sound>> sounds_;
            std::unordered_map<std::string, std::shared_ptr<Music>> musics_;
            std::unordered_map<std::string, std::shared_ptr<Font>> fonts_;
            std::shared_ptr<Font> defaultFont_;

            // フォント用コードポイント
            std::vector<int> fontCodepoints_;
            
            // ImGUI初期化状態
            bool imGuiInitialized_;
            
            // ImGUI日本語フォント（void* = ImFont*）
            void* imGuiJapaneseFont_;

            // ファイルリスト管理
            std::vector<ResourceFileInfo> resourceFileList_;
            size_t currentResourceIndex_;
            bool scanningCompleted_;

            // オーディオ管理
            float masterVolume_;                // マスターボリューム (0.0-1.0)
            float seVolume_;                    // SEボリューム (0.0-1.0)
            float bgmVolume_;                   // BGMボリューム (0.0-1.0)
            Music* currentMusic_;               // 現在再生中のMusic（重複不可のため1つのみ）
            std::string currentMusicName_;      // 現在再生中のMusic名
            std::unordered_map<std::string, Sound*> playingSounds_;  // 再生中のSound管理（重複可能）

            // 入力状態管理
            struct InputState {
                float mouseX = 0.0f;
                float mouseY = 0.0f;
                float mouseDeltaX = 0.0f;
                float mouseDeltaY = 0.0f;
                std::bitset<8> mouseButtonConsumed;
            };
            InputState inputState_;

            /// @brief ボリューム値をクランプ（0.0-1.0）
            float ClampVolume(float volume) const;

            /// @brief Soundのボリュームを更新
            void UpdateSoundVolume(Sound* sound) const;

            /// @brief Musicのボリュームを更新
            void UpdateMusicVolume(Music* music) const;
        };
    }
}

#endif // __GAME_CORE_BASESYSTEMAPI_HPP__
