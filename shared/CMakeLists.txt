cmake_minimum_required(VERSION 3.19)

# Shared Layer - 静的ライブラリ
project(SimpleTDC_Shared LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ソースファイル
set(SHARED_SOURCES
    # Core
    src/Core/GameContext.cpp
    src/Core/EventSystem.cpp
    src/Core/FileWatcher.cpp
    src/Core/FontManager.cpp
    src/Core/SettingsManager.cpp

    # Data
    src/Data/DefinitionRegistry.cpp
    src/Data/Loaders/EntityLoader.cpp
    src/Data/Loaders/SkillLoader.cpp
    src/Data/Loaders/StageLoader.cpp
    src/Data/Loaders/WaveLoader.cpp
    src/Data/Loaders/AbilityLoader.cpp
    src/Data/Loaders/SpriteSheetLoader.cpp
    src/Data/UserDataManager.cpp
    src/Data/Validators/DataValidator.cpp
)

# 静的ライブラリを作成
add_library(SimpleTDC_Shared STATIC ${SHARED_SOURCES})

# インクルードディレクトリ
target_include_directories(SimpleTDC_Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# nlohmann/json とグラフィックス依存関係
include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

if(NOT TARGET raylib)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif()

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
    target_compile_definitions(imgui_lib PUBLIC IMGUI_ENABLE_DOCKING IMGUI_ENABLE_VIEWPORTS)
endif()

FetchContent_MakeAvailable(json)

target_link_libraries(SimpleTDC_Shared
    PUBLIC
        nlohmann_json::nlohmann_json
        raylib
        imgui_lib
)

# コンパイラ警告
if(MSVC)
    target_compile_options(SimpleTDC_Shared PRIVATE /W4 /utf-8)
else()
    target_compile_options(SimpleTDC_Shared PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Shared Layer configured")
