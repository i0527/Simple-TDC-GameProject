@echo off
REM ============================================================================
REM Simple-TDC-GameProject - Emscripten (Web) Build Script
REM ============================================================================
REM 使用方法: build_web.bat [debug|release] [run]
REM   debug:   デバッグビルド
REM   release: リリースビルド（デフォルト）
REM   run:     ビルド後にローカルサーバーで実行
REM ============================================================================

setlocal enabledelayedexpansion

REM プロジェクトルートディレクトリを取得
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"

set "BUILD_TYPE=Release"
set "RUN_SERVER=0"

REM 引数解析
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="debug" set "BUILD_TYPE=Debug"
if /i "%~1"=="release" set "BUILD_TYPE=Release"
if /i "%~1"=="run" set "RUN_SERVER=1"
shift
goto parse_args
:end_parse

echo ============================================
echo  Cat TD Game - Emscripten (%BUILD_TYPE%) Build
echo ============================================
echo.

REM EMSDK環境変数をチェック
if not defined EMSDK (
    echo [ERROR] EMSDK environment variable not set
    echo.
    echo Please install and activate Emscripten SDK:
    echo   1. Download: https://github.com/emscripten-core/emsdk
    echo   2. Install:  emsdk install latest
    echo   3. Activate: emsdk activate latest
    echo   4. Source:   emsdk_env.bat
    echo.
    echo Or run: call C:\emsdk\emsdk_env.bat
    exit /b 1
)

echo [INFO] EMSDK Path: %EMSDK%

REM emcc確認
where emcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] emcc not found in PATH
    echo Please run: call %EMSDK%\emsdk_env.bat
    exit /b 1
)

REM Emscripten バージョン表示
echo [INFO] Emscripten Version:
emcc --version | findstr /C:"emcc"

REM CMakeが利用可能か確認
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH
    exit /b 1
)

REM ビルドディレクトリ
set "BUILD_DIR=%PROJECT_ROOT%\build_web"

REM CMAKE_TOOLCHAIN_FILE を設定
set "TOOLCHAIN_FILE=%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake"

if not exist "%TOOLCHAIN_FILE%" (
    echo [ERROR] Emscripten toolchain file not found:
    echo   %TOOLCHAIN_FILE%
    exit /b 1
)

echo [INFO] Toolchain File: %TOOLCHAIN_FILE%

REM assets と data フォルダの存在確認
echo.
echo [INFO] Checking asset directories...
if exist "%PROJECT_ROOT%\assets" (
    echo [OK] Assets directory found: %PROJECT_ROOT%\assets
) else (
    echo [WARNING] Assets directory not found
)

if exist "%PROJECT_ROOT%\data" (
    echo [OK] Data directory found: %PROJECT_ROOT%\data
) else (
    echo [WARNING] Data directory not found
)

echo.
echo [INFO] CMake構成中...

REM CMakeを直接呼び出す（プリセットを使用しない）
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" ^
    -DPLATFORM=Web ^
    -B "%BUILD_DIR%" ^
    -S "%PROJECT_ROOT%"

if errorlevel 1 (
    echo [ERROR] CMake構成に失敗しました
    exit /b 1
)

echo.
echo [INFO] ビルド中...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%

if errorlevel 1 (
    echo [ERROR] ビルドに失敗しました
    exit /b 1
)

echo.
echo ============================================
echo [SUCCESS] Emscripten ビルド完了!
echo ============================================
echo 出力: %BUILD_DIR%\game\CatTDGame.html
echo.
echo [INFO] 生成されたファイル:
if exist "%BUILD_DIR%\game\CatTDGame.html" (
    echo   - CatTDGame.html
)
if exist "%BUILD_DIR%\game\CatTDGame.js" (
    echo   - CatTDGame.js
)
if exist "%BUILD_DIR%\game\CatTDGame.wasm" (
    echo   - CatTDGame.wasm
)
if exist "%BUILD_DIR%\game\CatTDGame.data" (
    echo   - CatTDGame.data ^(preloaded assets^)
) else (
    echo   [WARNING] CatTDGame.data not found - assets may not be preloaded
)
echo.

REM ローカルサーバー起動（オプション）
if %RUN_SERVER%==1 (
    echo [INFO] ローカルサーバーを起動中...
    echo [INFO] ブラウザで http://localhost:8000/game/CatTDGame.html を開いてください
    echo [INFO] サーバーを停止するには Ctrl+C を押してください
    echo.
    echo [警告] file:// プロトコルでは開かないでください！
    echo [警告] 必ず http://localhost:8000/game/CatTDGame.html を使用してください
    echo.
    
    REM ブラウザを自動で開く（オプション）
    start http://localhost:8000/game/CatTDGame.html
    
    cd /d "%BUILD_DIR%"
    python -m http.server 8000
)

endlocal
exit /b 0
