cmake_minimum_required(VERSION 3.20)
project(pdg_manga_collage CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Static link FetchContent deps (mirrors ps1-vj-mix; see its CMakeLists for
# the glfw3.dll lesson learned).
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------------
# libvj (in-tree submodule, MIT). Reused for Filter / AutoMode / Preset Bank
# / MIDI / glitch axes — the same vocabulary the mixer uses.
# ---------------------------------------------------------------------------
set(VJ_BUILD_DEMO  OFF CACHE BOOL "" FORCE)
set(VJ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VJ_ENABLE_RTMIDI ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/libvj)

# ---------------------------------------------------------------------------
# FetchContent: GLFW + Dear ImGui
# ---------------------------------------------------------------------------
include(FetchContent)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.0
)
FetchContent_MakeAvailable(imgui)

# stb (header-only): stb_image for PNG/JPG load, stb_image_write for PNG out.
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC OpenGL::GL)

# ---------------------------------------------------------------------------
# App executable (M1: just window + ImGui)
# ---------------------------------------------------------------------------
add_executable(pdg-collage
    src/app/main.cpp
    src/app/panel_editor.cpp
    src/app/stb_impl.cpp
    src/app/gl_loader.cpp
    src/app/renderer.cpp
    src/app/assets.cpp
    src/app/compositor.cpp
    src/app/postfx.cpp
    src/app/exporter.cpp
)
target_link_libraries(pdg-collage PRIVATE vj imgui stb)
target_include_directories(pdg-collage PRIVATE src)

if(MINGW)
    target_link_options(pdg-collage PRIVATE
        -static-libstdc++ -static-libgcc -static)
endif()
