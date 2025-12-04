# WebUI エディター接続問題 - 根本原因

## 🔴 致命的問題: Node.js がインストールされていない

### 症状

```
npm : 用語 'npm' は、コマンドレット...として認識されません
node : 用語 'node' は...認識されません
```

### 解決方法

#### オプション1: Node.js を公式サイトからダウンロード（推奨）

1. **Node.js 公式サイトにアクセス**:
   - <https://nodejs.org/>

2. **LTS版（推奨）をダウンロード**:
   - Windows インストーラー (.msi)
   - v18.x または v20.x を選択

3. **インストール実行**:
   - デフォルト設定で進める
   - 「Add to PATH」オプションが有効か確認

4. **PowerShell / CMD を再起動**:
   - インストール後、新しいターミナルを開く

5. **確認**:

   ```bash
   node --version
   npm --version
   ```

#### オプション2: Chocolatey を使用（高速）

PowerShell (管理者権限) で：

```powershell
choco install nodejs
```

#### オプション3: Windows Package Manager を使用

```powershell
winget install OpenJS.NodeJS
```

---

## インストール後の確認

### Step 1: Node.js とnpm がインストールされたことを確認

新しいターミナルで：

```bash
node --version    # v18.0.0以上
npm --version     # 9.0.0以上
```

### Step 2: WebUI エディターを起動

```bash
cd z:\Simple-TDC-GameProject\tools\webui-editor
npm install
npm run dev
```

**期待される出力**:

```
added 150+ packages in 45s

  VITE v5.0.0  ready in 156 ms

  ➜  Local:   http://localhost:3000/
  ➜  press h to show help
```

### Step 3: ブラウザアクセス

```
http://localhost:3000
```

---

## 次のステップ

1. **Node.js をインストール** (5分)
2. **WebUI サーバーを起動** (3分)
3. **ブラウザで確認** (1分)

WebUI エディターが起動したら、詳細なデバッグガイドは `.cursor/WEBUI_DEBUGGING_GUIDE.md` を参照してください。
