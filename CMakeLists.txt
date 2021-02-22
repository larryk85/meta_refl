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

message(STATUS "Building meta_refl v${meta_refl_VERSION}...")

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
# Build meta_refl tests.
# ##################################################################################################
if(ENABLE_TESTS)
   add_project_dependency(Catch2
      REPO https://github.com/catchorg/Catch2
      TAG  v2.13.4
      DIR  ${CMAKE_BINARY_DIR}
      ARGS "-DCATCH_BUILD_TESTING=Off -DCATCH_ENABLE_WERROR=Off -DCATCH_INSTALL_DOCS=Off"
   )

   include(CTest)

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
   include(CMakePackageConfigHelpers)
   include(GNUInstallDirs)

   message(STATUS "Installing bluegrass meta_refl ...")
   set(meta_refl_INCLUDE_DIRS ${CMAKE_INSTALL_INCLUDEDIR})
   configure_package_config_file(modules/meta_refl-config.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config.cmake
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/meta_refl
      PATH_VARS meta_refl_INCLUDE_DIRS
   )
   write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config-version.cmake
      VERSION ${meta_refl_VERSION}
      COMPATIBILITY SameMajorVersion
   )

   install(TARGETS meta_refl
           EXPORT meta_refl-targets
           LIBRARY
           PUBLIC_HEADER
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
              COMPONENT Headers
   )
   install(EXPORT meta_refl-targets
      NAMESPACE bluegrass::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/meta_refl
   )
   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bluegrass
           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   )
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config.cmake
                 ${CMAKE_CURRENT_BINARY_DIR}/meta_refl-config-version.cmake
                 DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/meta_refl
   )
endif()
