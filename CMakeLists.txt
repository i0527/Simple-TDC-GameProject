cmake_minimum_required(VERSION 3.19)
project(SimpleTDCGame VERSION 0.1.0 LANGUAGES CXX C)

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

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
    GIT_SHALLOW TRUE
)

# Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
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
# メイン実行ファイル: SimpleTDCGame
# ============================================================================
# ソースファイルを明示的に指定（GLOB_RECURSEによる問題を回避）
# 注意: FileUtils.cpp, GameNew.cpp, main_new.cpp, main_roguelike.cppは
#       別ターゲット（SimpleTDCGame_NewArch, NetHackGame）用のため除外
set(MAIN_SOURCES
    src/main.cpp
    src/Game.cpp
    src/ConfigManager.cpp
    src/InputManager.cpp
    src/SceneManager.cpp
    src/SystemManager.cpp
    src/Systems.cpp
    src/ResourceManager.cpp
    src/UIManager.cpp
    src/TestScene.cpp
    src/Scenes/TitleScene.cpp
    src/Scenes/HomeScene.cpp
    src/Scenes/TDGameScene.cpp
    src/Scenes/TDTestGameScene.cpp
    src/Scenes/NethackGameScene.cpp
)

# 実行ファイル作成
add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

# インクルードディレクトリ
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raygui_SOURCE_DIR}/src
        ${dearimgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

# リンクライブラリ
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        raylib
        rlimgui_lib
        imgui_lib
)

# アセットコピー（ビルド後）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to build directory..."
)

# ============================================================================
# 新アーキテクチャテスト用ターゲット: SimpleTDCGame_NewArch
# ============================================================================
set(NEW_ARCH_SOURCES
    src/main_new.cpp
    src/GameNew.cpp
    src/FileUtils.cpp
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
)

add_custom_command(TARGET SimpleTDCGame_NewArch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SimpleTDCGame_NewArch>/assets
    COMMENT "Copying assets to build directory..."
)

# ============================================================================
# NetHack風ローグライク用ターゲット: NetHackGame
# (Roguelikeモジュールはヘッダーオンリーのため、main_roguelike.cppのみ)
# ============================================================================
set(NETHACK_SOURCES
    src/main_roguelike.cpp
)

add_executable(NetHackGame ${NETHACK_SOURCES})

target_include_directories(NetHackGame 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raygui_SOURCE_DIR}/src
        ${dearimgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

target_link_libraries(NetHackGame 
    PRIVATE 
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        raylib
        rlimgui_lib
        imgui_lib
)

add_custom_command(TARGET NetHackGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:NetHackGame>/assets
    COMMENT "Copying assets to build directory..."
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
