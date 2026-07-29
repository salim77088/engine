cmake_minimum_required(VERSION 3.16)
project(LuminusEngine VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# Platform configuration
# ============================================================================
if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_options(-Wno-deprecated-declarations -Wno-unused-result)
    add_compile_definitions(PLATFORM_MACOS)
else()
    add_compile_options(-Wno-unused-result -Wno-unused-variable -Wno-unused-but-set-variable)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# ============================================================================
# Dependencies via FetchContent
# ============================================================================
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# --- Raylib 5.5 (Zlib license) - foundation: windowing, input, audio, basic rendering
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(raylib)

# --- raygui 4.0 (MIT-like) - immediate-mode GUI for editor
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        4.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_GetProperties(raygui)
if(NOT raygui_POPULATED)
    FetchContent_Populate(raygui)
endif()
# raygui is a single-header library; expose its src dir
set(RAYGUI_INCLUDE_DIR ${raygui_SOURCE_DIR}/src)

# Compile raygui as a static library using our wrapper .c file
add_library(raygui STATIC src/editor/raygui_impl.c)
target_include_directories(raygui PUBLIC ${RAYGUI_INCLUDE_DIR} src/editor)
target_link_libraries(raygui PUBLIC raylib)

# --- EnTT (MIT license) - modern ECS, header-only
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(entt)

# --- nlohmann/json (MIT license) - serialization
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

# --- stb image (Public Domain) - already included via raylib, but expose for assets
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(stb)

# ============================================================================
# Luminus Engine sources
# ============================================================================
set(LUMINUS_SOURCES
    src/main.cpp
    src/core/Engine.cpp
    src/core/SceneManager.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Time.cpp
    src/graphics/Renderer.cpp
    src/graphics/Camera.cpp
    src/graphics/Material.cpp
    src/graphics/Mesh.cpp
    src/graphics/Shader.cpp
    src/graphics/PostProcessor.cpp
    src/scripting/Lexer.cpp
    src/scripting/Parser.cpp
    src/scripting/Interpreter.cpp
    src/scripting/LuminusScript.cpp
    src/editor/Editor.cpp
    src/editor/HierarchyPanel.cpp
    src/editor/InspectorPanel.cpp
    src/editor/ConsolePanel.cpp
    src/editor/AssetBrowserPanel.cpp
    src/audio/AudioSystem.cpp
    src/assets/AssetManager.cpp
    src/physics/PhysicsWorld.cpp
    src/contracts/SmartContract.cpp
    src/contracts/ContractVM.cpp
    src/utils/Logger.cpp
    src/utils/FileIO.cpp
)

add_executable(luminus ${LUMINUS_SOURCES})

target_include_directories(luminus PRIVATE
    src
    ${RAYGUI_INCLUDE_DIR}
    ${stb_SOURCE_DIR}
)

target_link_libraries(luminus PRIVATE
    raylib
    raygui
    EnTT::EnTT
    nlohmann_json::nlohmann_json
)

# macOS frameworks
if(APPLE)
    target_link_libraries(luminus PRIVATE
        "-framework OpenGL"
        "-framework OpenAL"
        "-framework IOKit"
        "-framework Cocoa"
        "-framework AppKit"
        "-framework AVFoundation"
        "-framework QuartzCore"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(luminus PRIVATE m pthread dl)
endif()

# Post-build: copy assets + examples
add_custom_command(TARGET luminus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:luminus>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/examples
        $<TARGET_FILE_DIR:luminus>/examples
)

message(STATUS "==============================================")
message(STATUS "  Luminus Engine v2.0.0")
message(STATUS "  Foundation: Raylib 5.5 + EnTT + raygui 4.0")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "==============================================")
