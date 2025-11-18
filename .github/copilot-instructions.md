# Copilot コーディング規約

## プロジェクト概要
Simple-TDC-GameProjectは、ECSアーキテクチャを採用したC++ゲームプロジェクトです。

## 技術スタック
- **言語**: C++17以上
- **ビルドシステム**: CMake
- **ECS**: EnTT
- **JSON**: nlohmann/json
- **レンダリング/入力**: Raylib

## コーディング規約

### ECSコンポーネント
- コンポーネントはシンプルなデータ構造（POD型）として定義する
- ロジックはコンポーネントではなくシステムに実装する
- 例:
```cpp
struct Position {
    float x;
    float y;
};
```

### JSON解析
- JSON解析は必ずtry-catchブロックで囲む
- parse_errorと一般的なexceptionの両方をキャッチする
- エラー時にはデフォルト値で継続できるようにする
- 例:
```cpp
try {
    json config = json::parse(file);
    // 処理
} catch (const json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### 命名規則
- クラス名: PascalCase (例: `GameManager`)
- 関数名: PascalCase (例: `UpdatePosition`)
- 変数名: camelCase、プライベートメンバーは末尾にアンダースコア (例: `playerSpeed`, `registry_`)
- 定数: UPPER_CASE (例: `MAX_ENTITIES`)
- 名前空間: PascalCase (例: `Components`, `Systems`)

### ファイル構成
- ヘッダーファイル: `include/`ディレクトリ
- ソースファイル: `src/`ディレクトリ
- 外部ライブラリ: `external/`ディレクトリ（FetchContentで管理）
- アセット: `assets/`ディレクトリ

### コミット規約
- 1コミット = 1機能単位
- コミットメッセージは日本語または英語で明確に
- プレフィックスを使用:
  - `feat:` 新機能
  - `fix:` バグ修正
  - `refactor:` リファクタリング
  - `docs:` ドキュメント更新
  - `chore:` ビルド・設定変更

### エラーハンドリング
- ファイル操作は必ずエラーチェックを行う
- リソースの読み込み失敗時は適切にフォールバックする
- ログ出力は`std::cerr`を使用

### メモリ管理
- 可能な限りスマートポインタ（`std::unique_ptr`, `std::shared_ptr`）を使用
- RAIIパターンに従う

### コメント
- 複雑なロジックには説明コメントを追加
- パブリックAPIにはドキュメンテーションコメントを記載
- 日本語または英語

## ブランチ戦略
- `main`: リリース可能な安定版
- `develop`: 開発中の最新版
- `feature/*`: 新機能開発
- `bugfix/*`: バグ修正
- `hotfix/*`: 緊急修正
