# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.11)

# Replace install() to do-nothing macro to avoid installing googletest
macro(install)
endmacro()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)

FetchContent_MakeAvailable(googletest)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

add_executable(AccessorFrameworkTests
    src/TestCases/BasicAccessorTests.cpp
    src/TestCases/BasicHostTests.cpp
    src/TestCases/SumVerifierTests.cpp
    src/TestCases/DynamicSumVerifierTests.cpp
)

target_link_libraries(AccessorFrameworkTests
    PRIVATE
    gtest
    gmock_main
    AccessorFramework::AccessorFramework
)

add_test(NAME AccessorFrameworkTests COMMAND AccessorFrameworkTests)

# Restore original install() behavior
macro(install)
    _install(${ARGN})
endmacro()