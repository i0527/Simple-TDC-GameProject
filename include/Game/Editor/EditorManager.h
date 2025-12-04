/**
 * @file EditorManager.h
 * @brief ImGuiベースのゲーム内蔵エディター
 * 
 * WebUIの代わりにImGuiを使用してゲーム内でデータ編集を可能にする。
 * ゲームと同じプロセスで動作し、リアルタイム編集が可能。
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include "imgui.h"
#include "rlImGui.h"
#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"

namespace Game {
namespace Editor {

/**
 * @brief エディターウィンドウの種類
 */
enum class EditorWindow {
    None,
    Characters,    // キャラクターエディター
    Stages,        // ステージエディター
    UI,            // UIエディター
    Skills,        // スキルエディター
    Effects,       // エフェクトエディター
    Sounds,        // サウンドエディター
    Inspector,     // インスペクター（エンティティ詳細）
    Console,       // コンソール（ログ表示）
    Hierarchy,     // ヒエラルキー（エンティティ一覧）
    Assets         // アセットブラウザ
};

/**
 * @brief ゲーム内蔵エディターマネージャー
 * 
 * ImGuiを使用してゲーム内でデータ編集UIを提供する。
 * F1キーでエディターの表示/非表示を切り替え。
 */
class EditorManager {
public:
    EditorManager();
    ~EditorManager();

    /**
     * @brief エディターを初期化
     * @param registry 定義レジストリ
     * @param loader 定義ローダー
     * @return 成功した場合true
     */
    bool Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);

    /**
     * @brief エディターを更新（入力処理）
     * @param deltaTime デルタタイム
     */
    void Update(float deltaTime);

    /**
     * @brief エディターUIを描画
     */
    void Render();

    /**
     * @brief エディターの表示/非表示を切り替え
     */
    void ToggleVisibility();

    /**
     * @brief エディターが表示中かどうか
     */
    bool IsVisible() const { return visible_; }

    /**
     * @brief エディターを表示
     */
    void Show() { visible_ = true; }

    /**
     * @brief エディターを非表示
     */
    void Hide() { visible_ = false; }

    /**
     * @brief 特定のウィンドウを開く
     */
    void OpenWindow(EditorWindow window);

    /**
     * @brief ログメッセージを追加
     */
    void AddLog(const std::string& message);

private:
    // 初期化状態
    bool initialized_ = false;
    bool visible_ = false;

    // データへの参照
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;

    // 現在開いているウィンドウ
    bool showCharacters_ = false;
    bool showStages_ = false;
    bool showUI_ = false;
    bool showSkills_ = false;
    bool showEffects_ = false;
    bool showSounds_ = false;
    bool showInspector_ = true;   // デフォルトで表示
    bool showConsole_ = true;      // デフォルトで表示
    bool showHierarchy_ = true;    // デフォルトで表示
    bool showAssets_ = false;

    // 選択状態
    std::string selectedCharacterId_;
    std::string selectedStageId_;
    std::string selectedUIId_;

    // コンソールログ
    std::vector<std::string> logs_;

    // UI描画関数
    void RenderMenuBar();
    void RenderCharacterEditor();
    void RenderStageEditor();
    void RenderUIEditor();
    void RenderSkillEditor();
    void RenderEffectEditor();
    void RenderSoundEditor();
    void RenderInspector();
    void RenderConsole();
    void RenderHierarchy();
    void RenderAssetBrowser();

    // ヘルパー関数
    void ShowHelpMarker(const char* desc);
};

} // namespace Editor
} // namespace Game
