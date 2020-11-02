# ##################################################################################################
# Defines the meta_refl library and associated tooling. See docs/cmake.md for instructions on how to
# build meta_refl or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
project(meta_refl)

add_subdirectory(modules/cmake_bits)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

create_version(
   MAJOR 1
   MINOR 0
   PATCH 0
   #SUFF  alpha
)

get_version( VERSION VERSION_FULL )

message(STATUS "Building meta_refl v${VERSION_FULL}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ##################################################################################################
# Defined options for building or integrating meta_refl.
# ##################################################################################################
include(CMakeDependentOption)
option(ENABLE_INSTALL "enable this library to be installed" ON)
option(ENABLE_TESTS "enable building of unit tests" OFF)
option(ENABLE_DOCS "enable building of documentation" OFF)

# ##################################################################################################
# Create the meta_refl library.
# ##################################################################################################
add_library(meta_refl INTERFACE)
target_include_directories(meta_refl INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   $<INSTALL_INTERFACE:include>)
add_library(bluegrass::meta_refl ALIAS meta_refl)

# ##################################################################################################
# Build liberror tests.
# ##################################################################################################
if(ENABLE_TESTS)
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
if(ENABLE_INSTALL)
   include(GNUInstallDirs)
   message(STATUS "Installing bluegrass meta_refl ...")
   install(TARGETS meta_refl
            EXPORT meta_refl-config
            LIBRARY
            PUBLIC_HEADER
               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
               COMPONENT Headers)
   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bluegrass
           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
   install(EXPORT meta_refl-config
           DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/meta_refl"
           NAMESPACE bluegrass::)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config-version.cmake
           DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/meta_refl")
endif()

add_subdirectory(docs)
