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
    add_definitions(-DSOKOL_GLES2)
    set(slang "glsl300es:glsl100")
else()
    add_definitions(-DSOKOL_GLCORE33)
    add_definitions(-DSOKOL_EXTERNAL_GL_LOADER)
    set(slang "glsl330")
endif()

set(FIPS_RTTI ON CACHE BOOL "Enable C++ RTTI" FORCE)

set(CMAKE_CXX_STANDARD 17)

fips_include_directories(libs libs/glad/include libs/nanovg libs/box2d/include)

fips_ide_group(libs)
fips_add_subdirectory(libs/glad)
fips_add_subdirectory(libs/nanovg)
fips_add_subdirectory(libs/box2d/src)

fips_import_fips_imgui_imgui()

fips_include_directories(framework)

fips_add_subdirectory(framework)

fips_ide_group("")
fips_add_subdirectory(projects)

fips_finish()
