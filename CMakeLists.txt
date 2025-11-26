cmake_minimum_required(VERSION 3.15)
project(SimpleTDCGame VERSION 0.1.0 LANGUAGES CXX C)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add UTF-8 support for MSVC
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Include FetchContent for dependency management
include(FetchContent)

# Fetch EnTT (ECS library)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.12.2
)

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# Fetch Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

# Fetch raygui (immediate mode GUI library for raylib)
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
)

# Fetch Dear ImGui
FetchContent_Declare(
    dearimgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)

# Fetch rlImGui (Dear ImGui integration for raylib)
# Using full commit hash for better reproducibility (compatible with ImGui v1.90.1)
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG 57efef06f69b50ed9ac0834a963a8aefcc59d588
)

# Make dependencies available
FetchContent_MakeAvailable(EnTT json raylib raygui dearimgui rlimgui)

# raygui is header-only and will be implemented in UIManager.cpp
# We only need to set up the include directory
# (RAYGUI_IMPLEMENTATION is defined in UIManager.cpp)

# Create Dear ImGui library
add_library(imgui_lib STATIC
    ${dearimgui_SOURCE_DIR}/imgui.cpp
    ${dearimgui_SOURCE_DIR}/imgui_demo.cpp
    ${dearimgui_SOURCE_DIR}/imgui_draw.cpp
    ${dearimgui_SOURCE_DIR}/imgui_tables.cpp
    ${dearimgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui_lib PUBLIC ${dearimgui_SOURCE_DIR})

# Create rlImGui library
add_library(rlimgui_lib STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui_lib PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# TestScene.cpp �𖾎��I�ɒǉ��i������GLOB�ŔF������Ȃ��\���X�V���̕ی��j
target_sources(${PROJECT_NAME} PRIVATE src/TestScene.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${raygui_SOURCE_DIR}/src
        ${dearimgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        raylib
        rlimgui_lib
        imgui_lib
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets 
     DESTINATION ${CMAKE_BINARY_DIR}/bin)
