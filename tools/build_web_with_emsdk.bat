@echo off
REM ============================================================================
REM Simple-TDC-GameProject - Emscripten (Web) Build Script with EMSDK Auto-activation
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

REM EMSDK自動アクティベート
if not defined EMSDK (
    echo [INFO] EMSDK not activated. Attempting to activate...
    if exist "C:\emsdk\emsdk_env.bat" (
        echo [INFO] Found EMSDK at C:\emsdk
        call C:\emsdk\emsdk_env.bat
    ) else (
        echo [ERROR] EMSDK not found. Please install Emscripten SDK.
        echo.
        echo Download: https://github.com/emscripten-core/emsdk
        echo Install to: C:\emsdk
        exit /b 1
    )
)

echo [INFO] EMSDK Path: %EMSDK%

REM emcc確認
where emcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] emcc not found in PATH
    exit /b 1
)

REM Emscripten バージョン表示
echo [INFO] Emscripten Version:
emcc --version 2>nul | findstr /C:"emcc"

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
echo [INFO] CMake configuration...

REM CMakeを直接呼び出す
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" ^
    -DPLATFORM=Web ^
    -B "%BUILD_DIR%" ^
    -S "%PROJECT_ROOT%"

if errorlevel 1 (
    echo [ERROR] CMake configuration failed
    exit /b 1
)

echo.
echo [INFO] Building...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%

if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo ============================================
echo [SUCCESS] Emscripten build completed!
echo ============================================
echo Output: %BUILD_DIR%\game\CatTDGame.html
echo.
echo [INFO] Generated files:
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
    for %%A in ("%BUILD_DIR%\game\CatTDGame.data") do (
        set /a "filesize=%%~zA/1024/1024"
        echo     Size: !filesize! MB
    )
) else (
    echo   [WARNING] CatTDGame.data not found - assets may not be preloaded
)
echo.

REM ローカルサーバー起動（オプション）
if %RUN_SERVER%==1 (
    echo [INFO] Starting local server...
    echo [INFO] Open http://localhost:8000/game/CatTDGame.html in your browser
    echo [INFO] Press Ctrl+C to stop the server
    echo.
    cd /d "%BUILD_DIR%"
    python -m http.server 8000
)

endlocal
exit /b 0
