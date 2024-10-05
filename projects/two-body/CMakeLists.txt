fips_begin_app(two-body windowed)
    fips_files(two-body.cpp
               unit.h)
    fips_deps(framework)
    fips_deps(kepler-orbit)

    if (FIPS_CLANG OR FIPS_GCC)
        target_compile_options(two-body PUBLIC -Wno-unused-variable
                                               -Wno-unused-const-variable)
    endif()

fips_end_app()
