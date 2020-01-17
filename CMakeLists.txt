# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.11)

project(AccessorFramework
    VERSION 1.0
    DESCRIPTION "A framework for using Accessors"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test executable (on by default)" ON)

add_library(AccessorFramework
    ${PROJECT_SOURCE_DIR}/src/Accessor.cpp
    ${PROJECT_SOURCE_DIR}/src/AccessorImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/AtomicAccessorImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/CompositeAccessorImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Director.cpp
    ${PROJECT_SOURCE_DIR}/src/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/HostHypervisorImpl.cpp
    ${PROJECT_SOURCE_DIR}/src/HostImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Port.cpp
)

add_library(AccessorFramework::AccessorFramework ALIAS AccessorFramework)
if(${VERBOSE})
  target_compile_definitions(AccessorFramework PRIVATE VERBOSE=${VERBOSE})
endif()

target_include_directories(AccessorFramework
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	    $<INSTALL_INTERFACE:include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm"))
    target_compile_options(AccessorFramework PRIVATE -pthread)
    target_link_libraries(AccessorFramework PRIVATE -lpthread)
endif()

set_target_properties(AccessorFramework PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)

# Install

install(TARGETS AccessorFramework
  CONFIGURATIONS Debug
  EXPORT AccessorFrameworkTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  COMPONENT library
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/AccessorFramework
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

# Package

if(WIN32 AND NOT CYGWIN)
  set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_PREFIX}/cmake)
else()
  set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/AccessorFramework)
endif()

install(EXPORT AccessorFrameworkTargets
  FILE AccessorFrameworkTargets.cmake
  NAMESPACE AccessorFramework::
  DESTINATION ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/cmake/AccessorFrameworkConfigVersion.cmake
  VERSION ${ACCESSORFRAMEWORK_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/AccessorFrameworkConfig.cmake.in
  ${PROJECT_BINARY_DIR}/cmake/AccessorFrameworkConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/cmake/AccessorFrameworkConfig.cmake
    ${PROJECT_BINARY_DIR}/cmake/AccessorFrameworkConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT AccessorFrameworkTargets
  FILE ${PROJECT_BINARY_DIR}/cmake/AccessorFrameworkTargets.cmake
  NAMESPACE AccessorFramework::
)

export(PACKAGE AccessorFramework)

if (BUILD_TESTS)
    add_definitions(-DUSE_GTEST)
    enable_testing()
    add_subdirectory(test)
endif (BUILD_TESTS)