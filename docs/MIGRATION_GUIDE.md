# UIManager v2.0 移行ガイド

## 概要
UIManager v2.0では、後方互換性を放棄してフォント管理を効率化しました。
このガイドでは、既存コードを新しいAPIに移行する方法を説明します。

## 主な変更点

### 1. フォント取得API

#### 旧API
```cpp
// v1.0（後方互換性維持版）
Font defaultFont = uiManager.GetDefaultFont();  // コピーが発生
Font rayguiFont = uiManager.GetRayguiFont();    // 同じフォントを重複取得
```

#### 新API
```cpp
// v2.0（効率化版）
const Font& font = uiManager.GetFont();         // 参照取得、コピーなし
const Font& font20 = uiManager.GetFont(20);     // サイズ指定＆キャッシュ
```

### 2. テキスト描画

#### 旧コード
```cpp
Font defaultFont = uiManager.GetDefaultFont();
DrawTextEx(defaultFont, u8"テキスト", {10, 10}, 20, 1, DARKGRAY);
```

#### 新コード（推奨）
```cpp
// ヘルパー関数を使用
UI::DrawText(u8"テキスト", {10, 10}, 20, DARKGRAY);
```

#### 新コード（直接アクセス）
```cpp
// カスタム描画が必要な場合
const Font& font = uiManager.GetFont();
DrawTextEx(font, u8"テキスト", {10, 10}, 20, 1, DARKGRAY);
```

### 3. 削除されたAPI

以下のメソッドは削除されました：

```cpp
// ❌ 削除されたメソッド
Font GetDefaultFont();      // → GetFont() に統合
Font GetRayguiFont();       // → GetFont() に統合
void BeginFrame();          // → 不要のため削除
void EndFrame();            // → 不要のため削除
```

## 移行手順

### Step 1: GetDefaultFont() / GetRayguiFont() の置き換え

**検索:** `GetDefaultFont()` または `GetRayguiFont()`

**置換パターン1: 値で受け取っている場合**
```cpp
// Before
Font font = uiManager.GetDefaultFont();

// After（参照に変更）
const Font& font = uiManager.GetFont();
```

**置換パターン2: DrawTextEx で直接使用している場合**
```cpp
// Before
Font defaultFont = uiManager.GetDefaultFont();
DrawTextEx(defaultFont, text, position, fontSize, 1, color);

// After（ヘルパー関数を使用）
UI::DrawText(text, position, fontSize, color);
```

### Step 2: BeginFrame() / EndFrame() の削除

```cpp
// Before
void Game::Render() {
    BeginDrawing();
    uiManager.BeginFrame();  // ❌ 削除
    
    // ... レンダリング処理 ...
    
    uiManager.EndFrame();    // ❌ 削除
    EndDrawing();
}

// After
void Game::Render() {
    BeginDrawing();
    
    // ... レンダリング処理 ...
    
    EndDrawing();
}
```

### Step 3: 複数サイズのフォント使用

**旧コード（非効率）**
```cpp
void RenderUI() {
    // 毎回フォントをロード？（実装依存）
    DrawTextEx(GetFontSize16(), u8"小さい", {10, 10}, 16, 1, WHITE);
    DrawTextEx(GetFontSize20(), u8"中くらい", {10, 30}, 20, 1, WHITE);
    DrawTextEx(GetFontSize24(), u8"大きい", {10, 50}, 24, 1, WHITE);
}
```

**新コード（効率的）**
```cpp
void RenderUI() {
    // 自動キャッシュされる
    UI::DrawText(u8"小さい", {10, 10}, 16, WHITE);
    UI::DrawText(u8"中くらい", {10, 30}, 20, WHITE);
    UI::DrawText(u8"大きい", {10, 50}, 24, WHITE);
}
```

## 具体的な移行例

### 例1: Game.cpp の Render メソッド

**Before**
```cpp
void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    sceneManager_.RenderCurrentScene(registry_);
    
    Font defaultFont = uiManager_.GetDefaultFont();
    DrawTextEx(defaultFont, u8"Simple TDC Game - ESC to Exit", {10, 10}, 20, 1, DARKGRAY);
    DrawFPS(10, 40);
    
    std::string sceneText = "Current Scene: " + sceneManager_.GetCurrentSceneName();
    DrawTextEx(defaultFont, sceneText.c_str(), {10, 70}, 16, 1, DARKGRAY);
    
    uiManager_.DrawSampleUI();
    EndDrawing();
}
```

**After**
```cpp
void Game::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    sceneManager_.RenderCurrentScene(registry_);
    
    UI::DrawText(u8"Simple TDC Game - ESC to Exit", {10, 10}, 20, DARKGRAY);
    DrawFPS(10, 40);
    
    std::string sceneText = "Current Scene: " + sceneManager_.GetCurrentSceneName();
    UI::DrawText(sceneText.c_str(), {10, 70}, 16, DARKGRAY);
    
    uiManager_.DrawSampleUI();
    EndDrawing();
}
```

**変更点:**
- `Font defaultFont = ...` の削除
- `DrawTextEx(defaultFont, ...)` → `UI::DrawText(...)` に変更

### 例2: カスタムシーンの Render メソッド

**Before**
```cpp
void SampleScene::Render(entt::registry& registry) {
    Systems::SpriteRenderSystem::Render(registry);
    
    UI::UIManager& uiManager = UI::UIManager::GetInstance();
    Font defaultFont = uiManager.GetDefaultFont();
    int fontSize = 16;
    
    DrawTextEx(defaultFont, u8"Arrow Keys: Move cupslime", {10, 100}, fontSize, 1, DARKGRAY);
    DrawTextEx(defaultFont, u8"WASD: Move yodarehaki", {10, 120}, fontSize, 1, DARKGRAY);
}
```

**After**
```cpp
void SampleScene::Render(entt::registry& registry) {
    Systems::SpriteRenderSystem::Render(registry);
    
    UI::DrawText(u8"Arrow Keys: Move cupslime", {10, 100}, 16, DARKGRAY);
    UI::DrawText(u8"WASD: Move yodarehaki", {10, 120}, 16, DARKGRAY);
}
```

**変更点:**
- UIManager取得の削除
- Font変数の削除
- `DrawTextEx()` → `UI::DrawText()` に変更
- コード行数が半減

## チェックリスト

移行が完了したら、以下を確認してください：

- [ ] `GetDefaultFont()` の使用箇所をすべて `GetFont()` に変更
- [ ] `GetRayguiFont()` の使用箇所をすべて `GetFont()` に変更
- [ ] フォントを値で受け取っている箇所を `const Font&` に変更
- [ ] `BeginFrame()` / `EndFrame()` の呼び出しを削除
- [ ] 可能な限り `UI::DrawText()` ヘルパー関数を使用
- [ ] ビルドエラーがないことを確認
- [ ] 実行して正しく動作することを確認

## よくある質問

### Q: なぜ後方互換性を削除したのか？

A: 以下の理由により、後方互換性を放棄してシンプル化しました：
- コピーオーバーヘッドの削除による性能向上
- APIのシンプル化による保守性向上
- 重複したメソッドの削除による混乱の回避

### Q: 古いコードはどうなるのか？

A: コンパイルエラーになります。このガイドに従って移行してください。

### Q: GetFont() と GetFont(fontSize) の違いは？

A:
- `GetFont()`: ベースフォント（Initialize時に指定したサイズ）を返す
- `GetFont(fontSize)`: 指定サイズのフォントを返す（キャッシュされる）

### Q: フォントキャッシュはいつクリアされるのか？

A: `UIManager::Shutdown()` 時に自動的にクリアされます。手動でクリアする必要はありません。

### Q: パフォーマンスはどれくらい向上するのか？

A: フォントコピーが完全に削除されるため、テキスト描画が多い場面で顕著な改善が見られます。
具体的な数値は環境依存ですが、フレームごとに数十回テキストを描画する場合、
フレームレートが5-10%向上する可能性があります。

## トラブルシューティング

### コンパイルエラー: 'GetDefaultFont' is not a member of 'UI::UIManager'

**原因:** 削除されたメソッドを使用している

**解決策:**
```cpp
// エラー
Font font = uiManager.GetDefaultFont();

// 修正
const Font& font = uiManager.GetFont();
```

### コンパイルエラー: 'BeginFrame' is not a member of 'UI::UIManager'

**原因:** 削除されたメソッドを使用している

**解決策:** `BeginFrame()` / `EndFrame()` の呼び出しを削除

### 実行時エラー: フォントが表示されない

**原因:** フォントファイルが見つからない、または初期化していない

**解決策:**
1. `assets/fonts/NotoSansJP-Medium.ttf` が存在するか確認
2. `uiManager.Initialize("assets/fonts/NotoSansJP-Medium.ttf", 18.0f)` を呼び出しているか確認
3. コンソールで "UIManager initialized successfully" を確認

## まとめ

UIManager v2.0への移行は、コードをよりシンプルで効率的にします。
主な作業は以下の3つです：

1. `GetDefaultFont()` → `GetFont()` または `UI::DrawText()` に変更
2. フォントを参照 (`const Font&`) で受け取る
3. 不要な `BeginFrame()` / `EndFrame()` を削除

移行後は、パフォーマンスと保守性が向上した、よりクリーンなコードベースになります。
