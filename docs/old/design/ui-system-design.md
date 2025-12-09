# UIシステム設計

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）のUI定義システム設計を定義します。

## 目的

- JSON定義によるUIレイアウトの完全なデータ駆動化
- スキン/テーマシステムによる見た目のカスタマイズ
- UIアニメーション定義
- 内部エディタでのUI編集
- エディタは固定3ペイン＋タイムラインのレイアウト（F1〜F4切替）

---

## 1. UIシステム概要

### 1.1 アーキテクチャ

```
UI定義（JSON）
    ↓
UIローダー
    ↓
UIレジストリ
    ↓
UIレンダラー
    ↓
ゲーム画面
```

### 1.2 主要コンポーネント

- **UILayoutDef**: UIレイアウト定義（JSON）
- **UIRenderer**: UI描画エンジン
- **UIElement**: UI要素（パネル、ボタン、ラベル等）
- **UIStyle**: スタイル定義（色、フォント、サイズ等）
- **UIAnimation**: アニメーション定義

### 1.3 シーン/状態との連動

- シーン粒度: タイトル / ホーム / ステージ選択 / 戦闘 / リザルト / 設定で UI レイヤセットを切替。Enter/Exit でレイアウトロードと BGM/入力バインドを再設定。
- 状態粒度（戦闘内）: `Ready/Playing/Paused/Victory/Defeat` で UI レイヤ/入力マップ/サウンドキューを切替。Paused では入力を最小限（再開/リトライ/戻る）に限定し、Playing へ復帰時に元のバインドを復元。
- pendingTransition 適用: 状態遷移は原則フレーム終端で適用し、UI レイヤ切替も同じタイミングで行う。緊急停止は Update 前に即時反映して安全側（Paused 等）に倒す。
- データ受け渡し: ステージ選択→戦闘への遷移時にステージ定義/編成/設定を UI バインディングへ配信。Victory/Defeat で Result データをリザルト UI へ渡し、戻る/リトライの遷移トリガを SceneManager へ送る。
- HotReload: UI 定義のリロード時は現在シーン/状態に合わせてプレビューを再構築し、失敗時は旧レイアウトを保持。OnHotReload で安全側にフェイルさせ、警告を出す。
- エディタ連動: F1〜F4 エディタ常駐を前提に、現在のシーン/状態を参照してプレビュー対象レイアウトと入力バインドを自動切替。シーンはエディタを停止せず、切替時にプレビューとバインドだけ更新する。
- UIツリー切替: シーンごとにロードするレイアウト一覧を持ち（例: タイトル: title_main, ホーム: home_main, ステージ選択: stage_select_main, 戦闘: battle_hud/battle_pause, リザルト: result_main, 設定: settings_main）、戦闘HUDは共通レイヤとして戦闘シーン内で保持。切替は既定で即時、フェードは任意オプション。
- 入力マップ優先度: エディタ > UI > ゲーム入力で固定（前提）。例外はデバッグ用グローバルショートカットのみ。ポーズ中は戦闘入力を無効化し、UIのみ受理。
- BGM/SEとの協調: 既定は短フェード切替。Victory/Defeat ではジングルを優先し、BGM はフェードアウト（短フェード基準）。
- 図鑑UI: 図鑑シーン用の `encyclopedia_main` レイアウトを追加し、タグ/ロール/レアリティフィルタと検索を前提にする。アイコン/立ち絵欠落時はデフォルト画像を表示し警告トースト。

### 1.4 図鑑シーン（シンプル版・後拡張可）

- レイアウト: 左リスト（タグ別リスト＋名前表示）、中央上=立ち絵、中央下=ステータス、右=説明/カウンタ。立ち絵欠落時はデフォ画像＋警告。
- 検索/フィルタ: 現状は検索なし。タグ別リストのみ提供（ロール/レアリティ検索は後付け）。各タグ内は名前昇順。多件数時はページング（例: 20件/ページ固定）。
- リソース: 立ち絵は遅延ロード＋キャッシュ。同時保持上限は2〜3枚までに制限。HotReload は図鑑メタ更新時のみ再読込し、更新対象IDのキャッシュを無効化。
- 拡張余地: 将来、検索/ロール/レアリティ/複数タグ選択を追加する場合はタグリストをフィルタUIに置き換える想定。
- 戻り先: ホーム固定、スタックなし。テストもこの前提で行う（他シーンへ戻らない）。

### 1.5 入力/BGM/UI切替一覧

| シーン          | 入力バインドセット        | BGM(既定フェード)     | UIレイアウトID                         | 備考 |
|-----------------|---------------------------|-----------------------|----------------------------------------|------|
| タイトル        | title_input               | title_bgm (0.2-0.3s)  | title_main                             |      |
| ホーム          | home_input                | home_bgm (0.2-0.3s)   | home_main                              |      |
| ステージ選択    | stage_select_input        | stage_select_bgm (0.2-0.3s) | stage_select_main + details/formation (同パネル) | HUDなし |
| 戦闘            | battle_input              | battle_bgm (0.2-0.3s) | battle_hud + battle_pause              | HUD常駐 |
| リザルト        | result_input              | result_bgm (0.2-0.3s) | result_main                            | Victory/Defeat時ジングル優先+フェードアウト |
| 設定            | settings_input            | settings_bgm (0.2-0.3s) | settings_main                          |      |
| 図鑑            | encyclopedia_input        | encyclopedia_bgm (0.2-0.3s) | encyclopedia_main                      | 立ち絵遅延ロード |

優先度はエディタ > UI > ゲーム入力で統一し、ポーズ中は戦闘入力を無効化してUIのみ許可。

### 1.6 HUD常駐の前提

- 常駐対象: 戦闘HUD（`battle_hud`）。非対象: タイトル/ホーム/ステージ選択/リザルト/図鑑/設定のUIはシーン切替でロード/アンロード。
- リソースと入力: 戦闘HUDのレイアウトと関連入力バインドは常駐させ、戦闘終了でも解放しない。その他レイアウトはシーン切替時に差し替える。

### 1.7 ステージ詳細・編成確認の粒度（固定）

- 当面はステージ選択シーン内の単一パネルでステージ詳細＋編成確認を完結させる（独立シーン化は後回し）。
- 将来シーン分割する場合は、遷移表/入力バインド/BGM/UIレイアウトの再整理が必要になる旨を記載。
- 検証エラー表示: 同パネル内トーストで表示し、同パネルから再試行を送れるようにする。
- 入力マップ: ステージ詳細/編成確認はステージ選択の入力マップを共有する。

---

## 2. UI要素タイプ

### 2.1 サポート要素

| 要素タイプ | 説明 | 用途 |
|-----------|------|------|
| **Panel** | コンテナ要素 | 背景、グループ化 |
| **Button** | ボタン | クリック可能なUI |
| **Label** | テキスト表示 | 情報表示 |
| **Image** | 画像表示 | アイコン、装飾 |
| **Progress** | プログレスバー | HP、経験値等 |
| **List** | リスト表示 | アイテム一覧等 |
| **Grid** | グリッド表示 | インベントリ等 |

### 2.2 要素階層

UI要素は階層構造を持ち、親要素の子として配置されます。

```
UILayout
  └─ Panel (top_panel)
      ├─ Label (gold_label)
      ├─ Progress (health_bar)
      └─ Panel (button_group)
          ├─ Button (attack_button)
          └─ Button (defend_button)
```

---

## 3. UIレンダラー設計

### 3.1 UIRendererクラス

```cpp
namespace New::Domain::UI {

class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();
    
    // 初期化・終了
    bool Initialize(Core::GameContext& context);
    void Shutdown();
    
    // レイアウト読み込み
    bool LoadLayout(const std::string& layoutId);
    void UnloadLayout(const std::string& layoutId);
    
    // 描画
    void Render(float deltaTime);
    
    // 入力処理
    bool HandleInput(const Core::InputEvent& event);
    
    // 要素取得
    UIElement* GetElement(const std::string& elementId);
    const UIElement* GetElement(const std::string& elementId) const;
    
    // バインディング更新
    void UpdateBindings(const std::unordered_map<std::string, nlohmann::json>& data);

private:
    Core::GameContext* context_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<UILayout>> layouts_;
    std::unordered_map<std::string, UIElement*> elementMap_;
    
    // 描画メソッド
    void RenderElement(UIElement& element, const UIStyle& parentStyle);
    void RenderPanel(PanelElement& panel, const UIStyle& style);
    void RenderButton(ButtonElement& button, const UIStyle& style);
    void RenderLabel(LabelElement& label, const UIStyle& style);
    void RenderImage(ImageElement& image, const UIStyle& style);
    void RenderProgress(ProgressElement& progress, const UIStyle& style);
    
    // 入力処理
    bool HandleElementInput(UIElement& element, const Core::InputEvent& event);
};

} // namespace New::Domain::UI
```

### 3.2 UI要素基底クラス

```cpp
namespace New::Domain::UI {

class UIElement {
public:
    UIElement(const std::string& id, UIElementType type);
    virtual ~UIElement() = default;
    
    // 基本プロパティ
    std::string GetId() const { return id_; }
    UIElementType GetType() const { return type_; }
    
    // 位置・サイズ
    Position GetPosition() const { return position_; }
    void SetPosition(const Position& pos) { position_ = pos; }
    Size GetSize() const { return size_; }
    void SetSize(const Size& s) { size_ = s; }
    
    // スタイル
    UIStyle GetStyle() const { return style_; }
    void SetStyle(const UIStyle& s) { style_ = s; }
    
    // 子要素
    void AddChild(std::unique_ptr<UIElement> child);
    const std::vector<std::unique_ptr<UIElement>>& GetChildren() const { return children_; }
    
    // 可視性
    bool IsVisible() const { return visible_; }
    void SetVisible(bool visible) { visible_ = visible; }
    
    // バインディング
    void SetBinding(const std::string& key, const std::string& expression);
    std::string GetBinding(const std::string& key) const;
    
    // 更新・描画（仮想関数）
    virtual void Update(float deltaTime) {}
    virtual void Render(UIRenderer& renderer, const UIStyle& parentStyle) = 0;
    
    // 入力処理
    virtual bool HandleInput(const Core::InputEvent& event) { return false; }

protected:
    std::string id_;
    UIElementType type_;
    Position position_;
    Size size_;
    UIStyle style_;
    std::vector<std::unique_ptr<UIElement>> children_;
    bool visible_ = true;
    std::unordered_map<std::string, std::string> bindings_;
};

} // namespace New::Domain::UI
```

### 3.3 スキーマとネイティブコードの対応

- スキーマは**データ宣言のみ**を担当し、実行ロジック（レイアウト構築・バインディング解決・イベント通知・ビヘイビア適用）はネイティブ/ローダ側が担う。
- タイプ/アンカーのマッピング（スキーマ→ネイティブ）:  

| スキーマ | ネイティブ | 備考 |
| --- | --- | --- |
| panel | Panel | コンテナ |
| button | Button | - |
| label | Text | 表示テキスト |
| image | Image | - |
| progress | ProgressBar | - |
| list | Container (+repeat) | `behaviorKey` でロジック差し替え |
| grid | Container (+repeat) | 同上 |
| slot | Slot | インベントリ等 |

| スキーマアンカー | ネイティブアンカー |
| --- | --- |
| top-left | top_left |
| top-center | topCenter |
| top-right | top_right |
| center-left | centerLeft |
| center | center |
| center-right | centerRight |
| bottom-left | bottom_left |
| bottom-center | bottomCenter |
| bottom-right | bottom_right |

- ローダ/バリデータ責務分離:
  - フロー: **スキーマ検証 → ネイティブ変換（マッピング） → バインディング/イベント通知**。
  - `ValidateUILayoutDef` は構造・型チェックのみ。ビヘイビア解決は別Registry（例: `UIBehaviorRegistry`）で行う。
  - バインディング式は簡易式のみ許容（プレースホルダ参照＋加減算/三項演算子）。複雑な計算はコード側に委譲。
  - デフォルト値の適用はローダが担当（未指定キーをネイティブ既定値で埋める）。
  - List/Grid/Slot は UI 定義ではデータ/見た目のみとし、`behaviorKey` でロジックを差し替える（スクロール/ページング/ソートなど）。
  - `behaviorKey` 命名: `ui.behavior.<domain>.<name>`。Registry: `UIBehaviorRegistry` に登録必須。未登録時はエラー（フォールバックなし）。
- HotReload/Validation フロー（Dev向け): 監視 → 検証 → ローダ → 適用/ロールバック → DevMode通知。
- 未実装項目:
  - `ui_layout.json` ローダと SchemaValidator の UI 対応が未実装。スキーマ→ `UIElementDef`/`UILayoutDef` の変換と検証を追加する。
  - リスト/グリッド専用の配置ロジックが必要なら、Container + repeat から拡張する。

### 3.3 バインディング検証（DevMode向け）

- 必須バインディング未解決時に警告を表示（赤ハイライト）。
- DevModeパネルに「Binding Warnings」リストを表示（要素ID / 欠落キー / ソース）。
- レンダリング時に毎フレーム検証は避け、UIロード時と HotReload 時に検証を実施。

### 3.4 アンカー/サイズの解釈と変換

- 位置・サイズで % 指定を許可し、親サイズを基準に解釈する。アンカー基準点で補正後に最終座標を算出。
- アンカー文字列変換表と%解釈ロジックは UI ローダ内で一元管理し、DevModeでも同じ経路を使う。

### 3.5 ビヘイビア差し替えガイド（List/Grid/Slot）

- UI定義はデータ/見た目のみ保持し、ロジックは `behaviorKey` で切り替える。
- `behaviorKey` 命名: `ui.behavior.<domain>.<name>`。Registry: `UIBehaviorRegistry` に必須登録。未登録はエラー（フォールバックなし）。
- 例: `ui.behavior.list.scroll`, `ui.behavior.grid.paginate`, `ui.behavior.slot.inventory`.

### 3.6 UX/操作/チュートリアル導線

- 速度変更UI: 0.5x/1x/2x トグルをHUDに常設し、ショートカットと連動。状態は `GameStateComponent.timeScale` と双方向同期。  
- ポーズUI: Esc/Startでポーズメニュー、再開/リトライ/ステージ選択を提示。  
- チュートリアル: 初回ステージ開始時にオーバーレイガイド、ポーズ中ヘルプパネルを表示。  
- 画面遷移: タイトル → ステージ選択 → ゲーム → リザルト → リトライ/戻る の導線をUIで固定する。  
- アクセシビリティ設定UI: 色覚プリセット/コントラスト、フォントサイズスライダ、マスター/SE/BGM音量スライダを設定メニューに配置。  
- 入力/カメラ操作: 仮想座標基準でホイールズーム/パン（許可ステージのみ）をUIトグルで切替。キー/マウス割り当てのリマップ画面を提供。

### 3.7 設定UIフローと操作仕様

- 画面遷移: タイトル/ポーズ/ホームから設定画面へ遷移可能。戻り先を保持しキャンセルで復帰。  
- グルーピング: 音量、表示（解像度/フルスクリーン/FPS上限/VSync/UIスケール）、入力リマップ、アクセシビリティ（色覚/フォントサイズ/コントラスト/字幕・表示強度）でタブ分け。  
- リセット/適用/キャンセル:  
  - 適用: 反映可能な項目は即時反映、解像度/フルスクリーン/VSync/FPS上限は再初期化/次起動扱いのモーダル確認。  
  - キャンセル: 未適用の変更を破棄し、元の値へ戻す。  
  - リセット: 現タブまたは全体を既定値に戻し、確認ダイアログを挟む。  
- 入力リマップUI:  
  - 変更対象を選択 → 入力待ち → 重複検出時に警告・上書き確認。  
  - 軸/ボタンを区別し、デッドゾーン/感度は数値入力可能。  
  - 仮想座標ズーム/パンの許可はステージ設定と連動し、無効ステージでは項目をグレーアウト。  
- アクセシビリティプレビュー: 色覚/コントラスト/フォントサイズを即時プレビューし、テスト用UIスニペットを同画面に表示。字幕・表示強度のサンプルも併置。

### 3.8 シンプル必須UIフロー（オフライン軽量版）

- 主要導線: タイトル → スロット選択(3固定) → ホーム → ステージ選択 → ステージ詳細 → 編成確認 → 戦闘 → リザルト。ホームから編成/強化/ショップ/設定へ分岐。  
- 編成UI: メイン3 + サブ5 + アビリティ2、プリセット3固定。メイン/サブ枠は色やラベルで明確に区別し、コストは合算表示。ロール警告はメイン+サブ合算で表示。プリセット上書きは確認ダイアログ。  
- 強化UI: 確定強化のみ。必要コスト表示、プレビュー、ロックで誤操作防止。  
- ショップUI: 素材/ゴールドの2タブ、毎日リセット、所持上限表示、購入確認ダイアログ。  
- ミッションUI簡略: 固定タスクの進捗サマリと未受取通知のみ（詳細リスト省略）。  
- ステージ詳細UI: 推奨戦力/敵プレビュー/報酬/難易度解放条件（Easy/Normal/Hard）を表示。  
- 設定UI（最小）: 解像度+適用、VSync/FPS上限、UIスケール、音量（BGM/SE/マスター）、字幕ON/OFF。即時反映と再初期化項目を区分。  
- 誤操作防止: 高額購入/強化/プリセット上書き/売却で確認ダイアログ。空状態ガイド（編成未設定・ショップ在庫なし等）を表示。

### 3.9 通知/トースト/カメラ振動方針

- トースト表示: 画面上部右寄せ、標準 2.5s、優先度付きキューで上書き抑制（同一イベントはクールダウン）。  
- 優先度例: ボス出現 > ウェーブ開始/終了 > ミッション/報酬通知 > 情報系。  
- 重複抑制: 同タイプ連続は最新のみ残し、残りをまとめて1件に集約。  
- カメラ振動設定: ユーザ設定でオン/オフ可。演出で要求された場合、設定がオフなら振動を無効化。強度は小/中/大の定数プリセット。

### 3.10 UIエディタ境界とテスト（簡易）

- 反映フロー: UIエディタ保存 → スキーマ検証 → ネイティブ変換 → 適用/部分適用（失敗はキャッシュ保持）→ Devトースト通知。  
- 監視パス: `assets/new/definitions/ui/*.json` を HotReload 監視し、SchemaValidator → UILoader の順で処理。  
- テストスモーク（最小）：スキーマ/ローダ回帰（ui_layout）、解像度変更後の再初期化、設定適用失敗時のロールバック、通知トーストの優先度・重複抑制確認。  
- エディタ境界: エディタ側のドラフトは保存前にローカル検証を行い、失敗時はゲームに送らない。ゲーム側は常に検証を再実行し、部分失敗は partial で告知。

---

## 4. 各要素の実装

### 4.1 PanelElement

```cpp
namespace New::Domain::UI {

class PanelElement : public UIElement {
public:
    PanelElement(const std::string& id) : UIElement(id, UIElementType::Panel) {}
    
    void Render(UIRenderer& renderer, const UIStyle& parentStyle) override {
        if (!IsVisible()) return;
        
        UIStyle finalStyle = MergeStyle(parentStyle, GetStyle());
        
        // 背景描画
        Rectangle rect = CalculateRect();
        DrawRectangleRec(rect, finalStyle.backgroundColor);
        
        // ボーダー描画
        if (finalStyle.borderWidth > 0) {
            DrawRectangleLinesEx(rect, (float)finalStyle.borderWidth, finalStyle.borderColor);
        }
        
        // 子要素描画
        for (auto& child : GetChildren()) {
            child->Render(renderer, finalStyle);
        }
    }

private:
    Rectangle CalculateRect() const {
        Position pos = GetPosition();
        Size size = GetSize();
        return {pos.x, pos.y, size.width, size.height};
    }
};

} // namespace New::Domain::UI
```

### 4.2 ButtonElement

```cpp
namespace New::Domain::UI {

class ButtonElement : public UIElement {
public:
    ButtonElement(const std::string& id) : UIElement(id, UIElementType::Button) {}
    
    void Update(float deltaTime) override {
        // ホバー・クリック状態の更新
        Vector2 mousePos = GetMousePosition();
        Rectangle rect = CalculateRect();
        
        isHovered_ = CheckCollisionPointRec(mousePos, rect);
        isPressed_ = isHovered_ && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        wasClicked_ = isHovered_ && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        if (wasClicked_ && onClickCallback_) {
            onClickCallback_();
        }
    }
    
    void Render(UIRenderer& renderer, const UIStyle& parentStyle) override {
        if (!IsVisible()) return;
        
        UIStyle finalStyle = MergeStyle(parentStyle, GetStyle());
        
        // 状態に応じた色変更
        Color bgColor = finalStyle.backgroundColor;
        if (isPressed_) {
            bgColor = DarkenColor(bgColor, 0.2f);
        } else if (isHovered_) {
            bgColor = LightenColor(bgColor, 0.1f);
        }
        
        Rectangle rect = CalculateRect();
        DrawRectangleRec(rect, bgColor);
        
        // テキスト描画
        std::string text = GetProperty<std::string>("text", "");
        if (!text.empty()) {
            DrawTextEx(GetFont(), text.c_str(), {rect.x + 10, rect.y + 10}, 
                      (float)finalStyle.fontSize, 2.0f, finalStyle.textColor);
        }
    }
    
    void SetOnClick(std::function<void()> callback) {
        onClickCallback_ = callback;
    }

private:
    bool isHovered_ = false;
    bool isPressed_ = false;
    bool wasClicked_ = false;
    std::function<void()> onClickCallback_;
};

} // namespace New::Domain::UI
```

### 4.3 LabelElement

```cpp
namespace New::Domain::UI {

class LabelElement : public UIElement {
public:
    LabelElement(const std::string& id) : UIElement(id, UIElementType::Label) {}
    
    void Render(UIRenderer& renderer, const UIStyle& parentStyle) override {
        if (!IsVisible()) return;
        
        UIStyle finalStyle = MergeStyle(parentStyle, GetStyle());
        
        // テキスト取得（バインディング解決）
        std::string text = ResolveText();
        
        // テキスト描画
        Position pos = GetPosition();
        DrawTextEx(GetFont(), text.c_str(), {pos.x, pos.y}, 
                  (float)finalStyle.fontSize, 2.0f, finalStyle.textColor);
    }

private:
    std::string ResolveText() {
        std::string text = GetProperty<std::string>("text", "");
        
        // バインディング解決
        if (HasBinding("text")) {
            std::string expression = GetBinding("text");
            text = EvaluateBinding(expression);
        }
        
        return text;
    }
    
    std::string EvaluateBinding(const std::string& expression);
};

} // namespace New::Domain::UI
```

### 4.4 ProgressElement

```cpp
namespace New::Domain::UI {

class ProgressElement : public UIElement {
public:
    ProgressElement(const std::string& id) : UIElement(id, UIElementType::Progress) {}
    
    void Update(float deltaTime) override {
        // バインディングから値を取得
        if (HasBinding("currentValue")) {
            std::string expr = GetBinding("currentValue");
            currentValue_ = EvaluateFloat(expr);
        }
        
        if (HasBinding("maxValue")) {
            std::string expr = GetBinding("maxValue");
            maxValue_ = EvaluateFloat(expr);
        }
        
        // アニメーション更新
        if (animated_) {
            animatedValue_ = Lerp(animatedValue_, currentValue_, animationSpeed_ * deltaTime);
        } else {
            animatedValue_ = currentValue_;
        }
    }
    
    void Render(UIRenderer& renderer, const UIStyle& parentStyle) override {
        if (!IsVisible()) return;
        
        UIStyle finalStyle = MergeStyle(parentStyle, GetStyle());
        Rectangle rect = CalculateRect();
        
        // 背景
        DrawRectangleRec(rect, finalStyle.backgroundColor);
        
        // プログレスバー
        float progress = maxValue_ > 0.0f ? (animatedValue_ / maxValue_) : 0.0f;
        progress = std::clamp(progress, 0.0f, 1.0f);
        
        Rectangle progressRect = rect;
        progressRect.width *= progress;
        
        Color progressColor = GetProperty<Color>("progressColor", GREEN);
        DrawRectangleRec(progressRect, progressColor);
        
        // ボーダー
        if (finalStyle.borderWidth > 0) {
            DrawRectangleLinesEx(rect, (float)finalStyle.borderWidth, finalStyle.borderColor);
        }
        
        // テキスト（オプション）
        if (GetProperty<bool>("showText", false)) {
            std::string text = std::to_string((int)currentValue_) + " / " + std::to_string((int)maxValue_);
            DrawTextEx(GetFont(), text.c_str(), {rect.x + 5, rect.y + 5}, 
                      (float)finalStyle.fontSize, 2.0f, finalStyle.textColor);
        }
    }

private:
    float currentValue_ = 0.0f;
    float maxValue_ = 100.0f;
    float animatedValue_ = 0.0f;
    bool animated_ = true;
    float animationSpeed_ = 5.0f;
    
    float EvaluateFloat(const std::string& expression);
    float Lerp(float a, float b, float t);
};

} // namespace New::Domain::UI
```

---

## 5. スキン/テーマシステム

### 5.1 スキン定義（JSON）

```json
{
  "skins": [
    {
      "id": "default",
      "name": "デフォルトスキン",
      "styles": {
        "panel": {
          "backgroundColor": "#2C2C2C",
          "borderColor": "#FFFFFF",
          "borderWidth": 2
        },
        "button": {
          "backgroundColor": "#4A4A4A",
          "textColor": "#FFFFFF",
          "fontSize": 16,
          "hoverColor": "#5A5A5A",
          "pressedColor": "#3A3A3A"
        },
        "label": {
          "textColor": "#FFFFFF",
          "fontSize": 20
        },
        "progress": {
          "backgroundColor": "#1A1A1A",
          "progressColor": "#00FF00",
          "borderColor": "#FFFFFF",
          "borderWidth": 1
        }
      }
    },
    {
      "id": "dark",
      "name": "ダークスキン",
      "styles": {
        "panel": {
          "backgroundColor": "#1A1A1A",
          "borderColor": "#666666"
        }
      }
    }
  ]
}
```

### 5.2 スキンマネージャー

```cpp
namespace New::Domain::UI {

class SkinManager {
public:
    // スキン読み込み
    bool LoadSkin(const std::string& skinId, const std::string& path);
    
    // スキン適用
    void ApplySkin(const std::string& skinId);
    
    // スタイル取得
    UIStyle GetStyle(const std::string& elementType) const;
    
    // 現在のスキンID
    std::string GetCurrentSkinId() const { return currentSkinId_; }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, UIStyle>> skins_;
    std::string currentSkinId_ = "default";
};

} // namespace New::Domain::UI
```

---

## 6. UIアニメーション

### 6.1 アニメーション定義（JSON）

```json
{
  "animations": [
    {
      "id": "fade_in",
      "type": "fade",
      "duration": 0.5,
      "from": 0.0,
      "to": 1.0
    },
    {
      "id": "slide_in_right",
      "type": "slide",
      "duration": 0.3,
      "direction": "right",
      "distance": 200
    },
    {
      "id": "scale_bounce",
      "type": "scale",
      "duration": 0.5,
      "from": 0.0,
      "to": 1.0,
      "easing": "bounce"
    }
  ]
}
```

### 6.2 アニメーションシステム

```cpp
namespace New::Domain::UI {

enum class AnimationType {
    Fade,
    Slide,
    Scale,
    Rotate
};

enum class EasingType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Bounce
};

class UIAnimation {
public:
    UIAnimation(AnimationType type, float duration);
    
    void Update(float deltaTime);
    bool IsComplete() const { return elapsed_ >= duration_; }
    void Reset() { elapsed_ = 0.0f; }
    
    float GetValue() const;

private:
    AnimationType type_;
    float duration_;
    float elapsed_ = 0.0f;
    EasingType easing_ = EasingType::Linear;
    
    float from_ = 0.0f;
    float to_ = 1.0f;
    
    float ApplyEasing(float t);
};

class UIAnimationManager {
public:
    void AddAnimation(const std::string& elementId, std::unique_ptr<UIAnimation> animation);
    void Update(float deltaTime);
    void RemoveAnimation(const std::string& elementId);

private:
    std::unordered_map<std::string, std::unique_ptr<UIAnimation>> animations_;
};

} // namespace New::Domain::UI
```

---

## 7. バインディングシステム

### 7.1 バインディング式

UI要素のプロパティをゲーム状態にバインドできます。

```json
{
  "properties": {
    "text": "ゴールド: {gameState.gold}",
    "currentValue": "{player.health}",
    "maxValue": "{player.maxHealth}"
  }
}
```

### 7.2 バインディング評価

```cpp
namespace New::Domain::UI {

class BindingEvaluator {
public:
    // バインディング式を評価
    std::string EvaluateString(const std::string& expression, const BindingContext& context);
    float EvaluateFloat(const std::string& expression, const BindingContext& context);
    bool EvaluateBool(const std::string& expression, const BindingContext& context);

private:
    // パース・評価ロジック
    nlohmann::json ResolvePath(const std::string& path, const BindingContext& context);
};

struct BindingContext {
    std::unordered_map<std::string, nlohmann::json> data;
    
    // ゲーム状態へのアクセス
    Core::GameContext* gameContext = nullptr;
};

} // namespace New::Domain::UI
```

---

## 8. 実装チェックリスト

### 8.1 UIレンダラー

- [ ] `UIRenderer`クラス実装
- [ ] レイアウト読み込み機能
- [ ] 要素描画機能
- [ ] 入力処理機能

### 8.2 UI要素

- [ ] `UIElement`基底クラス実装
- [ ] `PanelElement`実装
- [ ] `ButtonElement`実装
- [ ] `LabelElement`実装
- [ ] `ImageElement`実装
- [ ] `ProgressElement`実装
- [ ] `ListElement`実装
- [ ] `GridElement`実装

### 8.3 スキンシステム

- [ ] `SkinManager`実装
- [ ] スキン定義ローダー
- [ ] スタイル適用機能

### 8.4 アニメーション

- [ ] `UIAnimation`実装
- [ ] `UIAnimationManager`実装
- [ ] 各種イージング関数

### 8.5 バインディング

- [ ] `BindingEvaluator`実装
- [ ] バインディング式パーサー
- [ ] ゲーム状態へのアクセス

---

## 9. イベントとデータバインディング（簡素化方針）

### 9.1 バインド

- `bind` は「キー参照のみ」とし、式評価は行わない（例: `cost.current`, `wave.current`, `game.speed`）。整形や計算はネイティブ側で前処理して渡す。
- 欠損・型違いはデフォルト表示にフォールバックする（空文字/0など）。

### 9.2 UIイベント（最小セット）

- 送信: `ButtonClicked(elementId)`, `ToggleChanged(elementId, value)`
- 受信（HUD用通知）: `CostChanged(current, max)`, `WaveProgress(currentWave, total, isBoss)`, `UnitPlacementResult(success, reason)`, `BossIncoming(waveId)`
- 未登録イベントは無視するポリシーとする。

### 9.3 イベントフロー指針

- UI → Game: イベントバス経由で疎結合に送信。UI側にゲームロジックを持たせない。
- Game → UI: データバインドで状態反映。局所更新はイベントでトリガ可。
- 失敗/警告はトーストやダイアログで通知し、ハングさせない。

### 9.4 バインド例（キー参照のみ）

```json
{
  "id": "hud_main",
  "widgets": [
    {"id": "cost_bar", "type": "Progress", "bind": "cost.current"},
    {"id": "wave_label", "type": "Label", "bind": "wave.current"},
    {"id": "speed_toggle", "type": "Toggle", "bind": "game.speed", "events": ["ToggleChanged"]}
  ]
}
```

---

## 10. 参考資料

- [コアアーキテクチャ設計](core-architecture-design.md)
- [JSONスキーマ設計](json-schema-design.md)
- [内部エディタ設計](internal-editor-design.md)
- [設計原則と制約](design-principles-and-constraints.md)

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ開発者向け
