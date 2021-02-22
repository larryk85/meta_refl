# ##################################################################################################
# Define the main test executable.
# ##################################################################################################
add_executable( meta_refl_unit_tests main.cpp
                                     meta_tests.cpp
                                     traits_tests.cpp
              )

target_link_libraries( meta_refl_unit_tests PRIVATE bluegrass::meta_refl Catch2::Catch2 )