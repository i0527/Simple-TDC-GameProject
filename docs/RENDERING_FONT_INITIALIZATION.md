# RenderingManager フォント描画初期化順序

## 概要

RenderingManagerがResourceManagerの日本語フォント機能を使用するため、ServiceContainer初期化時に厳密な初期化順序の制御が必要です。

## 初期化順序（必須）

1. **ResourceManager の作成・初期化**
   - リソースマネージャーを最初に作成
   - フォント管理機能を準備

2. **RenderingManager の作成・初期化**
   - 次に描画マネージャーを作成
   - 内部解像度、スケーリング機能の初期化

3. **RenderingManager に ResourceManager 参照を設定**
   - `renderingManager->SetResourceManager(resourceManager)`
   - これにより DrawTextDefault系メソッドが日本語フォント使用可能に

## ポイント

### 逆序での初期化は NG

```cpp
// ❌ 間違い：RenderingManagerを先に作成すると参照が未設定
renderingManager->Initialize();
resourceManager->initialize();
renderingManager->SetResourceManager(resourceManager);  // <- 遅い
```

### 正順での初期化は OK

```cpp
// ✅ 正しい：依存関係の順序を守る
resourceManager->initialize();
renderingManager->Initialize();
renderingManager->SetResourceManager(resourceManager);  // <- すぐに設定
```

## メソッド使い分け

| メソッド | 用途 | フォント |
|---------|------|---------|
| `DrawTextDefault()` | 通常のテキスト（日本語対応） | ResourceManager デフォルト |
| `DrawTextDefaultEx()` | 詳細制御テキスト（日本語対応） | ResourceManager デフォルト |
| `DrawTextRaylib()` | Raylib標準フォントのみ | Raylib GetFontDefault() |
| `DrawTextRaylibEx()` | Raylib標準フォント詳細制御 | Raylib GetFontDefault() |
| `DrawTextWithFont()` | 明示的フォント指定 | 指定フォント |
| `DrawTextWithFontEx()` | 明示的フォント詳細指定 | 指定フォント |

### 推奨使用パターン

```cpp
// 日本語テキスト描画（推奨）
renderingManager.DrawTextDefault("ゲームタイトル", 960, 100, 48, WHITE);

// 英数字のみの場合はRaylib版も可
renderingManager.DrawTextRaylib("Score: 1000", 100, 50, 24, WHITE);

// 特定フォントを明示的に指定
Font* customFont = static_cast<Font*>(resourceManager.getFont("CustomFont.ttf"));
renderingManager.DrawTextWithFont(customFont, "Custom Text", 500, 300, 32, YELLOW);
```

## フォント変更時の動作

ResourceManager::setDefaultFont() で別フォントに切り替えた場合、RenderingManager は自動的に新フォントを使用します。

```cpp
// フォント変更時の自動反映例
resourceManager->setDefaultFont("NewFont.ttf", 32);
// 次の描画フレームから新フォント使用
renderingManager->DrawTextDefault("日本語テキスト", 960, 540, 32, WHITE);
```

**理由**：RenderingManager は GetDefaultFont() を通じてポインタ参照で取得しているため、ResourceManager 側の defaultFont_ 変更が自動的に反映されます。

## 推奨実装パターン

### ServiceContainer::create() 内での初期化順序

```cpp
std::unique_ptr<ServiceContainer> ServiceContainer::create() {
    // ステップ1: ECSレジストリを作成
    auto registry = std::make_unique<entt::registry>();
    
    // ステップ2: マネージャーを作成
    auto input = std::make_unique<managers::InputManager>();
    auto resource = std::make_unique<managers::ResouceManager>();  // <- 先にResourceManager
    auto rendering = std::make_unique<managers::RenderingManager>();
    auto ui = std::make_unique<managers::UIManager>();
    
    // ステップ3: SceneManagerを作成
    auto scene = std::make_unique<managers::SceneManager>(*registry, *resource);
    
    // ステップ4: ServiceContainerを作成
    auto container = std::unique_ptr<ServiceContainer>(
        new ServiceContainer(
            std::move(input),
            std::move(scene),
            std::move(resource),
            std::move(rendering),
            std::move(ui),
            std::move(registry)
        )
    );
    
    // ステップ5: RenderingManager に ResourceManager 参照を設定
    container->rendering_->SetResourceManager(container->resource_.get());
    
    // ステップ6: SceneManager に親参照を設定
    container->scene_->setServiceContainer(container.get());
    
    return container;
}
```

### InitScene での使用例

```cpp
void InitScene::_render() {
    auto& renderingManager = services_.getRenderingManager();
    
    // 日本語タイトルを描画（NotoSansJP-Medium.ttf 使用）
    const char* title = "ゲームタイトル";
    float titleFontSize = 48.0f;
    renderingManager.DrawTextDefault(title, 960, 100, titleFontSize, WHITE);
    
    // プログレスバー
    renderingManager.DrawProgressBar(760, 500, 400, 30, progress, SKYBLUE, DARKGRAY, WHITE);
    
    // 進捗テキスト
    std::string progressText = "リソース読み込み中... " + std::to_string(progress * 100) + "%";
    renderingManager.DrawTextDefault(progressText, 800, 550, 24, LIGHTGRAY);
}
```

## トラブルシューティング

### "日本語フォントが表示されない"

- [ ] ResourceManager::setDefaultFont() が呼ばれているか確認
- [ ] RenderingManager::SetResourceManager() が呼ばれているか確認
- [ ] DrawTextDefault() を使用しているか（DrawTextRaylib() ではないか）
- [ ] NotoSansJP-Medium.ttf が data/fonts/ に存在するか確認

### "nullptr exception"

- [ ] ResourceManager が有効に初期化されているか確認
- [ ] GetDefaultFont() がnullptrを返していないか確認
- [ ] フォント未設定時はフォールバック処理が動作（Raylibデフォルト使用）

### "テキストが切れる・表示が崩れる"

- [ ] フォントサイズが適切か確認（スケーリング考慮）
- [ ] MeasureTextEx() でテキストサイズを計測して中央配置しているか
- [ ] 内部座標系（1920x1080）で座標指定しているか確認

## データフロー図

```
ServiceContainer::create()
  ├─ ResourceManager 作成
  │  └─ フォントファイル読み込み (data/fonts/NotoSansJP-Medium.ttf)
  ├─ RenderingManager 作成
  │  └─ スケーリング機能初期化
  └─ SetResourceManager() 呼び出し
     └─ RenderingManager が ResourceManager 参照を取得
        └─ DrawTextDefault() メソッドで日本語フォント使用可能

シーン描画時:
  DrawTextDefault() 呼び出し
    ↓
  GetDefaultFont() でフォント取得
    ↓
  ResourceManager::getDefaultFont() から Font* 取得
    ↓
  スケーリング適用（内部座標 → ウィンドウ座標）
    ↓
  Raylib::DrawTextEx() でレンダリング
```

## 参考：コミット履歴

この機能は以下のコミットで実装されました：

- RenderingManager のフォント描画機能拡張
- DrawTextDefault/DrawTextRaylib/DrawTextWithFont メソッド追加
- ServiceContainer での初期化順序制御追加
- InitScene/TitleScene での日本語フォント対応

## 更新履歴

- 2025-12-31: 初版作成（フォント描画機能拡張実装時）
