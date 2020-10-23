# ##################################################################################################
# Defines the meta_refl library and associated tooling. See docs/cmake.md for instructions on how to
# build meta_refl or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
set(META_REFL_VERSION_MAJOR 1)
set(META_REFL_VERSION_MINOR 0)
set(META_REFL_VERSION_PATCH 0)
set(META_REFL_VERSION_SUFFIX rc1)
project(meta_refl VERSION ${META_REFL_VERSION_MAJOR}.${META_REFL_VERSION_MINOR}.${META_REFL_VERSION_PATCH})

if (META_REFL_VERSION_SUFFIX)
   set(VERSION_FULL "${META_REFL_VERSION_MAJOR}.${META_REFL_VERSION_MINOR}.${META_REFL_VERSION_PATCH}-${META_REFL_VERSION_SUFFIX}")
else()
   set(VERSION_FULL "${META_REFL_VERSION_MAJOR}.${META_REFL_VERSION_MINOR}.${META_REFL_VERSION_PATCH}")
endif()

message(STATUS "Building meta_refl v${VERSION_FULL}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ##################################################################################################
# Defined options for building or integrating meta_refl.
# ##################################################################################################
include(CMakeDependentOption)
option(META_REFL_ENABLE_INSTALL "enable this library to be installed" ON)
option(META_REFL_ENABLE_TESTS "enable building of unit tests" OFF)

find_package(Git QUIET)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   RESULT_VARIABLE   GIT_SUBMOD_RESULT)

if(NOT GIT_SUBMOD_RESULT EQUAL "0")
   message(FATAL_ERROR "Failed to update submodules")
endif()

add_subdirectory(modules/cmake_bits)

# ##################################################################################################
# Create the meta_refl library.
# ##################################################################################################
add_library(meta_refl INTERFACE)
target_include_directories(meta_refl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(bluegrass::meta_refl ALIAS meta_refl)

# ##################################################################################################
# Build liberror tests.
# ##################################################################################################
if(META_REFL_ENABLE_TESTS)
   add_project_dependency(Catch2
      REPO https://github.com/catchorg/Catch2
      TAG  devel
      DIR  ${CMAKE_BINARY_DIR}
   )

   include(CTest)
   include(Catch)

   enable_testing()
   add_subdirectory(tests)
endif()

# ##################################################################################################
# Configure version info.
# ##################################################################################################
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/modules/meta_refl-config-version.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config-version.cmake @ONLY)

# ##################################################################################################
# Installation.
# ##################################################################################################
if(BLUEGRASS_ENABLE_INSTALL)
   include(GNUInstallDirs)
   message(STATUS "Installing bluegrass meta_refl ...")
   install(TARGETS meta_refl
            EXPORT meta_refl_pkg_config
            LIBRARY
            PUBLIC_HEADER
               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
               COMPONENT Headers)
   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bluegrass
           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
   install(EXPORT meta_refl_pkg_config
           DESTINATION "${CMAKE_INSTALL_DATADIR}/meta_refl/cmake"
           NAMESPACE bluegrass::)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config-version.cmake
           DESTINATION "#{CMAKE_INSTALL_DATADIR}/meta_refl/cmake")
endif()

