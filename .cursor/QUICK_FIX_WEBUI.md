# WebUI エディター接続エラー - 緊急ガイド

## 🚨 症状

```
ERR_CONNECTION_REFUSED
localhost 接続が拒否されました
このページに到達できません
```

---

## ⚡ クイックフィックス（2分）

### 原因

**Node.js がインストールされていない** または **WebUI サーバーが起動していない**

### 解決

#### ステップ1: Node.js をインストール

```
https://nodejs.org/ にアクセス
→ LTS版をダウンロード
→ インストール実行
→ ✅ 「Add to PATH」を有効にする
→ VSCode を再起動
```

#### ステップ2: WebUI サーバーを起動

**方法A: PowerShell スクリプト（最も簡単）**

VSCode で新しいターミナルを開き：

```powershell
# これを実行
z:\Simple-TDC-GameProject\tools\webui-editor\setup-and-run.ps1
```

**方法B: 手動実行**

```bash
cd z:\Simple-TDC-GameProject\tools\webui-editor
npm install
npm run dev
```

**期待される出力**:

```
  ➜  Local:   http://localhost:3000/
  ➜  press h to show help
```

#### ステップ3: ブラウザで確認

```
http://localhost:3000
```

---

## ✅ 完全ガイド

詳細は以下を参照：

- 📖 **完全ガイド**: `.cursor/VSCODE_WEBUI_DEBUG_COMPLETE_GUIDE.md`
- 🔧 **デバッグガイド**: `.cursor/WEBUI_DEBUGGING_GUIDE.md`
- 📝 **Node.js 設定**: `.cursor/NODE_JS_INSTALLATION_REQUIRED.md`

---

## 💡 VSCode タスク使用

```
Ctrl+Shift+B → "WebUI: Start Dev Server (npm run dev)"
```

---

## 🆘 うまくいかない場合

以下のコマンドで診断：

```bash
# Node.js 確認
node --version    # v18以上必須
npm --version     # 9以上必須

# WebUI 状態確認
cd z:\Simple-TDC-GameProject\tools\webui-editor
npm list react    # React がインストール済みか確認

# ポート確認
netstat -ano | findstr ":3000"
netstat -ano | findstr ":8080"
```

**セットアップが正常に完了したら、WebUI エディターが起動します！**
