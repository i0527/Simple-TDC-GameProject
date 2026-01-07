@echo off
REM ============================================================================
REM Simple-TDC-GameProject - ゲーム実行スクリプト
REM ============================================================================
REM 使用方法: run_game.bat [debug|release]
REM   debug:   デバッグビルドを実行（デフォルト）
REM   release: リリースビルドを実行
REM ============================================================================

setlocal

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."

set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=debug"

if /i "%BUILD_TYPE%"=="release" (
    set "BUILD_DIR=%PROJECT_ROOT%\build_release"
) else (
    set "BUILD_DIR=%PROJECT_ROOT%\build_cli"
)

set "EXE_PATH=%BUILD_DIR%\game\CatTDGame.exe"

if not exist "%EXE_PATH%" (
    echo [ERROR] 実行ファイルが見つかりません: %EXE_PATH%
    echo まず build_debug.bat または build_release.bat を実行してください
    exit /b 1
)

echo [INFO] ゲームを起動中...
echo 実行ファイル: %EXE_PATH%
echo.

cd /d "%BUILD_DIR%\game"
"%EXE_PATH%"

endlocal
