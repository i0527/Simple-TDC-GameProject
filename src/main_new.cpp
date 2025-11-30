/**
 * @file main_new.cpp
 * @brief 新アーキテクチャのテストエントリポイント
 * 
 * Phase 3: UI/UX改善
 * - GameUIクラスによるUI描画
 * - クリック対応デッキスロット
 * - HPバー表示
 * - ゲージ表示
 * 
 * Phase 4A: FHD固定レンダリング
 * - すべての座標を1920x1080で統一
 * - ウィンドウサイズに依存しない描画
 */

// Platform.hを最初にインクルード（Windows API競合回避）
#include "Core/Platform.h"

#include "GameNew.h"
#include "Core/EntityFactory.h"
#include "Core/Definitions.h"
#include "Core/GameRenderer.h"
#include "TD/Components/TDComponents.h"
#include "TD/Managers/WaveManager.h"
#include "TD/Managers/SpawnManager.h"
#include "TD/Managers/GameStateManager.h"
#include "TD/UI/GameUI.h"

#include <iostream>
#include <iomanip>
#include <memory>

// グローバルUI（ラムダ内からアクセス用）
static std::unique_ptr<TD::UI::GameUI> g_gameUI;

int main() {
    GameNew game;
    
    // 設定（ウィンドウサイズは自由、内部は1920x1080固定）
    game.SetWindowSize(1280, 720);
    game.SetWindowTitle("Simple TD - Phase 4A FHD Rendering");
    game.SetTargetFPS(60);
    game.SetDefinitionsPath("assets/definitions");
    
    // 初期化コールバック
    game.OnInit([](GameNew& g) {
        std::cout << "=== Phase 4A: FHD Fixed Rendering ===" << std::endl;
        std::cout << "Internal render size: " << g.RenderWidth() << "x" << g.RenderHeight() << std::endl;
        
        // UI初期化（FHD固定）
        g_gameUI = std::make_unique<TD::UI::GameUI>();
        g_gameUI->Initialize();
        
        // 定義の確認
        auto& defs = g.Definitions();
        std::cout << defs.GetStats() << std::endl;
        
        // テストステージの選択
        std::string stageId = "test_stage";
        
        // ステージ読み込み
        const Data::StageDef* stageDef = defs.TryGetStage(stageId);
        if (!stageDef) {
            stageDef = defs.TryGetStage("fallback_test");
            if (stageDef) {
                stageId = "fallback_test";
                std::cout << "Using fallback_test stage\n";
            }
        }
        
        if (!stageDef) {
            std::cerr << "ERROR: Failed to load stage" << std::endl;
            auto stageIds = defs.GetAllStageIds();
            for (const auto& id : stageIds) {
                std::cerr << "  Available: " << id << std::endl;
            }
            return;
        }
        
        std::cout << "Loaded stage: " << stageDef->name << std::endl;
        std::cout << "  Waves: " << stageDef->waves.size() << std::endl;
        std::cout << "  Lanes: " << stageDef->laneCount << std::endl;
        
        // WaveManagerにステージをロード
        auto& waveManager = g.Waves();
        waveManager.LoadStage(*stageDef);
        
        // SpawnManagerを初期化
        auto& spawnManager = g.Spawns();
        spawnManager.Initialize(
            stageDef->startingCost,
            stageDef->costRegenRate,
            stageDef->maxCost
        );
        
        // デッキをセット
        auto characterIds = defs.GetAllCharacterIds();
        if (!characterIds.empty()) {
            std::vector<std::string> deck;
            for (size_t i = 0; i < std::min(characterIds.size(), size_t(5)); ++i) {
                deck.push_back(characterIds[i]);
            }
            spawnManager.SetDeck(deck);
            
            std::cout << "Deck: ";
            for (const auto& id : deck) {
                std::cout << id << " ";
            }
            std::cout << std::endl;
        }
        
        // スロットクリックコールバック設定
        g_gameUI->SetSlotClickCallback([&g](int slotIndex) {
            auto& gameState = g.GameState();
            auto& spawnManager = g.Spawns();
            auto& waveManager = g.Waves();
            
            if (!gameState.IsPlaying()) return;
            
            // FHD座標でマウス位置を取得
            Vector2 mouseWorld = g.GetMouseWorldPosition();
            int laneCount = waveManager.GetLaneCount();
            float renderHeight = static_cast<float>(g.RenderHeight());
            
            int lane = 0;
            if (laneCount > 1) {
                float centerY = renderHeight / 2.0f;
                float laneAreaHeight = laneCount * waveManager.GetLaneHeight();
                float laneTop = centerY - laneAreaHeight / 2.0f;
                lane = static_cast<int>((mouseWorld.y - laneTop) / waveManager.GetLaneHeight());
                lane = std::clamp(lane, 0, laneCount - 1);
            }
            
            float laneY = waveManager.GetLaneY(lane, renderHeight);
            
            auto entity = spawnManager.SpawnUnit(
                slotIndex, lane, laneY, g.World(), g.Context()
            );
            
            if (entity != entt::null) {
                g_gameUI->SetSelectedSlot(slotIndex);
            }
        });
        
        // GameStateManagerを設定
        auto& gameState = g.GameState();
        gameState.SetBaseHealth(stageDef->baseHealth, stageDef->enemyBaseHealth);
        gameState.StartGame(g.World(), waveManager);
        
        std::cout << "=== Game Started ===" << std::endl;
    });
    
    // 更新コールバック
    game.OnUpdate([](GameNew& g, float dt) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            g.Quit();
        }
        
        auto& gameState = g.GameState();
        auto& spawnManager = g.Spawns();
        auto& waveManager = g.Waves();
        
        // UI入力処理（FHD座標のマウス位置を渡す）
        if (g_gameUI) {
            Vector2 mouseWorld = g.GetMouseWorldPosition();
            g_gameUI->HandleInput(mouseWorld);
        }
        
        // ゲームオーバー時のリスタート
        if (gameState.IsGameOver()) {
            if (IsKeyPressed(KEY_R)) {
                auto& defs = g.Definitions();
                const Data::StageDef* stageDef = defs.TryGetStage("test_stage");
                if (!stageDef) stageDef = defs.TryGetStage("fallback_test");
                
                if (stageDef) {
                    // エンティティクリア
                    auto view = g.World().View<TD::Components::Unit>();
                    for (auto entity : view) {
                        g.World().MarkForDestruction(entity);
                    }
                    g.World().FlushDestruction();
                    
                    // 再初期化
                    waveManager.LoadStage(*stageDef);
                    spawnManager.Initialize(
                        stageDef->startingCost,
                        stageDef->costRegenRate,
                        stageDef->maxCost
                    );
                    gameState.SetBaseHealth(stageDef->baseHealth, stageDef->enemyBaseHealth);
                    gameState.StartGame(g.World(), waveManager);
                    
                    std::cout << "=== Game Restarted ===" << std::endl;
                }
            }
            return;
        }
        
        // プレイ中の入力処理
        if (gameState.IsPlaying()) {
            // 数字キーでユニット召喚
            int slotIndex = -1;
            if (IsKeyPressed(KEY_ONE)) slotIndex = 0;
            else if (IsKeyPressed(KEY_TWO)) slotIndex = 1;
            else if (IsKeyPressed(KEY_THREE)) slotIndex = 2;
            else if (IsKeyPressed(KEY_FOUR)) slotIndex = 3;
            else if (IsKeyPressed(KEY_FIVE)) slotIndex = 4;
            
            if (slotIndex >= 0) {
                // FHD座標でマウス位置を取得
                Vector2 mouseWorld = g.GetMouseWorldPosition();
                int laneCount = waveManager.GetLaneCount();
                float renderHeight = static_cast<float>(g.RenderHeight());
                
                int lane = 0;
                if (laneCount > 1) {
                    float centerY = renderHeight / 2.0f;
                    float laneAreaHeight = laneCount * waveManager.GetLaneHeight();
                    float laneTop = centerY - laneAreaHeight / 2.0f;
                    lane = static_cast<int>((mouseWorld.y - laneTop) / waveManager.GetLaneHeight());
                    lane = std::clamp(lane, 0, laneCount - 1);
                }
                
                float laneY = waveManager.GetLaneY(lane, renderHeight);
                
                auto entity = spawnManager.SpawnUnit(
                    slotIndex, lane, laneY, g.World(), g.Context()
                );
                
                if (entity != entt::null) {
                    g_gameUI->SetSelectedSlot(slotIndex);
                }
            }
            
            // Pキーで一時停止
            if (IsKeyPressed(KEY_P)) {
                gameState.PauseGame();
            }
        } else if (gameState.GetPhase() == TD::GamePhase::Paused) {
            if (IsKeyPressed(KEY_P)) {
                gameState.ResumeGame();
            }
        }
    });
    
    // 描画コールバック
    game.OnRender([](GameNew& g) {
        if (!g_gameUI) return;
        
        auto& gameState = g.GameState();
        auto& spawnManager = g.Spawns();
        auto& waveManager = g.Waves();
        
        // レーン背景
        g_gameUI->DrawLaneBackgrounds(waveManager);
        
        // 拠点
        g_gameUI->DrawBases(gameState);
        
        // ユニットHPバー
        g_gameUI->DrawUnitHealthBars(g.World());
        
        // 上部UI
        g_gameUI->DrawTopBar(waveManager, spawnManager, gameState);
        
        // デッキスロット
        g_gameUI->DrawDeckSlots(spawnManager, g.Context());
        
        // 操作ヘルプ
        g_gameUI->DrawControlsHelp();
        
        // ゲーム状態オーバーレイ
        g_gameUI->DrawGameStateOverlay(gameState);
    });
    
    // 終了コールバック
    game.OnShutdown([](GameNew& g) {
        std::cout << "=== Game Shutdown ===" << std::endl;
        std::cout << "Final entity count: " << g.World().EntityCount() << std::endl;
        g_gameUI.reset();
    });
    
    // ゲーム開始
    game.Run();
    
    return 0;
}
