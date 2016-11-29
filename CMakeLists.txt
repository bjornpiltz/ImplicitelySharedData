cmake_minimum_required(VERSION 2.8.11)

project(ImplicitelySharedData CXX C)
set(CMAKE_CXX_STANDARD 11) 
option(BUILD_COVERAGE "Enable code coverage generation (gcc only)" OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

SET(COW_HDRS ${PROJECT_SOURCE_DIR}/include/COW.h)

IF(BUILD_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    message("Building coverage information.")
    set(CMAKE_BUILD_TYPE "Debug")
    add_definitions(--coverage -fprofile-arcs -ftest-coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

enable_testing()
add_subdirectory(test)
add_subdirectory(examples)
