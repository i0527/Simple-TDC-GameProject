cmake_minimum_required(VERSION 3.19)
project(SimpleTDCGame VERSION 0.1.0 LANGUAGES CXX C)

# ============================================================================
# Windows限定設定
# ============================================================================
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only. Please build on Windows.")
endif()

# ============================================================================
# コンパイラ設定 (MSVC)
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
    # Windows API定義
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    # 警告レベル設定
    add_compile_options(/W4)
    # エラーとして扱う警告
    add_compile_options(/WX-)
    
    message(STATUS "Compiler: MSVC ${MSVC_VERSION}")
    message(STATUS "C++ Standard: C++17")
else()
    message(WARNING "MSVC compiler is recommended. Other compilers may not work correctly.")
endif()

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
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

# raygui (immediate mode GUI library for raylib)
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
)

# Dear ImGui
FetchContent_Declare(
    dearimgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)

# rlImGui (Dear ImGui integration for raylib)
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG 57efef06f69b50ed9ac0834a963a8aefcc59d588
)

# 依存ライブラリを取得
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
# ソースファイル収集
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# 除外ファイル（別ターゲット用）
list(FILTER SOURCES EXCLUDE REGEX "src/Roguelike/.*")
list(FILTER SOURCES EXCLUDE REGEX "src/main_roguelike\\.cpp")
list(FILTER SOURCES EXCLUDE REGEX "src/main_new\\.cpp")
list(FILTER HEADERS EXCLUDE REGEX "include/Roguelike/.*")

# 実行ファイル作成
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# TestScene.cppを明示的に追加（GLOBで拾えない場合の保険）
target_sources(${PROJECT_NAME} PRIVATE src/TestScene.cpp)

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
# ============================================================================
file(GLOB_RECURSE ROGUELIKE_SOURCES "src/Roguelike/*.cpp")

set(NETHACK_SOURCES
    src/main_roguelike.cpp
    ${ROGUELIKE_SOURCES}
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
