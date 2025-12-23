#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Editor/Windows/IEditorWindow.h"
#include "Data/Definitions/EntityDef.h"

// 前方宣言
namespace Shared::Core {
    class GameContext;
}

namespace Shared::Data {
    class DefinitionRegistry;
    struct EntityDef;
}

namespace Editor::Windows {
    class PreviewWindow;
}

namespace Shared::Simulation {
    class SimulationContext;
}

namespace Editor::Windows {

/**
 * @brief スプライト・アニメーション設定を編集するエディタウィンドウ
 * 
 * atlas_texture、sprite_actions、draw_type、hitbox設定を編集。
 * PreviewWindowと連携してリアルタイムアニメーションプレビューを提供。
 */
class SpriteEditorWindow : public IEditorWindow {
public:
    SpriteEditorWindow();
    ~SpriteEditorWindow() override;

    // IEditorWindowインターフェース実装
    void Initialize(Shared::Core::GameContext& context, 
                   Shared::Data::DefinitionRegistry& definitions) override;
    void Shutdown() override {}
    void OnUpdate(float deltaTime) override;
    void OnDrawUI() override;
    
    std::string GetWindowTitle() const override { return "スプライトエディタ"; }
    std::string GetWindowId() const override { return "SpriteEditorWindow"; }
    bool IsOpen() const override { return isOpen_; }
    void SetOpen(bool open) override { isOpen_ = open; }

    /**
     * @brief 外部から編集対象エンティティを設定
     */
    void SetActiveEntity(const std::string& entityId);

    /**
     * @brief 現在編集中のエンティティIDを取得
     */
    const std::string& GetActiveEntityId() const { return activeEntityId_; }
    
    /**
     * @brief PreviewWindowを設定（連携用）
     */
    void SetPreviewWindow(PreviewWindow* previewWindow) { previewWindow_ = previewWindow; }

private:
    // === UI描画 ===
    void DrawToolbar();
    void DrawEditForm();
    
    void DrawTextureSettings();      // atlas_texture, sprite_sheet設定
    void DrawDrawTypeSelector();     // draw_type切替（警告ダイアログ付き）
    void DrawSpriteActionsEditor();  // sprite_actions マップ編集
    void DrawFourAnimSettings();     // sprite_for4animation 用の4アニメシート設定
    void DrawIconSettings();         // アイコン設定
    void DrawAnimationSettings();    // レガシーアニメーション設定（GridSheet用）
    
    // === 操作 ===
    void LoadEntityToForm(const std::string& entityId);
    void SaveChanges();
    void AddSpriteAction();          // sprite_actions に新規アクション追加
    void RemoveSpriteAction(const std::string& actionName);
    
    // === draw_type切替確認 ===
    void ShowDrawTypeChangeDialog(); // 警告ダイアログ表示
    void ApplyDrawTypeChange();      // draw_type変更を適用
    void CancelDrawTypeChange();     // draw_type変更をキャンセル
    
    // === バリデーション ===
    bool ValidateForm() const;
    bool ValidateTexturePath(const std::string& path) const;
    bool ValidateJsonPath(const std::string& path) const;
    
    // === 保存機能 ===
    bool SaveEntityDefToJson(const std::string& entityId);
    
    // === メンバ変数 ===
    Shared::Core::GameContext* context_;
    Shared::Data::DefinitionRegistry* definitions_;
    PreviewWindow* previewWindow_ = nullptr;  // 連携用
    bool isOpen_;
    
    // 編集状態
    std::string activeEntityId_;
    bool isDirty_;
    
    // フォームデータ
    struct FormData {
        char draw_type[32];          // "sprite" / "parts_animation"
        
        // スプライトシート形式（draw_type = "sprite"）
        char atlas_texture[512];
        std::unordered_map<std::string, char[512]> sprite_actions;  // action名 -> JSONパス
        
        // GridSheet形式（レガシー）
        char sprite_sheet[512];
        // sprite_for4animation 用（各アクションごとにJSONとPNGを設定）
        char idle_animation[512];
        char idle_image[512];
        char walk_animation[512];
        char walk_image[512];
        char attack_animation[512];
        char attack_image[512];
        char death_animation[512];
        char death_image[512];
        
        // アイコン
        char icon[512];
        // 既定ミラー
        bool mirror_h = false;
        bool mirror_v = false;
    };
    FormData formData_;
    
    // 4アニメ用JSON生成パラメータ
    struct GenParams {
        int frameW = 0;
        int frameH = 0;
        int frames = 0;
        int durationMs = 100;
        int columns = 0;  // グリッド列数（0なら横一列）
        int rows = 0;     // グリッド行数（0なら自動計算）
        int yOffset = 0;  // Y軸オフセット
        float pivotX = 0.5f; // スプライト原点X（0.0-1.0）
        float pivotY = 0.5f; // スプライト原点Y（0.0-1.0）
        bool mirrorH = false; // 水平ミラー
        bool mirrorV = false; // 垂直ミラー
    };
    GenParams genIdle_{};
    GenParams genWalk_{};
    GenParams genAttack_{};
    GenParams genDeath_{};
    
    // draw_type切替ダイアログ
    bool showDrawTypeDialog_;
    char pendingDrawType_[32];       // 切り替え予定のdraw_type
    
    // sprite_actions 新規追加UI
    char newActionName_[128];
    char newActionPath_[512];
    
    // バリデーションエラー
    mutable std::vector<std::string> validationErrors_;
    
    // 生成成功メッセージ
    std::string successMessage_;
    float successMessageTimer_ = 0.0f;
    
    // 生成エラーメッセージ
    std::string errorMessage_;
    float errorMessageTimer_ = 0.0f;
};

} // namespace Editor::Windows
