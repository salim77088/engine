cmake_minimum_required(VERSION 3.16)
project(LuminusEngine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform-specific definitions
if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(APPLE)
    add_compile_options(-Wno-deprecated-declarations -Wno-unused-result)
else()
    add_compile_options(-Wno-unused-result)
endif()

# ============================================================================
# Fetch Cocos2d-x v4 from our fork (MIT-licensed)
# ============================================================================
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Use shallow clone to save bandwidth
set(FETCHCONTENT_GIT_REMOTE TRUE)

FetchContent_Declare(
    cocos2d_x
    GIT_REPOSITORY https://github.com/salim77088/cocos2d-x.git
    GIT_TAG        v4
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

# Don't build cocos2d-x tests/examples (saves time)
set(BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_LUA_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TEMPLATE_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_ENGINE_DONE ON CACHE BOOL "" FORCE)
# Don't build cocos2d-x's own external extensions we don't need
set(BUILD_EDITOR_COCOS_SUPPORT OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cocos2d_x)

# ============================================================================
# Fetch Dear ImGui for the visual editor
# ============================================================================
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ============================================================================
# Fetch nlohmann/json
# ============================================================================
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

# ============================================================================
# Luminus Engine wrapper sources
# ============================================================================
set(LUMINUS_SOURCES
    src/main.cpp
    src/core/Engine.cpp
    src/core/SceneManager.cpp
    src/graphics/GraphicsAdapter.cpp
    src/scripting/LuminusScript.cpp
    src/scripting/Lexer.cpp
    src/scripting/Parser.cpp
    src/scripting/Interpreter.cpp
    src/editor/Editor.cpp
    src/editor/ImGuiBackend.cpp
    src/audio/AudioSystem.cpp
    src/assets/AssetManager.cpp
)

add_executable(luminus ${LUMINUS_SOURCES})

target_include_directories(luminus PRIVATE
    src
    ${cocos2d_x_SOURCE_DIR}/cocos
    ${cocos2d_x_SOURCE_DIR}/cocos/editor-support
    ${cocos2d_x_SOURCE_DIR}/extensions
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(luminus PRIVATE
    cocos2d_x
    nlohmann_json::nlohmann_json
)

# ImGui sources (compiled as part of luminus)
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_sources(luminus PRIVATE ${IMGUI_SOURCES})

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

# Post-build: copy assets
add_custom_command(TARGET luminus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:luminus>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/examples
        $<TARGET_FILE_DIR:luminus>/examples
)

message(STATUS "==============================================")
message(STATUS "  Luminus Engine v1.0.0 (Cocos2d-x based)")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "==============================================")
