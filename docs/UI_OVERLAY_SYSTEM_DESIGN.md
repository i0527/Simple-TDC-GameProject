# Cat Tower Defense - UI・オーバーレイシステム最終設計仕様（P0〜P2反映版）

**最終更新**: 2026-01-07  
**対象**: オーバーレイシステム + UIイベント構造 + UIコンポーネント  
**優先度**: 迅速開発 × 将来柔軟性 × シンプル

---

## 1. 変更サマリー

この版では、以前の「UI・ステートシステム改善設計仕様」に以下を反映しています。

- P0:  
  - SharedContext から GameSystem* を削除（循環参照リスク除去）
  - OverlayManager が「遷移リクエスト」を内部に保持し、GameSystem がそれをポーリングする形に変更
- P1:  
  - UI イベントの構造化（`UIEvent` / `UIEventResult`）  
  - OverlayManager がオーバーレイ背景（半透明黒）を一括描画
- P2:  
  - IUIComponent に `OnShow()` / `OnHide()` を追加（ライフサイクル明確化）
  - UI コンポーネント ID 管理の指針を記載（実装はシンプル案）

---

## 2. ステート / コンテキスト定義（P0 反映）

### 2.1 GameState.hpp

```cpp
// game/core/config/GameState.hpp
#pragma once

namespace game {
namespace core {

enum class GameState {
    Initializing,  // リソース初期化中
    Title,         // タイトル画面
    Home,          // ホーム画面
    Game,          // ゲーム画面
};

enum class OverlayState {
    None = 0,
    StageSelect = 1,
    Formation = 2,
    Enhancement = 3,
    Codex = 4,
    Settings = 5,
    Gacha = 6,
};

} // namespace core
} // namespace game
```

### 2.2 SharedContext.hpp（GameSystem* を削除）

```cpp
// game/core/config/SharedContext.hpp
#pragma once

#include "GameState.hpp"

namespace game {
namespace core {

// 前方宣言
class BaseSystemAPI;
class GameModuleAPI;

struct SharedContext {
    BaseSystemAPI* systemAPI = nullptr;
    GameModuleAPI* gameAPI = nullptr;

    float deltaTime = 0.0f;
    bool isPaused = false;
    bool requestShutdown = false;
};

} // namespace core
} // namespace game
```

## 3. UI イベントモデル（P1: 構造化）

UI コンポーネント層で入力座標を受け取り、
「何が起きたか」を UIEventResult として返す形に統一します。

### 3.1 UIEvent / UIEventResult

```cpp
// game/core/ui/UIEvent.hpp
#pragma once

#include <string>

namespace game {
namespace core {
namespace ui {

enum class UIEventType {
    None,
    Click,
    Hover,
    Key,
};

struct UIEvent {
    UIEventType type = UIEventType::None;
    float x = 0.0f;   // Click / Hover 時の座標（内部座標系）
    float y = 0.0f;
    int key = 0;      // Key イベント時
};

struct UIEventResult {
    bool handled = false;         // このコンポーネント（または子）がイベントを処理したか
    std::string componentId;      // イベント発生元の UI コンポーネント ID
    std::string actionId;         // ビジネスロジック側で解釈するためのアクション ID（例: "start_battle"）
};

} // namespace ui
} // namespace core
} // namespace game
```

## 4. IUIComponent インターフェース（P1 / P2 反映）

### 4.1 IUIComponent.hpp

```cpp
// game/core/ui/IUIComponent.hpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "UIEvent.hpp"

namespace game {
namespace core {
namespace ui {

struct Rect {
    float x, y, width, height;
};

struct Margin {
    float top, right, bottom, left;
};

enum class UIComponentType {
    Card,
    List,
    Tile,
    Panel,
    Button,
    Text,
    Image,
};

class IUIComponent {
public:
    virtual ~IUIComponent() = default;

    // ライフサイクル（1回きり）
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // 表示制御用フック（複数回呼ばれる可能性あり）（P2）
    // Overlay が Push された瞬間 / Pop された瞬間などに呼ぶ想定
    virtual void OnShow() {}
    virtual void OnHide() {}

    // 毎フレーム処理
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;  // ImGui / 将来レンダラー経由

    // レイアウト
    virtual void SetPosition(float x, float y) = 0;
    virtual void SetSize(float width, float height) = 0;
    virtual Rect GetBounds() const = 0;
    virtual void SetMargin(const Margin& margin) = 0;

    // 表示制御
    virtual void SetVisible(bool visible) = 0;
    virtual bool IsVisible() const = 0;
    virtual void SetEnabled(bool enabled) = 0;
    virtual bool IsEnabled() const = 0;

    // イベント処理（P1: 構造化）
    // 呼び出し側（Overlay）が UIEvent を投げて、UIEventResult で結果を受け取る。
    // 子要素を持つコンポーネント（Panel 等）は内部で子へ伝播する。
    virtual UIEventResult HandleEvent(const UIEvent& ev) = 0;

    // 子要素管理
    virtual void AddChild(std::shared_ptr<IUIComponent> child) = 0;
    virtual void RemoveChild(std::shared_ptr<IUIComponent> child) = 0;

    // その他
    virtual UIComponentType GetType() const = 0;
    virtual const std::string& GetId() const = 0;
    virtual void SetId(const std::string& id) = 0;
};

} // namespace ui
} // namespace core
} // namespace game
```

旧設計の OnMouseClick / OnMouseHover / OnKey は
HandleEvent(const UIEvent&) 1本に統合されたイメージです。

## 5. OverlayManager と遷移リクエスト（P0 / P1）

### 5.1 IOverlay（インターフェース）

```cpp
// game/core/states/overlays/IOverlay.hpp
#pragma once

#include "core/config/SharedContext.hpp"
#include "core/config/GameState.hpp"
#include "core/api/BaseSystemAPI.hpp"

namespace game {
namespace core {

class IOverlay {
public:
    virtual ~IOverlay() = default;

    virtual bool Initialize(BaseSystemAPI* systemAPI) = 0;
    virtual void Update(SharedContext& ctx, float deltaTime) = 0;
    virtual void Render(SharedContext& ctx) = 0;
    virtual void Shutdown() = 0;

    virtual OverlayState GetState() const = 0;

    // オーバーレイ自体を閉じたいとき（戻るボタンなど）
    virtual bool RequestClose() const = 0;

    // オーバーレイから独立シーンへの遷移要求
    // true を返したら nextState に遷移先を設定している前提
    virtual bool RequestTransition(GameState& nextState) const = 0;
};

} // namespace core
} // namespace game
```

### 5.2 OverlayManager.hpp（P0: 遷移リクエスト保持）

```cpp
// game/core/system/OverlayManager.hpp
#pragma once

#include "core/config/SharedContext.hpp"
#include "core/states/overlays/IOverlay.hpp"
#include <memory>
#include <vector>

namespace game {
namespace core {

class OverlayManager {
public:
    OverlayManager();
    ~OverlayManager();

    // スタック操作
    bool PushOverlay(OverlayState state, BaseSystemAPI* systemAPI);
    void PopOverlay();
    void PopAllOverlays();

    // ライフサイクル
    void Update(SharedContext& ctx, float deltaTime);
    void Render(SharedContext& ctx);
    void Shutdown();

    // 状態確認
    bool IsEmpty() const;
    size_t Count() const;
    IOverlay* GetTopOverlay() const;
    bool IsOverlayActive(OverlayState state) const;

    // P0: 遷移リクエスト
    bool HasTransitionRequest() const { return hasTransitionRequest_; }
    GameState GetRequestedTransition() const { return requestedTransition_; }
    void ClearTransitionRequest() { hasTransitionRequest_ = false; }

private:
    std::vector<std::unique_ptr<IOverlay>> stack_;
    std::unique_ptr<IOverlay> CreateOverlay(OverlayState state, BaseSystemAPI* systemAPI);

    // P0: オーバーレイからの遷移要求をバッファ
    GameState requestedTransition_ = GameState::Initializing;
    bool hasTransitionRequest_ = false;
};

} // namespace core
} // namespace game
```

### 5.3 OverlayManager.cpp（P0/P1: 遷移リクエスト + 背景描画）

```cpp
// game/core/system/OverlayManager.cpp
#include "OverlayManager.hpp"
#include "utils/Log.h"

namespace game {
namespace core {

OverlayManager::OverlayManager() = default;

OverlayManager::~OverlayManager() {
    Shutdown();
}

bool OverlayManager::PushOverlay(OverlayState state, BaseSystemAPI* systemAPI) {
    auto overlay = CreateOverlay(state, systemAPI);
    if (!overlay) {
        LOG_ERROR("Failed to create overlay for state: {}", static_cast<int>(state));
        return false;
    }

    if (!overlay->Initialize(systemAPI)) {
        LOG_ERROR("Failed to initialize overlay");
        return false;
    }

    // 表示開始フック（必要なら IOverlay 側に OnShow 追加も可）
    stack_.push_back(std::move(overlay));
    LOG_INFO("Overlay pushed. Stack size: {}", stack_.size());
    return true;
}

void OverlayManager::PopOverlay() {
    if (stack_.empty()) {
        LOG_WARN("PopOverlay called on empty stack");
        return;
    }

    auto& top = stack_.back();
    top->Shutdown();
    stack_.pop_back();
    LOG_INFO("Overlay popped. Stack size: {}", stack_.size());
}

void OverlayManager::PopAllOverlays() {
    while (!stack_.empty()) {
        PopOverlay();
    }
}

void OverlayManager::Update(SharedContext& ctx, float deltaTime) {
    if (stack_.empty()) return;

    auto& top = stack_.back();
    top->Update(ctx, deltaTime);

    // Close 要求
    if (top->RequestClose()) {
        PopOverlay();
        return;
    }

    // 遷移要求（P0）
    GameState nextState;
    if (top->RequestTransition(nextState)) {
        requestedTransition_ = nextState;
        hasTransitionRequest_ = true;
    }
}

void OverlayManager::Render(SharedContext& ctx) {
    if (stack_.empty()) return;

    // P1: 背景半透明をここで一括描画
    // 背景シーンを見せたまま、全オーバーレイを重ねる想定
    // 必要に応じて alpha や条件は調整
    ctx.systemAPI->DrawRectangle(
        0, 0,
        ctx.systemAPI->GetInternalWidth(),
        ctx.systemAPI->GetInternalHeight(),
        Color{0, 0, 0, 100}
    );

    // 下から順に描画（奥 → 手前）
    for (auto& overlay : stack_) {
        overlay->Render(ctx);
    }
}

void OverlayManager::Shutdown() {
    PopAllOverlays();
}

bool OverlayManager::IsEmpty() const {
    return stack_.empty();
}

size_t OverlayManager::Count() const {
    return stack_.size();
}

IOverlay* OverlayManager::GetTopOverlay() const {
    if (stack_.empty()) return nullptr;
    return stack_.back().get();
}

bool OverlayManager::IsOverlayActive(OverlayState state) const {
    for (auto& o : stack_) {
        if (o && o->GetState() == state) return true;
    }
    return false;
}

} // namespace core
} // namespace game
```

背景半透明の描画タイミングは好みで調整してください。
（「1枚でもオーバーレイがあれば暗くする」か、「最上位だけ暗くする」かなど）

## 6. GameSystem 側の統合ポイント（P0）

GameSystem のメインループで OverlayManager の遷移リクエストを処理します。

### 6.1 GameSystem::Run() での処理フロー（擬似コード）

```cpp
int GameSystem::Run() {
    while (!systemAPI_->WindowShouldClose() && !requestShutdown_) {
        float dt = systemAPI_->GetFrameTime();
        sharedContext_.deltaTime = dt;

        systemAPI_->UpdateInput();

        // 1. 独立シーンの Update
        switch (currentState_) {
        case GameState::Initializing:
            // ResourceInitializer 更新...
            break;
        case GameState::Title:
            if (titleScreen_) titleScreen_->Update(dt);
            break;
        case GameState::Home:
            if (homeScreen_) homeScreen_->Update(dt);
            break;
        case GameState::Game:
            if (gameScreen_) gameScreen_->Update(dt);
            break;
        }

        // 2. モジュールの Update（必要なら）
        moduleSystem_->Update(sharedContext_, dt);

        // 3. オーバーレイの Update
        overlayManager_->Update(sharedContext_, dt);

        // 4. オーバーレイによる遷移リクエスト処理（P0）
        if (overlayManager_->HasTransitionRequest()) {
            GameState next = overlayManager_->GetRequestedTransition();
            overlayManager_->PopAllOverlays();
            transitionTo(next);
            overlayManager_->ClearTransitionRequest();
        }

        // 5. 描画
        systemAPI_->BeginRender();

        // 背景シーン描画
        switch (currentState_) {
        case GameState::Initializing:
            // ResourceInitializer 描画...
            break;
        case GameState::Title:
            if (titleScreen_) titleScreen_->Render();
            break;
        case GameState::Home:
            if (homeScreen_) homeScreen_->Render();
            break;
        case GameState::Game:
            if (gameScreen_) gameScreen_->Render();
            break;
        }

        // モジュール描画
        moduleSystem_->Render(sharedContext_);

        // オーバーレイ描画（P1: 背景半透明含む）
        overlayManager_->Render(sharedContext_);

        systemAPI_->EndRender();
        systemAPI_->EndFrame();
    }
    return 0;
}
```

## 7. UI コンポーネント具体例（イベント統一後）

### 7.1 Button コンポーネントの HandleEvent 実装イメージ

```cpp
UIEventResult Button::HandleEvent(const UIEvent& ev) {
    UIEventResult result;

    if (!visible_ || !enabled_) return result;

    if (ev.type == UIEventType::Click) {
        const Rect r = GetBounds();
        const bool inside =
            ev.x >= r.x && ev.x <= r.x + r.width &&
            ev.y >= r.y && ev.y <= r.y + r.height;

        if (inside) {
            result.handled = true;
            result.componentId = id_;
            result.actionId = actionId_;  // ボタン生成時に設定しておく
            if (onClick_) onClick_();     // 即時呼び出しも可
        }
    }

    // 子要素を持つならここで子へ伝播

    return result;
}
```

### 7.2 Overlay 側でのイベント処理

```cpp
void StageSelectOverlay::Update(SharedContext& ctx, float dt) {
    // UI コンポーネントは通常通り更新
    rootPanel_->Update(dt);

    // 入力 → UIEvent に変換
    auto mousePos = ctx.systemAPI->GetMousePosition();

    if (ctx.systemAPI->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ui::UIEvent ev;
        ev.type = ui::UIEventType::Click;
        ev.x = mousePos.x;
        ev.y = mousePos.y;

        ui::UIEventResult res = rootPanel_->HandleEvent(ev);

        if (res.handled) {
            HandleUIAction(res.actionId);
        }
    }
}

void StageSelectOverlay::HandleUIAction(const std::string& actionId) {
    if (actionId == "start_battle") {
        // 遷移フラグを立てる
        requestTransition_ = true;
        requestedNextState_ = GameState::Game;
    } else if (actionId == "close_overlay") {
        requestClose_ = true;
    }
    // ほかのアクションもここで処理
}
```

## 8. ID 管理（P2: シンプル版）

現時点では「ID 重複時はログで警告を出す」程度の軽量実装を推奨します。
（本格的な ID レジストリは必要になった時点で導入）

指針だけ記載：

- 各 UI コンポーネントは SetId("StageList") のような 論理名 を持つ
- 画面ごとに ID が被らないよう命名規約を決める
- 例: "StageSelect.StageList", "StageSelect.StartButton" など

## 9. 実装順の提案（この版に合わせて）

1. GameState.hpp / SharedContext.hpp を P0 設計に更新
2. OverlayManager（ヘッダ + cpp）を P0 / P1 仕様で実装
3. GameSystem::Run() を P0 統合仕様に更新
4. UIEvent.hpp と IUIComponent.hpp を P1 / P2 仕様で追加・更新
5. Button だけ先にイベント対応で実装 → Overlay から試す
6. 問題なければ Card / List / Tile / Panel へ同じパターンを横展開

## 10. まとめ

- P0 の修正により、所有関係と遷移制御がかなりクリーンになります。
- P1 のイベント統一で、UI レイヤとゲームロジックの分離がしやすくなります。
- P2 の OnShow/OnHide は後からでも入れられますが、今のタイミングで入れておいた方がライフサイクル設計がブレません。

この仕様で問題なければ、次のステップとして：

- この MD を UI_OVERLAY_SYSTEM_DESIGN.md として保存
- それをベースに 具体的なヘッダ/CPP の骨組みコード を生成していきましょう。

---

**関連ドキュメント**: [UI_OVERLAY_SYSTEM.md](./UI_OVERLAY_SYSTEM.md) - 実装ガイド
