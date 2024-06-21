#
# project: projects
#
cmake_minimum_required(VERSION 2.8)
project(projects)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

set(FIPS_RTTI ON CACHE BOOL "Enable C++ RTTI" FORCE)
LIST(APPEND CMAKE_PROGRAM_PATH  "data" ...)

fips_setup()
if (FIPS_EMSCRIPTEN)
    if (FIPS_EMSCRIPTEN_USE_WEBGPU)
        set(sokol_backend SOKOL_WGPU)
        set(slang "wgsl")
    else()
        set(sokol_backend SOKOL_GLES3)
        set(slang "glsl300es")
    endif()
elseif (FIPS_ANDROID)
    set(sokol_backend SOKOL_GLES3)
    set(slang "glsl300es")
elseif (SOKOL_USE_D3D11)
    set(sokol_backend SOKOL_D3D11)
    set(slang "hlsl5")
elseif (SOKOL_USE_EGL_GLES3)
    set(sokol_backend SOKOL_GLES3)
    set(slang "glsl300es")
else()
    add_definitions(-DSOKOL_EXTERNAL_GL_LOADER)
    set(sokol_backend SOKOL_GLCORE)
    set(slang "glsl430")
endif()

set(FIPS_RTTI ON CACHE BOOL "Enable C++ RTTI" FORCE)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs
                    libs/glad/include
                    libs/nanovg
                    libs/nanosvg
                    libs/box2d/include
                    libs/json
                    libs/kepler-orbit
                    libs/miniz)

fips_ide_group(libs)
add_subdirectory(libs/glad)
add_subdirectory(libs/nanovg)
#add_subdirectory(libs/nanosvg)
add_subdirectory(libs/box2d/src)
add_subdirectory(libs/miniz)

fips_import_fips_imgui_imgui()

include_directories(libs/framework)
add_subdirectory(libs/framework)

add_subdirectory(libs/kepler-orbit)

fips_ide_group("")
add_subdirectory(projects)
