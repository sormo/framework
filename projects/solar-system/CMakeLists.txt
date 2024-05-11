fips_begin_app(solar-system windowed)
    fips_files(solar-system.cpp
               unit.h
               unit.cpp
               camera.h
               trajectory_resolutions.h
               commons.h
               bodies_tree.h
               bodies_tree.cpp
               quadtree.h)
    fips_dir(data)
    fipsutil_copy(data.yml)
    fips_deps(framework)
    fips_deps(kepler-orbit)
fips_end_app()
