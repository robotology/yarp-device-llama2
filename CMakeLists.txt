# SPDX-FileCopyrightText: 2023 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(yarp-device-llama2
        LANGUAGES CXX
        VERSION 3.8.0)

include(FeatureSummary)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(YCM 0.15.3 REQUIRED)

include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(YARP_FORCE_DYNAMIC_PLUGINS TRUE CACHE INTERNAL "yarp-devices-ros2 is always built with dynamic plugins")
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

find_package(YARP 4.0.0 COMPONENTS os sig dev serversql OPTIONAL_COMPONENTS math REQUIRED)
find_package(YARP 4.0.0 COMPONENTS catch2 dev_tests QUIET)

if(YARP_catch2_FOUND AND YARP_dev_tests_FOUND)
  option(YARP_COMPILE_TESTS "Enable YARP tests" OFF)
  if(YARP_COMPILE_TESTS)
    enable_testing()
  endif()
  message(STATUS "yarp_catch2 and yarp_dev_tests libraries found")
else()
  set(YARP_COMPILE_TESTS OFF CACHE BOOL "yarp_catch2 and yarp_dev_tests libraries NOT found. Tests cannot be enabled" FORCE)
  message(STATUS "yarp_catch2 and yarp_dev_tests libraries NOT found. Tests cannot be enabled")
endif()

include(YarpValgrindOptions)

include(AddInstallRPATHSupport)
add_install_rpath_support(
  BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
  LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
  INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
  USE_LINK_PATH
)

# Prefer libraries located next to the built/installed binaries.
# This helps avoid accidentally loading an unrelated system-wide libggml.so.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(UNIX AND NOT APPLE)
  list(APPEND CMAKE_BUILD_RPATH
       "$ORIGIN"
       "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
  list(APPEND CMAKE_INSTALL_RPATH
       "$ORIGIN"
       "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
  list(APPEND CMAKE_BUILD_RPATH
       "@loader_path"
       "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  list(APPEND CMAKE_INSTALL_RPATH
       "@loader_path"
       "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
  endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS 1)

include(AddUninstallTarget)

set(CMAKE_C_FLAGS "${YARP_C_FLAGS} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${YARP_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

yarp_configure_plugins_installation(${PROJECT_NAME})
yarp_configure_external_installation(${PROJECT_NAME})

find_package(Doxygen)

#
# Embedded llama.cpp policy:
# - Build it statically to avoid runtime collisions with system-installed libggml.so
# - Force Vulkan OFF in this embedded copy
#
set(_PROJECT_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as shared as opposed to static" FORCE)
set(GGML_VULKAN OFF CACHE BOOL "Disable Vulkan for embedded llama.cpp" FORCE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp")

# Restore the parent project's choice for the rest of the build
set(BUILD_SHARED_LIBS "${_PROJECT_BUILD_SHARED_LIBS}" CACHE BOOL "Build libraries as shared as opposed to static" FORCE)
unset(_PROJECT_BUILD_SHARED_LIBS)

add_subdirectory(src)
# add_subdirectory(doc)
add_subdirectory(tests)
# add_subdirectory(models)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)