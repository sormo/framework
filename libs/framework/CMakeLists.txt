fips_begin_lib(framework)
    fips_files(framework.h
               framework.cpp
               utils.h
               utils.cpp
               world.h
               world.cpp
               point_type.h
               matrix_type_3.h
               matrix_type_4.h
               imgui_impl.h
               imgui_impl.cpp
               drawing.cpp
               drawing_sg.h
               drawing_sg.cpp
               buffer_sg.h
               buffer_sg.cpp
               events.h
               events.cpp
               color_type.h
               color_type.cpp
               svg.h
               svg.cpp
               zip.h
               zip.cpp)
    fips_deps(nanovg)
    #fips_deps(nanosvg)
    fips_deps(glad)
    fips_deps(box2d)
    fips_deps(imgui)
    fips_deps(miniz)
    fips_deps(tiny-obj-loader)
    fips_deps(tiny-gltf)
    sokol_shader(shaders/basic_depth_instanced.glsl ${slang})
    sokol_shader(shaders/basic_depth.glsl ${slang})
    sokol_shader(shaders/basic_instanced.glsl ${slang})
    sokol_shader(shaders/basic.glsl ${slang})
    sokol_shader(shaders/sshapes.glsl ${slang})

    if (FIPS_CLANG OR FIPS_GCC)
        target_compile_options(framework PUBLIC -Wno-missing-braces
                                                -Wno-switch)
    endif()

fips_end_lib()
