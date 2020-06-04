# ##################################################################################################
# Define the main test executable.
# ##################################################################################################
include(CTest)
enable_testing()
add_executable( unit_tests main.cpp
                           meta_tests.cpp
                           traits_tests.cpp
              )

target_link_libraries( unit_tests PRIVATE bluegrass_meta_refl Catch2::Catch2 )
catch_discover_tests( unit_tests )
