# Cat Tower Defense - UI・オーバーレイシステム実装ガイド

**最終更新**: 2026-01-07  
**実装状況**: ✅ フェーズ1-3完了 + P0/P1/P2仕様反映完了 + LicenseOverlay実装完了

> **設計仕様**: 詳細な設計仕様については [UI_OVERLAY_SYSTEM_DESIGN.md](./UI_OVERLAY_SYSTEM_DESIGN.md) を参照してください。
- ✅ 基盤システム（OverlayManager、遷移リクエスト機能）
- ✅ UIコンポーネント層（Card, List, Tile, Panel, Button）
- ✅ 構造化イベントシステム（UIEvent / UIEventResult）
- ✅ ライフサイクルフック（OnShow / OnHide）
- ✅ 6つのオーバーレイ実装（StageSelect, Formation, Enhancement, Codex, Settings, Gacha）

---

## 📋 目次

1. [概要](#概要)
2. [オーバーレイシステム](#オーバーレイシステム)
3. [UIコンポーネント層](#uiコンポーネント層)
4. [使用例](#使用例)
5. [実装の詳細](#実装の詳細)
6. [構造化イベントシステム](#構造化イベントシステムp1仕様)

---

## 🎯 概要

UI・オーバーレイシステムは、ゲーム内のUI要素とオーバーレイ（モーダル画面）を統一的に管理するためのシステムです。

**実装済み機能:**
- ✅ オーバーレイスタック管理（LIFO）
- ✅ UIコンポーネント層（Card, List, Tile, Panel, Button）
- ✅ 構造化イベントシステム（UIEvent / UIEventResult、HandleEvent()）
- ✅ ライフサイクルフック（OnShow() / OnHide()）
- ✅ 7つのオーバーレイ実装（StageSelect, Formation, Enhancement, Codex, Settings, Gacha, License）
- ✅ オーバーレイからのステート遷移（P0: 遷移リクエストポーリング方式）

**将来の拡張:**
- 描画バックエンドの抽象化（IUIRenderer）
- Canvas描画バックエンド
- レイアウトシステム

---

## 🔄 オーバーレイシステム

### OverlayManagerの使い方

```cpp
#include "core/system/OverlayManager.hpp"
#include "core/config/GameState.hpp"

// OverlayManagerはGameSystemが所有
auto& overlayManager = gameSystem->GetOverlayManager();

// オーバーレイを開く
overlayManager->PushOverlay(OverlayState::StageSelect, systemAPI_.get());

// オーバーレイを閉じる
overlayManager->PopOverlay();

// すべてのオーバーレイを閉じる
overlayManager->PopAllOverlays();

// 状態確認
if (overlayManager->IsEmpty()) {
    // オーバーレイが開いていない
}

if (overlayManager->IsOverlayActive(OverlayState::StageSelect)) {
    // ステージ選択オーバーレイが開いている
}
```

### オーバーレイの実装

新しいオーバーレイを実装する場合：

```cpp
// game/core/states/overlays/MyOverlay.hpp
#include "IOverlay.hpp"
#include "../../ui/components/Panel.hpp"
#include <memory>

namespace game {
namespace core {

class MyOverlay : public IOverlay {
public:
    MyOverlay();
    ~MyOverlay() = default;

    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::MyOverlay; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    // UIコンポーネント
    std::shared_ptr<ui::Panel> panel_;
};

} // namespace core
} // namespace game
```

`OverlayManager::CreateOverlay()`に新しいオーバーレイを追加：

```cpp
// game/core/system/OverlayManager.cpp
std::unique_ptr<IOverlay> OverlayManager::CreateOverlay(OverlayState state, BaseSystemAPI* systemAPI) {
    switch (state) {
    // ... 既存のオーバーレイ
    case OverlayState::MyOverlay:
        return std::make_unique<MyOverlay>();
    // ...
    }
}
```

---

## 🎨 UIコンポーネント層

### Cardコンポーネント

```cpp
#include "core/ui/components/Card.hpp"

auto card = std::make_shared<ui::Card>();
card->SetId("character_card");
card->SetPosition(100.0f, 100.0f);
card->SetSize(200.0f, 300.0f);
card->Initialize();

ui::CardContent content;
content.title = "Character Name";
content.description = "Character description";
content.imageId = "character_texture";
content.metadata["level"] = "50";
content.metadata["rarity"] = "SSR";
card->SetContent(content);

card->SetOnClickCallback([]() {
    LOG_INFO("Card clicked!");
});

card->Update(deltaTime);
card->Render();
```

### Listコンポーネント

```cpp
#include "core/ui/components/List.hpp"

auto list = std::make_shared<ui::List>();
list->SetId("character_list");
list->SetPosition(100.0f, 100.0f);
list->SetSize(300.0f, 400.0f);
list->SetItemHeight(50.0f);
list->Initialize();

ui::ListItem item1;
item1.id = "item1";
item1.label = "Item 1";
item1.value = "Value 1";
item1.enabled = true;
list->AddItem(item1);

list->SetOnSelectionChanged([](const ui::ListItem& item) {
    LOG_INFO("Selected: {}", item.id);
});

list->Update(deltaTime);
list->Render();
```

### Tileコンポーネント

```cpp
#include "core/ui/components/Tile.hpp"

auto tile = std::make_shared<ui::Tile>();
tile->SetId("stage_tile");
tile->SetPosition(100.0f, 100.0f);
tile->SetSize(400.0f, 400.0f);
tile->SetGridSize(4, 3);
tile->SetTileSize(100.0f, 100.0f);
tile->Initialize();

ui::TileData stage1;
stage1.id = "stage_1";
stage1.label = "Stage 1";
stage1.imageId = "stage1_texture";
stage1.enabled = true;
tile->AddTile(stage1);

tile->SetOnTileSelected([](const ui::TileData& data) {
    LOG_INFO("Selected tile: {}", data.id);
});

tile->Update(deltaTime);
tile->Render();
```

### Panelコンポーネント

```cpp
#include "core/ui/components/Panel.hpp"

auto panel = std::make_shared<ui::Panel>();
panel->SetId("main_panel");
panel->SetPosition(0.0f, 0.0f);
panel->SetSize(1920.0f, 1080.0f);
panel->Initialize();

// 子要素を追加
panel->AddChild(card);
panel->AddChild(list);

panel->Update(deltaTime);
panel->Render();
```

### Buttonコンポーネント

```cpp
#include "core/ui/components/Button.hpp"

auto button = std::make_shared<ui::Button>();
button->SetId("close_button");
button->SetPosition(100.0f, 100.0f);
button->SetSize(150.0f, 40.0f);
button->SetLabel("Close");
button->SetActionId("close_overlay");  // P1: 構造化イベント用
button->Initialize();

button->SetOnClickCallback([]() {
    LOG_INFO("Button clicked!");
});

button->Update(deltaTime);
button->Render();
```

---

## 📝 使用例

### オーバーレイを開く

```cpp
// GameSystemまたはシーンから
auto& overlayManager = gameSystem->GetOverlayManager();
overlayManager->PushOverlay(OverlayState::StageSelect, systemAPI_.get());
```

### オーバーレイからステート遷移

```cpp
// オーバーレイ内で
bool StageSelectOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = GameState::Game;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}
```

### UIコンポーネントの組み合わせ

```cpp
// オーバーレイ内で
auto panel = std::make_shared<ui::Panel>();
auto list = std::make_shared<ui::List>();
auto button = std::make_shared<ui::Button>();

// 初期化
panel->Initialize();
list->Initialize();
button->Initialize();

// 配置
panel->SetPosition(100.0f, 100.0f);
panel->SetSize(800.0f, 600.0f);
list->SetPosition(150.0f, 150.0f);
list->SetSize(500.0f, 400.0f);
button->SetPosition(700.0f, 150.0f);
button->SetSize(150.0f, 40.0f);

// 階層構造
panel->AddChild(list);
panel->AddChild(button);

// 更新・描画
panel->Update(deltaTime);
panel->Render();
```

---

## 📝 実装の詳細

### オーバーレイのライフサイクル

1. **Initialize()**: UIコンポーネントの初期化
2. **Update()**: UIコンポーネントの更新、入力処理
3. **Render()**: 半透明背景とUIコンポーネントの描画
4. **Shutdown()**: UIコンポーネントのクリーンアップ

### UIコンポーネントのライフサイクル

1. **Initialize()**: コンポーネントの初期化（1回のみ）
2. **OnShow()**: コンポーネントが表示されたときに呼ばれる（P2仕様）
3. **Update()**: 子要素の更新
4. **Render()**: ImGui経由で描画、子要素の描画
5. **OnHide()**: コンポーネントが非表示になったときに呼ばれる（P2仕様）
6. **Shutdown()**: 子要素のクリーンアップ（1回のみ）

### イベント処理（P1仕様: 構造化イベント）

#### UIEvent / UIEventResult

```cpp
#include "core/ui/UIEvent.hpp"

// UIイベントの種類
enum class UIEventType {
    None,   // イベントなし
    Click,  // マウスクリック
    Hover,  // マウスホバー
    Key,    // キー入力
};

// UIイベント構造体
struct UIEvent {
    UIEventType type = UIEventType::None;
    float x = 0.0f;   // Click / Hover 時の座標
    float y = 0.0f;
    int key = 0;      // Key イベント時のキーコード
};

// UIイベント処理結果
struct UIEventResult {
    bool handled = false;         // イベントを処理したか
    std::string componentId;      // イベント発生元のコンポーネントID
    std::string actionId;         // ビジネスロジック側で解釈するアクションID（例: "start_battle"）
};
```

#### HandleEvent() メソッド（推奨）

すべてのUIコンポーネントは `HandleEvent(const UIEvent&)` メソッドを実装しています：

```cpp
// オーバーレイ側での使用例
void StageSelectOverlay::Update(SharedContext& ctx, float deltaTime) {
    rootPanel_->Update(deltaTime);

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
        requestTransition_ = true;
        requestedNextState_ = GameState::Game;
    } else if (actionId == "close_overlay") {
        requestClose_ = true;
    }
}
```

#### 旧API（後方互換性のため残存）

- **OnMouseClick()**: マウスクリックイベント（親から子へ伝播）
- **OnMouseHover()**: マウスホバーイベント
- **OnKey()**: キー入力イベント（親から子へ伝播）

**注意**: 新規実装では `HandleEvent()` の使用を推奨します。

#### アクションIDの設定

```cpp
// Buttonコンポーネント
button->SetActionId("start_battle");
button->SetOnClickCallback([]() {
    // 即時コールバックも実行可能
});

// Cardコンポーネント
card->SetActionId("select_card");
card->SetOnClickCallback([]() {
    // 即時コールバックも実行可能
});
```

### 実装済みコンポーネントのHandleEvent()対応状況

- ✅ **Button**: クリック/ホバー/キーイベントを処理、アクションID返却
- ✅ **Panel**: 子要素へのイベント伝播を実装
- ✅ **Card**: クリック/ホバーイベントを処理、子要素への伝播対応
- ✅ **List**: クリック位置からアイテム選択、キー操作（上下キー）対応
- ✅ **Tile**: グリッドレイアウトでのクリック位置計算、タイル選択対応

### 注意点

1. **ImGuiウィンドウID**: `"ComponentType##" + id_`形式で一意性を確保
2. **子要素の描画**: 親コンポーネントの`Render()`内で実行
3. **イベント伝播**: 親から子へ伝播（`Panel`など）
4. **構造化イベント**: `HandleEvent()`を使用することで、UIレイヤとゲームロジックの分離が容易
5. **アクションID**: ビジネスロジック側で解釈するための識別子（例: "start_battle", "close_overlay"）
6. **描画バックエンド**: 現在は直接ImGuiを呼び出し（将来は`IUIRenderer`経由）

---

## 🎯 構造化イベントシステム（P1仕様）

### 概要

P1仕様として、UIイベントを構造化し、`UIEvent` / `UIEventResult` による統一的なイベント処理を実装しました。

### 利点

1. **UIレイヤとゲームロジックの分離**: アクションIDにより、UIコンポーネント側とビジネスロジック側を分離
2. **イベント処理の統一**: すべてのイベント（クリック、ホバー、キー）を1つのメソッドで処理
3. **拡張性**: 新しいイベントタイプを追加しやすい

### 使用例

```cpp
// オーバーレイ側での実装
void MyOverlay::Update(SharedContext& ctx, float deltaTime) {
    rootPanel_->Update(deltaTime);

    // マウスクリックイベント
    if (ctx.systemAPI->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        auto mousePos = ctx.systemAPI->GetMousePosition();
        
        ui::UIEvent ev;
        ev.type = ui::UIEventType::Click;
        ev.x = mousePos.x;
        ev.y = mousePos.y;

        ui::UIEventResult res = rootPanel_->HandleEvent(ev);
        if (res.handled) {
            HandleUIAction(res.actionId, res.componentId);
        }
    }

    // キーイベント
    int key = ctx.systemAPI->GetKeyPressed();
    if (key != 0) {
        ui::UIEvent ev;
        ev.type = ui::UIEventType::Key;
        ev.key = key;

        ui::UIEventResult res = rootPanel_->HandleEvent(ev);
        if (res.handled) {
            HandleUIAction(res.actionId, res.componentId);
        }
    }
}

void MyOverlay::HandleUIAction(const std::string& actionId, const std::string& componentId) {
    if (actionId == "start_battle") {
        requestTransition_ = true;
        requestedNextState_ = GameState::Game;
    } else if (actionId == "close_overlay") {
        requestClose_ = true;
    } else if (actionId.starts_with("select_item:")) {
        // リストアイテム選択
        std::string itemId = actionId.substr(12); // "select_item:" を除去
        LOG_INFO("Selected item: {}", itemId);
    } else if (actionId.starts_with("select_tile:")) {
        // タイル選択
        std::string tileId = actionId.substr(12); // "select_tile:" を除去
        LOG_INFO("Selected tile: {}", tileId);
    }
}
```

### ライフサイクルフック（P2仕様）

```cpp
// コンポーネントが表示されたときに呼ばれる
void MyComponent::OnShow() {
    // アニメーション開始、データ再読み込みなど
}

// コンポーネントが非表示になったときに呼ばれる
void MyComponent::OnHide() {
    // アニメーション停止、リソース解放など
}
```

**使用タイミング**: OverlayがPushされた瞬間、Popされた瞬間などに呼び出されます。

---

## 🔮 将来の拡張

### 描画バックエンドの抽象化

```cpp
// 将来の実装
class IUIRenderer {
public:
    virtual void DrawCard(const Rect& bounds, const CardContent& content) = 0;
    virtual void DrawList(const Rect& bounds, const std::vector<ListItem>& items) = 0;
    // ...
};

class ImGuiRenderer : public IUIRenderer {
    // ImGui実装
};

class CanvasRenderer : public IUIRenderer {
    // Canvas実装
};
```

### レイアウトシステム

```cpp
// 将来の実装
class ILayout {
public:
    virtual void Arrange(std::vector<std::shared_ptr<IUIComponent>>& children) = 0;
};

class GridLayout : public ILayout {
    // グリッド配置
};

class LinearLayout : public ILayout {
    // 線形配置
};
```

---

**詳細な実装例やAPIリファレンスは、各コンポーネントのヘッダーファイルを参照してください。**
