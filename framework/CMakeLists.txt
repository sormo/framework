fips_begin_lib(framework)
    fips_files(framework.h
               framework.cpp
               world.h
               world.cpp
               point.h
               point.cpp
               imgui_impl.h
               imgui_impl.cpp
               drawing.cpp
               events.h
               events.cpp)
    fips_deps(nanovg)
    fips_deps(glad)
    fips_deps(box2d)
    fips_deps(imgui)
fips_end_lib()
