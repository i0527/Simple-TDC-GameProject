cmake_minimum_required(VERSION 3.19)

# RC.exe を明示的に検出してから project() を呼び出す
find_program(RC_EXECUTABLE rc PATHS ENV PATH)
if(NOT RC_EXECUTABLE)
    # Visual Studio に内蔵の RC.exe を探す
    find_program(RC_EXECUTABLE rc
        PATHS "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.50.35717/bin/Hostx64/x64"
    )
endif()

if(RC_EXECUTABLE)
    message(STATUS "Found RC.exe: ${RC_EXECUTABLE}")
    set(CMAKE_RC_COMPILER ${RC_EXECUTABLE})
else()
    message(WARNING "RC.exe not found; using default")
endif()

project(SimpleTDCGame VERSION 0.1.0 LANGUAGES CXX C RC)

# ============================================================================
# Ninja自動セットアップ (Windows)
# ============================================================================
if(WIN32 AND NOT DEFINED ENV{NINJA_SKIP_AUTO_SETUP})
    # Ninjaがシステムにインストールされているかチェック
    find_program(NINJA_EXECUTABLE ninja PATHS ENV PATH NO_DEFAULT_PATH)
    
    if(NOT NINJA_EXECUTABLE)
        # プロジェクトローカルのNinjaパスをチェック
        set(LOCAL_NINJA_PATH "${CMAKE_SOURCE_DIR}/.tools/bin/ninja.exe")
        
        if(NOT EXISTS ${LOCAL_NINJA_PATH})
            # bootstrap-ninja.ps1を実行してNinjaをダウンロード
            message(STATUS "Ninja not found. Downloading Ninja to .tools/bin/...")
            
            execute_process(
                COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/scripts/bootstrap-ninja.ps1"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE NINJA_DOWNLOAD_RESULT
                OUTPUT_VARIABLE NINJA_DOWNLOAD_OUTPUT
                ERROR_VARIABLE NINJA_DOWNLOAD_ERROR
            )
            
            if(NOT NINJA_DOWNLOAD_RESULT EQUAL 0)
                message(WARNING "Failed to download Ninja automatically: ${NINJA_DOWNLOAD_ERROR}")
                message(WARNING "You can manually run: .\\scripts\\bootstrap-ninja.ps1")
            else()
                message(STATUS "Ninja downloaded successfully to ${LOCAL_NINJA_PATH}")
            endif()
        endif()
        
        # PATHに追加（CMake内部でのみ有効）
        if(EXISTS ${LOCAL_NINJA_PATH})
            get_filename_component(NINJA_DIR ${LOCAL_NINJA_PATH} DIRECTORY)
            set(ENV{PATH} "${NINJA_DIR};$ENV{PATH}")
            message(STATUS "Added ${NINJA_DIR} to PATH for this CMake session")
        endif()
    else()
        message(STATUS "Using system Ninja: ${NINJA_EXECUTABLE}")
    endif()
endif()

# ============================================================================
# Windows限定設定
# ============================================================================
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only. Please build on Windows.")
endif()

# ============================================================================
# コンパイラ設定 (MSVC/MinGW/Clang)
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC固有の設定
if(MSVC)
    # UTF-8エンコーディング対応
    add_compile_options(/utf-8)
    # 大きなオブジェクトファイル対応
    add_compile_options(/bigobj)
    # 並列コンパイル時のファイルロック回避
    add_compile_options(/FS)
    # 並列コンパイル
    add_compile_options(/MP)
    # Windows API定義
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    # 警告レベル設定
    add_compile_options(/W4)
    # 警告をエラーとして扱わない（/WXの無効化）
    add_compile_options(/WX-)
    # マニフェスト生成を無効化（RC.exe エラー対応）
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /MANIFEST:NO")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " /MANIFEST:NO")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " /MANIFEST:NO")
    
    message(STATUS "Compiler: MSVC ${MSVC_VERSION}")
else()
    # MinGW/Clang on Windows
    # Note: This block only runs on Windows (due to WIN32 check above)
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-fexec-charset=UTF-8)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")

# ============================================================================
# ビルド設定
# ============================================================================
# デフォルトビルドタイプ（Ninja使用時）
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 出力ディレクトリ
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# 依存ライブラリ管理 (FetchContent)
# ============================================================================
include(FetchContent)

# EnTT (ECS library)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.12.2
    GIT_SHALLOW TRUE
)

# nlohmann/json (using newer version compatible with modern CMake)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

# Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

# raygui (immediate mode GUI library for raylib)
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW TRUE
)

# Dear ImGui
FetchContent_Declare(
    dearimgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)

# rlImGui (Dear ImGui integration for raylib)
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG 57efef06f69b50ed9ac0834a963a8aefcc59d588
)

# 依存ライブラリを取得
message(STATUS "Fetching dependencies (this may take a while on first run)...")
FetchContent_MakeAvailable(EnTT json raylib raygui dearimgui rlimgui)

# ============================================================================
# ImGuiライブラリの構築
# ============================================================================
# Dear ImGui static library
add_library(imgui_lib STATIC
    ${dearimgui_SOURCE_DIR}/imgui.cpp
    ${dearimgui_SOURCE_DIR}/imgui_demo.cpp
    ${dearimgui_SOURCE_DIR}/imgui_draw.cpp
    ${dearimgui_SOURCE_DIR}/imgui_tables.cpp
    ${dearimgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui_lib PUBLIC ${dearimgui_SOURCE_DIR})

# rlImGui static library
add_library(rlimgui_lib STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui_lib PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)

# ============================================================================
# 新アーキテクチャ: SimpleTDCGame_NewArch
# - 新階層（include/, src/）に昇格済みのコードのみを使用
# - 最小限のウィンドウとレンダラーを構築
# ============================================================================
set(NEW_ARCH_SOURCES
    src/main_new.cpp
    src/Core/GameContext.cpp
    src/Core/InputManager.cpp
    src/Core/ResourceManager.cpp
    src/Core/World.cpp
    src/Core/GameRenderer.cpp
    src/Core/HotReloadApplier.cpp
    src/Core/WorkspaceSettings.cpp
    src/Data/DefinitionRegistry.cpp
    src/Data/Loaders/DataLoaderBase.cpp
    src/Data/Loaders/EntityLoader.cpp
    src/Data/Loaders/WaveLoader.cpp
    src/Data/Loaders/AbilityLoader.cpp
    src/Data/Loaders/StageLoader.cpp
    src/Data/Loaders/DeckLoader.cpp
    src/Game/Editor/HotReload/HotReloadSystem.cpp
    src/Data/Validators/SchemaValidator.cpp
    src/Data/Validators/ReferenceValidator.cpp
    src/Data/Validators/MapValidator.cpp
    src/new/Game/Systems/RenderSystem.cpp
    src/new/Game/Systems/HudSystem.cpp
    src/new/Game/Systems/CostSystem.cpp
    src/new/Game/Systems/PlacementSystem.cpp
    src/new/Domain/TD/LineTD/Systems/LaneMovementSystem.cpp
    src/new/Domain/TD/LineTD/Systems/SimpleCombatSystem.cpp
    src/new/Domain/TD/LineTD/Systems/LineTDSpawnSystem.cpp
    src/new/Domain/TD/LineTD/Systems/CostSystem.cpp
    src/new/Domain/TD/LineTD/Systems/PlacementSystem.cpp
    src/new/Domain/TD/LineTD/Systems/DeathCheckSystem.cpp
    src/new/Domain/TD/LineTD/Systems/WaveSchedulerSystem.cpp
    src/new/Domain/TD/LineTD/Systems/BaseArrivalSystem.cpp
    src/new/Domain/TD/LineTD/Systems/CombatResolveSystem.cpp
    src/new/Domain/TD/LineTD/Systems/FrontlineSystem.cpp
    src/new/Domain/TD/LineTD/Managers/WaveManager.cpp
)

add_executable(SimpleTDCGame_NewArch ${NEW_ARCH_SOURCES})

target_include_directories(SimpleTDCGame_NewArch
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raygui_SOURCE_DIR}/src
        ${dearimgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

target_link_libraries(SimpleTDCGame_NewArch
    PRIVATE
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        raylib
        rlimgui_lib
        imgui_lib
)

add_custom_command(TARGET SimpleTDCGame_NewArch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SimpleTDCGame_NewArch>/assets
    COMMENT "Copying assets to build directory for NewArch..."
)

# ============================================================================
# ビルド情報表示
# ============================================================================
message(STATUS "")
message(STATUS "==========================================")
message(STATUS "Build Configuration")
message(STATUS "==========================================")
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "==========================================")
message(STATUS "")
