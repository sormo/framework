fips_begin_app(ape-hunter windowed)
    fips_files(main.cpp
               objects.h
               objects.cpp
               colorLerp.h
               colorLerp.cpp
               textManager.h
               textManager.cpp)
    fips_dir(../data)
    fipsutil_copy(ape-hunter.yml)
    fips_deps(framework)
fips_end_app()
