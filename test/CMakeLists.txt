cmake_minimum_required(VERSION 2.8.11)

option(BUILD_COVERAGE "Enable code coverage generation (gcc only)" OFF)

IF(BUILD_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    message("Building coverage information.")
    set(CMAKE_BUILD_TYPE "Debug")
    add_definitions(--coverage -fprofile-arcs -ftest-coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# Add the gtest library
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(gtest 
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gtest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gmock/gmock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gmock_gtest_all.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gmock_main.cc
)
 
# https://cmake.org/Wiki/CMakeEmulateMakeCheck How to emulate GNU Autotools 'make check'
find_program(CMAKE_CTEST_COMMAND ctest ${CMAKE_HOST_PATH} ${CMAKE_TARGET_PATH})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>)

macro(wrap_test test_name)
    add_executable(${test_name} ${ARGN} ${COW_HDRS})
    target_link_libraries(${test_name} gtest)
    add_test(${test_name} ${test_name})
    add_dependencies(check ${test_name})
endmacro()

if(NOT WIN32)
    # Defines CMAKE_USE_PTHREADS_INIT and CMAKE_THREAD_LIBS_INIT.
    find_package(Threads)
    if (CMAKE_USE_PTHREADS_INIT)  # The pthreads library is available and allowed.
        add_definitions(-DGTEST_HAS_PTHREAD=1)
        target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})
    else()
        add_definitions(-DGTEST_HAS_PTHREAD=0)
    endif()
endif()

# Add a standard test
wrap_test(test_basic test_basic.cpp SharedInt.h SharedInt.cpp)

# Test that the will_fail.cpp compiles if no defines have been set.
wrap_test(wont_fail will_fail.cpp)

# Add a couple of failing-to-compile targets
foreach(i RANGE 1 3)
    add_executable(will_fail${i} will_fail.cpp ${COW_HDRS})
    target_compile_definitions(will_fail${i} PRIVATE FAIL_LEVEL=${i})
    set_target_properties(will_fail${i} PROPERTIES
        EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    target_link_libraries(will_fail${i} gtest)
    add_test(NAME will_fail${i}
        COMMAND ${CMAKE_COMMAND} --build . --target will_fail${i} --config $<CONFIGURATION>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_tests_properties(will_fail${i} PROPERTIES WILL_FAIL TRUE)
endforeach()
