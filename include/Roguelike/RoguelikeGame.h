/**
 * @file RoguelikeGame.h
 * @brief ローグライクゲームのメインクラス
 * 
 * Phase 5: 空腹度、経験値/レベルアップシステム対応
 */
#pragma once

#include <raylib.h>
#include <entt/entt.hpp>
#include <string>
#include <ctime>

#include "Components/RoguelikeComponents.h"
#include "Components/CombatComponents.h"
#include "Components/ItemComponents.h"
#include "Components/HungerComponents.h"
#include "Rendering/TileRenderer.h"
#include "Managers/TurnManager.h"
#include "Systems/InputSystem.h"
#include "Systems/ActionSystem.h"
#include "Systems/FOVSystem.h"
#include "Systems/AISystem.h"
#include "Systems/CombatSystem.h"
#include "Systems/ItemSystem.h"
#include "Systems/HungerSystem.h"
#include "Systems/LevelUpSystem.h"
#include "Generators/DungeonGenerator.h"
#include "Generators/MonsterSpawner.h"
#include "Generators/ItemSpawner.h"

namespace Roguelike {

/**
 * @brief ゲームの画面モード
 */
enum class GameMode {
    Explore,        ///< 通常探索（移動）
    ActionMenu,     ///< 足元アクションメニュー
    Inventory,      ///< インベントリ画面
    ItemAction,     ///< アイテム操作選択
};

/**
 * @brief ローグライクゲームクラス
 * 
 * ダンジョン自動生成、視界システム、階層移動、モンスター戦闘を統合。
 * シンプル操作（矢印移動、Enter決定、ESCキャンセル）対応。
 */
class RoguelikeGame {
public:
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;
    static constexpr int MAX_FLOOR = 10;  ///< 最大階層
    
    RoguelikeGame() = default;
    ~RoguelikeGame() { Shutdown(); }
    
    /**
     * @brief ゲームを初期化
     * @param skipWindowInit ウィンドウ初期化をスキップするか（シーン統合用）
     */
    bool Initialize(bool skipWindowInit = false) {
        // ウィンドウ初期化
        if (!skipWindowInit) {
            InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Roguelike RPG");
            SetTargetFPS(60);
        }
        
        // 乱数初期化
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        
        TraceLog(LOG_INFO, "Loading Japanese font...");
        
        // 日本語フォント読み込み
        // 日本語表示に必要なコードポイント範囲を定義
        LoadJapaneseFont();
        
        TraceLog(LOG_INFO, "Initializing tile renderer...");
        
        // タイルレンダラー初期化
        tileRenderer_.Initialize(font_);
        
        TraceLog(LOG_INFO, "Initializing dungeon generator...");
        
        // ダンジョンジェネレータ初期化
        dungeonGenerator_.SetSeed(static_cast<unsigned int>(std::time(nullptr)));
        
        TraceLog(LOG_INFO, "Generating first floor...");
        
        // 最初の階層を生成
        currentFloor_ = 1;
        GenerateFloor(currentFloor_);
        
        TraceLog(LOG_INFO, "Creating player...");
        
        // プレイヤー生成
        CreatePlayer();
        
        TraceLog(LOG_INFO, "Spawning monsters...");
        
        // モンスター生成
        SpawnMonsters();
        
        TraceLog(LOG_INFO, "Spawning items...");
        
        // アイテム生成
        SpawnItems();
        
        TraceLog(LOG_INFO, "Calculating initial FOV...");
        
        // 初期FOV計算
        UpdateFOV();
        
        TraceLog(LOG_INFO, "Initialization complete!");
        
        isInitialized_ = true;
        return true;
    }
    
    /**
     * @brief 日本語フォントを読み込み
     */
    void LoadJapaneseFont() {
        // 実行ファイルのディレクトリを取得
        const char* appDir = GetApplicationDirectory();
        std::string fontPath = std::string(appDir) + "assets/fonts/NotoSansJP-Medium.ttf";
        
        TraceLog(LOG_INFO, "Loading Japanese font from: %s", fontPath.c_str());
        
        if (!FileExists(fontPath.c_str())) {
            // フォントが見つからない場合はデフォルトを使用
            TraceLog(LOG_WARNING, "Font file not found at %s, using default font", fontPath.c_str());
            font_ = GetFontDefault();
            return;
        }
        
        // ゲームで使用する文字を直接指定（より確実な方法）
        // これにより必要な文字のみをロードしてメモリを節約
        static const char* textToLoad = 
            // ASCII基本文字
            " !\"#$%&'()*+,-./0123456789:;<=>?@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
            "abcdefghijklmnopqrstuvwxyz{|}~"
            // ゲームで使用する日本語
            "あなたのターン処理中ゲームオーバー"
            "地下階到着目標下り上り階段見つけてまで降りよう進行"
            "シードマップサイズ視界半径タイル"
            "移動矢印キー方向斜めピリオドその場で待機"
            "アイテム拾う落とす使う装備外した選択閉じる"
            "持ち物所持金空だ"
            "操作ガイドデバッグ状態"
            "床通路開閉扉水溶岩壁"
            "攻撃命中回避ダメージ防御"
            "経験値レベルアップになった得"
            "死んだ倒した殺された当たらなかった"
            "ネズミコウモリゴブリンコボルド"
            "オークスケルトンゾンビヘビ"
            "トロルオーガレイスドラゴン"
            "小動物闘飛ぶ生物小柄人型モンスター卑怯爬虫類"
            "凶暴戦士動く骨腐った死体毒蛇"
            "再生巨大人喰い実体霊恐怖竜"
            "素早いプレイヤーキャラクター"
            "現在最大ここにはがない昇った降りた"
            "地上出た冒険終わり"
            "弱強敵序盤中盤終盤"
            "全回復傷癒えた"
            // アイテム関連
            "回復薬上級完全"
            "ダガーショートソードロングソードグレートソード"
            "革鎧チェインメイルプレートアーマー"
            "携帯食料パン金貨輝く山"
            "軽い短剣扱いやすい剣標準的長剣両手剣"
            "軽い革製防具鎖編まれた重厚板金鎧"
            "腹持ちのよい焼きたて"
            "ゴールドスロット既に何もない";
        
        // 使用する文字のコードポイントを収集
        int codepoints[512];
        int count = 0;
        
        const char* p = textToLoad;
        while (*p && count < 512) {
            int codepoint = 0;
            unsigned char c = static_cast<unsigned char>(*p);
            
            if (c < 0x80) {
                // ASCII
                codepoint = c;
                p++;
            } else if ((c & 0xE0) == 0xC0) {
                // 2-byte UTF-8
                codepoint = (c & 0x1F) << 6;
                codepoint |= (static_cast<unsigned char>(p[1]) & 0x3F);
                p += 2;
            } else if ((c & 0xF0) == 0xE0) {
                // 3-byte UTF-8 (日本語はここ)
                codepoint = (c & 0x0F) << 12;
                codepoint |= (static_cast<unsigned char>(p[1]) & 0x3F) << 6;
                codepoint |= (static_cast<unsigned char>(p[2]) & 0x3F);
                p += 3;
            } else if ((c & 0xF8) == 0xF0) {
                // 4-byte UTF-8
                codepoint = (c & 0x07) << 18;
                codepoint |= (static_cast<unsigned char>(p[1]) & 0x3F) << 12;
                codepoint |= (static_cast<unsigned char>(p[2]) & 0x3F) << 6;
                codepoint |= (static_cast<unsigned char>(p[3]) & 0x3F);
                p += 4;
            } else {
                p++;
                continue;
            }
            
            // 重複チェック
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (codepoints[i] == codepoint) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                codepoints[count++] = codepoint;
            }
        }
        
        TraceLog(LOG_INFO, "Loading %d unique characters for font", count);
        
        // フォントをロード（サイズ32、指定コードポイント）
        font_ = LoadFontEx(fontPath.c_str(), 32, codepoints, count);
        
        // テクスチャフィルタを設定（見た目を良くする）
        SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
    }
    
    /**
     * @brief ゲームループを実行
     */
    void Run() {
        while (!WindowShouldClose()) {
            Update();
            Render();
        }
    }
    
    /**
     * @brief シャットダウン
     */
    void Shutdown() {
        if (isInitialized_) {
            tileRenderer_.Shutdown();
            if (font_.texture.id != GetFontDefault().texture.id) {
                UnloadFont(font_);
            }
            CloseWindow();
            isInitialized_ = false;
        }
    }

private:
    /**
     * @brief 指定階層のダンジョンを生成
     */
    void GenerateFloor(int floor) {
        // 階層に応じてダンジョンサイズを調整
        int width = 40 + floor * 2;
        int height = 25 + floor;
        if (width > 80) width = 80;
        if (height > 40) height = 40;
        
        // ダンジョン生成
        map_ = dungeonGenerator_.Generate(width, height, floor);
        
        // 階段位置を取得
        stairsUpPos_ = dungeonGenerator_.GetStairsUpPosition();
        stairsDownPos_ = dungeonGenerator_.GetStairsDownPosition();
        
        // メッセージログに追加
        AddMessage(TextFormat("地下%d階に到着した。", floor));
    }
    
    /**
     * @brief FOVを更新
     */
    void UpdateFOV() {
        if (playerEntity_ == entt::null) return;
        
        auto* pos = registry_.try_get<Components::GridPosition>(playerEntity_);
        if (pos) {
            Systems::FOVSystem::Calculate(map_, pos->x, pos->y, viewRadius_);
        }
    }
    
    /**
     * @brief メッセージをログに追加
     */
    void AddMessage(const std::string& msg) {
        messageLog_.push_back(msg);
        // 最新10件だけ保持
        while (messageLog_.size() > 10) {
            messageLog_.erase(messageLog_.begin());
        }
    }
    
    /**
     * @brief プレイヤーを生成
     */
    void CreatePlayer() {
        playerEntity_ = registry_.create();
        
        // 上り階段の位置にスポーン（または最初の部屋）
        int startX = stairsUpPos_.first;
        int startY = stairsUpPos_.second;
        
        // 位置
        registry_.emplace<Components::GridPosition>(playerEntity_, startX, startY);
        
        // ターンアクター（プレイヤー）
        Components::TurnActor actor;
        actor.speed = 100;
        actor.energy = 100;  // 最初から行動可能
        actor.isPlayer = true;
        registry_.emplace<Components::TurnActor>(playerEntity_, actor);
        
        // 行動コマンド
        registry_.emplace<Components::ActionCommand>(playerEntity_);
        
        // 見た目
        registry_.emplace<Components::Appearance>(playerEntity_, '@', 255, 255, 255);
        
        // 名前
        registry_.emplace<Components::Name>(playerEntity_, "あなた", "プレイヤーキャラクター");
        
        // プレイヤータグ
        registry_.emplace<Components::PlayerTag>(playerEntity_);
        
        // ヘルス
        Components::Health health;
        health.max = 30;
        health.current = 30;
        registry_.emplace<Components::Health>(playerEntity_, health);
        
        // 戦闘ステータス
        Components::CombatStats stats;
        stats.attack = 5;
        stats.defense = 2;
        stats.accuracy = 85;
        stats.evasion = 15;
        stats.critChance = 10;
        registry_.emplace<Components::CombatStats>(playerEntity_, stats);
        
        // 経験値
        registry_.emplace<Components::Experience>(playerEntity_);
        
        // インベントリ
        registry_.emplace<Components::Inventory>(playerEntity_);
        
        // 装備
        registry_.emplace<Components::Equipment>(playerEntity_);
        
        // 空腹度
        registry_.emplace<Components::Hunger>(playerEntity_);
        
        // マップにoccupantを設定
        if (map_.InBounds(startX, startY)) {
            map_.At(startX, startY).occupant = playerEntity_;
        }
    }
    
    /**
     * @brief モンスターを生成
     */
    void SpawnMonsters() {
        auto* playerPos = registry_.try_get<Components::GridPosition>(playerEntity_);
        int px = playerPos ? playerPos->x : 0;
        int py = playerPos ? playerPos->y : 0;
        
        monsterSpawner_.SpawnMonstersForFloor(
            registry_, map_, currentFloor_, px, py, 
            dungeonGenerator_.GetSeed() + currentFloor_ * 999);
    }
    
    /**
     * @brief 階移動時のクリーンアップ
     */
    void CleanupFloor() {
        monsterSpawner_.ClearMonsters(registry_, map_);
        itemSpawner_.ClearItems(registry_, map_);
    }
    
    /**
     * @brief アイテムを生成
     */
    void SpawnItems() {
        itemSpawner_.SpawnItemsForFloor(
            registry_, map_, currentFloor_,
            dungeonGenerator_.GetSeed() + currentFloor_ * 777);
    }
    
    /**
     * @brief プレイヤーを指定位置に移動（階移動時）
     */
    void MovePlayerTo(int x, int y) {
        if (playerEntity_ == entt::null) return;
        
        auto* pos = registry_.try_get<Components::GridPosition>(playerEntity_);
        if (pos) {
            // 元の位置のoccupantをクリア
            if (map_.InBounds(pos->x, pos->y)) {
                map_.At(pos->x, pos->y).occupant = entt::null;
            }
            
            // 新しい位置に移動
            pos->x = x;
            pos->y = y;
            
            // 新しい位置にoccupantを設定
            if (map_.InBounds(x, y)) {
                map_.At(x, y).occupant = playerEntity_;
            }
        }
    }
    
    /**
     * @brief 階を降りる
     */
    void DescendFloor() {
        if (currentFloor_ >= MAX_FLOOR) {
            AddMessage("これ以上降りられない。ここが最深部だ。");
            return;
        }
        
        // 現在の階のモンスターをクリア
        CleanupFloor();
        
        currentFloor_++;
        GenerateFloor(currentFloor_);
        
        // プレイヤーを上り階段の位置に配置
        MovePlayerTo(stairsUpPos_.first, stairsUpPos_.second);
        
        // 新しい階にモンスターとアイテムを生成
        SpawnMonsters();
        SpawnItems();
        
        UpdateFOV();
        
        AddMessage(TextFormat("階段を降りた。地下%d階。", currentFloor_));
    }
    
    /**
     * @brief 階を昇る
     */
    void AscendFloor() {
        if (currentFloor_ <= 1) {
            AddMessage("地上に出た！冒険は終わりだ。");
            // ゲームクリア処理（今はメッセージのみ）
            return;
        }
        
        // 現在の階のモンスターをクリア
        CleanupFloor();
        
        currentFloor_--;
        GenerateFloor(currentFloor_);
        
        // プレイヤーを下り階段の位置に配置
        MovePlayerTo(stairsDownPos_.first, stairsDownPos_.second);
        
        // 新しい階にモンスターとアイテムを生成
        SpawnMonsters();
        SpawnItems();
        
        UpdateFOV();
        
        AddMessage(TextFormat("階段を昇った。地下%d階。", currentFloor_));
    }
    
public:
    /**
     * @brief ゲーム更新（シーン統合用）
     */
    void Update() {
        // ゲームオーバーチェック
        if (isGameOver_) return;
        
        // 死亡エンティティの処理
        Systems::CombatSystem::ProcessDeaths(registry_, map_, 
            [this](const std::string& msg) { AddMessage(msg); });
        
        // ターンマネージャー更新
        auto state = turnManager_.Update(registry_);
        
        if (state == Managers::TurnManager::State::AwaitingInput) {
            // 画面モード別の入力処理
            switch (gameMode_) {
                case GameMode::ActionMenu:
                    UpdateActionMenu();
                    return;
                    
                case GameMode::Inventory:
                    UpdateInventory();
                    return;
                    
                case GameMode::ItemAction:
                    UpdateItemAction();
                    return;
                    
                case GameMode::Explore:
                default:
                    UpdateExplore();
                    return;
            }
        }
        else if (state == Managers::TurnManager::State::ProcessingTurns) {
            // 行動の実行
            ProcessActions();
        }
    }
    
    /**
     * @brief 探索モードの更新
     */
    void UpdateExplore() {
        // Enter/Spaceで足元メニューを開く
        if (Systems::InputSystem::IsConfirmPressed()) {
            OpenActionMenu();
            return;
        }
        
        // Iでインベントリを開く
        if (Systems::InputSystem::IsInventoryPressed()) {
            gameMode_ = GameMode::Inventory;
            menuSelection_ = 0;
            return;
        }
        
        // 移動入力
        Systems::InputSystem::ProcessInput(registry_);
    }
    
    /**
     * @brief 足元アクションメニューを開く
     */
    void OpenActionMenu() {
        actionMenuOptions_.clear();
        
        auto* pos = registry_.try_get<Components::GridPosition>(playerEntity_);
        if (!pos) return;
        
        // 足元にあるものを確認
        auto& tile = map_.At(pos->x, pos->y);
        
        // 階段チェック
        if (tile.type == Components::TileType::StairsDown) {
            actionMenuOptions_.push_back({"階段を降りる", ActionType::Descend});
        }
        if (tile.type == Components::TileType::StairsUp) {
            actionMenuOptions_.push_back({"階段を昇る", ActionType::Ascend});
        }
        
        // 足元アイテムチェック
        if (tile.item != entt::null) {
            auto* item = registry_.try_get<Components::Item>(tile.item);
            if (item) {
                actionMenuOptions_.push_back({TextFormat("%sを拾う", item->name.c_str()), ActionType::PickUp});
            }
        }
        
        // 待機は常に選択可能
        actionMenuOptions_.push_back({"待機", ActionType::Wait});
        
        if (!actionMenuOptions_.empty()) {
            gameMode_ = GameMode::ActionMenu;
            menuSelection_ = 0;
        }
    }
    
    /**
     * @brief アクションメニューの更新
     */
    void UpdateActionMenu() {
        int result = Systems::InputSystem::ProcessMenuInput(
            static_cast<int>(actionMenuOptions_.size()), menuSelection_);
        
        if (result == -1) {
            // キャンセル
            gameMode_ = GameMode::Explore;
            return;
        }
        
        if (result == 1) {
            // 決定
            ExecuteActionMenuSelection();
            gameMode_ = GameMode::Explore;
        }
    }
    
    /**
     * @brief アクションメニューの選択を実行
     */
    void ExecuteActionMenuSelection() {
        if (menuSelection_ < 0 || menuSelection_ >= static_cast<int>(actionMenuOptions_.size())) return;
        
        auto& option = actionMenuOptions_[menuSelection_];
        auto* cmd = registry_.try_get<Components::ActionCommand>(playerEntity_);
        if (!cmd) return;
        
        switch (option.action) {
            case ActionType::Descend:
                *cmd = Components::ActionCommand::MakeDescend();
                break;
            case ActionType::Ascend:
                *cmd = Components::ActionCommand::MakeAscend();
                break;
            case ActionType::PickUp:
                *cmd = Components::ActionCommand::MakePickUp();
                break;
            case ActionType::Wait:
                *cmd = Components::ActionCommand::MakeWait();
                break;
            default:
                break;
        }
    }
    
    /**
     * @brief インベントリ画面の更新
     */
    void UpdateInventory() {
        auto* inventory = registry_.try_get<Components::Inventory>(playerEntity_);
        if (!inventory) {
            gameMode_ = GameMode::Explore;
            return;
        }
        
        // 所持アイテム数をカウント
        int itemCount = 0;
        for (int i = 0; i < Components::Inventory::MAX_SLOTS; i++) {
            if (inventory->items[i] != entt::null) itemCount++;
        }
        
        if (itemCount == 0) {
            // アイテムがない
            if (Systems::InputSystem::IsCancelPressed() || Systems::InputSystem::IsConfirmPressed()) {
                gameMode_ = GameMode::Explore;
            }
            return;
        }
        
        int result = Systems::InputSystem::ProcessMenuInput(itemCount, menuSelection_);
        
        if (result == -1) {
            // キャンセル
            gameMode_ = GameMode::Explore;
            return;
        }
        
        if (result == 1) {
            // アイテムを選択 -> アイテム操作メニューへ
            selectedItemSlot_ = GetNthItemSlot(menuSelection_);
            if (selectedItemSlot_ >= 0) {
                gameMode_ = GameMode::ItemAction;
                menuSelection_ = 0;
            }
        }
    }
    
    /**
     * @brief n番目のアイテムがあるスロットを取得
     */
    int GetNthItemSlot(int n) {
        auto* inventory = registry_.try_get<Components::Inventory>(playerEntity_);
        if (!inventory) return -1;
        
        int count = 0;
        for (int i = 0; i < Components::Inventory::MAX_SLOTS; i++) {
            if (inventory->items[i] != entt::null) {
                if (count == n) return i;
                count++;
            }
        }
        return -1;
    }
    
    /**
     * @brief アイテム操作メニューの更新
     */
    void UpdateItemAction() {
        // 使う、装備する、捨てる、やめる
        static const int NUM_OPTIONS = 4;
        
        int result = Systems::InputSystem::ProcessMenuInput(NUM_OPTIONS, menuSelection_);
        
        if (result == -1) {
            // キャンセル
            gameMode_ = GameMode::Inventory;
            menuSelection_ = 0;
            return;
        }
        
        if (result == 1) {
            ExecuteItemAction();
        }
    }
    
    /**
     * @brief アイテム操作を実行
     */
    void ExecuteItemAction() {
        auto* cmd = registry_.try_get<Components::ActionCommand>(playerEntity_);
        if (!cmd) return;
        
        switch (menuSelection_) {
            case 0: // 使う
                *cmd = Components::ActionCommand::MakeUse(selectedItemSlot_);
                gameMode_ = GameMode::Explore;
                break;
            case 1: // 装備する/外す
                {
                    auto* inventory = registry_.try_get<Components::Inventory>(playerEntity_);
                    if (inventory && selectedItemSlot_ >= 0) {
                        auto itemEntity = inventory->items[selectedItemSlot_];
                        if (itemEntity != entt::null) {
                            Systems::ItemSystem::EquipItem(registry_, playerEntity_, selectedItemSlot_,
                                [this](const std::string& msg) { AddMessage(msg); });
                        }
                    }
                }
                gameMode_ = GameMode::Explore;
                break;
            case 2: // 捨てる
                *cmd = Components::ActionCommand::MakeDrop(selectedItemSlot_);
                gameMode_ = GameMode::Explore;
                break;
            case 3: // やめる
                gameMode_ = GameMode::Inventory;
                menuSelection_ = 0;
                break;
        }
    }
    
    /**
     * @brief 行動の実行
     */
    void ProcessActions() {
        auto currentActor = turnManager_.GetCurrentActor();
        if (currentActor == entt::null) return;
        
        // モンスターの場合はAIで行動決定
        if (registry_.any_of<Components::MonsterTag>(currentActor)) {
            Systems::AISystem::DecideAction(registry_, map_, currentActor, playerEntity_);
        }
        
        auto* cmd = registry_.try_get<Components::ActionCommand>(currentActor);
        if (!cmd || cmd->type == Components::ActionCommand::Type::None) return;
        
        // 行動タイプを記録（空腹度処理用）
        auto actionType = cmd->type;
        
        switch (cmd->type) {
            case Components::ActionCommand::Type::Descend:
                ProcessDescend(currentActor);
                break;
            case Components::ActionCommand::Type::Ascend:
                ProcessAscend(currentActor);
                break;
            case Components::ActionCommand::Type::Attack:
                ProcessAttack(currentActor, cmd->targetX, cmd->targetY);
                break;
            case Components::ActionCommand::Type::PickUp:
                if (registry_.any_of<Components::PlayerTag>(currentActor)) {
                    Systems::ItemSystem::PickupItem(registry_, map_, currentActor,
                        [this](const std::string& msg) { AddMessage(msg); });
                }
                break;
            case Components::ActionCommand::Type::Use:
                if (registry_.any_of<Components::PlayerTag>(currentActor) && cmd->itemSlot >= 0) {
                    Systems::ItemSystem::UseItem(registry_, currentActor, cmd->itemSlot,
                        [this](const std::string& msg) { AddMessage(msg); });
                }
                break;
            case Components::ActionCommand::Type::Drop:
                if (registry_.any_of<Components::PlayerTag>(currentActor) && cmd->itemSlot >= 0) {
                    Systems::ItemSystem::DropItem(registry_, map_, currentActor, cmd->itemSlot,
                        [this](const std::string& msg) { AddMessage(msg); });
                }
                break;
            case Components::ActionCommand::Type::Move:
                ProcessMove(currentActor, cmd->targetX, cmd->targetY);
                break;
            default:
                Systems::ActionSystem::ExecuteAction(registry_, map_, currentActor);
                break;
        }
        
        // プレイヤーの行動後に空腹度を更新
        if (registry_.any_of<Components::PlayerTag>(currentActor)) {
            Systems::HungerSystem::OnAction(registry_, currentActor, actionType,
                [this](const std::string& msg) { AddMessage(msg); });
            
            // 餓死判定
            auto* hunger = registry_.try_get<Components::Hunger>(currentActor);
            auto* health = registry_.try_get<Components::Health>(currentActor);
            if (hunger && health && hunger->GetState() == Components::HungerState::Starving) {
                if (!health->IsAlive()) {
                    isGameOver_ = true;
                    AddMessage("餓死した...");
                }
            }
        }
        
        cmd->type = Components::ActionCommand::Type::None;
        turnManager_.CompleteAction(registry_);
    }
    
    /**
     * @brief 移動処理
     */
    void ProcessMove(entt::entity actor, int targetX, int targetY) {
        if (map_.InBounds(targetX, targetY)) {
            auto targetEntity = map_.At(targetX, targetY).occupant;
            if (targetEntity != entt::null && targetEntity != actor) {
                // 敵がいるので攻撃
                ProcessAttack(actor, targetX, targetY);
                if (registry_.any_of<Components::PlayerTag>(actor)) {
                    UpdateFOV();
                }
                return;
            }
        }
        
        // 通常の移動
        Systems::ActionSystem::ExecuteAction(registry_, map_, actor);
        
        if (registry_.any_of<Components::PlayerTag>(actor)) {
            UpdateFOV();
        }
    }
    
    /**
     * @brief 階段降下処理
     */
    void ProcessDescend(entt::entity actor) {
        auto* pos = registry_.try_get<Components::GridPosition>(actor);
        if (pos && map_.InBounds(pos->x, pos->y)) {
            if (map_.At(pos->x, pos->y).type == Components::TileType::StairsDown) {
                DescendFloor();
            } else {
                AddMessage("ここには下り階段がない。");
            }
        }
    }
    
    /**
     * @brief 階段上昇処理
     */
    void ProcessAscend(entt::entity actor) {
        auto* pos = registry_.try_get<Components::GridPosition>(actor);
        if (pos && map_.InBounds(pos->x, pos->y)) {
            if (map_.At(pos->x, pos->y).type == Components::TileType::StairsUp) {
                AscendFloor();
            } else {
                AddMessage("ここには上り階段がない。");
            }
        }
    }
    
    /**
     * @brief 攻撃処理
     */
    void ProcessAttack(entt::entity attacker, int targetX, int targetY) {
        if (!map_.InBounds(targetX, targetY)) return;
        
        auto defender = map_.At(targetX, targetY).occupant;
        if (defender == entt::null) return;
        
        // 戦闘実行
        auto result = Systems::CombatSystem::Attack(registry_, attacker, defender);
        AddMessage(result.message);
        
        // プレイヤーがモンスターを倒した場合
        if (result.killed && registry_.any_of<Components::PlayerTag>(attacker)) {
            // TODO: 経験値を与える（モンスターデータから取得）
            Systems::CombatSystem::GiveExperience(registry_, attacker, 10,
                [this](const std::string& msg) { AddMessage(msg); });
        }
        
        // プレイヤーが死亡した場合
        if (result.killed && registry_.any_of<Components::PlayerTag>(defender)) {
            isGameOver_ = true;
            AddMessage("ゲームオーバー...");
        }
    }
    
    /**
     * @brief ゲーム描画
     */
    void Render() {
        BeginDrawing();
        RenderContent();
        EndDrawing();
    }
    
public:
    /**
     * @brief ゲーム描画（BeginDrawing/EndDrawingなし）
     * シーン統合用のメソッド
     */
    void RenderContent() {
        ClearBackground(BLACK);
        
        // プレイヤー位置をカメラ中心に
        auto* playerPos = registry_.try_get<Components::GridPosition>(playerEntity_);
        int cameraX = playerPos ? playerPos->x : 10;
        int cameraY = playerPos ? playerPos->y : 7;
        
        // マップ描画領域（デバッグUI用に下部を確保）
        int mapHeight = SCREEN_HEIGHT - 180;
        
        // マップ描画（FOV反映）
        tileRenderer_.RenderMap(map_, cameraX, cameraY, SCREEN_WIDTH, mapHeight);
        
        // エンティティ描画（視界内のみ）
        auto view = registry_.view<Components::GridPosition, Components::Appearance>();
        for (auto entity : view) {
            auto& pos = view.get<Components::GridPosition>(entity);
            auto& appearance = view.get<Components::Appearance>(entity);
            
            // 視界内のみ描画
            if (!map_.InBounds(pos.x, pos.y)) continue;
            if (!map_.At(pos.x, pos.y).visible) continue;
            
            Vector2 screenPos = tileRenderer_.GridToScreen(
                pos.x, pos.y, cameraX, cameraY, 
                SCREEN_WIDTH, mapHeight);
            
            tileRenderer_.RenderEntity(
                static_cast<int>(screenPos.x), 
                static_cast<int>(screenPos.y),
                appearance.symbol,
                Color{appearance.r, appearance.g, appearance.b, 255});
        }
        
        // UI描画
        RenderUI();
        
        // アクションメニュー描画
        RenderActionMenu();
        
        // インベントリUI描画
        RenderInventoryUI();
        
        // デバッグUI描画（操作説明付き）
        RenderDebugUI();
    }
    
    /**
     * @brief UI描画
     */
    void RenderUI() {
        int uiY = SCREEN_HEIGHT - 180;
        
        // 背景
        DrawRectangle(0, uiY, SCREEN_WIDTH, 80, Color{30, 30, 30, 255});
        DrawLine(0, uiY, SCREEN_WIDTH, uiY, GRAY);
        
        // ステータス表示
        auto* playerPos = registry_.try_get<Components::GridPosition>(playerEntity_);
        auto* playerHealth = registry_.try_get<Components::Health>(playerEntity_);
        auto* playerExp = registry_.try_get<Components::Experience>(playerEntity_);
        
        // HP表示
        if (playerHealth) {
            DrawTextJ(TextFormat("HP: %d/%d", playerHealth->current, playerHealth->max), 
                     10, uiY + 5, 18, playerHealth->GetRatio() > 0.3f ? GREEN : RED);
            
            // HPバー
            int barWidth = 100;
            int barHeight = 8;
            int barX = 120;
            int barY = uiY + 8;
            DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
            DrawRectangle(barX, barY, static_cast<int>(barWidth * playerHealth->GetRatio()), barHeight, 
                         playerHealth->GetRatio() > 0.3f ? GREEN : RED);
        }
        
        // レベル・経験値
        if (playerExp) {
            DrawTextJ(TextFormat("Lv.%d", playerExp->level), 230, uiY + 5, 18, YELLOW);
        }
        
        DrawTextJ(TextFormat("地下 %d 階", currentFloor_), 300, uiY + 5, 18, WHITE);
        DrawTextJ(TextFormat("ターン: %d", turnManager_.GetTurnCount()), 420, uiY + 5, 18, WHITE);
        
        // 状態表示
        const char* stateText = "処理中...";
        Color stateColor = GRAY;
        if (isGameOver_) {
            stateText = "ゲームオーバー";
            stateColor = RED;
        } else if (turnManager_.IsAwaitingInput()) {
            stateText = "あなたのターン";
            stateColor = GREEN;
        }
        DrawTextJ(stateText, 560, uiY + 5, 18, stateColor);
        
        // 現在のタイル情報
        auto* playerPos2 = registry_.try_get<Components::GridPosition>(playerEntity_);
        if (playerPos2 && map_.InBounds(playerPos2->x, playerPos2->y)) {
            auto& tile = map_.At(playerPos2->x, playerPos2->y);
            const char* tileInfo = "";
            switch (tile.type) {
                case Components::TileType::Floor: tileInfo = "[床]"; break;
                case Components::TileType::Corridor: tileInfo = "[通路]"; break;
                case Components::TileType::StairsDown: tileInfo = "[下り階段 >]"; break;
                case Components::TileType::StairsUp: tileInfo = "[上り階段 <]"; break;
                default: tileInfo = ""; break;
            }
            DrawTextJ(tileInfo, 720, uiY + 5, 18, SKYBLUE);
        }
        
        // メッセージログ（直近3件）
        int msgY = uiY + 28;
        int msgCount = 0;
        for (auto it = messageLog_.rbegin(); it != messageLog_.rend() && msgCount < 3; ++it, ++msgCount) {
            Color msgColor = (msgCount == 0) ? WHITE : LIGHTGRAY;
            DrawTextJ(it->c_str(), 10, msgY + msgCount * 16, 14, msgColor);
        }
    }
    
    /**
     * @brief 日本語テキスト描画ヘルパー
     */
    void DrawTextJ(const char* text, int x, int y, int fontSize, Color color) {
        float scale = static_cast<float>(fontSize) / 24.0f;  // フォントサイズ24を基準
        DrawTextEx(font_, text, Vector2{static_cast<float>(x), static_cast<float>(y)}, 
                   static_cast<float>(fontSize), 1.0f, color);
    }
    
    // アクションメニュー用の列挙型と構造体
    enum class ActionType {
        None, Descend, Ascend, PickUp, Wait, Use, Drop, Equip
    };
    
    struct ActionOption {
        std::string label;
        ActionType action;
    };
    
    // メンバー変数
    entt::registry registry_;
    Components::MapData map_;
    Managers::TurnManager turnManager_;
    Rendering::TileRenderer tileRenderer_;
    Generators::DungeonGenerator dungeonGenerator_;
    Font font_{};
    
    entt::entity playerEntity_ = entt::null;
    bool isInitialized_ = false;
    
    int currentFloor_ = 1;                              ///< 現在の階層
    int viewRadius_ = 8;                                ///< 視界半径
    std::pair<int, int> stairsUpPos_{0, 0};            ///< 上り階段位置
    std::pair<int, int> stairsDownPos_{0, 0};          ///< 下り階段位置
    std::vector<std::string> messageLog_;               ///< メッセージログ
    Generators::MonsterSpawner monsterSpawner_;         ///< モンスター生成器
    Generators::ItemSpawner itemSpawner_;               ///< アイテム生成器
    bool isGameOver_ = false;                           ///< ゲームオーバーフラグ
    
    // 新操作システム用メンバー
    GameMode gameMode_ = GameMode::Explore;             ///< 現在の画面モード
    int menuSelection_ = 0;                             ///< メニュー選択位置
    std::vector<ActionOption> actionMenuOptions_;       ///< アクションメニュー選択肢
    int selectedItemSlot_ = -1;                         ///< 選択中のアイテムスロット
    
    /**
     * @brief アクションメニューUIを描画
     */
    void RenderActionMenu() {
        if (gameMode_ != GameMode::ActionMenu) return;
        
        int winX = SCREEN_WIDTH / 2 - 150;
        int winY = SCREEN_HEIGHT / 2 - 100;
        int winW = 300;
        int winH = static_cast<int>(actionMenuOptions_.size()) * 30 + 40;
        
        // ウィンドウ背景
        DrawRectangle(winX, winY, winW, winH, Color{40, 40, 60, 240});
        DrawRectangleLines(winX, winY, winW, winH, WHITE);
        
        // タイトル
        DrawTextJ("どうする？", winX + 20, winY + 8, 18, YELLOW);
        DrawLine(winX + 10, winY + 30, winX + winW - 10, winY + 30, GRAY);
        
        // 選択肢
        for (int i = 0; i < static_cast<int>(actionMenuOptions_.size()); i++) {
            int y = winY + 40 + i * 30;
            Color color = (i == menuSelection_) ? YELLOW : WHITE;
            
            if (i == menuSelection_) {
                DrawRectangle(winX + 10, y - 2, winW - 20, 26, Color{80, 80, 100, 255});
                DrawTextJ("▶", winX + 15, y, 18, YELLOW);
            }
            DrawTextJ(actionMenuOptions_[i].label.c_str(), winX + 40, y, 18, color);
        }
        
        // 操作説明
        DrawTextJ("[↑↓]選択 [Enter]決定 [ESC]戻る", winX + 20, winY + winH - 25, 12, LIGHTGRAY);
    }
    
    /**
     * @brief インベントリUIを描画（カーソル選択式）
     */
    void RenderInventoryUI() {
        if (gameMode_ != GameMode::Inventory && gameMode_ != GameMode::ItemAction) return;
        
        // 半透明背景
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 180});
        
        // インベントリウィンドウ
        int winX = 250;
        int winY = 80;
        int winW = 780;
        int winH = 480;
        
        DrawRectangle(winX, winY, winW, winH, Color{40, 40, 60, 255});
        DrawRectangleLines(winX, winY, winW, winH, WHITE);
        
        // タイトル
        DrawTextJ("持ち物", winX + 20, winY + 10, 22, YELLOW);
        DrawLine(winX + 10, winY + 38, winX + winW - 10, winY + 38, GRAY);
        
        // プレイヤーのインベントリを取得
        auto* inventory = registry_.try_get<Components::Inventory>(playerEntity_);
        auto* equipment = registry_.try_get<Components::Equipment>(playerEntity_);
        if (!inventory) return;
        
        // ゴールド表示
        DrawTextJ(TextFormat("所持金: %d G", inventory->gold), winX + winW - 180, winY + 10, 18, GOLD);
        
        // アイテムリスト（カーソル選択式）
        int itemY = winY + 50;
        int itemCount = 0;
        
        for (int i = 0; i < Components::Inventory::MAX_SLOTS; i++) {
            if (inventory->items[i] != entt::null) {
                auto* item = registry_.try_get<Components::Item>(inventory->items[i]);
                if (item) {
                    int y = itemY + itemCount * 32;
                    bool isSelected = (itemCount == menuSelection_);
                    
                    // 選択中のハイライト
                    if (isSelected) {
                        DrawRectangle(winX + 10, y - 2, winW - 20, 28, Color{80, 80, 120, 255});
                        DrawTextJ("▶", winX + 15, y, 18, YELLOW);
                    }
                    
                    // 装備中チェック
                    bool equipped = false;
                    if (equipment) {
                        auto* equippable = registry_.try_get<Components::Equippable>(inventory->items[i]);
                        if (equippable) {
                            equipped = (equipment->GetSlot(equippable->slot) == inventory->items[i]);
                        }
                    }
                    
                    // アイテムシンボル
                    Color itemColor = Color{item->r, item->g, item->b, 255};
                    DrawTextJ(TextFormat("%c", item->symbol), winX + 45, y, 20, itemColor);
                    
                    // アイテム名
                    std::string itemName = item->name;
                    if (equipped) {
                        itemName += " [E]";
                    }
                    if (item->quantity > 1) {
                        itemName += " x" + std::to_string(item->quantity);
                    }
                    Color textColor = isSelected ? YELLOW : (equipped ? GREEN : WHITE);
                    DrawTextJ(itemName.c_str(), winX + 80, y, 18, textColor);
                    
                    itemCount++;
                }
            }
        }
        
        if (itemCount == 0) {
            DrawTextJ("持ち物がありません", winX + 40, itemY, 18, GRAY);
        }
        
        // アイテム操作サブメニュー
        if (gameMode_ == GameMode::ItemAction) {
            RenderItemActionMenu();
        }
        
        // 操作説明
        DrawTextJ("[↑↓]選択 [Enter]決定 [ESC]戻る", winX + 20, winY + winH - 28, 14, LIGHTGRAY);
    }
    
    /**
     * @brief アイテム操作メニューを描画
     */
    void RenderItemActionMenu() {
        int winX = SCREEN_WIDTH / 2 + 100;
        int winY = SCREEN_HEIGHT / 2 - 80;
        int winW = 200;
        int winH = 160;
        
        DrawRectangle(winX, winY, winW, winH, Color{50, 50, 70, 250});
        DrawRectangleLines(winX, winY, winW, winH, WHITE);
        
        DrawTextJ("どうする？", winX + 20, winY + 10, 16, YELLOW);
        DrawLine(winX + 10, winY + 32, winX + winW - 10, winY + 32, GRAY);
        
        static const char* options[] = {"使う", "装備/外す", "捨てる", "やめる"};
        
        for (int i = 0; i < 4; i++) {
            int y = winY + 42 + i * 28;
            bool isSelected = (i == menuSelection_);
            
            if (isSelected) {
                DrawRectangle(winX + 10, y - 2, winW - 20, 24, Color{80, 80, 120, 255});
                DrawTextJ("▶", winX + 15, y, 16, YELLOW);
            }
            
            Color color = isSelected ? YELLOW : WHITE;
            DrawTextJ(options[i], winX + 40, y, 16, color);
        }
    }
    
    /**
     * @brief デバッグUI描画（操作説明を含む）
     * 
     * 初心者向けに操作方法を画面下部に表示。
     */
    void RenderDebugUI() {
        int debugY = SCREEN_HEIGHT - 100;
        
        // デバッグパネル背景
        DrawRectangle(0, debugY, SCREEN_WIDTH, 100, Color{20, 25, 30, 255});
        DrawLine(0, debugY, SCREEN_WIDTH, debugY, Color{60, 80, 100, 255});
        
        // タイトル
        DrawTextJ("=== 操作ガイド ===", 10, debugY + 5, 16, Color{100, 150, 200, 255});
        
        // 左側: 基本操作
        int col1X = 10;
        DrawTextJ("[基本操作]", col1X, debugY + 25, 14, YELLOW);
        DrawTextJ("矢印キー : 移動", col1X, debugY + 42, 12, LIGHTGRAY);
        DrawTextJ("Enter/Space : 調べる・決定", col1X, debugY + 56, 12, LIGHTGRAY);
        DrawTextJ("ESC : キャンセル・戻る", col1X, debugY + 70, 12, LIGHTGRAY);
        
        // 中央左: その他操作
        int col2X = 280;
        DrawTextJ("[メニュー]", col2X, debugY + 25, 14, YELLOW);
        DrawTextJ("I : 持ち物を開く", col2X, debugY + 42, 12, LIGHTGRAY);
        DrawTextJ("W : 待機（ターンを消費）", col2X, debugY + 56, 12, LIGHTGRAY);
        DrawTextJ("※敵に隣接すると自動で攻撃", col2X, debugY + 70, 12, ORANGE);
        
        // 中央右: ヒント
        int col3X = 560;
        DrawTextJ("[ヒント]", col3X, debugY + 25, 14, YELLOW);
        DrawTextJ("階段の上でEnterを押すと", col3X, debugY + 42, 12, LIGHTGRAY);
        DrawTextJ("「降りる」「昇る」を選べます", col3X, debugY + 56, 12, LIGHTGRAY);
        DrawTextJ("アイテムの上でEnterで拾えます", col3X, debugY + 70, 12, LIGHTGRAY);
        
        // 右側: 状態・目標
        int col4X = 880;
        DrawTextJ("[現在の状態]", col4X, debugY + 25, 14, GREEN);
        DrawTextJ(TextFormat("地下 %d 階 / 目標 %d 階", currentFloor_, MAX_FLOOR), col4X, debugY + 42, 12, WHITE);
        DrawTextJ(TextFormat("マップ: %dx%d", map_.width, map_.height), col4X, debugY + 56, 12, GRAY);
        DrawTextJ("下り階段 > を探そう！", col4X, debugY + 70, 12, SKYBLUE);
        
        // FPS表示
        DrawTextJ(TextFormat("FPS: %d", GetFPS()), SCREEN_WIDTH - 80, debugY + 5, 14, LIME);
    }
};

} // namespace Roguelike

