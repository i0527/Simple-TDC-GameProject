@echo off
REM ============================================================================
REM Simple-TDC-GameProject - Release Build Script (VS2026 Build Tools)
REM ============================================================================
REM 使用方法: build_release.bat [clean]
REM   clean: ビルドディレクトリをクリーンしてから再構築
REM ============================================================================

setlocal enabledelayedexpansion

REM プロジェクトルートディレクトリを取得
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"

echo ============================================
echo  Cat TD Game - Release Build
echo ============================================
echo.

REM VS2026 Build Tools の環境変数を設定
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe が見つかりません
    echo Visual Studio 2026 または Build Tools をインストールしてください
    exit /b 1
)

REM VS2026のインストールパスを取得（バージョン19.0以上を優先、見つからない場合は最新版）
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -version "[19.0,20.0)" -latest -property installationPath`) do (
    set "VS_PATH=%%i"
)

REM VS2026が見つからない場合は最新版を試す
if not defined VS_PATH (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if not defined VS_PATH (
    echo [ERROR] Visual Studio 2026 が見つかりません
    exit /b 1
)

echo [INFO] Visual Studio Path: %VS_PATH%

REM Developer Command Prompt 環境をロード
set "VCVARSALL=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%VCVARSALL%" (
    echo [ERROR] vcvarsall.bat が見つかりません
    exit /b 1
)

echo [INFO] 開発環境を初期化中...
call "%VCVARSALL%" x64 >nul 2>&1

REM CMakeが利用可能か確認
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake が見つかりません
    exit /b 1
)

REM Ninjaが利用可能か確認
where ninja >nul 2>&1
if errorlevel 1 (
    set "CMAKE_GENERATOR=Visual Studio 19 2026"
    set "CMAKE_ARGS=-A x64"
) else (
    set "CMAKE_GENERATOR=Ninja"
    set "CMAKE_ARGS="
)

REM ビルドディレクトリ
set "BUILD_DIR=%PROJECT_ROOT%\build_release"

REM cleanオプションの処理
if /i "%1"=="clean" (
    echo [INFO] ビルドディレクトリをクリーン中...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo.
echo [INFO] CMake構成中...
cmake -G "%CMAKE_GENERATOR%" %CMAKE_ARGS% -DCMAKE_BUILD_TYPE=Release -B "%BUILD_DIR%" -S "%PROJECT_ROOT%"

if errorlevel 1 (
    echo [ERROR] CMake構成に失敗しました
    exit /b 1
)

echo.
echo [INFO] ビルド中...
cmake --build "%BUILD_DIR%" --config Release --parallel

if errorlevel 1 (
    echo [ERROR] ビルドに失敗しました
    exit /b 1
)

echo.
echo ============================================
echo [SUCCESS] Release ビルド完了!
echo ============================================
echo 実行ファイル: %BUILD_DIR%\game\CatTDGame.exe
echo.

endlocal
exit /b 0
