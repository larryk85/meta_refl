# ##################################################################################################
# Defines the meta_refl library and associated tooling. See docs/cmake.md for instructions on how to
# build meta_refl or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_SUFFIX rc1)
project(bluegrass_meta_refl VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if (VERSION_SUFFIX)
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

message(STATUS "Building liberror v${VERSION_FULL}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
link_libraries(Threads::Threads)

# ##################################################################################################
# Defined options for building or integrating liberror.
# ##################################################################################################
include(CMakeDependentOption)
option(BLUEGRASS_ENABLE_INSTALL "enable this library to be installed" ON)
option(BLUEGRASS_ENABLE_TESTS "enable building of unit tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# ##################################################################################################
# Create the liberror library.
# ##################################################################################################
add_library(bluegrass_meta_refl INTERFACE)
target_include_directories(bluegrass_meta_refl
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Bluegrass::MetaRefl ALIAS bluegrass_meta_refl)

# ##################################################################################################
# Build liberror tests.
# ##################################################################################################
if(BLUEGRASS_ENABLE_TESTS)
   include(CTest)
   if (NOT Catch2::Catch2)
      set(CATCH_BUILD_TESTING OFF CACHE BOOL "disable building tests")
      set(CATCH_INSTALL_DOCS OFF CACHE BOOL "disable installing docs")
      set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "disable installing helpers")

      configure_file(modules/catch2_install.cmake.in ${CMAKE_BINARY_DIR}/catch2-download/CMakeLists.txt @ONLY)
      execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/catch2-download")
      execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/catch2-download")
      add_subdirectory(${CMAKE_BINARY_DIR}/catch2)
      include(${CMAKE_BINARY_DIR}/catch2/contrib/Catch.cmake)
   endif()
   add_subdirectory(tests)
endif()

# ##################################################################################################
# Installation.
# ##################################################################################################
if(BLUEGRASS_ENABLE_INSTALL)
   include(GNUInstallDirs)
   message(STATUS "Installing...")
   install(TARGETS bluegrass_meta_refl
            LIBRARY
               DESTINATION ${CMAKE_INSTALL_LIBDIR}
               COMPONENT Libraries
            PUBLIC_HEADER
               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
               COMPONENT Headers)

   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bluegrass
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

