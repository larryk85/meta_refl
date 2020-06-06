# ##################################################################################################
# Defines the meta_refl library and associated tooling. See docs/cmake.md for instructions on how to
# build meta_refl or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_SUFFIX rc1)
project(meta_refl VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if (VERSION_SUFFIX)
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

message(STATUS "Building liberror v${VERSION_FULL}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ##################################################################################################
# Defined options for building or integrating liberror.
# ##################################################################################################
include(CMakeDependentOption)
option(ENABLE_INSTALL "enable this library to be installed" ON)
option(ENABLE_TESTS "enable building of unit tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# ##################################################################################################
# Create the liberror library.
# ##################################################################################################
add_library(meta_refl INTERFACE)
target_include_directories(meta_refl
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Bluegrass::MetaRefl ALIAS meta_refl)

# ##################################################################################################
# Build liberror tests.
# ##################################################################################################
if(BLUEGRASS_META_REFL_ENABLE_TESTS)
   include(CTest)
   include(catch2_macro)
   use_catch2(${CMAKE_BINARY_DIR})
   add_subdirectory(tests)
endif()

# ##################################################################################################
# Exporting.
# ##################################################################################################
export(TARGETS meta_refl NAMESPACE bluegrass:: FILE BluegrassMetaRefl.cmake)
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE meta_refl)

# ##################################################################################################
# Installation.
# ##################################################################################################
if(BLUEGRASS_ENABLE_INSTALL)
   include(GNUInstallDirs)
   message(STATUS "Installing bluegrass meta_refl ...")
   install(TARGETS meta_refl
            LIBRARY
            PUBLIC_HEADER
               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
               COMPONENT Headers)
   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bluegrass
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

