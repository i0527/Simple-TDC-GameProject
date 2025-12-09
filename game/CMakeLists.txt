cmake_minimum_required(VERSION 3.19)

# Game Executable
project(SimpleTDCGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 依存関係
include(FetchContent)

# ImGui（rlImGui互換の v1.89.9 に固定） - 既存ターゲットが無ければ定義
if(NOT TARGET imgui_lib)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )

    FetchContent_MakeAvailable(imgui)

    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )

    target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})
endif()

# rlImGui
if(NOT TARGET rlimgui_lib)
    FetchContent_Declare(
        rlimgui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
        GIT_TAG main
    )

    FetchContent_GetProperties(rlimgui)
    if(NOT rlimgui_POPULATED)
        FetchContent_Populate(rlimgui)
    endif()

    add_library(rlimgui_lib STATIC
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    )

    target_include_directories(rlimgui_lib PUBLIC
        ${rlimgui_SOURCE_DIR}
    )

    target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)
    target_compile_definitions(rlimgui_lib PUBLIC IMGUI_ENABLE_DOCKING IMGUI_ENABLE_VIEWPORTS)
endif()

# ソースファイル
set(GAME_SOURCES
    src/main_game.cpp
    src/Game/Application/GameApp.cpp
    src/Game/Systems/MovementSystem.cpp
    src/Game/Systems/AttackSystem.cpp
    src/Game/Systems/SkillSystem.cpp
    src/Game/Systems/RenderingSystem.cpp
    src/Game/Scenes/SceneManager.cpp
    src/Game/Scenes/LoadingScene.cpp
    src/Game/Scenes/TitleScene.cpp
    src/Game/Scenes/HomeScene.cpp
    src/Game/Scenes/FormationScene.cpp
    src/Game/Scenes/StageSelectScene.cpp
    src/Game/Scenes/TDGameScene.cpp
    src/Game/UI/SettingsPanel.cpp
    src/Game/Managers/EntityManager.cpp
    src/Game/Managers/SkillManager.cpp
    src/Game/Managers/StageManager.cpp
    src/Game/Managers/FormationManager.cpp
)

# 実行ファイルを作成
add_executable(SimpleTDCGame ${GAME_SOURCES})

# インクルードディレクトリ
target_include_directories(SimpleTDCGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.12.2
)

FetchContent_MakeAvailable(entt)

# Shared Layer とリンク
target_link_libraries(SimpleTDCGame
    PRIVATE
        SimpleTDC_Shared
        EnTT::EnTT
        raylib
        imgui_lib
        rlimgui_lib
)

# コンパイラ警告
if(MSVC)
    target_compile_options(SimpleTDCGame PRIVATE /W4 /utf-8)
else()
    target_compile_options(SimpleTDCGame PRIVATE -Wall -Wextra -Wpedantic)
endif()

# assets ディレクトリをコピー
add_custom_command(TARGET SimpleTDCGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SimpleTDCGame>/assets
)

message(STATUS "Game Executable configured")
