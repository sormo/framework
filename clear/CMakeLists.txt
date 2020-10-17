# sokol_config()

fips_ide_group(projects/clear)
fips_begin_app(clear windowed)
    fips_files(clear.cpp)

if(NOT FIPS_EMSCRIPTEN)
    fips_deps(dbgui)
endif()

fips_end_app()

# step that should be done in root cmakelists but I don't know how
add_custom_command(TARGET clear POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/../data/ $<TARGET_FILE_DIR:clear>)