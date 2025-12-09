cmake_minimum_required(VERSION 3.19)

# Game Executable
project(SimpleTDCGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/Game/Managers/EntityManager.cpp
    src/Game/Managers/SkillManager.cpp
    src/Game/Managers/StageManager.cpp
)

# 実行ファイルを作成
add_executable(SimpleTDCGame ${GAME_SOURCES})

# インクルードディレクトリ
target_include_directories(SimpleTDCGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# EnTT の依存関係
include(FetchContent)

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
