#pragma once

#include <string>
#include <vector>
#include <memory>
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

namespace Shared::Simulation {
    class SimulationContext;
}

namespace Editor::Windows {

/**
 * @brief ユニット（エンティティ）定義の作成・編集・削除を行うエディタウィンドウ
 * 
 * 左側にフィルタ付きエンティティ一覧、右側に基本パラメータ編集フォームを配置。
 * PropertyPanelパターンで EntityDef を直接編集し、リアルタイムバリデーションを実施。
 */
class UnitEditorWindow : public IEditorWindow {
public:
    UnitEditorWindow();
    ~UnitEditorWindow() override;

    // IEditorWindowインターフェース実装
    void Initialize(Shared::Core::GameContext& context, 
                   Shared::Data::DefinitionRegistry& definitions) override;
    void Shutdown() override {}
    void OnUpdate(float deltaTime) override;
    void OnDrawUI() override;
    
    std::string GetWindowTitle() const override { return "ユニットエディタ"; }
    std::string GetWindowId() const override { return "UnitEditorWindow"; }
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

private:
    // === UI描画 ===
    void DrawLeftPanel();     // エンティティ一覧
    void DrawRightPanel();    // 編集フォーム
    
    void DrawEntityList();    // フィルタ付き一覧テーブル
    void DrawEditForm();      // パラメータ編集フォーム
    void DrawToolbar();       // 新規作成・削除・保存ボタン
    
    // === フォーム描画の内訳 ===
    void DrawBasicInfoFields();    // ID/名前/レアリティ/type
    void DrawTeamFields();         // 敵/味方選択
    void DrawCostFields();         // コスト/クールダウン（味方のみ）
    void DrawStatsFields();        // HP/攻撃/速度など
    void DrawCombatFields();       // attack_point/attack_frame/hitbox

    // === 操作 ===
    void CreateNewEntity();        // 新規作成
    void DuplicateEntity();        // 複製
    void DeleteActiveEntity();     // 削除
    void SaveChanges();            // JSON保存
    void LoadEntityToForm(const std::string& entityId);  // フォーム読み込み
    void ClearForm();              // フォームクリア

    // === バリデーション ===
    bool ValidateForm() const;     // 全体バリデーション
    bool ValidateId(const std::string& id, bool isNewEntity) const;  // ID重複チェック
    bool ValidateName(const std::string& name) const;
    bool ValidateStats() const;    // ステータス範囲チェック

    // === ユーティリティ ===
    std::vector<const Shared::Data::EntityDef*> GetFilteredEntities() const;
    bool MatchesSearchQuery(const Shared::Data::EntityDef* entity) const;

    // === メンバ変数 ===
    Shared::Core::GameContext* context_;
    Shared::Data::DefinitionRegistry* definitions_;
    bool isOpen_;
    
    // 編集状態
    std::string activeEntityId_;     // 現在編集中のエンティティID（空文字列=新規作成モード）
    bool isEditMode_;                // true: 編集, false: 新規作成
    bool isDirty_;                   // 未保存の変更あり
    
    // フォームデータ（一時バッファ）
    struct FormData {
        char id[128];
        char name[256];
        int rarity;
        char type[32];            // "main", "sub", "enemy"
        bool is_enemy;
        
        int cost;
        float cooldown;
        
        struct {
            int hp;
            int attack;
            float attack_speed;
            float move_speed;
            int range;
            int knockback;
        } stats;
        
        struct {
            float attack_point;
            int attack_frame;
            struct {
                float width;
                float height;
                float offset_x;
                float offset_y;
            } hitbox;
        } combat;
        
        char description[512];
        char tags[512];  // カンマ区切り
    };
    FormData formData_;
    
    // バリデーションエラー
    mutable std::vector<std::string> validationErrors_;
    
    // フィルタ・検索
    char searchBuffer_[256];
    int filterType_;  // 0: 全て, 1: 味方のみ, 2: 敵のみ, 3: メインのみ, 4: サブのみ
    int sortMode_;    // 0: ID, 1: 名前, 2: レアリティ, 3: タイプ
    
    // UI状態
    bool showDeleteConfirm_;
    float leftPanelWidth_;
};

} // namespace Editor::Windows
