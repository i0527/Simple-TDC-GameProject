#pragma once

#include "../api/BaseSystemAPI.hpp"
#include <string>

namespace game {
namespace core {

/// @brief リソース初期化専用クラス
///
/// 責務:
/// - リソーススキャン・読み込み処理
/// - 進捗管理
/// - 初期化画面・エラー画面の描画
/// - 初期化完了/失敗の状態通知
///
/// 安全化:
/// - 二重初期化防止
/// - エラーハンドリング
/// - 状態フラグで安全な遷移
class ResourceInitializer {
public:
    ResourceInitializer();
    ~ResourceInitializer();

    /// @brief リソース初期化を開始
    /// @param systemAPI BaseSystemAPIへのポインタ（所有権なし）
    /// @return 成功時true、失敗時false
    bool Initialize(BaseSystemAPI* systemAPI);

    /// @brief 毎フレーム呼び出される更新処理
    /// @param deltaTime デルタタイム（秒）
    /// @return 更新が正常に処理された場合true
    bool Update(float deltaTime);

    /// @brief 初期化画面/エラー画面の描画
    void Render();

    /// @brief 初期化完了チェック
    /// @return 初期化が完了している場合true
    bool IsCompleted() const;

    /// @brief エラー発生チェック
    /// @return エラーが発生している場合true
    bool HasFailed() const;

    /// @brief エラー表示時間が経過し、終了すべきかチェック
    /// @return 終了すべき場合true（エラー表示時間が5秒を超えた場合）
    bool ShouldShutdown() const;

    /// @brief 状態をリセット（再初期化用）
    void Reset();

private:
    // リソース初期化状態
    struct InitState {
        bool scanningCompleted = false;
        bool initializationStarted = false;
        bool initializationCompleted = false;
        bool initializationFailed = false;
        int currentProgress = 0;
        int totalProgress = 0;
        std::string currentMessage = "初期化中...";
        std::string errorMessage = "";
        float completionDelay = 0.5f;
        float errorDisplayTime = 0.0f;
    };

    BaseSystemAPI* systemAPI_;
    InitState initState_;
    bool isInitialized_;

    // 初期化画面描画
    void renderInitScreen();

    // エラー画面描画
    void renderErrorScreen();
};

} // namespace core
} // namespace game
